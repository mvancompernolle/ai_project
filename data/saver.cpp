#include "saver.h"

/*! \file
    \ingroup GlutMaster

    Based on the Generic Screen Saver Manager
    by James Ward (espresso@crema.co.uk)
*/

#include <glutm/config.h>

#if defined(GLUTM_SAVER)

#if !defined(WIN32)
#error GLUTM_SAVER is Windows only!
#endif

#include <windows.h>
#include <scrnsave.h>

#include <glt/gl.h>
#include <glt/glu.h>
#include <glt/error.h>

#include <glutm/window.h>
#include <glutm/master.h>
#include <glutm/main.h>

#include <vector>
#include <string>
#include <cassert>
using namespace std;

////////////////////////////

static GlutWindow *_instance = NULL;    // singleton screensaver window instance

static HDC       _hdc     = 0;  // device context   (GL child)
static HGLRC     _hrc     = 0;  // resource context (GL child)
static HWND      _hWnd    = 0;  // main window handle
static int       _width   = 0;  // width  (pixels)
static int       _height  = 0;  // height (pixels)

// TODO - Schedule more than one timer callback properly
static int       _miscData = 0;

const UINT idleID = 1;
const UINT tickID = 2;
const UINT miscID = 3;

////////////////////////////

VOID CALLBACK  TimerIdleProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime);
VOID CALLBACK  TimerTickProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime);
VOID CALLBACK  TimerMiscProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime);
LRESULT WINAPI ScreenSaverProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL    WINAPI ScreenSaverConfigureDialog(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL    WINAPI RegisterDialogClasses(HANDLE hInst);

////////////////////////////

//
//
//

VOID CALLBACK TimerIdleProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime)
{
    if (_instance)
        _instance->OnIdle();
}

VOID CALLBACK TimerTickProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime)
{
    if (_instance)
        _instance->OnTick();
}

VOID CALLBACK TimerMiscProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime)
{
    KillTimer(hwnd,miscID);
    if (_instance)
        _instance->OnTimer(_miscData);
}

//
//
//

LRESULT WINAPI ScreenSaverProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    //

    switch (message)
    {
        case WM_CREATE:
            {
                _hWnd = hWnd;

                // Do GLUT initialisation, create window instance

                assert(!_instance);
                std::vector<std::string> arg;
                GlutMain(arg);
                assert(_instance);

                // Get the dimensions

                RECT rect;
                GetClientRect( hWnd, &rect );
                _width = rect.right;
                _height = rect.bottom;

                // Create OpenGL context

                PIXELFORMATDESCRIPTOR pfd;
                memset(&pfd, 0, sizeof(pfd));
                pfd.nSize = sizeof(pfd);
                pfd.nVersion = 1;
                pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
                pfd.iPixelType = PFD_TYPE_RGBA;
                pfd.cColorBits = 24;
                pfd.cDepthBits = 32;
                pfd.iLayerType = PFD_MAIN_PLANE;

                // Select closest pixel format

                _hdc = GetDC(_hWnd);
                int format = ChoosePixelFormat(_hdc, &pfd);
                if (SetPixelFormat(_hdc, format, &pfd))
                {
                    // Create rendering context
                    _hrc = wglCreateContext(_hdc);

                    if (_hrc)
                        wglMakeCurrent(_hdc,_hrc);
                    else
                        gltError("Failed to create OpenGL context.");
                }

                // High-resolution timer

                timeBeginPeriod(1);

                // Do some sanity checking

                if (!_hdc || !_hrc)
                {
                    assert(_hdc);
                    assert(_hrc);

                    PostMessage(hWnd,WM_CLOSE,0,0);
                    return 0;
                }

                SetCursor(NULL);

                if (_instance)
                {
                    _instance->OnOpen();
                    _instance->OnReshape(_width,_height);
                }
            }
            return TRUE;

        case WM_PAINT:
            if (hWnd && _hdc && _hrc)
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                if (wglMakeCurrent(_hdc,_hrc) && _instance)
                {
                    _instance->OnPreDisplay();
                    _instance->OnDisplay();
                    _instance->OnPostDisplay();
                }
                EndPaint(hWnd, &ps);
            }
            return TRUE;

        case WM_DESTROY:
            KillTimer(_hWnd,idleID);
            KillTimer(_hWnd,tickID);
            KillTimer(_hWnd,miscID);

            if (hWnd && _hdc && _hrc)
                wglMakeCurrent(_hdc,_hrc);

            if (_instance)
            {
                _instance->OnClose();
                delete _instance;
                _instance = NULL;
            }

            if (hWnd && _hdc && _hrc)
            {
                wglMakeCurrent(NULL,NULL);
                wglDeleteContext(_hrc);
                ReleaseDC(_hWnd,_hdc);
                _hdc    = 0;
                _hrc    = 0;
            }

            timeEndPeriod(1);
            SetCursor(LoadCursor(NULL,IDC_ARROW));
            PostQuitMessage(0);
            return TRUE;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_KEYDOWN:
            PostMessage(hWnd,WM_CLOSE,0,0);
            return TRUE;

        case WM_ERASEBKGND:
            return TRUE;
    }

    return DefScreenSaverProc(hWnd,message,wParam,lParam);
}

