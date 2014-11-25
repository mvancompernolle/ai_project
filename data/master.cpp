#include "master.h"

/*! \file
    \ingroup GlutMaster
*/

#include <glutm/window.h>
#include <glutm/timer.h>
#include <glutm/winiv.h>
#include <glutm/config.h>

#ifdef _MSC_VER
#pragma warning(disable : 4786)     // Lengthy STL symbols
#endif

#include <glutm/glut.h>

#include <map>
#include <list>
#include <vector>
#include <cassert>
#include <algorithm>
using namespace std;

//#define GLUTM_DEBUG

#ifdef GLUTM_DEBUG
#include <iostream>
#include <iomanip>
#define DEBUG_TITLE(x) setw(27) << setiosflags(ios::left) << (x) << ' '
#endif

///////////////////////////// GlutWindows ///////////////////////////////////////
//
// Keep a database of Glut ID's and GlutWindow pointers
//

typedef list<GlutWindow *>                             GlutWindowList;
typedef map<int,GlutWindowList,less<int> >             GlutWindowDatabase;

GlutWindowDatabase _windows;
GlutWindowList     _toOpen;
GlutWindowList     _toClose;

///////////////////////////// GlutMaster ///////////////////////////////////////

bool GlutMaster::_glutInit = false;

GlutMaster::GlutMaster()
{
}

GlutMaster::~GlutMaster()
{
}

bool GlutMaster::active() { return _toOpen.size()!=0 || _toClose.size()!=0; }

//
// GLUT specific
//

#if !defined(GLUTM_SAVER)

GlutWindow *
GlutMaster::currentWindow()
{
    assert(_glutInit);

    // Query GLUT for the current window ID
    const int id = glutGetWindow();

    // Look in the window database for the owner of GLUT ID
    GlutWindowDatabase::const_iterator i = _windows.find(id);

    if (i==_windows.end())
        return NULL;
    else
    {
        const GlutWindowList &list = i->second;

        // There should be at least one owner

        assert(list.size()>0);
        if (list.size()<=0)
            return NULL;

        // The front-most owner is current
        return list.front();
    }
}

void
GlutMaster::openWindow(GlutWindow *glutWindow)
{
    if (!glutWindow)
        return;

    // GLUT ID for current GlutWindow

    int &id = glutWindow->_windowID;

    // Create a new GLUT window, if necessary

    if (id==-1)
    {
        GlutMaster::init();
        glutInitDisplayMode(glutWindow->_displayMode);

        if (glutWindow->_gameMode)
        {
            // Create game-mode window
            id = glutEnterGameMode();
        }
        else
        {
            // Create a desktop window
            glutInitWindowSize(glutWindow->_width,glutWindow->_height);
            glutInitWindowPosition(glutWindow->_x,glutWindow->_y);
            id = glutCreateWindow(glutWindow->_title.c_str());
        }
    }

    //
    // Do GlutMaster callback registration and so on...
    //

    assert(_glutInit);

    GlutWindowDatabase::iterator i = _windows.find(id);

    //
    // If the GLUT Window is new add it to the database
    //

    if (i==_windows.end())
    {
        // Add the GlutWindow to a global database so that
        // GlutMaster can send route events appropriately.

        GlutWindowList list;
        list.push_back(glutWindow);
        _windows.insert(GlutWindowDatabase::value_type(id,list));

        // Register GlutMaster callback functions for all the
        // GLUT event handlers.  This needs to be done for each new
        // GLUT window.

        glutDisplayFunc      (OnDisplay);
        glutIdleFunc         (NULL);
        glutKeyboardFunc     (OnKeyboard);
        glutKeyboardUpFunc   (OnKeyboardUp);
        glutSpecialFunc      (OnSpecial);
        glutSpecialUpFunc    (OnSpecialUp);
        glutMotionFunc       (OnMotion);
        glutMouseFunc        (OnMouse);
        glutPassiveMotionFunc(OnPassiveMotion);
        glutEntryFunc        (OnEntry);
        glutReshapeFunc      (OnReshape);
        glutVisibilityFunc   (OnVisibility);
    }

    //
    // If the GLUT window exists already, put this GlutWindow
    // at the front of the list.
    //

    else
    {
        GlutWindowList &list = i->second;

        // We don't expect the list to be empty
        assert(list.size()>0);

        // We need to know which GlutWindow was previously
        // front-most
        GlutWindow *previous = list.front();

        if (glutWindow!=previous)
        {
            // Close the GlutWindow previously front-most
            glutSetWindow(id);
            previous->OnClose();

            // Insert the current GlutWindow at the front
            list.remove(glutWindow);
            list.push_front(glutWindow);
        }
    }

    // Trigger OnOpen() GlutWindow callback in the near future,
    // we can't do it until GlutWindow is finished constructing
    // and initialising.

    _toOpen.push_back(glutWindow);
    glutTimerFunc(0,GlutMaster::CheckOnOpen,0);
}

