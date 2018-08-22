#include "fileclient.h"

FileClient::FileClient(QObject* parent, const QString &ip_, quint16 port_):
    QObject(parent),
    ip(ip_),
    port(port_),
    socket(std::make_unique<QTcpSocket>(this))
{
    //Get username
    std::array envs = {"USER", "USERNAME", "COMPUTERNAME"};
    for(auto const& env: envs)
    {
        if(name = qgetenv(env); ! name.isEmpty())
            break;
    }

    QObject::connect(socket.get(), &QAbstractSocket::connected, this, &FileClient::sendData);
    QObject::connect(socket.get(), &QAbstractSocket::bytesWritten, this, &FileClient::sendData);
    QObject::connect(socket.get(), static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
                     [](QAbstractSocket::SocketError socketError)
    {
        qDebug() << socketError;
    });
}

void FileClient::sendAndDisconnect(const QString& data)
{
    QObject::disconnect(socket.get(), &QAbstractSocket::connected, nullptr, nullptr);

    socket->disconnectFromHost();
    socket->connectToHost(ip, port, QIODevice::WriteOnly);

    if (socket->waitForConnected(1000))
    {
        socket->write(data::String{data}.write());

        if (socket->waitForBytesWritten(1000))
            socket->disconnectFromHost();
    }
}

void FileClient::enqueueDataAndConnect(std::unique_ptr<data::Data> &&data)
{
    dataQueue.push(std::move(data));
    connect();
}

void FileClient::connect()
{
    if (socket->state() == QAbstractSocket::UnconnectedState)
    {
        qDebug() << "Connecting to " << ip << ':' << port;
        socket->connectToHost(ip, port, QIODevice::WriteOnly);
    }
}

void FileClient::sendData()
{
    if(socket->state() == QAbstractSocket::UnconnectedState)
    {
        //Reconnect
        socket->connectToHost(ip, port, QIODevice::WriteOnly);
        return;
    }

    //If queue is not empty
    if ( ! dataQueue.empty())
    {
        auto& data = dataQueue.front();
        if (data->getRemained() > 0)
        {
            //If there is data send it, otherwise try again
            auto array {data->write()};
            if (array.size() > 0)
                socket->write(std::move(array));
            else
                sendData();

            return;
        }

        dataQueue.pop();
        sendData();
    }
    else //If queue is empty
    {
        disconnect();
    }
}

void FileClient::disconnect()
{
    socket->disconnectFromHost();
    qDebug() << "Disconnected from host.";
    emit transmitted();
}

const QString& FileClient::getName()
{
    return name;
}

const QString& FileClient::getIp()
{
    return ip;
}

void FileClient::changePeer(const QString& newIp, quint16 newPort)
{
    ip = newIp;
    port = newPort;
}
