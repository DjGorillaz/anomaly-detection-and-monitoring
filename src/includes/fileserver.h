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
    ~FileServer() = default;

    bool start();

signals:
    void dataSaved(QString path, QString ip);
    void stringReceived(QString string, QString ip);

private slots:
    void newConnection();
    void readyRead();
    void disconnected();

private:
    void resetSocketMap(QTcpSocket*);

    quint16 port;
    QString path;
    std::unique_ptr<QTcpServer> server;
    std::unordered_map<QTcpSocket*, std::tuple<std::unique_ptr<QByteArray>, qint64, QString, bool>> socketMap;
};

qint64 arrToInt(const QByteArray& qba);

QString getIp(const QTcpSocket* socket);
