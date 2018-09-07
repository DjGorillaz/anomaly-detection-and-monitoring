#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include <bitset>

#include <QDir>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QMutex>
#include <QThread>

#include "enums.h"

namespace AnomalyDetection
{
    class MouseHook : public QObject
    {
        Q_OBJECT
    public:
        static MouseHook &instance();
        static LRESULT CALLBACK getMouse(int Code, WPARAM wParam, LPARAM lParam);
        void setParameters(const std::bitset<int(Buttons::count)>& buttons_, const int& timerSeconds);

        std::bitset<int(Buttons::count)> getButtons() const;
        void setPath(QString& path_);
        void setPrevName(QString&);
        QString& getPrevName();

    signals:
        void mouseClicked();
        void screenSaved(QString screen);

    private:
        HHOOK mHook;
        QString prevName;
        QString path;
        std::bitset<int(Buttons::count)> buttons;
        std::mutex mutex;
        std::unique_ptr<QTimer> timer;

        MouseHook(QObject *parent = nullptr);
        ~MouseHook() = default;

        void makeThreadForScreen();
    };

    class MakeScreen : public QObject
    {
        Q_OBJECT
    public:
        explicit MakeScreen(QObject* parent, std::mutex* m, const QString newPath = QDir::currentPath(), QString prevName = QString());
        ~MakeScreen() = default;

    public slots:
        void makeScreenshot();

    signals:
        void screenSaved(QString path);

    private:
        QString path;
        QString prevName;
        std::mutex* mutex;
        bool isNearlyTheSame(const QString& prevName, const QString& currName);
    };
}
