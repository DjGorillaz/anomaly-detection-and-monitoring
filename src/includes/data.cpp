#include "data.h"

//Get size of array
qint64 arrToInt(const QByteArray& qba)
{
    qint64 temp;
    QDataStream data(qba);
    data >> temp;
    return temp;
}

data::DataReader::DataReader(const QString& ip_, const QString& path_):
    ip{ip_},
    path{path_},
    empty{true},
    buffer{std::make_unique<QByteArray>()}
{  }

void data::DataReader::initData()
{
    //Get size, name, size of name
    quint64 size = arrToInt(buffer->mid(0,8));
    qint64 nameSize = arrToInt(buffer->mid(8,8));
    QString name = QString(buffer->mid(16, nameSize));
    //Delete read data
    buffer->remove(0, 16 + nameSize);

    if (name == "str") //string
    {
        //Create data::String and connect
        data = std::make_unique<data::String>(ip, size, name);
        QObject::connect(data.get(), &Data::dataReceived, this, &DataReader::stringReceived);
    }
    else
    {
        //Create data::File and connect
        data = std::make_unique<data::File>(ip, size, name, path);
        QObject::connect(data.get(), &Data::dataReceived, this, &DataReader::fileReceived);
    }

    empty = false; //Flag for first time initialization
}

bool data::DataReader::isEmpty()
{
    return empty;
}

void data::DataReader::readData()
{
    quint64 remained = data->getRemained();
    if (buffer->size() >= remained)
    {
        //If all data was received
        data->read(buffer->left(remained));
        buffer->remove(0, remained);
        empty = true;
        data->emitSignal();
    }
    else //Read all buffer
    {
        //If we didn't receive all data (buffer->size() < remained)
        data->read(*buffer);
        buffer->clear();
    }
}


data::Data::Data(const QString& ip_, quint64 totalSize_, const QString& name_):
    ip{ip_},
    totalSize{totalSize_},
    remained{totalSize_},
    name{name_}
{  qDebug() << "data created"; }

data::String::String(const QString& ip_, quint64 totalSize_, const QString& name_):
    data::Data{ip_, totalSize_, name_},
    str{}
{  }

void data::String::read(const QByteArray &buffer)
{
    qDebug() << buffer;
    str += buffer;
    remained -= buffer.size();
}

void data::String::emitSignal()
{
    emit dataReceived(str, ip);
}

quint64 data::Data::getRemained()
{
    return remained;
}


data::File::File(const QString& ip_, quint64 totalSize_, const QString& name_, const QString& path_):
    data::Data{ip_, totalSize_, name_},
    file{path_ + "/" + ip_ + "/" + name_},
    extension{name_.section('.', -1, -1)},
    path(path_)
{
    //If file exists -> change filename to "name (i).ext"
    //If extension == "log" -> do nothing (data will be appended data to existing file)
    if(file.exists() && (extension != "log"))
    {
        int ctr = 1;
        QString newFileName;
        while (file.exists())
        {
            newFileName = name_.section('.', 0, -2) +           //file name without extension
                        " (" + QString::number(ctr) + ")." +    //(i).
                        extension;
            file.setFileName(path + '/' + ip + '/' + newFileName);
            ++ctr;
        }
        name =  newFileName;
    }
}

void data::File::read(const QByteArray &buffer)
{
    //Open file and write to it
    if(!(file.open(QIODevice::Append)))
    {
        qDebug("File cannot be opened.");
        return;
    }

    file.write(buffer);
    file.close();

    remained -= buffer.size();
}

void data::File::emitSignal()
{
    emit dataReceived(path + '/' + ip + '/' + name, ip);
}
