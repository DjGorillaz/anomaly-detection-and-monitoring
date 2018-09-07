#pragma once

#include <memory>

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QDebug>

namespace AnomalyDetection::FileLib
{
    qint64 arrToInt(const QByteArray& qba);

    QByteArray intToArr(qint64 value);

    class Data: public QObject
    {
       Q_OBJECT
    public:
       Data(const QString& ip_, qint64 totalSize_, const QString& name_);
       virtual ~Data() = default;

       virtual void read(const QByteArray& buffer) = 0;
       virtual QByteArray write() = 0;
       virtual void emitSignal() = 0;
       qint64 getRemained();

    signals:
        void dataReceived(QString string, QString ip);

    protected:
       QString ip;
       qint64 totalSize;
       qint64 remained;
       QString name;
    };

    class String: public Data
    {
    public:
        String(const QString& ip_, qint64 totalSize_, const QString& name_);
        String(const QString& input);
        ~String() override = default;

        void read(const QByteArray& buffer) override;
        QByteArray write() override;
        void emitSignal() override;

    private:
        QString str;
    };

    class File: public Data
    {
    public:
        File(const QString& ip_, qint64 totalSize_, const QString& name_, const QString& path);
        File(const QString& input);
        ~File() override = default;

        void read(const QByteArray& buffer) override;
        QByteArray write() override;
        void emitSignal() override;

    private:
        QFile file;
        QString path;
        bool isInitDataSent;
    };
}
