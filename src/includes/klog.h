#ifndef KLOG_H
#define KLOG_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QDateTime>
#include <QDebug>

#include <windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <string.h>
#include <iostream>

class Klog : public QObject
{
    Q_OBJECT
public:
    static Klog &instance();
    static LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    void setParameters(const bool& isWorking, const int& timerSeconds);
    void setPath(const QString& newPath);

signals:
    void timerIsUp();

private:
    QTimer* timer;
    QFile* logFile;
    bool isWorking;
    QString currProcess;
    QString path;


    Klog(QObject *parent = nullptr);
    ~Klog() {}
};

#endif // KLOG_H
