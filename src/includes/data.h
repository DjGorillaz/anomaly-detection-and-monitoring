#pragma once

#include <memory>

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QDebug>

class FileServer;

qint64 arrToInt(const QByteArray& qba);

namespace data
{

    class Data;

    class DataReader: public QObject
    {
        Q_OBJECT
    public:
        DataReader(const QString& ip_, const QString& path_);
        ~DataReader() = default;
        void initData();
        bool isEmpty();
        void readData();

        //todo -> private
        std::unique_ptr<QByteArray> buffer;

    signals:
        void stringReceived(QString string, QString ip);
        void fileReceived(QString path, QString ip);

    private:
        QString ip;
        QString path;
        bool empty;
        std::unique_ptr<Data> data;
    };

    class Data: public QObject
    {
       Q_OBJECT
    public:
       Data(const QString& ip_, quint64 totalSize_, const QString& name_);
       virtual ~Data() = default;
       virtual void read(const QByteArray& buffer) = 0;
       virtual void emitSignal() = 0;
       quint64 getRemained();

    signals:
        void dataReceived(QString string, QString ip);

    protected:
       QString ip;
       quint64 totalSize;
       quint64 remained;
       QString name;
       //bool empty;quint64 remained;
    };

    class String: public Data//, public QObject
    {
        //Q_OBJECT
    public:
        String(const QString& ip_, quint64 totalSize_, const QString& name_);
        ~String() override = default;
        void read(const QByteArray& buffer) override;
        void emitSignal() override;

    //signals:
    //   void stringReceived(QString string, QString ip);

    private:
        QString str;
    };

    class File: public Data//, public QObject
    {
        //Q_OBJECT
    public:
        File(const QString& ip_, quint64 totalSize_, const QString& name_, const QString& path);
        ~File() override = default;
        void read(const QByteArray& buffer) override;
        void emitSignal() override;

    //signals:
    //    void fileReceived(QString path, QString ip);

    private:
        QFile file;
        QString extension;
        QString path;
    };
}
