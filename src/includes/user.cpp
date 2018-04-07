#include "user.h"


User::User(QString name, QString ip_, quint16 port_, bool online_):
    username(name),
    ip(ip_),
    port(port_),
    online(online_),
    timer(),
    cfg(std::make_unique<Config>())
{
    timer.setInterval(120000);
    timer.start();
    QObject::connect(&timer, &QTimer::timeout, [this](){
        online = false;
        timer.stop();
        emit offline(ip);
    });
}
