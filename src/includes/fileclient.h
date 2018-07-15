#pragma once

#include <memory>
#include <array>
#include <queue>

#include <QTcpSocket>
#include <QDataStream>
#include <QFile>

#include <data.h>

class FileClient : public QObject
{
    Q_OBJECT
public:
    FileClient(QObject* parent, const QString& ip, quint16 port);
    ~FileClient() = default;

    void sendAndDisconnect(const QString& data);
    void enqueueDataAndConnect(std::unique_ptr<data::Data>&& data);
    void changePeer(const QString &ip, const quint16 port);
    void connect();

    const QString& getIp();
    const QString& getName();

signals:
    void error(QAbstractSocket::SocketError socketError);
    void transmitted();

private:
    void sendData();
    void disconnect();

    QString ip;
    quint16 port;
    std::unique_ptr<QTcpSocket> socket;
    QString name;
    std::queue<std::unique_ptr<data::Data>> dataQueue;
};