//
//
//

BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_CLOSE:
            EndDialog(hDlg, TRUE);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg,TRUE);
                    return TRUE;
            }
    }

    return FALSE;
}

//
//
//

BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
    // Since we do not register any special window class
    // for the configuration dialog box, we must return TRUE

    return TRUE;
}

///////////////////////////// GlutMaster ///////////////////////////////////////

// Here are the necessary parts of
// the GlutMaster interface not using
// the GLUT library.

GlutWindow *GlutMaster::currentWindow()
{
    return _instance;
}

void GlutMaster::openWindow(GlutWindow *glutWindow)
{
    _instance = glutWindow;
    glutWindow->_windowID = 1;
}

void GlutMaster::closeWindow(GlutWindow *glutWindow)
{
    _instance = NULL;
    glutWindow->_windowID = -1;
}

void GlutMaster::init(int argc,char *argv[])
{
    if (!_glutInit)
    {
        // Initialise Open Inventor, if supported

        #ifdef GLUTM_OPEN_INVENTOR
        GlutWindowInventor::initOpenInventor();
        #endif
    }
    _glutInit = true;
}

void GlutMaster::mainLoop()
{
}

void GlutMaster::positionWindow(GlutWindow *glutWindow,int x,int y)              {}
void GlutMaster::reshapeWindow(GlutWindow *glutWindow,int width,int height)      {}
void GlutMaster::setCursor(GlutWindow *glutWindow,int cursor)                    {}
void GlutMaster::updateModifiers(GlutWindow *window)                             {}

void GlutMaster::setIdle(GlutTimer *target,bool idle)
{
    assert(target);
    if (!target)
        return;

    target->_idle = idle;

    if (idle)
        ::SetTimer(_hWnd,idleID,1,TimerIdleProc);
    else
        ::KillTimer(_hWnd,idleID);
}

void GlutMaster::setTick(GlutTimer *target,unsigned int msec)
{
    assert(target);
    if (!target)
        return;

    target->_tick = msec;

    if (msec>0)
        ::SetTimer(_hWnd,tickID,msec,TimerTickProc);
    else
        ::KillTimer(_hWnd,tickID);
}

void GlutMaster::setTimer(GlutTimer *target,unsigned int msec,int val)
{
    assert(target);
    if (!target)
        return;

    _miscData = val;

    if (msec>0)
        ::SetTimer(_hWnd,miscID,msec,TimerMiscProc);
    else
        ::KillTimer(_hWnd,miscID);
}

void
GlutMaster::postRedisplay(GlutWindow *glutWindow)
{
    assert(_hWnd);
    if (_hWnd)
        ::InvalidateRect(_hWnd, NULL, FALSE);
}

void
GlutMaster::swapBuffers(GlutWindow *glutWindow)
{
    assert(_hdc);
    if (_hdc)
        ::SwapBuffers(_hdc);
}

// Get GLUT info

int GlutMaster::get(const GlutWindow *window,int info) { return 0; }

int GlutMaster::getScreenWidth()    { return _width;  }
int GlutMaster::getScreenHeight()   { return _height; }
int GlutMaster::getScreenWidthMm()  { return 0;       }
int GlutMaster::getScreenHeightMm() { return 0;       }
int GlutMaster::getElapsedTime()    { return 0;       }

void GlutMaster::fullScreen(GlutWindow *window) {}
void GlutMaster::gameMode  (GlutWindow *window) {}

#endif
