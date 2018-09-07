#include "data.h"

namespace AnomalyDetection::FileLib
{
    //Get size of array
    qint64 arrToInt(const QByteArray& qba)
    {
        qint64 temp;
        QDataStream data(qba);
        data >> temp;
        return temp;
    }

    //Convert qint64 to QByteArray
    QByteArray intToArr(qint64 value)
    {
        QByteArray temp;
        QDataStream data(&temp, QIODevice::ReadWrite);
        data << value;
        return temp;
    }

    Data::Data(const QString& ip_, qint64 totalSize_, const QString& name_):
        ip{ip_},
        totalSize{totalSize_},
        remained{totalSize_},
        name{name_}
    {  }

    qint64 Data::getRemained()
    {
        return remained;
    }

    String::String(const QString& ip_, qint64 totalSize_, const QString& name_):
        Data{ip_, totalSize_, name_},
        str{}
    {  }

    String::String(const QString &input):
        Data{"", input.size(), "str"},
        str{input}
    {  }

    void String::read(const QByteArray &buffer)
    {
        qDebug() << buffer;
        str += buffer;
        remained -= buffer.size();
    }

    QByteArray String::write()
    {
        QByteArray string {str.toUtf8()};
        QByteArray type {name.toUtf8()};
        qint64 strSize = string.size();

        string.prepend(type);
        string.prepend(intToArr(type.size()));
        string.prepend(intToArr(strSize));

        //Final string:    size(string) + size("str") + "str" + string
        remained = 0;
        return string;
    }

    void String::emitSignal()
    {
        emit dataReceived(str, ip);
    }

    File::File(const QString& ip_, qint64 totalSize_, const QString& name_, const QString& path_):
        Data{ip_, totalSize_, name_},
        file{path_ + "/" + ip_ + "/" + name_},
        path(path_),
        isInitDataSent{false}
    {
        if(file.exists())
        {
            //If file exists rename it to "name (i).extension"
            //If extension == "log" -> do nothing (data will be appended to existing file)
            QString extension = name_.section('.', -1, -1);

            if (extension != "log")
            {
                int ctr = 1;
                QString fileName = name_.section('.', 0, -2);
                while (file.exists())
                {
                    name = fileName +  //file name without extension
                           " (" + QString::number(ctr) + ")." + //(i).
                           extension;

                    file.setFileName(path + '/' + ip + '/' + name);
                    ++ctr;
                }
            }
        }
        else
        {
            //Create file if it doesn't exist
            file.open(QIODevice::WriteOnly);
            file.close();
        }
    }

    File::File(const QString &input):
        Data("", 0, input.section('/',-1,-1)),
        file{input},
        path{""},
        isInitDataSent{false}
    {
        totalSize = file.size();
        remained = totalSize;
    }

    void File::read(const QByteArray &buffer)
    {
        //Open file and write to it
        if(file.open(QIODevice::Append))
        {
            file.write(buffer);
            file.close();
        }
        else
            qDebug("File cannot be read.");

        remained -= buffer.size();
    }

    QByteArray File::write()
    {
        if( ! file.exists())
        {
            qDebug("File cannot be written");
            //If file doesn't exist, set remained = 0 and delete it from queue
            remained = 0;
            return {};
        }

        //For the first time
        if ( ! isInitDataSent)
        {
            QByteArray array = name.toUtf8();       //file_name
            array.prepend(intToArr(array.size()));  //size(file_name)
            array.prepend(intToArr(totalSize));     //file size

            //Final array: size(data) + size(file_name) + file_name + ...data
            isInitDataSent = true;
            return (array);
        }

        //Open file
        if ( ! file.open(QIODevice::ReadOnly))
            return {};

        //Write file by chunks
        file.seek(totalSize-remained);
        QByteArray fileArray = file.read(32768*8);
        file.close();

        remained -= fileArray.size();
        return fileArray;
    }

    void File::emitSignal()
    {
        emit dataReceived(path + '/' + ip + '/' + name, ip);
    }
}
