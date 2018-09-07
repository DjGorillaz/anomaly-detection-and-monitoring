#pragma once

#include <memory>

#include "sniffer.h"
#include "config.h"
#include "fileserver.h"
#include "fileclient.h"
#include "mousehook.h"
#include "enums.h"
#include "klog.h"

namespace AnomalyDetection
{
    class Client : public QObject
    {
        Q_OBJECT
    public:
        explicit Client(QObject *parent = nullptr, const QString& ip = "127.0.0.1", const quint16& destPort = 12345,
                        const quint16& locPort = 1234, const QString& path = QDir::currentPath());
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
        std::unique_ptr<AnomalyDetection::FileLib::Config> config;
        std::unique_ptr<AnomalyDetection::FileLib::FileServer> fileServer;
        std::unique_ptr<AnomalyDetection::FileLib::FileClient> fileClient;
        std::unique_ptr<Sniffer> sniffer;
    };
}
