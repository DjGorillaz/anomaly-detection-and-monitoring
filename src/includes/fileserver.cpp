#include "fileserver.h"
#include <QByteArrayMatcher>
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
    path(defaultPath)
{
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &FileServer::newConnection);
}

FileServer::~FileServer()
{
    delete server;
    qDebug() << "File server deleted.";
}

bool FileServer::start()
{
    return (server->listen(QHostAddress::Any, port));
}

void FileServer::newConnection()
{
    QTcpSocket* socket =  server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &FileServer::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &FileServer::disconnected);

    QByteArray* buffer = new QByteArray("");
    QString fileName("");
    buffers.insert(socket, buffer);
    sizes.insert(socket, 0);
    names.insert(socket, fileName);
    areNamesFinal.insert(socket, false);

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
    QByteArray* buffer = buffers.value(socket);
    qint64& size = sizes[socket];
    QString& fileName = names[socket];

    //For differrent users
    QString subFolder = getIp(socket);

    while (socket->bytesAvailable() > 0 || buffer->size() >= 16)
    {
        QByteArray tempArray = socket->readAll();
        buffer->append(tempArray);
        //Read data for the first time
        if (buffer->size() >= 16 && size == 0)
        {
            size = arrToInt(buffer->mid(0,8));
            qint64 fileNameSize = arrToInt(buffer->mid(8,8));
            fileName = QString(buffer->mid(16, fileNameSize));
            //Remove read data
            buffer->remove(0, 16 + fileNameSize);
        }

        //If we get file
        if (fileName != "str")
        {
            QFile file(path + '/' + subFolder + '/' + fileName);
            QString newFileName;
            int ctr = 1;
            //Check extension
            QString extension = fileName.section('.', -1, -1);

            qint64 fileSize = file.size();
            //If log received
            if (extension == "log")
            {
                //For correct log saving
                fileSize = 0;
            }
            //If file already exists add (i)
            else if (file.exists() && areNamesFinal.value(socket) == false)
            {
                while (file.exists())
                {
                    newFileName = fileName.section('.', 0, -2) +        //file name
                                " (" + QString::number(ctr) + ")." +    //(i).
                                fileName.section('.', -1, -1);          //extension
                    file.setFileName(path + '/' + subFolder + '/' + newFileName);
                     ++ctr;
                }
                //rename file in QHash
                names[socket] =  newFileName;
            }
            if (areNamesFinal.value(socket)== false)
                areNamesFinal[socket] = true;

            //Open file and write to it
            if(!(file.open(QIODevice::Append)))
            {
                qDebug("File cannot be opened.");
            }

            //Signal for progress bar
            emit dataGet(fileSize, size);

            if (fileSize + buffer->size() < size)
            {
                file.write(*buffer);
                buffer->clear();
                file.close();
            }
            //If we receive all data and
            //buffer size + file size >= actual file size
            else
            {
                //Write to file first (size - fileSize) bytes from buffer
                file.write(buffer->left(size - fileSize));
                buffer->remove(0, size - fileSize);

                file.close();
                qDebug() << "File received";

                QString savePath(path + '/' + subFolder + '/' + names.value(socket));
                nullBuffer(socket);
                emit dataSaved(savePath, subFolder);
            }
        }
        //If we get string
        else
        {
            //If not empty string received
            //buffer->size = size + 16 + name->toUtf8().size()
            if (buffer->size() >= size)
            {
                qDebug() << buffer->left(size);
                emit stringReceived( buffer->left(size), subFolder); //subFolder == ip
                buffer->remove(0, size);
                nullBuffer(socket);

                //Check if directory is empty
                QDir dir;
                dir.rmdir(path + "/" + subFolder);

                //If buffer is not empty
                if (buffer->size() >= 16)
                {
                    emit socket->readyRead();
                }
            }
            else
                nullBuffer(socket);
        }
    }
}

void FileServer::disconnected()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    disconnect(socket, 0, 0, 0);

    //Delete buffers
    delete buffers.value(socket);

    buffers.remove(socket);
    sizes.remove(socket);
    names.remove(socket);
    areNamesFinal.remove(socket);

    qDebug() << "Client disconnected";
    socket->deleteLater();
}

void FileServer::nullBuffer(QTcpSocket* socket)
{
    //Null buffer before next data arrive
    sizes[socket] = 0;
    names[socket].clear();
    areNamesFinal[socket] = false;
}

void FileServer::progress(const qint64 current, const qint64 overall)
{
    qDebug() << current << "MB of " << overall << "MB";
}