void
GlutMaster::closeWindow(GlutWindow *glutWindow)
{
    // We can't close the window properly until we
    // know the the window isn't in the current call
    // stack.  Then it's time to call OnClose, destroy
    // OpenGL context, and so on...

    _toClose.push_back(glutWindow);
    glutTimerFunc(0,GlutMaster::CheckOnClose,0);
}

void
GlutMaster::init(int argc,char *argv[])
{
    // glutInit should only be called once,
    // therefore we use a global variable to
    // record the fact that GLUT has already
    // been initialised

    if (!_glutInit)
    {
        glutInit(&argc,argv);

        // Initialise Open Inventor,
        // if necessary

        #ifdef GLUTM_OPEN_INVENTOR
        GlutWindowInventor::initOpenInventor();
        #endif
    }

    // We know that GLUT has been initialised,
    // we don't want it to happen again...

    _glutInit = true;
}

void
GlutMaster::init()
{
    // Fake a trivial command-line
    int argc = 1;
    char *argv = "";
    init(argc,&argv);
}

void
GlutMaster::mainLoop()
{
    // The GlutMaster main loop simply
    // initialises GLUT and enters the
    // GLUT event loop.

    if (_windows.size())
    {
        init();
        glutMainLoop();
    }
}

void
GlutMaster::positionWindow(GlutWindow *window,int x,int y)
{
    assert(window);
    if (!window)
        return;

    const int id = window->_windowID;

    if (id!=-1)
    {
        glutSetWindow(id);
        glutPositionWindow(x,y);
        window->_x = x;
        window->_y = y;
    }
}

// glutReshapeWindow

void
GlutMaster::reshapeWindow(GlutWindow *window,int width,int height)
{
    assert(window);
    if (!window)
        return;

    const int id = window->_windowID;

    if (id!=-1)
    {
        glutSetWindow(id);
        glutReshapeWindow(width,height);
        window->_width  = width;
        window->_height = height;
    }
}

void
GlutMaster::setIdle(GlutTimer *target,bool idle)
{
    assert(target);
    if (!target)
        return;

    list<GlutTimer *> &idleList = GlutTimer::_idleList;

    if (find(idleList.begin(),idleList.end(),target)==idleList.end() && idle)
    {
        // Start the GLUT idle function, if necessary

        idleList.push_back(target);
        glutIdleFunc(GlutTimer::idleCallback);
    }
    else
        if (find(idleList.begin(),idleList.end(),target)!=idleList.end() && !idle)
        {
            idleList.remove(target);
            if (idleList.size()==0)
                glutIdleFunc(NULL);
        }

    target->_idle = idle;
}

void
GlutMaster::setTick(GlutTimer *target,unsigned int msec)
{
    assert(target);
    if (!target)
        return;

    target->_tick = msec;

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::setTick");
    cout << target << ' ';
    cout << msec << " msec" << endl;
    #endif

    if (msec!=0 && !target->_tickPending)
    {
        int index = GlutTimer::Slot::add(target,0,true);
        glutTimerFunc(msec,GlutTimer::timerCallback,index);
        target->_tickPending = true;
    }
}

void
GlutMaster::setTimer(GlutTimer *target,unsigned int msec,int val)
{
    assert(target);
    if (!target)
        return;

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::setTimer");
    cout << target << ' ';
    cout << val    << ' ';
    cout << msec   << " msec" << endl;
    #endif

    // index is used later to lookup taget and val

    int index = GlutTimer::Slot::add(target,val);
    glutTimerFunc(msec,GlutTimer::timerCallback,index);
}

