#include "fileclient.h"

//Convert qint64 to QByteArray
QByteArray intToArr(qint64 value)
{
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << value;
    return temp;
}

FileClient::FileClient(QObject* parent, const QString &i, const quint16 &p):
    QObject(parent),
    ip(i),
    port(p)
{
    socket = new QTcpSocket(this);

    //Get username
    name = qgetenv("USER");
    if (name.isEmpty())
    {
        name = qgetenv("USERNAME");
        if (name.isEmpty())
            name = qgetenv("COMPUTERNAME");
    }

    QObject::connect(socket, &QAbstractSocket::connected, this, &FileClient::sendData);
    QObject::connect(socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
                     this, &FileClient::error);
}

FileClient::~FileClient()
{
    socket->deleteLater();
    qDebug() << "File client deleted.";
}

void FileClient::getOffline()
{
    QObject::disconnect(socket, &QAbstractSocket::connected, 0, 0);
    socket->connectToHost(ip, port, QIODevice::WriteOnly);

    if (socket->waitForConnected(500))
    {
        QString str = "OFFLINE|" + name;
        QByteArray stringSize = intToArr(str.toUtf8().size());

        //size("str") + "str"
        QString fileName = "str";
        QByteArray fileNameArr = fileName.toUtf8();
        QByteArray fileNameArrSize = intToArr(fileNameArr.size());

        //Write string
        socket->write(stringSize + fileNameArrSize + fileNameArr + str.toUtf8());

        if (socket->waitForBytesWritten(500))
            qDebug() << "Offline string was sent.";
        else
            qDebug() << socket->errorString();
    }
    else
        qDebug() << socket->errorString();
    socket->disconnectFromHost();
}

void FileClient::enqueueData(const type& T, const QString &data)
{
    dataQueue.enqueue(qMakePair(T, data));
}

void FileClient::connect()
{
    qDebug() << "Connecting to " << ip << ':' << port;
    socket->connectToHost(ip, port, QIODevice::WriteOnly);
}

bool FileClient::isDataQueueEmpty()
{
    return dataQueue.isEmpty();
}

void FileClient::sendData()
{
    //Disconnect because sendFile & sendStr will create another connection
    QObject::disconnect(socket, &QAbstractSocket::bytesWritten, 0, 0);
    //While queue is not empty
    if ( !dataQueue.isEmpty())
    {
        QPair<type, QString> pair = dataQueue.first();
        QString str = pair.second;
        if (pair.first == _STRING)
        {
            //Send string
            sendStr(str);
        }
        else
        {
            //Send file
            sendFile(str);
        }
    }
    else
        disconnect();
}

/*
 * Packet structure:
 * size(string) + size("str") + "str" + string
 *
 * size(string), size("str") - qint64
 * string, "str" - QByteArray
 */
void FileClient::sendStr(const QString& str)
{
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        QObject::connect(socket, &QAbstractSocket::bytesWritten, this, &FileClient::sendData);
        //Get size(string)
        QByteArray stringSize = intToArr(str.toUtf8().size());
        //Get size("str") and "str"
        QString fileName = "str";
        QByteArray fileNameArr = fileName.toUtf8();
        QByteArray fileNameArrSize = intToArr(fileNameArr.size());

        //Write string
        socket->write(stringSize + fileNameArrSize + fileNameArr + str.toUtf8());
        dataQueue.dequeue();
    }
    else
    {
        qDebug() << "No conection established.";
    }
}

/*
 * Packet structure:
 * size(data) + size(file_name) + file_name + data
 *
 * size(data), size(file_name) - qint64
 * file_name, data - QByteArray
 */
void FileClient::sendFile(const QString& path)
{
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        //Open file
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

        QObject::connect(socket, &QAbstractSocket::bytesWritten, this, &FileClient::writeFileToSocket);
        socket->write(fileSize + fileNameArrSize + fileNameArr);
    }
    else
    {
        qDebug() << "No conection established";
    }
}


void FileClient::writeFileToSocket(qint64 bytesWritten)
{
    static bool first = true;
    static qint64 pos = 0;
    if (first)
    {
         pos -= bytesWritten;
         first = false;
    }
    pos += bytesWritten;

    //Open file
    QString path = dataQueue.first().second;
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    //Write file by chunks
    file.seek(pos);
    QByteArray fileArray = file.read(32768*8);
    file.close();

    if( !fileArray.isEmpty())
    {
        socket->write(fileArray);
    }
    else
    {
        //Delete file from queue
        dataQueue.dequeue();
        pos = 0;
        first = true;
        sendData();
    }
}

void FileClient::disconnect()
{
    socket->disconnectFromHost();
    qDebug() << "Disconnected from host.";
    //Disconnect everything from bytesWritten
    QObject::disconnect(socket, &QAbstractSocket::bytesWritten, 0, 0);
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

void FileClient::changePeer(const QString& newIp, const quint16& newPort)
{
    ip = newIp;
    port = newPort;
}



