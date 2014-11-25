#include "window.h"

/*! \file
    \ingroup GlutMaster
*/

#include <glutm/master.h>
#include <glt/buffer.h>

#include <fstream>
#include <cstdio>
using namespace std;


///////////////////////////// GlutWindow ///////////////////////////////////////

GlutWindow::GlutWindow(const std::string &title,const int width,const int height,const int x,const int y,const unsigned int displayMode)
: _leftButtonMenu(GLUT_LEFT_BUTTON),
  _rightButtonMenu(GLUT_RIGHT_BUTTON),
  _fileIndex(0),
  _windowID(-1),
  _shiftModifier(false),
  _ctrlModifier(false),
  _altModifier(false),
  _x(x),
  _y(y),
  _width(width),
  _height(height),
  _displayMode(displayMode),
  _gameMode(false),
  _title(title),
  _mouseEvents(false),
  _mouseX(0),
  _mouseY(0),
  _windowSaved(false),
  _windowX(0),
  _windowY(0),
  _windowWidth(0),
  _windowHeight(0)
{
    _swapBuffers = (displayMode & GLUT_DOUBLE)!=0;
}

GlutWindow::~GlutWindow()
{
}

void
GlutWindow::open()
{
    GlutMaster::openWindow(this);
}

void
GlutWindow::close()
{
    if (_windowID!=-1)
        GlutMaster::closeWindow(this);
}

void
GlutWindow::grab(const GlutWindow &window)
{
    //
    // Grab another GLUT window
    //

    _windowID = window._windowID;

    if (_windowID!=-1)
        GlutMaster::openWindow(this);
}

void
GlutWindow::enterGameMode()
{
    if (!_windowSaved)
    {
        _windowSaved = true;

        _windowX = _x;
        _windowY = _y;
        _windowWidth = _width;
        _windowHeight = _height;
        GlutMaster::fullScreen(this);
    }
}

void
GlutWindow::leaveGameMode()
{
    if (_windowSaved)
    {
        _windowSaved = false;
        position(_windowX,_windowY);
        reshape(_windowWidth,_windowHeight);
    }
}

void
GlutWindow::toggleGameMode()
{
    if (_windowSaved)
        leaveGameMode();
    else
        enterGameMode();
}

void
GlutWindow::position(int x,int y)
{
    GlutMaster::positionWindow(this,x,y);
}

void
GlutWindow::reshape(int width,int height)
{
    GlutMaster::reshapeWindow(this,width,height);
}

void
GlutWindow::setCursor(int cursor)
{
    GlutMaster::setCursor(this,cursor);
}

void
GlutWindow::postRedisplay()
{
    GlutMaster::postRedisplay(this);
}

void
GlutWindow::swapBuffers()
{
    GlutMaster::swapBuffers(this);
}

////////////////////////////////////////////////////////////////////

void
GlutWindow::setSwapBuffers(const bool swap)
{
    _swapBuffers = swap;
}

bool
GlutWindow::getSwapBuffers() const
{
    return _swapBuffers;
}

      int &GlutWindow::windowID()       { return _windowID; }
const int &GlutWindow::windowID() const { return _windowID; }

     bool  GlutWindow::getOpen() const  { return _windowID!=-1; }

  int GlutWindow::getX()              const { return GlutMaster::get(this,GLUT_WINDOW_X);      }
  int GlutWindow::getY()              const { return GlutMaster::get(this,GLUT_WINDOW_Y);      }
  int GlutWindow::getWidth()          const { return _width;  }
  int GlutWindow::getHeight()         const { return _height; }

  int GlutWindow::getBufferSize()     const { return GlutMaster::get(this,GLUT_WINDOW_BUFFER_SIZE);  }
  int GlutWindow::getStencilSize()    const { return GlutMaster::get(this,GLUT_WINDOW_STENCIL_SIZE); }
  int GlutWindow::getDepthSize()      const { return GlutMaster::get(this,GLUT_WINDOW_DEPTH_SIZE);   }
  int GlutWindow::getRedSize()        const { return GlutMaster::get(this,GLUT_WINDOW_RED_SIZE);     }
  int GlutWindow::getGreenSize()      const { return GlutMaster::get(this,GLUT_WINDOW_GREEN_SIZE);   }
  int GlutWindow::getBlueSize()       const { return GlutMaster::get(this,GLUT_WINDOW_BLUE_SIZE);    }
  int GlutWindow::getAlphaSize()      const { return GlutMaster::get(this,GLUT_WINDOW_ALPHA_SIZE);   }

  int GlutWindow::getAccumRedSize()   const { return GlutMaster::get(this,GLUT_WINDOW_ACCUM_RED_SIZE);   }
  int GlutWindow::getAccumGreenSize() const { return GlutMaster::get(this,GLUT_WINDOW_ACCUM_GREEN_SIZE); }
  int GlutWindow::getAccumBlueSize()  const { return GlutMaster::get(this,GLUT_WINDOW_ACCUM_BLUE_SIZE);  }
  int GlutWindow::getAccumAlphaSize() const { return GlutMaster::get(this,GLUT_WINDOW_ACCUM_ALPHA_SIZE); }

 bool GlutWindow::getDoubleBuffer()   const { return GlutMaster::get(this,GLUT_WINDOW_DOUBLEBUFFER)!=0; }
 bool GlutWindow::getRGBA()           const { return GlutMaster::get(this,GLUT_WINDOW_RGBA)!=0;         }
 bool GlutWindow::getStereo()         const { return GlutMaster::get(this,GLUT_WINDOW_STEREO)!=0;       }

 bool GlutWindow::getTopLevel()       const { return GlutMaster::get(this,GLUT_WINDOW_PARENT)==0;     }
  int GlutWindow::getNumChildren()    const { return GlutMaster::get(this,GLUT_WINDOW_NUM_CHILDREN);  }

  int GlutWindow::getColormapSize()   const { return GlutMaster::get(this,GLUT_WINDOW_COLORMAP_SIZE); }
  int GlutWindow::getNumSamples()     const { return GlutMaster::get(this,GLUT_WINDOW_NUM_SAMPLES);   }
  int GlutWindow::getCursor()         const { return GlutMaster::get(this,GLUT_WINDOW_CURSOR);        }

