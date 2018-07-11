#include "fileserver.h"

//Get peer name or ip
QString getIp(const QTcpSocket* socket)
{
    quint32 ipv4 = socket->peerAddress().toIPv4Address();
    QString name =  QString::number( (ipv4 >> 24) & 0xFF ) + '.' +
                    QString::number( (ipv4 >> 16) & 0xFF ) + '.' +
                    QString::number( (ipv4 >> 8) & 0xFF ) + '.' +
                    QString::number( ipv4 & 0xFF );
    return name;
}

FileServer::FileServer(QObject* parent, const quint16 &p, const QString& defaultPath):
    QObject(parent),
    port(p),
    path(defaultPath),
    server(std::make_unique<QTcpServer>(this))
{
    connect(server.get(), &QTcpServer::newConnection, this, &FileServer::newConnection);
}

bool FileServer::start()
{
    return server->listen(QHostAddress::Any, port);
}

void FileServer::newConnection()
{
    QTcpSocket* socket =  server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &FileServer::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &FileServer::disconnected);

    QString ip = getIp(socket);
    //Insert {socket, DataReader}
    auto [itPair, isInserted] = socketMap.try_emplace(socket, ip, path);

    //connect file and string receiving for each new socket (dataReader)
    QObject::connect(&(itPair->second), &data::DataReader::stringReceived, this, &FileServer::stringReceived);
    QObject::connect(&(itPair->second), &data::DataReader::fileReceived, this, &FileServer::fileReceived);

    //Make subfolder for each user
    QDir dir;
    dir.mkpath(path + "/" + ip);
}

/*
 * Recieved packet structure:
 * size(data) + size(file_name) + file_name + data
 *
 * size(data), size(file_name) - qint64
 * file_name, data - QByteArray
 */
void FileServer::readyRead()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());

    auto res = socketMap.find(socket);
    if(res == socketMap.end())
        return;
    data::DataReader& dataReader = res->second;

    while (socket->bytesAvailable() > 0 || dataReader.buffer->size() >= 16)
           //buffer.size() >= 16)
    {
        dataReader.buffer->append(socket->readAll());

        //Init data for every new file
        if (dataReader.isEmpty())
            dataReader.initData();

        dataReader.readData();
    }
}

void FileServer::disconnected()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    disconnect(socket, 0, 0, 0);

    socketMap.erase(socket);
    socket->deleteLater();

    qDebug() << "Client disconnected";
}
