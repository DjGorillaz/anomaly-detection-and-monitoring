#include "client.h"

#include <QApplication>
#include <QSettings>
#include <QProcess>

void setAutorun(const QApplication& app, const quint16& localPort, const QString& ip, const quint16& destPort)
{
    //Path to exe and parameters
    QString path = QDir::toNativeSeparators(app.applicationFilePath()) + " " +
                  QString::number(localPort) + " " +
                  ip + " " +
                  QString::number(destPort);

    QSettings adminSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QSettings userSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    //Write to registry
    adminSettings.setValue("client", path);
    adminSettings.sync();

    if(adminSettings.status() == QSettings::NoError)
    {
        //Delete duplicate
        userSettings.remove("client");
    }
    else //User doesn't have admin rights
    {
        //Check if autorun key already exists
        if (adminSettings.value("client", "").toString() == path)
            return;

        userSettings.setValue("client", path);
    }
}

int main(int argc, char *argv[])
{
    try
    {
        QApplication app(argc, argv);
        Client* client;

        //Get command line arguments
        if (argc <= 2)
        {
            //Default parameters
            client = new Client(&app, app.applicationDirPath(), 1234, "127.0.0.1", 12345);
            setAutorun(app, 1234, "127.0.0.1", 12345);
        }
        else if (argc == 3)
        {
            //Set destination ip + port
            client = new Client(&app, app.applicationDirPath(), 1234, argv[1], QString(argv[2]).toInt());
            setAutorun(app, 1234, argv[1], QString(argv[2]).toInt());
        }
        else if (argc == 4)
        {
            //Set path + destination ip + port
            QDir dir;
            dir.mkpath(argv[1]);
            client = new Client(&app, argv[1], 1234, argv[2], QString(argv[3]).toInt());
            setAutorun(app, 1234, argv[2], QString(argv[3]).toInt());
        }
        else if (argc > 4)
        {
            //Set path + local port + destination ip + port
            QDir dir;
            dir.mkpath(argv[1]);
            client = new Client(&app, argv[1], QString(argv[2]).toInt(), argv[3], QString(argv[4]).toInt());
            setAutorun(app, QString(argv[2]).toInt(), argv[3], QString(argv[4]).toInt());
        }

        return app.exec();
    }
    catch(...)
    {
        qDebug() << "error";
    }
}
