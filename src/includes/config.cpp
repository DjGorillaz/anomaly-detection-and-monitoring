#include "config.h"

//Read structure
QDataStream & operator << (QDataStream& stream, const Config& config)
{
    stream << config.secondsScreen
           << config.secondsLog
           << QString::fromStdString(config.mouseButtons.to_string())
           << config.logRun;
    return stream;
}

//Write structure
QDataStream & operator >> (QDataStream& stream, Config& config)
{
    QString buttons;
    stream >> config.secondsScreen
           >> config.secondsLog
           >> buttons
           >> config.logRun;
    config.mouseButtons = std::bitset<int(Buttons::count)>(buttons.toStdString());
    return stream;
}

bool loadConfig(Config& config, const QString& defaultPath)
{
    QFile cfgFile(defaultPath);
    if (cfgFile.exists() && cfgFile.open(QIODevice::ReadOnly))
    {
        QDataStream cfgStream(&cfgFile);
        cfgStream >> config;
        cfgFile.close();
        return true;
    }
    else
        return false;
}

bool saveConfig(const Config& config, const QString& defaultPath)
{
    QFile cfgFile(defaultPath);
    if (cfgFile.open(QIODevice::WriteOnly))
    {
        QDataStream cfgStream(&cfgFile);
        cfgStream << config;
        cfgFile.close();
        return true;
    }
    else
        return false;
}

