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

    setEnabledUi(false);

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
    uiMapper->addMapping(ui->nLine, 10);
    uiMapper->addMapping(ui->d0Line, 11);
    uiMapper->addMapping(ui->kLine, 12);
    uiMapper->addMapping(ui->v1, 13);
    uiMapper->addMapping(ui->v2, 14);
    uiMapper->addMapping(ui->v3, 15);
    uiMapper->addMapping(ui->v4, 16);
    uiMapper->addMapping(ui->v5, 17);
    uiMapper->addMapping(ui->v6, 18);
    uiMapper->addMapping(ui->v7, 19);
    uiMapper->addMapping(ui->w1, 20);
    uiMapper->addMapping(ui->w2, 21);
    uiMapper->addMapping(ui->w3, 22);
    uiMapper->addMapping(ui->w4, 23);
    uiMapper->addMapping(ui->w5, 24);
    uiMapper->addMapping(ui->w6, 25);
    uiMapper->addMapping(ui->w7, 26);


    //Hide model items in tree view
    for (int i=3; i<=26; ++i)
        ui->treeUsers->setColumnHidden(i, true);


    //TODO check disconnect ??
    auto conn = std::make_shared<QMetaObject::Connection>();
    *conn = QObject::connect(ui->treeUsers->selectionModel(), &QItemSelectionModel::currentRowChanged,
                        [this, conn](const QModelIndex& current, const QModelIndex&  /* previous */ ) {
        uiMapper->setCurrentIndex(current.row());
        setEnabledUi(true);
        disconnect(*conn);
    });


    //Change index and load combobox when other user is selected
    connect(ui->treeUsers->selectionModel(), &QItemSelectionModel::currentRowChanged,
            [this](const QModelIndex& current, const QModelIndex& /*previous */) {
        int row = current.row();
        uiMapper->setCurrentIndex(row);
        loadCombobox(row);
    });

    //Load combobox and features
    connect(ui->dateComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), [this](const QString& text)
    {
        const QModelIndex& curr =  ui->treeUsers->currentIndex();
        if (curr.isValid() && ( !text.isEmpty()) )
        {
            const QString& ip = treeModel->index(curr.row(), 1).data().toString();
            auto features = users[ip]->features[text];
            ui->f1->setText(QString::number((features.at(0))));
            ui->f2->setText(QString::number((features.at(1))));
            ui->f3->setText(QString::number((features.at(2))));
            ui->f4->setText(QString::number((features.at(3))));
            ui->f5->setText(QString::number((features.at(4))));
            ui->f6->setText(QString::number((features.at(5))));
            ui->f7->setText(QString::number((features.at(6))));
        }
    });

    ////////////////////////////////////////////////////

    QDir configDir;
    configDir.mkpath(path + "/configs");

    //Start modules
    fileServer->start();
    connect(fileServer.get(), &FileServer::stringReceived,
            [this](QString str, QString ip) { this->getString(str, ip); });

    //If data recieved => set online
    connect(fileServer.get(), &FileServer::dataSaved, [this](QString path, QString ip) {
        users[ip]->setStatus(State::ONLINE);
    });

    //Delete old config & rename after sending
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
    });

    //Connect buttons clicks
    connect(ui->buttonSendConfig, &QPushButton::clicked, this, &Server::configSendClicked);
    connect(ui->buttonSaveConfig, &QPushButton::clicked, this, &Server::configSaveClicked);
    connect(ui->buttonFileDialog, &QPushButton::clicked, this, &Server::fileDialogClicked);
    connect(ui->calculate, &QPushButton::clicked, this, &Server::calculateClicked);
}

Server::~Server()
{
    saveUsers();
}

