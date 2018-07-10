#pragma once

#include <memory>
#include <array>

#include <QTcpSocket>
#include <QDataStream>
#include <QFile>
#include <QQueue>

enum class Type {
    STRING,
    FILE
};

class FileClient : public QObject
{
    Q_OBJECT
public:
    FileClient(QObject* parent, const QString& ip, quint16 port);
    ~FileClient() = default;

    void getOffline();
    void enqueueData(const Type& T, const QString& data);
    void changePeer(const QString &ip, const quint16 port);
    void connect();
    bool isDataQueueEmpty();

    const QString& getIp();
    const QString& getName();

signals:
    void error(QAbstractSocket::SocketError socketError);
    void transmitted();

private:
    void sendData();
    void writeToSocketCycle(qint64 bytesWritten);
    void disconnect();

    QString ip;
    quint16 port;
    std::unique_ptr<QTcpSocket> socket;
    QString name;
    QQueue <QPair<Type, QString>> dataQueue;
};