void
GlutMaster::setCursor(GlutWindow *window,int cursor)
{
    assert(window);
    if (!window)
        return;

    const int id = window->_windowID;

    if (id!=-1)
    {
        glutSetWindow(id);
        glutSetCursor(cursor);
    }
}

void
GlutMaster::postRedisplay(GlutWindow *window)
{
    assert(window);
    if (!window)
        return;

    const int id = window->_windowID;

    if (id!=-1)
    {
        glutSetWindow(id);
        glutPostRedisplay();
    }
}

void
GlutMaster::swapBuffers(GlutWindow *window)
{
    assert(window);
    if (!window)
        return;

    const int id = window->_windowID;

    if (id!=-1)
    {
        glutSetWindow(id);
        glutSwapBuffers();
    }
}

int
GlutMaster::get(const GlutWindow *window,int info)
{
    assert(window);
    if (!window)
        return 0;

    const int id = window->_windowID;

    assert(id!=-1);

    if (id!=-1)
    {
        glutSetWindow(id);
        return glutGet(info);
    }

    return 0;
}

void
GlutMaster::fullScreen(GlutWindow *window)
{
    assert(window);

    if (window)
    {
        const int id = window->_windowID;

        assert(id!=-1);

        if (id!=-1)
        {
            glutSetWindow(id);
            glutFullScreen();
        }
    }
}

void
GlutMaster::gameMode(GlutWindow *window)
{
}

void
GlutMaster::updateModifiers(GlutWindow *window)
{
    assert(window);
    if (window)
    {
        window->_shiftModifier = (glutGetModifiers()&GLUT_ACTIVE_SHIFT)!=0;
        window->_ctrlModifier  = (glutGetModifiers()&GLUT_ACTIVE_CTRL) !=0;
        window->_altModifier   = (glutGetModifiers()&GLUT_ACTIVE_ALT)  !=0;
    }
}

//
// Get GLUT info
//

int GlutMaster::getScreenWidth()    { return glutGet(GLUT_SCREEN_WIDTH);     }
int GlutMaster::getScreenHeight()   { return glutGet(GLUT_SCREEN_HEIGHT);    }
int GlutMaster::getScreenWidthMm()  { return glutGet(GLUT_SCREEN_WIDTH_MM);  }
int GlutMaster::getScreenHeightMm() { return glutGet(GLUT_SCREEN_HEIGHT_MM); }
int GlutMaster::getElapsedTime()    { return glutGet(GLUT_ELAPSED_TIME);     }

///////////////////////////////////////////////////////////////////////////////////////

void
GlutMaster::CheckShutdown(int val)
{
    assert(_glutInit);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::CheckShutdown") << endl;
    #endif

    // If all the windows are closed, it's time
    // to shutdown the application...

    if (_windows.size()==0)
        exit(1);
}

void
GlutMaster::CheckOnOpen(int val)
{
    assert(_glutInit);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::CheckOnOpen") << endl;
    #endif

    // Open all waiting windows...

    while (_toOpen.size())
    {
        GlutWindow *window = _toOpen.front();
        const int id       = window->_windowID;

        #ifdef GLUTM_DEBUG
        cout << DEBUG_TITLE("GlutMaster::OnOpen") << window << endl;
        #endif

        glutSetWindow(id);
        window->OnOpen();
        setTick(window,window->_tick);
        _toOpen.pop_front();
    }
}

void
GlutMaster::CheckOnClose(int val)
{
    assert(_glutInit);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::CheckOnClose") << endl;
    #endif

    // Close all windows waiting for OnClose...

    while (_toClose.size())
    {
        GlutWindow *window  = _toClose.front();
        const int  id       = window->_windowID;
        const bool gameMode = window->_gameMode;

        #ifdef GLUTM_DEBUG
        cout << DEBUG_TITLE("GlutMaster::OnClose") << window << endl;
        #endif

        // Set context and notify window

        glutSetWindow(id);
        window->OnClose();
        _toClose.pop_front();

        // Find the window in database

        GlutWindowDatabase::iterator i = _windows.find(id);
        assert(i!=_windows.end());

        // Remove it from window list

        GlutWindowList &list = i->second;
        assert(find(list.begin(),list.end(),window)!=list.end());

        // Call window destructor

        delete window;

        // If the list will become empty
        //   remove GLUT window from database
        //   destroy GLUT window

        if (list.size()==1)
        {
            _windows.erase(i);
            if (gameMode)
                glutLeaveGameMode();
            else
                glutDestroyWindow(id);
        }
        else
        {
            // Check if another window needs to be woken

            if (list.front()==window)
            {
                list.pop_front();
                _toOpen.push_back(list.front());
            }
            else
                list.remove(window);
        }

        // Prevent the window from receiving OnOpen event

        _toOpen.remove(window);

        // Schedule a check for GLUT shutdown condition

        glutTimerFunc(0,GlutMaster::CheckShutdown,0);
    }
}

