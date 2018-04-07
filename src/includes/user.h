#pragma once

#include <memory>
#include <QString>
#include <QTimer>
#include <QObject>

#include "config.h"

class User: public QObject
{
    Q_OBJECT
public:
    User(QString name, QString ip, quint16 port_, bool online_);
    //~User() = default;
signals:
    void offline(QString ip);
private:
    QString username;
    QString ip;
    quint16 port;
    bool online;
    std::unique_ptr<Config> cfg;
    QTimer timer;
    friend class Server;
};
