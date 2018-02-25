#include "config.h"

#include <QDebug>

//Read structure
QDataStream & operator << (QDataStream& stream, const Config& config)
{
    stream << config.secondsScreen
           << config.secondsLog
           << QString::fromStdString(config.mouseButtons.to_string())
           << config.bindEnter
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
           >> config.bindEnter
           >> config.logRun;
    config.mouseButtons = std::bitset<int(Buttons::count)>(buttons.toStdString());
    return stream;
}

Config::Config():
    secondsScreen(0),
    secondsLog(0),
    mouseButtons(""),
    bindEnter(false),
    logRun(false)
{   }

bool loadConfig(Config& config, const QString& defaultPath)
{
    QFile cfgFile(defaultPath);
    if ( cfgFile.exists() )
    {
        if ( !cfgFile.open(QIODevice::ReadOnly) )
            return false;
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
    if ( !cfgFile.open(QIODevice::WriteOnly) )
        return false;
    QDataStream cfgStream(&cfgFile);
    cfgStream << config;
    cfgFile.close();
    return true;
}

