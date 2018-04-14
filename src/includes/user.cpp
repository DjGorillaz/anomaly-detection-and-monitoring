#include "user.h"


User::User(const QString& name, const QString& ip_, const quint16& port_, const bool& online_):
    username{name},
    ip{ip_},
    port{port_},
    online{online_},
    offlineTimer{std::make_unique<QTimer>()},
    cfg{std::make_unique<Config>()}
{
    offlineTimer->setInterval(120000);
    offlineTimer->start();
    QObject::connect(offlineTimer.get(), &QTimer::timeout, [this](){
        setStatus(State::OFFLINE);
    });

    onesided = {0, 0, 0, 0, 0, 0, 0};
    weights = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    //TODO
    N = 0;
    k = 0.1123;
    d0 = 65000000000000;

    //TODO del
    features.insert("13.04.2018", {1,2,3,4,5,6,7});
    features.insert("14.04.2018", {10,20,30,40,50,60,70});
    features.insert("12.04.2018", {0,3,4,10,117,434,14343424});
}

User::User(const QString& name, const QString& ip_, const quint16& port_, const bool& online_,
           const quint64& d0_, const uint& N_, const float& k_,
           const QVector<int>& onesided_, const QMap<QString, QVector<int>>& features_, QVector<float>& weights_):
    User{name, ip_, port_, online_}
{
    onesided = onesided_;
    weights = weights_;
    features = features_;
    d0 = d0_;
    N = N_;
    k = k_;
}


void User::setStatus(State st)
{
    if (st == State::ONLINE)
    {
        offlineTimer->start();
        online = true;
        emit changedStatus(State::ONLINE, ip);
    }
    else
    {
        offlineTimer->stop();
        online = false;
        emit changedStatus(State::OFFLINE, ip);
    }
}
