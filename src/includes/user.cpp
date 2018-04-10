#include "user.h"


User::User(QString name, QString ip_, quint16 port_, bool online_):
    username(name),
    ip(ip_),
    port(port_),
    online(online_),
    offlineTimer(),
    cfg(std::make_unique<Config>())
{
    offlineTimer.setInterval(120000);
    offlineTimer.start();
    QObject::connect(&offlineTimer, &QTimer::timeout, [this](){
        setStatus(State::OFFLINE);
    });
}

void User::setStatus(State st)
{
    if (st == State::ONLINE)
    {
        offlineTimer.start();
        online = true;
        emit changedStatus(State::ONLINE, ip);
    }
    else
    {
        offlineTimer.stop();
        online = false;
        emit changedStatus(State::OFFLINE, ip);
    }
}
