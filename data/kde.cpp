#include "kde.h"

#if defined(GLUTM_SAVER) && defined(GLT_UNIX)

#define LONG64


//#include <qslider.h>
//#include <qlayout.h>
//
//
//#include <krandomsequence.h>
//

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>

#include <qcolor.h>

#include <X11/Intrinsic.h>
#include <GL/glx.h>

#include <cmath>

#include <glt/gl.h>
#include <glt/glu.h>


Display *dsp = NULL;
int screen = 0;
bool mono = false;


void
drawmorph3d(Window window)
{
/*
    Display    *display = dsp;

    glXMakeCurrent(display, window, mp->glx_context);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();

    ..

    glFlush();
    glXSwapBuffers(display, window);
*/
}

static void
reshape(int width, int height)
{
/*
    glViewport(0, 0, mp->WindW = (GLint) width, mp->WindH = (GLint) height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
*/
}

#define MAXSCREENS 1

static XVisualInfo *glVis[MAXSCREENS];

int
getVisual(XVisualInfo * wantVis, int visual_count)
{
    Display    *display = dsp;
    static int  first;
    int i;

        if (first) {
                for (screen = 0; screen < MAXSCREENS; screen++)
                        glVis[screen] = NULL;
        }

        if (!glVis[screen]) {
                if (mono) {
                        /* Monochrome display - use color index mode */
                        int         attribList[] = {GLX_DOUBLEBUFFER, None};

                        glVis[screen] = glXChooseVisual(display, screen, attribList);
                } else {
                        int         attribList[] =
                        {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 1, None};

                        glVis[screen] = glXChooseVisual(display, screen, attribList);
                }
        }
     // Make sure we have a visual
        if (!glVis[screen]) {
                return (0);
        }

        /* check if GL can render into root window. */
       for(i=0;i<visual_count;i++)
                if ( (glVis[screen]->visual == (wantVis+i)->visual) )
                        return (1); // success

        // The visual we received did not match one we asked for
        return (0);
}



//-----------------------------------------------------------------------------


GlutMasterKdeSaver::GlutMasterKdeSaver( Drawable drawable )
: QObject()
{
    Window root;
    int ai;
    unsigned int au;

    _mDrawable = drawable;
    _mGc = XCreateGC(qt_xdisplay(), _mDrawable, 0, 0);
    XGetGeometry(qt_xdisplay(), _mDrawable, &root, &ai, &ai,
        &_mWidth, &_mHeight, &au, &au);

    readSettings();

    // Clear to background colour when exposed
    XSetWindowBackground(qt_xdisplay(), _mDrawable,BlackPixel(qt_xdisplay(), qt_xscreen()));

    _colorContext = QColor::enterAllocContext();

//  batchcount = maxLevels;

//  initXLock( mGc );
//  initmorph3d( mDrawable );

//  _timer.start( speed );
    _timer.start(100); // 10 FPS??
    connect(&_timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

GlutMasterKdeSaver::~GlutMasterKdeSaver()
{
    XFreeGC(qt_xdisplay(), _mGc);
    _timer.stop();
//  release_morph3d();
    QColor::leaveAllocContext();
    QColor::destroyAllocContext(_colorContext);
}

void GlutMasterKdeSaver::readSettings()
{
    KConfig *config = klock_config();
    config->setGroup( "Settings" );

/*
    QString str;

    str = config->readEntry( "Speed" );
    if ( !str.isNull() )
        speed = MAXSPEED - str.toInt();
    else
        speed = DEFSPEED;

    maxLevels = config->readNumEntry( "MaxLevels", DEFBATCH );
    // CC: fixed MaxLevels <-> ObjectType inconsistency
*/
    delete config;
}

void GlutMasterKdeSaver::slotTimeout()
{
//  drawmorph3d( mDrawable );
}

/////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//
// Screen savers for KDE
//
// Copyright (c)  Martin R. Jones 1999
//

//#include <klocale.h>
//#include <kconfig.h>
//#include <kstandarddirs.h>
//#include <kdebug.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kcrash.h>

//#include "demowin.h"
//#include "saver.h"
//#include <kscreensaver_vroot.h>

static const char *appName = "klock";

static const char *description = I18N_NOOP("KDE Screen Lock / Saver");

static const char *version = "2.0.0";

static const KCmdLineOptions options[] =
{
  { "setup", I18N_NOOP("Setup screen saver."), 0 },
  { "window-id wid", I18N_NOOP("Run in the specified XWindow."), 0 },
  { "root", I18N_NOOP("Run in the root XWindow."), 0 },
  { "demo", I18N_NOOP("Start screen saver in demo mode."), "default"},
  { 0,0,0 }
};

static void crashHandler( int sig )
{
#ifdef SIGABRT
    signal ( SIGABRT, SIG_DFL );
#endif
    abort();
}

//----------------------------------------------------------------------------

static const char *appName = "klock";

static const char *description = I18N_NOOP("KDE Screen Lock / Saver");

static const char *version = "2.0.0";

static const KCmdLineOptions options[] =
{
  { "setup", I18N_NOOP("Setup screen saver."), 0 },
  { "window-id wid", I18N_NOOP("Run in the specified XWindow."), 0 },
  { "root", I18N_NOOP("Run in the root XWindow."), 0 },
  { "demo", I18N_NOOP("Start screen saver in demo mode."), "default"},
  { 0,0,0 }
};

static void crashHandler( int sig )
{
#ifdef SIGABRT
    signal ( SIGABRT, SIG_DFL );
#endif
    abort();
}

int main(int argc, char *argv[])
{
    KCmdLineArgs::init(argc, argv, appName, description, version);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;
    KCrash::setCrashHandler( crashHandler );

//    DemoWindow *demoWidget = 0;

    Window saveWin = 0;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("setup"))
    {
       setupScreenSaver();
       exit(0);
    }

    if (args->isSet("window-id"))
    {
        saveWin = atol(args->getOption("window-id"));
    }

    if (args->isSet("root"))
    {
        saveWin = RootWindow(qt_xdisplay(), qt_xscreen());
    }

    if (args->isSet("demo"))
    {
        saveWin = 0;
    }

    if (saveWin == 0)
    {
        demoWidget = new DemoWindow();
        demoWidget->setBackgroundMode(QWidget::NoBackground);
//        demoWidget->setBackgroundColor(Qt::black);
        demoWidget->show();
        saveWin = demoWidget->winId();
        app.setMainWidget(demoWidget);
        app.processEvents();
    }

    startScreenSaver(saveWin);
    app.exec();
    stopScreenSaver();

    if (demoWidget)
    {
        delete demoWidget;
    }

    return 0;
}


#endif
