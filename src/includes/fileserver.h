#ifndef FILESERVER_H
#define FILESERVER_H

#include <QTcpServer>
#include <QTcpSocket>

#include <QDataStream>
#include <QFile>
#include <QDir>

class FileServer : public QObject
{
    Q_OBJECT
public:
    FileServer(QObject *parent, const quint16& port, const QString& defaultPath = QDir::currentPath());
    ~FileServer();

    bool start();

signals:
    void dataSaved(QString path, QString ip);
    void stringReceived(QString string, QString ip);
    void dataGet(qint64, qint64);

private slots:
    void newConnection();
    void readyRead();
    void disconnected();
    void progress(const qint64, const qint64); //test function

private:
    void nullBuffer(QTcpSocket*);

    quint16 port;
    QString path;
    QTcpServer* server;
    QHash<QTcpSocket*, QByteArray*> buffers;
    QHash<QTcpSocket*, qint64> sizes;
    QHash<QTcpSocket*, QString> names;
    QHash<QTcpSocket*, bool> areNamesFinal;
};

qint64 arrToInt(const QByteArray& qba);

QString getIp(const QTcpSocket* socket);

#endif // FILESERVER_H
