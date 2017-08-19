#include "mousehookWinApi.h"
#include <QDebug>

//Look
//https://msdn.microsoft.com/en-us/library/ms533843(v=vs.85).aspx
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
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

MouseHook::MouseHook(QObject *parent) : QObject(parent)
{
    QDir dir;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MouseHook::mouseClicked);
    //MakeScreen* scr = new MakeScreen(this);
    //scr->deleteLater();

    HINSTANCE hInstance = GetModuleHandle(NULL);

    //Register hook
    mHook = SetWindowsHookEx (WH_MOUSE_LL,  //monitor mouse input events
                              getMouse,     //pointer to hook procedure
                              hInstance,    //handle to an instance
                              0);           //thread id
    if (mHook == NULL)
        qDebug() << "Mouse hook installation error";

    //Set initial parameters
    LMB = false;
    RMB = false;
    MMB = false;
    MWH = false;
}

void MouseHook::setParameters(const int& buttons, const int& seconds)
{
    LMB = (buttons & 0x0008) ? 1 : 0;
    RMB = (buttons & 0x0004) ? 1 : 0;
    MMB = (buttons & 0x0002) ? 1 : 0;
    MWH = (buttons & 0x0001) ? 1 : 0;

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
            if (instance().getLMB()) emit instance().mouseClicked();
            break;
        case WM_RBUTTONDOWN:
            if (instance().getRMB()) emit instance().mouseClicked();
            break;
        case WM_MBUTTONDOWN:
            if (instance().getMMB()) emit instance().mouseClicked();
            break;
        case WM_MOUSEWHEEL:
            if (instance().getMWH()) emit instance().mouseClicked();
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

bool MouseHook::getMWH() const
{
    return MWH;
}

bool MouseHook::getMMB() const
{
    return MMB;
}

bool MouseHook::getRMB() const
{
    return RMB;
}

bool MouseHook::getLMB() const
{
    return LMB;
}

MakeScreen::MakeScreen(QObject* parent, const QString& newPath): QObject(parent), path(newPath)
{

}

MakeScreen::~MakeScreen()
{

}

void MakeScreen::makeScreenshot()
{
    //path with another slash
    QString pathForGDI = path;
    pathForGDI.replace('/', '\\');

    QString name = QDateTime::currentDateTime().toString("hh-mm-ss dd.MM.yyyy") + ".jpg";

    //Don't make new screenshot if frequency > 1/sec
    if (QFile::exists(path + '/' + name))
        return;

    //DPI support
    //SetProcessDPIAware();
    //windows 10 only
    //SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

    HDC hScreen = GetDC(NULL);
    HDC hMem = CreateCompatibleDC(hScreen);

    //Current monitor
    //int width = GetDeviceCaps(hScreen, HORZRES);
    //int height = GetDeviceCaps(hScreen, VERTRES);
    //int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    //int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

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
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    if ( GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Ok )
    {
        Bitmap *image = new Bitmap(hBitmap, NULL);
        CLSID jpegClsId;
        GetEncoderClsid(L"image/jpeg", &jpegClsId);
        std::wstring wstr = pathForGDI.toStdWString() + L"\\" + name.toStdWString();
        image->Save(wstr.c_str(), &jpegClsId, NULL);
        delete image;
    }

    //Free memory
    DeleteDC(hMem);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);
    emit screenSaved(path + '/' + name);
}




