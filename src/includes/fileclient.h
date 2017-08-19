#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QAbstractSocket>
#include <QTcpSocket>

#include <QDataStream>
#include <QFile>
#include <QQueue>
#include <QPair>

enum type {
    _STRING = 0,
    _FILE = 1
};

class FileClient : public QObject
{
    Q_OBJECT
public:
    FileClient(QObject* parent, const QString& ip, const quint16& port);
    ~FileClient();

    void getOffline();
    void enqueueData(const type& T, const QString& data);
    void changePeer(const QString &ip, const quint16 &port);
    void connect();
    bool isDataQueueEmpty();

    const QString &getIp();
    const QString &getName();

signals:
    void error(QAbstractSocket::SocketError socketError);
    void transmitted();

private:
    void sendFile(const QString& file);
    void sendStr(const QString& str);
    void sendData();
    void writeFileToSocket(qint64 bytesWritten);
    void disconnect();

    QString ip;
    quint16 port;
    QTcpSocket* socket;
    QString name;
    QQueue <QPair<type, QString> > dataQueue;
};

#endif // FILECLIENT_H
