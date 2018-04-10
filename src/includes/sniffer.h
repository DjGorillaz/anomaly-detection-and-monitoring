#pragma once
#include <QtGlobal>
#include <QObject>
#include <QSet>
#include <QTimer>
#include <QDebug>
#include "tins/tins.h"

class Sniffer: public QObject
{
    Q_OBJECT
public:
    Sniffer(QObject* parent, const std::string& f = "");
    ~Sniffer() = default;
    void start();
    void printData();
signals:
    void newData(const QString&);
private:
    bool callbackIP(const Tins::PDU &pdu);
    std::unique_ptr<Tins::NetworkInterface> iface;
    std::unique_ptr<Tins::SnifferConfiguration> config;
    std::unique_ptr<Tins::Sniffer> sniffer;
    qint64 totalUpSize = 0;
    qint64 totalDownSize = 0;
    std::string ip;
    QSet<QString> upConn;
    QSet<QString> downConn;
    std::unique_ptr<QTimer> timer;
    friend class Client;
};