void Server::setupModels()
{
    treeModel = std::make_unique<QStandardItemModel>(this);
    treeModel->setColumnCount(27);

    //Set header names
    treeModel->setHeaderData(0, Qt::Horizontal, "Пользователь");
    treeModel->setHeaderData(1, Qt::Horizontal, "IP-адрес");
    treeModel->setHeaderData(2, Qt::Horizontal, "Порт");
    treeModel->setHeaderData(3, Qt::Horizontal, "secondsScreen");
    treeModel->setHeaderData(4, Qt::Horizontal, "LMB");
    treeModel->setHeaderData(5, Qt::Horizontal, "RMB");
    treeModel->setHeaderData(6, Qt::Horizontal, "MMB");
    treeModel->setHeaderData(7, Qt::Horizontal, "MWH");
    treeModel->setHeaderData(8, Qt::Horizontal, "isLogWorking");
    treeModel->setHeaderData(9, Qt::Horizontal, "SecondLog");
    treeModel->setHeaderData(10, Qt::Horizontal, "N");
    treeModel->setHeaderData(11, Qt::Horizontal, "d0");
    treeModel->setHeaderData(12, Qt::Horizontal, "k");
    treeModel->setHeaderData(13, Qt::Horizontal, "v1");
    treeModel->setHeaderData(14, Qt::Horizontal, "v2");
    treeModel->setHeaderData(15, Qt::Horizontal, "v3");
    treeModel->setHeaderData(16, Qt::Horizontal, "v4");
    treeModel->setHeaderData(17, Qt::Horizontal, "v5");
    treeModel->setHeaderData(18, Qt::Horizontal, "v6");
    treeModel->setHeaderData(19, Qt::Horizontal, "v7");
    treeModel->setHeaderData(20, Qt::Horizontal, "w1");
    treeModel->setHeaderData(21, Qt::Horizontal, "w2");
    treeModel->setHeaderData(22, Qt::Horizontal, "w3");
    treeModel->setHeaderData(23, Qt::Horizontal, "w4");
    treeModel->setHeaderData(24, Qt::Horizontal, "w5");
    treeModel->setHeaderData(25, Qt::Horizontal, "w6");
    treeModel->setHeaderData(26, Qt::Horizontal, "w7");

    //Traverse existing users and add to model
    for (const auto& user: users)
    {
        const User& currUser = *user.second.get();

        //load configs
        Config& cfg = *currUser.cfg.get();
        const QString& ip = user.first;
        if (!loadConfig(cfg, path + "/configs/" + ip + ".cfg"))
            saveConfig(cfg, path + "/configs/" + ip + ".cfg");

        addUserToModel(currUser, State::OFFLINE);
    }
}

void Server::addUserToModel(const User& user, const State& st)
{
    QList<QStandardItem*> items;
    //Create items and write to model
    QStandardItem* nameItem = new QStandardItem(QIcon((st == State::ONLINE) ? ":/icons/online.png" : ":/icons/offline.png"), user.username);
    QStandardItem* ipItem = new QStandardItem(user.ip);
    QStandardItem* portItem = new QStandardItem( QString::number(user.port) );

    //Set not editable item
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    ipItem->setFlags(ipItem->flags() & ~Qt::ItemIsEditable);
    portItem->setFlags(portItem->flags() & ~Qt::ItemIsEditable);

    items << nameItem << ipItem << portItem;

    items << new QStandardItem(QString::number(user.cfg->secondsScreen))
          <<  new QStandardItem(QString(user.cfg->mouseButtons[int(Buttons::left)] ? "1" : "0"))
          << new QStandardItem(QString(user.cfg->mouseButtons[int(Buttons::right)] ? "1" : "0"))
          << new QStandardItem(QString(user.cfg->mouseButtons[int(Buttons::middle)] ? "1" : "0"))
          << new QStandardItem(QString(user.cfg->mouseButtons[int(Buttons::wheel)] ? "1" : "0"))
          << new QStandardItem(user.cfg->logRun ? "1" : "0")
          << new QStandardItem(QString::number(user.cfg->secondsLog));

    items << new QStandardItem(QString::number(user.N))
          << new QStandardItem(QString::number(user.d0))
          << new QStandardItem(QString::number(user.k));

    //Add one-sided deviations vector & weights
    for(int i = 0; i < 7; ++i)
        items << new QStandardItem(QString::number(user.onesided.at(i)));

    for(int i = 0; i < 7; ++i)
        items << new QStandardItem(QString::number(user.weights.at(i)));

    treeModel->appendRow(items);
}

