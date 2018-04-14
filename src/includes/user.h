#pragma once

#include <memory>
#include <QString>
#include <QTimer>
#include <QObject>
#include <QMap>
#include <QVector>

#include "config.h"

class User: public QObject
{
    Q_OBJECT
public:
    User(const QString &name, const QString &ip, const quint16 &port_, const bool &online_);
    User(const QString& name, const QString& ip, const quint16& port_, const bool& online_,
         const quint64& d0_, const uint& N_, const float& k_,
         const QVector<int>& onesided_, const QMap<QString, QVector<int>>& features_, QVector<float>& weights_);
    void setStatus(State st);
    ~User() = default;
signals:
    void changedStatus(State st, QString ip);
private:
    QString username;
    QString ip;
    quint16 port;
    bool online;
    quint64 d0;
    uint N;
    float k;
    QVector<int> onesided;
    QVector<float> weights;
    QMap<QString, QVector<int>> features;
    std::unique_ptr<Config> cfg;
    std::unique_ptr<QTimer> offlineTimer;
    friend class Server;
};
