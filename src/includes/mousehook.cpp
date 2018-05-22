#include "mousehook.h"

#include "gdiplus.h"

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   https://msdn.microsoft.com/en-us/library/ms533843(v=vs.85).aspx

   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

MouseHook &MouseHook::instance()
{
    static MouseHook _instance;
    return _instance;
}

MouseHook::MouseHook(QObject *parent) : QObject(parent), prevName(QString())
{
    timer = std::make_unique<QTimer>(this);
    connect(timer.get(), &QTimer::timeout, this, &MouseHook::mouseClicked);

    HINSTANCE hInstance = GetModuleHandle(NULL);

    //Register hook
    mHook = SetWindowsHookEx (WH_MOUSE_LL,  //monitor mouse input events
                              getMouse,     //pointer to hook procedure
                              hInstance,    //handle to an instance
                              0);           //thread id
    if (mHook == NULL)
        qDebug() << "Mouse hook installation error";

    buttons.reset();
}

void MouseHook::setParameters(const std::bitset<int(Buttons::count)>& buttons_, const int& seconds)
{
    buttons = buttons_;

    if (seconds == 0)
        timer->stop();
    else
        timer->start(seconds*1000);
}

LRESULT CALLBACK MouseHook::getMouse(int Code, WPARAM wParam, LPARAM lParam)
{
    MOUSEHOOKSTRUCT* pMouseStruct = reinterpret_cast<MOUSEHOOKSTRUCT*> (lParam);

    if (pMouseStruct != nullptr)
    {
        //check event type
        switch (wParam) {
        case WM_LBUTTONDOWN:
            if (instance().getButtons()[int(Buttons::left)]) emit instance().mouseClicked();
            break;
        case WM_RBUTTONDOWN:
            if (instance().getButtons()[int(Buttons::right)]) emit instance().mouseClicked();
            break;
        case WM_MBUTTONDOWN:
            if (instance().getButtons()[int(Buttons::middle)]) emit instance().mouseClicked();
            break;
        case WM_MOUSEWHEEL:
            if (instance().getButtons()[int(Buttons::wheel)]) emit instance().mouseClicked();
            break;
        default:
            break;
        }
        /*
         * Other events:
         * WM_MOUSEMOVE
         * WM_LBUTTONUP
         * WM_RBUTTONUP
         * WM_MBUTTONUP
        */
    }

    //get hook event back
    return CallNextHookEx(NULL, Code, wParam, lParam);
}

std::bitset<int(Buttons::count)> MouseHook::getButtons() const
{
    return buttons;
}

void MouseHook::setPrevName(QString& name)
{
    prevName = name;
}

QString& MouseHook::getPrevName()
{
    return prevName;
}

MakeScreen::MakeScreen(QObject* parent, const QString newPath, QString prevName_): QObject(parent), path(newPath), prevName(prevName_)
{   }

MakeScreen::~MakeScreen()
{   }

bool MakeScreen::isNearlyTheSame(const QString& prevName, const QString& currName)
{
    long long currSize = QFile(path + currName).size();
    long long prevSize = QFile(path + prevName).size();
    double diff = static_cast<double>(currSize - prevSize) / currSize;
    return abs(diff) < 0.001 ? true : false;
}

void MakeScreen::makeScreenshot()
{
    //path with another slash
    QString pathForGDI = path;
    pathForGDI.replace('/', '\\');

    QString name = QDateTime::currentDateTime().toString("dd.MM.yyyy hh-mm-ss") + ".jpg";

    //Don't make new screenshot if frequency > 1/sec
    if (QFile::exists(path + name))
        return;

    HDC hScreen = GetDC(NULL);
    HDC hMem = CreateCompatibleDC(hScreen);

    //Get coordinates
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width =  GetSystemMetrics(SM_CXMAXTRACK);
    int height =  GetSystemMetrics(SM_CYMAXTRACK);

    //create hBitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    HGDIOBJ  hOldBitmap = SelectObject(hMem, hBitmap);
    if ( !BitBlt(hMem, 0, 0, width, height, hScreen, x, y, SRCCOPY) )
        return;
    hBitmap = static_cast<HBITMAP> ( SelectObject(hMem, hOldBitmap) );

    //save hBitmap using GDI
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    if ( Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Ok )
    {
        Gdiplus::Bitmap* image = new Gdiplus::Bitmap(hBitmap, NULL);
        CLSID jpegClsId;
        GetEncoderClsid(L"image/jpeg", &jpegClsId);
        std::wstring wstr = pathForGDI.toStdWString() + name.toStdWString();
        image->Save(wstr.c_str(), &jpegClsId, NULL);
        delete image;
        //Delete new screen if it's the same
        if (isNearlyTheSame(prevName, name)) QFile::remove(path + name);
        else emit screenSaved(name);
    }
    //Release memory
    DeleteDC(hMem);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);
}