////////////////////////////////////////////////////////////////////////////////

//
// Static callbacks for receiving events from Glut
//

void
GlutMaster::OnDisplay()
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnDisplay") << window << endl;
    #endif

    if (window)
    {
        window->OnPreDisplay();
        window->OnDisplay();
        window->OnPostDisplay();
    }
}

void
GlutMaster::OnKeyboard(unsigned char key, int x, int y)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnKeyboard");
    cout << window << ' ' << key << ' ' << x << ',' << y << endl;
    #endif

    if (window)
    {
        updateModifiers(window);
        window->OnKeyboard(key, x, y);
    }
}

void
GlutMaster::OnKeyboardUp(unsigned char key, int x, int y)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnKeyboardUp");
    cout << window << ' ' << key << ' ' << x << ',' << y << endl;
    #endif

    if (window)
    {
        updateModifiers(window);
        window->OnKeyboardUp(key, x, y);
    }

}

void
GlutMaster::OnSpecial(int key, int x, int y)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnSpecial ");
    cout << window << ' ' << key << ' ' << x << ',' << y << endl;
    #endif

    if (window)
    {
        updateModifiers(window);
        window->OnSpecial(key, x, y);
    }
}

void
GlutMaster::OnSpecialUp(int key, int x, int y)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnSpecialUp");
    cout << window << ' ' << key << ' ' << x << ',' << y << endl;
    #endif

    if (window)
    {
        updateModifiers(window);
        window->OnSpecialUp(key, x, y);
    }
}

void
GlutMaster::OnMenu(int value)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnMenu");
    cout << window << ' ' << value << endl;
    #endif

    if (window)
        window->OnMenu(value);
}

void
GlutMaster::OnMotion(int x, int y)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnMotion");
    cout << window << ' ' << x << ',' << y << endl;
    #endif

    if (window)
    {
        // OnMotion events are limited to the tick
        // frequency of the GlutWindow

        if (window->_tick)
        {
            window->_mouseEvents = true;
            window->_mouseX = x;
            window->_mouseY = y;
        }
        else
            window->OnMotion(x, y);
    }
}

void
GlutMaster::OnMouse(int button, int state, int x, int y)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnMouse");
    cout << window << ' ' << button << ' ' << state << ' ' << x << ',' << y << endl;
    #endif

    if (window)
    {
        updateModifiers(window);
        window->OnMouse(button, state, x, y);
    }
}

void
GlutMaster::OnPassiveMotion(int x, int y)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnPassiveMotion");
    cout << window << ' ' << x << ',' << y << endl;
    #endif

    if (window)
        window->OnPassiveMotion(x, y);
}

void
GlutMaster::OnEntry(int state)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnEntry");
    cout << window << (state==GLUT_ENTERED ? " entered" : " left") << endl;
    #endif

    // glutDestroyWindow triggers OnEntry event,
    // but if window has been forgotten, the
    // currentWindow pointer will be NULL
    // TODO: Does it make sense to handle this
    // before destroying GLUT window and GlutWindow?

    if (window)
        window->OnEntry(state);
}

void
GlutMaster::OnReshape(int w, int h)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnReshape");
    cout << window << ' ' << w << ',' << h << endl;
    #endif

    if (window)
        window->OnReshape(w,h);
}

void
GlutMaster::OnVisibility(int visible)
{
    GlutWindow *window = currentWindow();

    assert(_glutInit);
    assert(window);

    #ifdef GLUTM_DEBUG
    cout << DEBUG_TITLE("GlutMaster::OnVisibility");
    cout << window << ' ' << visible << endl;
    #endif

    if (window)
        window->OnVisibility(visible);
}

#endif
