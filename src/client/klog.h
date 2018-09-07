#pragma once

#include <memory>
#include <windows.h>
#include <Psapi.h>

#include <QTimer>
#include <QFile>
#include <QDateTime>
#include <QDebug>

namespace AnomalyDetection
{
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
        std::unique_ptr<QTimer> timer;
        std::unique_ptr<QFile> logFile;
        bool isWorking;
        QString currProcess;
        QString path;

        Klog(QObject *parent = nullptr);
        ~Klog() = default;
    };
}
