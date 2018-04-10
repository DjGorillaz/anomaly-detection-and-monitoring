#define TINS_STATIC

#include "sniffer.h"

Sniffer::Sniffer(QObject* parent, const std::string& f):
    QObject(parent),
    iface{std::make_unique<Tins::NetworkInterface>(Tins::NetworkInterface::default_interface())},
    timer(std::make_unique<QTimer>())
{
    timer->setInterval(60000); //1min
    timer->start();
    connect(timer.get(), &QTimer::timeout, [this](){
        emit newData(QString::number(totalUpSize >> 10) + '|' + QString::number(upConn.size()) + '|'
                    + QString::number(totalDownSize >> 10) + '|' + QString::number(downConn.size()));
    });

    // Get the default interface
    iface = std::make_unique<Tins::NetworkInterface>(Tins::NetworkInterface::default_interface());

    ip = iface->addresses().ip_addr.to_string();

    config = std::make_unique<Tins::SnifferConfiguration>();
    config->set_promisc_mode(true);
    config->set_filter(f);

    // Now instantiate the sniffer
    sniffer = std::make_unique<Tins::Sniffer>(iface->name(), *config);
}

void Sniffer::start()
{
    sniffer->sniff_loop(bind(
                            &Sniffer::callbackIP,
                            this,
                            std::placeholders::_1
                            )
                        );
}

void Sniffer::printData()
{
    qDebug() << "Up: total=" << (totalUpSize >> 10) << " conn=" << upConn.size() <<"\n";
    qDebug() << "Down: total=" << (totalDownSize >> 10) << " conn=" << downConn.size() <<"\n";
}

bool Sniffer::callbackIP(const Tins::PDU &pdu) {
    // Find the IP layer
    const Tins::IP &ip = pdu.rfind_pdu<Tins::IP>();

    // Find the TCP layer
    //const Tins::TCP &tcp = pdu.rfind_pdu<Tins::TCP>();
    //const Tins::UDP &udp = pdu.rfind_pdu<Tins::UDP>();
    //cout << ip.src_addr() << ':' << tcp.sport() << " -> "
    //     << ip.dst_addr() << ':' << tcp.dport() << "\t" << ip.size() << endl;


    //cout << (totalSize >> 20);
    //cout << endl << (totalSize >> 20) << endl;
    //if (old != totalSize >> 20) cout << (totalSize >> 20) << endl;
    //old = totalSize >> 20;

    //upload
    if (ip.src_addr() == this->ip)
    {
        totalUpSize += ip.size();

        //Add new adress
        QString dstIp = QString::fromStdString(ip.dst_addr().to_string());
        upConn.insert(dstIp);

    }
    else //download
    {
        totalDownSize += ip.size();

        //Add new adress
        QString srcIp = QString::fromStdString(ip.src_addr().to_string());
        downConn.insert(srcIp);
    }

    return true;
}
