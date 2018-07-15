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

    //Insert {socket, pair{QByteArray, std::nullopt} }
    socketMap.try_emplace(socket, std::make_unique<QByteArray>(), std::nullopt);

    //Make subfolder for each user
    QDir dir;
    dir.mkpath(path + "/" + getIp(socket));
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
    auto& [buffer, optData] = socketMap[socket];

    while (socket->bytesAvailable() > 0 || buffer->size() >= 16)
    {
        buffer->append(socket->readAll());

        if ( ! optData.has_value())
        {
            //Get size, name, size(name)
            qint64 size {arrToInt(buffer->mid(0,8))};
            qint64 nameSize {arrToInt(buffer->mid(8,8))};
            QString name {buffer->mid(16, nameSize)};
            //Delete read data
            buffer->remove(0, 16 + nameSize);

            QString ip = getIp(socket);
            if (name == "str") //Create data::String and connect
            {
                optData = std::make_unique<data::String>(ip, size, name);
                QObject::connect(optData.value().get(), &data::Data::dataReceived, this, &FileServer::stringReceived);
            }
            else //Create data::File and connect
            {
                optData = std::make_unique<data::File>(ip, size, name, path);
                QObject::connect(optData.value().get(), &data::Data::dataReceived, this, &FileServer::fileReceived);
            }
        }

        auto& data = optData.value();

        qint64 remained = data->getRemained();
        if (buffer->size() >= remained)
        {
            //If all data was received
            data->read(buffer->left(remained));
            buffer->remove(0, remained);
            data->emitSignal();
            optData = std::nullopt;
        }
        else //Read all buffer
        {
            //If we didn't receive all data (buffer->size() < remained)
            data->read(*(buffer));
            buffer->clear();
        }
    }
}

void FileServer::disconnected()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());

    if ( ! socketMap.empty()) //Gets error without .empty()
    {
        if(auto it = socketMap.find(socket); it != socketMap.end())
            socketMap.erase(it);
    }

    socket->disconnect(); //signals
    socket->deleteLater();

    qDebug() << "Client disconnected";
}
