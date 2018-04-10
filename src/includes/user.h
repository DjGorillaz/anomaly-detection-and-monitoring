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
    void setStatus(State st);
    ~User() = default;
signals:
    void changedStatus(State st, QString ip);
private:
    QString username;
    QString ip;
    quint16 port;
    bool online;
    std::unique_ptr<Config> cfg;
    QTimer offlineTimer;
    friend class Server;
};
