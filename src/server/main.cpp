#include "server.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    try
    {
        QApplication app(argc, argv);

        //default values
        quint16 port = 12345;
        QString path = app.applicationDirPath();

        if (argc > 1)
        {
            port = QString(argv[1]).toInt();
        }
        if (argc > 2)
        {
            path = argv[2];
        }

        Server server(nullptr, port, path);
        server.show();

        return app.exec();
    }
    catch(...)
    {
        qDebug() << "error";
    }
}