void Server::setStatus(const State& status, const QString& ip)
{
    //find user in model
    QList<QStandardItem*> items{treeModel->findItems(ip, Qt::MatchFixedString, 1)};
    if (items.size() == 1)
    {
        QIcon icon;
        if (status == State::ONLINE)
            icon = QIcon(":/icons/online.png");
        else
            icon = QIcon(":/icons/offline.png");

        //Change icon, ip and port
        treeModel->item(items.at(0)->row(), 0)->setIcon(icon);
        QModelIndex index = treeModel->index(items.at(0)->row(), 0);
        treeModel->setData(index, users[ip]->username);
        index = treeModel->index(items.at(0)->row(), 2);
        treeModel->setData(index, users[ip]->port);
    }
    else
    {
        //If user not found
        return;
    }
}

void Server::setEnabledUi(bool b)
{
    ui->buttonFileDialog->setEnabled(b);
    ui->buttonSaveConfig->setEnabled(b);
    ui->buttonSendConfig->setEnabled(b);
    ui->calculate->setEnabled(b);
    //Screenshot
    ui->spinSeconds->setEnabled(b);
    ui->checkLMB->setEnabled(b);
    ui->checkRMB->setEnabled(b);
    ui->checkMMB->setEnabled(b);
    ui->checkMWH->setEnabled(b);
    //Keylogger
    ui->checkOnOff->setEnabled(b);
    ui->spinSeconds2->setEnabled(b);

    ui->nLine->setEnabled(b);
    ui->d0Line->setEnabled(b);
    ui->kLine->setEnabled(b);
    ui->dateComboBox->setEnabled(b);

    //Features and weights
    ui->v1->setEnabled(b);
    ui->v2->setEnabled(b);
    ui->v3->setEnabled(b);
    ui->v4->setEnabled(b);
    ui->v5->setEnabled(b);
    ui->v6->setEnabled(b);
    ui->v7->setEnabled(b);

    ui->w1->setEnabled(b);
    ui->w2->setEnabled(b);
    ui->w3->setEnabled(b);
    ui->w4->setEnabled(b);
    ui->w5->setEnabled(b);
    ui->w6->setEnabled(b);
    ui->w7->setEnabled(b);
}

void Server::loadCombobox(int& row)
{
    QStringList sl;
    //Get current user
    const QString& ip = treeModel->index(row, 1).data().toString();
    const auto& features = users[ip]->features;
    for(auto f : features.keys())
    {
        sl << f;
    }
    ui->dateComboBox->clear();
    ui->dateComboBox->addItems(sl);
}

void Server::setupUserConnections(const User& user)
{
    //Update combobox after receiving new data
    connect(&user, &User::dataChanged, [this, &user](const QString data)
    {
        const QModelIndex& curr =  ui->treeUsers->currentIndex();
        if (curr.isValid())
        {
            //Check if the same user is selected & the same date
            if ((treeModel->index(curr.row(), 0).data().toString() == user.username) &&
                (ui->dateComboBox->currentText() == data))
            {
                    const QString& ip = treeModel->index(curr.row(), 1).data().toString();
                    auto features = users[ip]->features[data];
                    ui->f1->setText(QString::number((features.at(0))));
                    ui->f2->setText(QString::number((features.at(1))));
                    ui->f3->setText(QString::number((features.at(2))));
                    ui->f4->setText(QString::number((features.at(3))));
                    ui->f5->setText(QString::number((features.at(4))));
                    ui->f6->setText(QString::number((features.at(5))));
                    ui->f7->setText(QString::number((features.at(6))));
            }
        }
    });

    //add new data to combobox
    connect(&user, &User::newData, [this, &user](const QString date)
    {
        //The same user
        const QModelIndex& curr =  ui->treeUsers->currentIndex();
        if (curr.isValid() && (treeModel->index(curr.row(), 0).data().toString() == user.username))
        {
            ui->dateComboBox->addItem(date);
        }
    });

    //Set offline after 2 min
    connect(&user, &User::changedStatus, [this](State st, QString ip)
    {
        setStatus(st, ip);
    });
}