bool GlutWindow::getShiftModifier()  const { return _shiftModifier; }
bool GlutWindow::getCtrlModifier()   const { return _ctrlModifier; }
bool GlutWindow::getAltModifier()    const { return _altModifier; }

////////////////////////////////////////////////////////////////////

void GlutWindow::OnPreDisplay()
{
}

void GlutWindow::OnDisplay()
{
}

void GlutWindow::OnPostDisplay()
{
    if (_swapBuffers)
        swapBuffers();
}

void GlutWindow::OnIdle()         { postRedisplay(); }
void GlutWindow::OnTick()         { postRedisplay(); }
void GlutWindow::OnTimer(int val) { postRedisplay(); }

void GlutWindow::OnPreIdle()
{
    if (_windowID!=-1)
        glutSetWindow(_windowID);
}

void GlutWindow::OnPreTimer()
{
    if (_windowID!=-1)
    {
        glutSetWindow(_windowID);
        if (_mouseEvents)
        {
            OnMotion(_mouseX,_mouseY);
            _mouseEvents = false;
        }
    }
}

void GlutWindow::OnReshape(int w, int h)
{
    _width = w;
    _height = h;
    _viewport.set(w,h);
    postRedisplay();
}

void GlutWindow::OnOpen()  {}
void GlutWindow::OnClose() {}

void GlutWindow::OnMotion(int x, int y)             {}
void GlutWindow::OnMouse(int button, int state, int x, int y)   {}
void GlutWindow::OnPassiveMotion(int x, int y)          {}
void GlutWindow::OnEntry(int state)             {}
void GlutWindow::OnVisibility(int visible)          {}

void GlutWindow::OnKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'q':
    case 'Q':
    case  27:
        close();
        return;

    case '\t':
        toggleGameMode();
        break;
    }
}

void GlutWindow::OnKeyboardUp(unsigned char key, int x, int y)
{
}

void GlutWindow::OnSpecial(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_F9:  captureRGB    (nextFilename()); break;
    case GLUT_KEY_F10: captureStencil(nextFilename()); break;
    case GLUT_KEY_F11: captureZBuffer(nextFilename()); break;

    default:
        break;
    }
}

void GlutWindow::OnSpecialUp(int key, int x, int y)
{
}

void GlutWindow::OnMenu(int value)
{
    switch (value)
    {
    case MENU_CLOSE:
        close();
        break;

    case MENU_CAPTURE_RGB:       captureRGB    (nextFilename()); break;
    case MENU_CAPTURE_STENCIL:   captureStencil(nextFilename()); break;
    case MENU_CAPTURE_ZBUFFER:   captureZBuffer(nextFilename()); break;

    default:
        break;
    }
}

bool
GlutWindow::captureRGB(const std::string &filename)
{
    // Redraw

    OnDisplay();

    // Get the frame buffer

    GltFrameBufferRGB rgb;
    rgb.read();

/*
#ifdef GLT_JPEG
    // Write JPG file
    string file(filename + ".jpg");
    ofstream os(file.c_str(),ios::binary);
    rgb.writeJPG(os);
#else
*/

#ifdef GLT_PNG
    // Write PNG file
    string file(filename + ".png");
    ofstream os(file.c_str(),ios::binary);
    rgb.writePNG(os);
#else
    // Write PPM file
    string file(filename + ".ppm");
    ofstream os(file.c_str(),ios::binary);
    rgb.writePPM(os);
#endif

    return true;
}

bool
GlutWindow::captureZBuffer(const std::string &filename)
{
//  if (!getDepthSize())
//      return false;

    // Redraw

    OnDisplay();

    // Get the frame buffer

    GltFrameBufferZUint z;
    z.read();

    // Write PPM file

    string file(filename + ".ppm");
    ofstream os(file.c_str(),ios::binary);
    z.writePPM(os);

    return true;
}

bool
GlutWindow::captureStencil(const std::string &filename)
{
//  if (!getStencilSize())
//      return false;

    // Redraw

    OnDisplay();

    // Get the frame buffer

    GltFrameBufferStencilUbyte s;
    s.read();

    // Write PPM file

    string file(filename + ".ppm");
    ofstream os(file.c_str(),ios::binary);
    s.writePPM(os);

    return true;
}

string
GlutWindow::nextFilename()
{
    char filename[9];
    sprintf(filename,"%08d",_fileIndex++);
    return string(filename);
}
