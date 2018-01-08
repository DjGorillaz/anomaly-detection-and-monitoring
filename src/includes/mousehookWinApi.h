#ifndef MOUSEHOOKWINAPI_H
#define MOUSEHOOKWINAPI_H

#include <windows.h>
#include <gdiplus.h>

#include <QDir>
#include <QTimer>
#include <QTime>

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
    void setPrevName(QString&);
    QString& getPrevName();

signals:
    void mouseClicked();

private:
    HHOOK mHook;
    QString prevName;
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
    explicit MakeScreen(QObject* parent = 0, const QString& newPath = QDir::currentPath(), QString& prevName = QString());
    ~MakeScreen();

public slots:
    void makeScreenshot();

signals:
    void screenSaved(QString path);

private:
    QString path;
    QString prevName;
    bool isNearlyTheSame(const QString& prevName, const QString& currName);
};


#endif // MOUSEHOOKWINAPI_H
