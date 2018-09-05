#pragma once

#include <memory>

#include <QTimer>
#include <QVector>
#include <QMap>
#include <QDate>

#include <Eigen/Dense>

#include "config.h"


class User: public QObject
{
    Q_OBJECT
public:
    User(const QString &name, const QString &ip, const quint16 &port_, const bool &online_);
    User(const QString& name, const QString& ip, const quint16& port_, const bool& online_,
         const double& d0_, const int& N_, const double& k_,
         const QVector<int>& onesided_,
         const QMap<QString, QPair<QVector<double>, QVector <double>>>& features_,
         QVector<float>& weights_);
    void setStatus(State st);
    void setFeatures(const double& up, const double& upConn, const double& dwn, const double& dwnConn, bool sniffer = true);
    void setFeatures(bool sniffer = false);
    QVector<double> getScore(const QString &date);
    ~User() = default;
signals:
    void changedStatus(State st, QString ip);
    void dataChanged(QString date);
    void newData(QString date);
private:
    QString username;
    QString ip;
    quint16 port;
    bool online;
    double d0;
    int N;
    double k;
    QVector<int> onesided;
    QVector<float> weights;
    QMap<QString, QPair<QVector<double>, QVector <double>>> features;
    std::unique_ptr<Config> cfg;
    std::unique_ptr<QTimer> offlineTimer;
    friend class Server;
};
