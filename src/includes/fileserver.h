#pragma once

#include <memory>
#include <unordered_map>
#include <optional>

#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QFile>
#include <QDir>

#include <data.h>

namespace AnomalyDetection::FileLib
{
    QString getIp(const QTcpSocket* socket);

    class FileServer : public QObject
    {
        Q_OBJECT
    public:
        FileServer(QObject *parent, const quint16& port, const QString& defaultPath = QDir::currentPath());
        ~FileServer() = default;
        bool start();

    signals:
        void fileReceived(QString path, QString ip);
        void stringReceived(QString string, QString ip);

    private slots:
        void newConnection();
        void readyRead();
        void disconnected();

    private:
        quint16 port;
        QString path;
        std::unique_ptr<QTcpServer> server;
        std::unordered_map<QTcpSocket*, std::pair<std::unique_ptr<QByteArray>,
                                                  std::optional<std::unique_ptr<Data>>> > socketMap;
    };
}