bool Server::saveUsers()
{
    QFile usersFile(path + "/list.usr");
    if ( !usersFile.open(QIODevice::WriteOnly) )
        return false;
    QDataStream stream(&usersFile);
    for(const auto& user: users)
    {
        User* currUser = user.second.get();
        setData(*currUser); //load from model to user
        stream << currUser->username
               << currUser->ip
               << currUser->port
               << currUser->N
               << currUser->d0
               << currUser->k
               << currUser->onesided
               << currUser->features
               << currUser->weights;
    }

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
        QDataStream stream(&usersFile);
        QString username, ip;
        quint16 port;
        bool b = false;
        double d0, k;
        uint N;
        QVector<int> onesided;
        QVector<float> weights;
        QMap<QString, QVector<double>> features;
        //Read from file
        while( !stream.atEnd() )
        {
            stream >> username >> ip >> port >> N >> d0 >> k >> onesided >> features >> weights;
            auto pair = users.emplace(std::piecewise_construct,
                                std::forward_as_tuple(ip),
                                std::forward_as_tuple(std::make_unique<User>(username, ip, port, b, d0, N, k, onesided, features, weights)));

            User& currUser = *(*pair.first).second.get();
            setupUserConnections(currUser);
        }
        usersFile.close();

        //Check all configs in folder by user's ip
        QDirIterator iter(path + "/configs", QStringList() << "*.cfg", QDir::Files);
        while (iter.hasNext())
        {
            iter.next();
            ip = iter.fileName().section(".", 0, -2);
            //If user already exists => load config to memory
            if(users.find(ip) != users.end())
            {
                User* user = users[ip].get();
                user->cfg = std::make_unique<Config>();
                loadConfig(*user->cfg.get(), path + "/configs/" + ip + ".cfg");
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
    const QString& command = str.section('|', 0, 0);

    //Check if user already exists
    bool userExists = false;
    auto user = users.find(ip);
    if (user != users.end())
        userExists = true;

    const QString& username = str.section("|", 1, 1);
    const quint16& port = str.section("|", 2, 2).toInt();

    if (command == "ONLINE")
    {
        //If QHash doesn't contain user's ip => add user
        if ( !userExists )
        {
            //Add new user
            bool b = true;
            auto pair = users.emplace(std::piecewise_construct,
                      std::forward_as_tuple(ip),
                      std::forward_as_tuple(std::make_unique<User>(username, ip, port, b)));

            User& currUser = *(*pair.first).second.get();

            QString cfgPath = path + "/configs/" + ip + ".cfg";
            Config& config = *currUser.cfg.get();
            //If config doesn't exist
            if( ! loadConfig(config, cfgPath) )
            {
                qDebug() << "Config not found. Creating new.";
                saveConfig(config, cfgPath);
            }

            //Add new user to tree model
            addUserToModel(currUser, State::ONLINE);

            setupUserConnections(currUser);

            //add time data
            currUser.setFeatures();
        }
        else
        {
            user->second->username = username;
            user->second->port = port;
            user->second->setStatus(State::ONLINE);

            user->second->setFeatures();
        }
    }
    else if (command == "OFFLINE")
    {
        if (userExists)
            user->second->setStatus(State::OFFLINE);
    }
    else if (command == "DATA")
    {
        if (userExists)
        {
            user->second->setStatus(State::ONLINE);
            user->second->setFeatures(str.section("|", 1, 1).toInt(),
                                      str.section("|", 2, 2).toInt(),
                                      str.section("|", 3, 3).toInt(),
                                      str.section("|", 4, 4).toInt());
        }
        else
            return;
    }
}

void Server::setConfig(Config &cfg)
{
    //If nothing was selected
    if ( ! ui->treeUsers->currentIndex().isValid())
        return;

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

void Server::setData(User &user)
{
    //If nothing was selected
    if ( ! ui->treeUsers->currentIndex().isValid())
        return;

    int currentRow = ui->treeUsers->currentIndex().row();
    for(int i = 0; i < 7; ++i)
    {
        user.onesided[i] = treeModel->index(currentRow, 13+i).data().toInt();
        user.weights[i] = treeModel->index(currentRow, 20+i).data().toFloat();
    }

    user.N = treeModel->index(currentRow, 10).data().toInt();
    user.d0 = treeModel->index(currentRow, 11).data().toDouble();
    user.k = treeModel->index(currentRow, 12).data().toDouble();
}

void Server::configSendClicked()
{
    auto currIndex = ui->treeUsers->currentIndex();
    if ( ! currIndex.isValid()) //If nothing was selected
        return;

    QModelIndex ipIndex = treeModel->index(currIndex.row(), 1);
    QModelIndex portIndex = treeModel->index(currIndex.row(), 2);
    QString ip = ipIndex.data().toString();
    quint16 port = portIndex.data().toInt();

    QString tempCfgPath = path + "/configs/" + ip + "_temp.cfg";
    Config* cfg = users[ip]->cfg.get();
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
    fileClient->enqueueData(Type::FILE, tempCfgPath);
    fileClient->connect();
}

void Server::configSaveClicked()
{
    //If nothing was selected
    if ( ! ui->treeUsers->currentIndex().isValid())
        return;

    QModelIndex ipIndex = treeModel->index(ui->treeUsers->currentIndex().row(), 1);
    QString ip = ipIndex.data().toString();

    QString cfgPath = path + "/configs/" + ip + ".cfg";
    Config* cfg = users[ip]->cfg.get();

    setConfig(*cfg);
    saveConfig(*cfg, cfgPath);
}

void Server::fileDialogClicked()
{
    fileDialog = std::make_unique<FileDialog>(this);
    fileDialog->show();

    connect(fileDialog.get(), &FileDialog::accepted, this, &Server::fileDialogAccepted);
}

void Server::fileDialogAccepted()
{
    uint& mask = fileDialog->getFileMask();

    //If no parameters set or user isn't selected
    auto currIndex = ui->treeUsers->currentIndex();
    if ( (mask == 0) || (! currIndex.isValid()) )
        return;

    QModelIndex ipIndex = treeModel->index(currIndex.row(), 1);
    QModelIndex portIndex = treeModel->index(currIndex.row(), 2);
    QString ip = ipIndex.data().toString();
    quint16 port = portIndex.data().toInt();
    fileClient->changePeer(ip, port);

    //Send string
    fileClient->enqueueData(Type::STRING, "FILES|" + QString::number(mask));
    fileClient->connect();
}

void Server::calculateClicked()
{
    //Find user
    if ( ! ui->treeUsers->currentIndex().isValid())
        return;

    int row = ui->treeUsers->currentIndex().row();
    auto userIter = users.find(treeModel->index(row, 1).data().toString());
    if (userIter == users.end()) return;
    User& user = *(userIter->second.get());

    //get score
    setData(user);
    const QString& date = ui->dateComboBox->currentText();
    QVector<double> result = user.getScore(date);
    ui->score->setText(QString::number(result.at(0)));

    ui->c1->setText(QString::number(round(result.at(1)/result.at(8)*10000)/100));
    ui->c2->setText(QString::number(round(result.at(2)/result.at(8)*10000)/100));
    ui->c3->setText(QString::number(round(result.at(3)/result.at(8)*10000)/100));
    ui->c4->setText(QString::number(round(result.at(4)/result.at(8)*10000)/100));
    ui->c5->setText(QString::number(round(result.at(5)/result.at(8)*10000)/100));
    ui->c6->setText(QString::number(round(result.at(6)/result.at(8)*10000)/100));
    ui->c7->setText(QString::number(round(result.at(7)/result.at(8)*10000)/100));
}
