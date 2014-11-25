#include "winexam.h"

/*! \file
    \ingroup GlutMaster
*/

#include <glutm/glut.h>

#include <glt/matrix.h>

#include <cstddef>
#include <cmath>
#include <iostream>
using namespace std;

GlutWindowExaminer::GlutWindowExaminer(const std::string &title,int width,int height,int x,int y,unsigned int displayMode)
: GlutWindow(title,width,height,x,y,displayMode),
  _mouseZoom(true),
  _mousePan(true),
  _mouseRotate(true),
  _mouseMode(MODE_MOUSE_ONLY),
  _mouseLeft(false),
  _mouseMiddle(false),
  _mouseRight(false),
  _mouseX(0),
  _mouseY(0),
  _oldCursor(0)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

GlutWindowExaminer::~GlutWindowExaminer()
{
}

bool   &GlutWindowExaminer::mouseZoom()      { return _mouseZoom;      }
bool   &GlutWindowExaminer::mousePan()       { return _mousePan;       }
bool   &GlutWindowExaminer::mouseRotate()    { return _mouseRotate;    }

GlutWindowExaminer::MouseMode &GlutWindowExaminer::mouseMode() { return _mouseMode; }

Vector &GlutWindowExaminer::referencePoint() { return _referencePoint; }

bool
GlutWindowExaminer::mousePressed() const
{
    return _mouseLeft || _mouseMiddle || _mouseRight;
}

Vector GlutWindowExaminer::xAxisOfRotation() const
{
    return _viewMatrixInverse*VectorX - _viewMatrixInverse*Vector0;
}

Vector GlutWindowExaminer::yAxisOfRotation() const
{
    return _viewMatrixInverse*VectorY - _viewMatrixInverse*Vector0;
}

Vector GlutWindowExaminer::axisOfRotation(const Vector &v) const
{
    return _viewMatrixInverse*v - _viewMatrixInverse*Vector0;
}

void GlutWindowExaminer::OnPreDisplay()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    _viewMatrix.glMultMatrix();
}

void GlutWindowExaminer::OnOpen()
{
    GlutWindow::OnOpen();
    OnOrient(_viewMatrix,_viewMatrixInverse);
}

void GlutWindowExaminer::OnClose()
{
    GlutWindow::OnClose();
}

void GlutWindowExaminer::OnReshape(int w, int h)
{
    GlutWindow::OnReshape(w,h);
    _ortho.set(w,h);
    OnOrient(_viewMatrix,_viewMatrixInverse);
}

void GlutWindowExaminer::OnKeyboard(unsigned char key, int x, int y)
{
    bool draw = true;

    switch (key)
    {
    case 'i':  if (_mouseZoom) _viewMatrix = matrixScale(1.2)          * _viewMatrix; break;
    case 'o':  if (_mouseZoom) _viewMatrix = matrixScale(1.0/1.2)      * _viewMatrix; break;

    case 'g':  if (_mousePan) _viewMatrix = _viewMatrix * matrixTranslate(VectorY* 0.1); break;
    case 'b':  if (_mousePan) _viewMatrix = _viewMatrix * matrixTranslate(VectorY*-0.1); break;
    case 'n':  if (_mousePan) _viewMatrix = _viewMatrix * matrixTranslate(VectorX*-0.1); break;
    case 'm':  if (_mousePan) _viewMatrix = _viewMatrix * matrixTranslate(VectorX* 0.1); break;
    case 'f':  if (_mousePan) _viewMatrix = _viewMatrix * matrixTranslate(VectorZ* 0.1); break;
    case 'v':  if (_mousePan) _viewMatrix = _viewMatrix * matrixTranslate(VectorZ*-0.1); break;

    case 'a':  if (_mouseRotate) _viewMatrix = matrixRotate(xAxisOfRotation(),10.0)  * _viewMatrix; break;
    case 'z':  if (_mouseRotate) _viewMatrix = matrixRotate(xAxisOfRotation(),-10.0) * _viewMatrix; break;
    case 'x':  if (_mouseRotate) _viewMatrix = matrixRotate(yAxisOfRotation(),10.0)  * _viewMatrix; break;
    case 'c':  if (_mouseRotate) _viewMatrix = matrixRotate(yAxisOfRotation(),-10.0) * _viewMatrix; break;

    case 'q':
    case 27:  close(); return;

    default:
        GlutWindow::OnKeyboard(key,x,y);
        draw = false;
    }

    if (draw)
    {
        _viewMatrixInverse = _viewMatrix.inverse();
        postRedisplay();
        OnOrient(_viewMatrix,_viewMatrixInverse);
    }
}

void
GlutWindowExaminer::OnSpecial(int key, int x, int y)
{
    GlutWindow::OnSpecial(key,x,y);

    bool draw = true;

    switch (key)
    {
    case GLUT_KEY_HOME: _viewMatrix = Matrix(); break;
    default:
        draw = false;
    }

    if (draw)
    {
        _viewMatrixInverse = _viewMatrix.inverse();
        postRedisplay();
        OnOrient(_viewMatrix,_viewMatrixInverse);
    }
}

void GlutWindowExaminer::OnMenu(int value)
{
    GlutWindow::OnMenu(value);
}

