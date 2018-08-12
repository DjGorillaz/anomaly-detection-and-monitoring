#include "client.h"

#include <QApplication>
#include <QSettings>
#include <QProcess>

void setAutorun(const QApplication& app, const QString& ip, const quint16& destPort, const quint16& localPort, const QString& savePath)
{
    //Path to exe and parameters
    QString path = QDir::toNativeSeparators(app.applicationFilePath()) + " " +
                   ip + " " +
                   QString::number(destPort) + " " +
                   QString::number(localPort) + " " +
                   savePath;

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

        //default values:
        QString ip = "127.0.0.1";
        quint16 destPort = 12345;
        quint16 localPort = 1234;
        QString savePath = app.applicationDirPath();

        if (argc > 2)
        {
            ip = argv[1];
            destPort =  QString(argv[2]).toInt();
        }
        if (argc > 3)
        {
            localPort = QString(argv[3]).toInt();
        }
        if (argc > 4)
        {
            savePath = argv[4];
            QDir dir;
            dir.mkpath(savePath);
        }

        Client client(&app, ip, destPort, localPort, savePath);
        setAutorun(app, ip, destPort, localPort, savePath);

        return app.exec();
    }
    catch(...)
    {
        qDebug() << "error";
    }
}
