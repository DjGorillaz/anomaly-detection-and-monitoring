#pragma once

#include <memory>
#include <unordered_map>

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
    std::unique_ptr<QTcpServer> server;
    std::unordered_map<QTcpSocket*, std::unique_ptr<QByteArray>> buffers;
    std::unordered_map<QTcpSocket*, qint64> sizes;
    std::unordered_map<QTcpSocket*, QString> names;
    std::unordered_map<QTcpSocket*, bool> areNamesFinal;
};

qint64 arrToInt(const QByteArray& qba);

QString getIp(const QTcpSocket* socket);