void GlutWindowExaminer::OnMouse(int button, int state, int x, int y)
{
    // Keep mouse position for later

    _mouseX = x;
    _mouseY = y;

    // Suspend idle, if necessary

    if (!mousePressed())
    {
        // TODO: Is this really necessary??
        // Idle is a bad way to drive redraw anyway!
        _wasIdle = getIdle();
        setIdle(false);
        _oldCursor = getCursor();
    }

    // Update mouse button state

    if (state==GLUT_UP)
        switch (button)
        {
        case GLUT_LEFT_BUTTON:   _mouseLeft   = false; break;
        case GLUT_MIDDLE_BUTTON: _mouseMiddle = false; break;
        case GLUT_RIGHT_BUTTON:  _mouseRight  = false; break;
        }
    else
    {
        _referencePoint = getReferencePoint(x,y,dragPos(x,y),_viewMatrix,_viewMatrixInverse);

        switch (button)
        {
        case GLUT_LEFT_BUTTON:      _mouseLeft   = true; break;
        case GLUT_MIDDLE_BUTTON:    _mouseMiddle = true; break;
        case GLUT_RIGHT_BUTTON:     _mouseRight  = true; break;
        }
    }

    // Restart idle, if necessary

    if (!mousePressed())
    {
        setIdle(_wasIdle);
        setCursor(_oldCursor);
    }

    // Keep track of world-space drag position

    _dragPosPrev = dragPos(x,y);
}

void GlutWindowExaminer::OnMotion(int x, int y)
{
    bool changed = false;

    const int dx = x - _mouseX;
    const int dy = y - _mouseY;

    if (dx==0 && dy==0)
        return;

    bool zoom   = false;
    bool pan    = false;
    bool rotate = false;

    switch (_mouseMode)
    {
        //
        // Left rotate, middle zoom, right pan
        //

        default:
        case MODE_MOUSE_ONLY:
            zoom   = _mouseZoom   && (_mouseMiddle || (_mouseLeft && _mouseRight));
            pan    = _mousePan    && _mouseRight && !zoom;
            rotate = _mouseRotate && _mouseLeft  && !zoom;
            break;

        //
        // Right mouse button, shift to pan, control to zoom
        //

        case MODE_MOUSE_RIGHT:
            zoom   = _mouseZoom   && _mouseRight && getCtrlModifier();
            pan    = _mousePan    && _mouseRight && getShiftModifier() && !zoom;
            rotate = _mouseRotate && _mouseRight && !zoom && !pan;
            break;
    }

    if (zoom)
    {
        // ZOOM

        _viewMatrix =
            matrixTranslate(-_referencePoint) *
            matrixScale(exp(double(dy)*0.01)) *
            matrixTranslate(_referencePoint) *
            _viewMatrix;

        setCursor(GLUT_CURSOR_UP_DOWN);

        changed = true;
    }
    else
    if (rotate)
    {
        // ROTATE

        Vector axis;
        real angle = 0.0;

        if (getAltModifier())
        {
            // Axis is perpendicular to viewing plane
            // Angle is proportional to mouse y velocity

            axis  = VectorZ;
            angle = -dy/(_viewport.height()+1.0)*720.0;
        }
        else
        {
            // Axis is perpendicular to delta
            // (The "-dy" is because mouse has top left origin)
            // Angle is proportional to the distance moved

            axis  = xProduct(Vector(real(dx),real(-dy),0.0),VectorZ);
            angle = axis.length()/(_viewport.width()+1.0)*180.0;
        }

        _viewMatrix =
            matrixTranslate(-_referencePoint) *
            matrixRotate(axisOfRotation(axis),angle) *
            matrixTranslate(_referencePoint) *
            _viewMatrix;

        setCursor(GLUT_CURSOR_CYCLE);

        changed = true;
    }
    else
    if (pan)
    {
        // PAN

        const Vector pos = dragPos(x,y);

        if (getAltModifier())
            _viewMatrix = _viewMatrix * matrixTranslate(VectorZ*dy/_viewport.height()*(_ortho.zFar()-_ortho.zNear()));
        else
            _viewMatrix = _viewMatrix * matrixTranslate(pos-_dragPosPrev);

        _dragPosPrev = pos;

        setCursor(GLUT_CURSOR_CROSSHAIR);

        changed = true;
    }

    // Save the current mouse position for later

    _mouseX = x;
    _mouseY = y;

    if (changed)
    {
        _viewMatrixInverse = _viewMatrix.inverse();
        postRedisplay();
        OnOrient(_viewMatrix,_viewMatrixInverse);
    }
}

Vector
GlutWindowExaminer::dragPos(const int x,const int y) const
{
    // Use the ortho projection and viewport information
    // to map from mouse co-ordinates back into world
    // co-ordinates

    real wx = real(x-_viewport.x())/real(_viewport.width());
    real wy = real(y-_viewport.y())/real(_viewport.height());

    wx = _ortho.left() + wx*(_ortho.right() -_ortho.left());
    wy = _ortho.top()  + wy*(_ortho.bottom()-_ortho.top());

    return Vector(wx,wy,_ortho.zNear());
}

Vector
GlutWindowExaminer::getReferencePoint(int x,int y,const Vector &pos,const Matrix &modelView,const Matrix &modelViewInverse) const
{
    return Vector0;
}

void
GlutWindowExaminer::OnPassiveMotion(int x, int y)
{
}

void
GlutWindowExaminer::OnOrient(const Matrix &viewing,const Matrix &inverse)
{
}
