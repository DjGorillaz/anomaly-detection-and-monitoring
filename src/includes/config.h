#pragma once

#include <QDataStream>
#include <QDir>

struct Config
{
    //Screenshot
    quint32 secondsScreen;
    quint32 secondsLog;
    int mouseButtons;
    bool bindEnter;
    bool logRun;
    Config();
};

//Read config
QDataStream & operator << (QDataStream& stream, Config& config);

//Write config
QDataStream & operator >> (QDataStream& stream, Config& config);

bool loadConfig(Config& config, const QString& defaultPath = QDir::currentPath() + "/config.cfg");
bool saveConfig(const Config& config, const QString& defaultPath = QDir::currentPath() + "/config.cfg");
