#pragma once

#include <bitset>

#include <QDataStream>
#include <QDir>

#include "enums.h"

struct Config
{
    quint32 secondsScreen = 0; //Screenshot
    quint32 secondsLog = 0;
    std::bitset<int(Buttons::count)> mouseButtons;
    bool logRun = false;
};

//Read config
QDataStream & operator << (QDataStream& stream, Config& config);

//Write config
QDataStream & operator >> (QDataStream& stream, Config& config);

bool loadConfig(Config& config, const QString& defaultPath = QDir::currentPath() + "/config.cfg");
bool saveConfig(const Config& config, const QString& defaultPath = QDir::currentPath() + "/config.cfg");
