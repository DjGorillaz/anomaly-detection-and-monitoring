#include "fileclient.h"

//Convert qint64 to QByteArray
QByteArray intToArr(qint64 value)
{
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << value;
    return temp;
}

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
    QObject::connect(socket.get(), &QAbstractSocket::bytesWritten, this, &FileClient::writeToSocketCycle);
    QObject::connect(socket.get(), static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), [this](QAbstractSocket::SocketError socketError)
    {
        qDebug() << socketError;
    });

}

void FileClient::getOffline()
{
    QObject::disconnect(socket.get(), &QAbstractSocket::connected, 0, 0);
    socket->connectToHost(ip, port, QIODevice::WriteOnly);

    if (socket->waitForConnected(500))
    {
        QByteArray str = QString{"OFFLINE|" + name}.toUtf8();
        QByteArray stringSize = intToArr(str.size());

        //size("str") + "str"
        QByteArray fileTypeArr = QString{"str"}.toUtf8();
        QByteArray fileTypeArrSize = intToArr(fileTypeArr.size());

        //Write string
        socket->write(stringSize + fileTypeArrSize + fileTypeArr + str);

        if (socket->waitForBytesWritten(500))
        {
            socket->disconnectFromHost();
        }
    }
    else
        qDebug() << socket->errorString();
}

void FileClient::enqueueData(const Type& T, const QString& data)
{
    dataQueue.enqueue(qMakePair(T, data));
}

void FileClient::connect()
{
    auto state = socket->state();
    if(state == QAbstractSocket::ConnectedState)
    {
        sendData();
    }
    else if (state == QAbstractSocket::UnconnectedState)
    {
        qDebug() << "Connecting to " << ip << ':' << port;
        socket->connectToHost(ip, port, QIODevice::WriteOnly);
    }
}

bool FileClient::isDataQueueEmpty()
{
    return dataQueue.isEmpty();
}

void FileClient::sendData()
{
    if(socket->state() == QAbstractSocket::UnconnectedState)
    {
        //Reconnect
        connect();
        return;
    }

    //If queue is not empty
    if ( ! dataQueue.isEmpty())
    {
        auto [dataType, str] = dataQueue.first();
        QByteArray strUtf8 = str.toUtf8();

        if (dataType == Type::STRING) //Send string
        {
            //Get size(string)
            QByteArray stringSize = intToArr(strUtf8.size());
            //Get size("str") and "str"
            QByteArray fileTypeArr = QString{"str"}.toUtf8();
            QByteArray fileTypeArrSize = intToArr(fileTypeArr.size());

            //Write string
            //structure:    size(string) + size("str") + "str" + string
            socket->write(stringSize + fileTypeArrSize + fileTypeArr + strUtf8);

        }
        else if (dataType == Type::FILE) //Send file
        {
            //Open file
            QString& path = str;
            QFile file(path);
            if ( ! file.open(QIODevice::ReadOnly))
            {
                qDebug() << "Cannot open the file: " + path;
                dataQueue.dequeue();
                sendData();
                return;
            }

            //Get size(file)
            QByteArray fileSize = intToArr(file.size());
            //Get size(file_name) + file_name
            QString fileName = file.fileName();
            fileName = fileName.section('/',-1,-1);
            QByteArray fileNameArr = fileName.toUtf8();
            QByteArray fileNameArrSize = intToArr(fileNameArr.size());

            //Write file
            //structure: size(data) + size(file_name) + file_name + data
            socket->write(fileSize + fileNameArrSize + fileNameArr);
        }
    }
    else //If queue is empty
    {
        disconnect();
    }
}

void FileClient::writeToSocketCycle(qint64 /* bytesWritten */)
{
    if ( ! dataQueue.isEmpty())
    {
        auto [dataType, path] = dataQueue.first();
        if (dataType == Type::FILE)
        {
            static quint64 fileOffset = 0;

            //Open file
            QFile file(path);
            file.open(QIODevice::ReadOnly);

            //Write file by chunks
            file.seek(fileOffset);
            QByteArray fileArray = file.read(32768*8);
            file.close();

            if ( ! fileArray.isEmpty())
            {
                fileOffset += fileArray.size();
                socket->write(fileArray);
                return;
            }

            fileOffset = 0;
        }
        dataQueue.dequeue();
        sendData();
    }
    else
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
