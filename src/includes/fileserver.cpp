#include "fileserver.h"

//Get size of array
qint64 arrToInt(const QByteArray& qba)
{
    qint64 temp;
    QDataStream data(qba);
    data >> temp;
    return temp;
}

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

    socketMap.emplace(socket, std::tuple{std::make_unique<QByteArray>(""),
                                         0,
                                         "",
                                         false}
                      );

    //Make subfolder for each user
    QString subFolder = getIp(socket);
    QDir dir;
    dir.mkpath(path + "/" + subFolder);
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
    auto& [pBuffer, size, fileName, isNameFinal] = socketMap[socket];
    QByteArray& buffer = *(pBuffer.get());

    //For differrent users
    QString subFolder =  getIp(socket);

    while (socket->bytesAvailable() > 0 || buffer.size() >= 16)
    {
        buffer.append(socket->readAll());

        //Read data for the first time
        if (buffer.size() >= 16 && size == 0)
        {
            size = arrToInt(buffer.mid(0,8));
            qint64 fileNameSize = arrToInt(buffer.mid(8,8));
            fileName = QString(buffer.mid(16, fileNameSize));
            qDebug() << fileName;
            //Remove read data
            buffer.remove(0, 16 + fileNameSize);
        }

        //If we get file
        if (fileName != "str")
        {
            QFile file(path + '/' + subFolder + '/' + fileName);

            //Check extension
            QString extension = fileName.section('.', -1, -1);

            if( ( ! isNameFinal) && file.exists() && (extension != "log"))
            {
                int ctr = 1;
                QString newFileName;

                while (file.exists())
                {
                    newFileName = fileName.section('.', 0, -2) +        //file name
                                " (" + QString::number(ctr) + ")." +    //(i).
                                fileName.section('.', -1, -1);          //extension
                    file.setFileName(path + '/' + subFolder + '/' + newFileName);
                     ++ctr;
                }
                fileName =  newFileName;
            }

            isNameFinal = true;

            //Open file and write to it
            if(!(file.open(QIODevice::Append)))
            {
                qDebug("File cannot be opened.");
                return;
            }

            if (buffer.size() < size)
            {
                file.write(buffer);
                size -= buffer.size();
                buffer.clear();
                file.close();
            }
            else
            {
                //If we receive all data
                //Write to file first (size - fileSize) bytes from buffer
                file.write(buffer.left(size));
                buffer.remove(0, size);
                file.close();
                size = 0; //del

                qDebug() << "File received";

                QString savePath(path + '/' + subFolder + '/' + fileName);
                resetSocketMap(socket);
                emit dataSaved(savePath, subFolder); //subFolder == ip
            }
        }
        //If we get string
        else
        {
            //Complete string received
            if (buffer.size() >= size)
            {
                qDebug() << buffer.left(size);
                emit stringReceived(buffer.left(size), subFolder); //subFolder == ip
                buffer.remove(0, size);
                resetSocketMap(socket);

                //Delete if directory is empty
                QDir dir;
                dir.rmdir(path + "/" + subFolder);

            }
            else
            {
                //todo
                //Erase buffer and socketMap
                resetSocketMap(socket);
                buffer.clear();
            }
        }
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
void FileServer::resetSocketMap(QTcpSocket* socket)
{
    //Buffer is not erased
    auto& [pBuffer, size, fileName, isNameFinal] = socketMap[socket];
    //Reset values before next data arrives
    size = 0;
    fileName.clear();
    isNameFinal = false;
}
