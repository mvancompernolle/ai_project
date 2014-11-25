#include "vrmlview.h"

#ifdef OPENVRML
#include <OpenVRML/doc2.hpp>
#include <OpenVRML/System.h>
#include <OpenVRML/VrmlScene.h>
#include <OpenVRML-GL/ViewerOpenGL.h>

// Adaptor

class ViewerGLT : public ViewerOpenGL
{
public:

  ViewerGLT(GlutWindow &window,VrmlScene *);
  virtual ~ViewerGLT();

protected:

  // Window system specific methods

  virtual void wsPostRedraw();
  virtual void wsSetCursor( CursorStyle c);
  virtual void wsSwapBuffers();
  virtual void wsSetTimer( double );

private:

  GlutWindow &_window;
};

/////////////////////////

ViewerGLT::ViewerGLT(GlutWindow &window,VrmlScene *scene)
: _window(window), ViewerOpenGL(scene)
{
}

ViewerGLT::~ViewerGLT()
{
}

void ViewerGLT::wsPostRedraw()               { _window.postRedisplay(); }
void ViewerGLT::wsSetCursor( CursorStyle c)  { }
void ViewerGLT::wsSwapBuffers()              { }
void ViewerGLT::wsSetTimer( double )         { }

/////////////////////////

void
GlutVrmlViewer::loadScene(const std::string &filename)
{
    if (_scene)
        delete _scene;

    _scene = new VrmlScene(filename.c_str(),filename.c_str());

    if (_viewer)
        delete _viewer;

    _viewer = new ViewerGLT(*this,_scene);
}

void
GlutVrmlViewer::drawScene()
{
    if (_viewer)
        _viewer->redraw();
}

#endif

