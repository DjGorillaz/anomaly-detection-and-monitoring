#include "client.h"
#include <QApplication>
#include <QSettings>

void setAutorun(const QApplication& app, const QString& appPath, const quint16& localPort, const QString& ip, const quint16& destPort)
{
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.setValue("client",
                      QDir::toNativeSeparators(app.applicationFilePath()) + " \""+
                      appPath + "\" " +
                      QString::number(localPort) + " " +
                      ip + " " +
                      QString::number(destPort)
                      );
    settings.sync();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Client* client1;

    //Get command line arguments
    if (argc <= 1)
    {
        client1 = new Client(&app, app.applicationDirPath(), 1234, "127.0.0.1", 12345);
        setAutorun(app, app.applicationDirPath(), 1234, "127.0.0.1", 12345);
    }
    else if (argc == 3)
    {
        //Set destination ip + port
        client1 = new Client(&app, app.applicationDirPath(), 1234, argv[1], QString(argv[2]).toInt());
        setAutorun(app, app.applicationDirPath(), 1234, argv[1], QString(argv[2]).toInt());
    }
    else if (argc == 4)
    {
        //Set path + destination ip + port
        QDir dir;
        dir.mkpath(argv[1]);
        client1 = new Client(&app, argv[1], 1234, argv[2], QString(argv[3]).toInt());
        setAutorun(app, argv[1], 1234, argv[2], QString(argv[3]).toInt());
    }
    else if (argc > 4)
    {
        //Set path + local port + destination ip + port
        QDir dir;
        dir.mkpath(argv[1]);
        client1 = new Client(&app, argv[1], QString(argv[2]).toInt(), argv[3], QString(argv[4]).toInt());
        setAutorun(app, argv[1], QString(argv[2]).toInt(), argv[3], QString(argv[4]).toInt());
    }

    QObject::connect(&app, &QCoreApplication::aboutToQuit, client1, &Client::deleteLater);

    return app.exec();
}
