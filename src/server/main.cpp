#include "server.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Server* server;

    if (argc <= 1)
    {
        server = new Server;
    }
    else if (argc == 2)
    {
        //Set local port
        server = new Server(0, app.applicationDirPath(), QString(argv[1]).toInt() );
    }
    else if (argc >= 3)
    {
        //Set path + local port
        server = new Server(0, argv[1], QString(argv[2]).toInt());
    }

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [=](){ server->deleteLater(); } );
    server->show();

    return app.exec();
}
