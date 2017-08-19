#ifndef MOUSEHOOKWINAPI_H
#define MOUSEHOOKWINAPI_H

#include <QObject>
#include <QDir>
#include <QTimer>
#include <QTime>

#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

class MouseHook : public QObject
{
    Q_OBJECT
public:
    static MouseHook &instance();
    static LRESULT CALLBACK getMouse(int Code, WPARAM wParam, LPARAM lParam);
    void setParameters(const int& buttons, const int& timerSeconds);

    bool getLMB() const;
    bool getRMB() const;
    bool getMMB() const;
    bool getMWH() const;

signals:
    void mouseClicked();

private:
    HHOOK mHook;
    bool LMB;
    bool RMB;
    bool MMB;
    bool MWH;
    QTimer* timer;

    MouseHook(QObject *parent = nullptr);
    ~MouseHook() {}
};

class MakeScreen : public QObject
{
    Q_OBJECT
public:
    explicit MakeScreen(QObject* parent = 0, const QString& newPath = QDir::currentPath());
    ~MakeScreen();
    static void setPath();

public slots:
    void makeScreenshot();

signals:
    void screenSaved(QString path);

private:
    QString path;
};


#endif // MOUSEHOOKWINAPI_H
