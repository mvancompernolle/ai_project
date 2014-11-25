#include "project.h"

/*! \file
    \ingroup GLT

    $Id: project.cpp,v 1.16 2003/03/14 15:34:31 nigels Exp $

    $Log: project.cpp,v $
    Revision 1.16  2003/03/14 15:34:31  nigels
    Some cleanup, added mult(), rethinking tiling

    Revision 1.15  2003/01/22 08:38:21  nigels
    *** empty log message ***

    Revision 1.14  2003/01/22 08:14:31  nigels
    *** empty log message ***

    Revision 1.11  2002/11/27 00:57:28  nigels
    expand

    Revision 1.10  2002/11/07 15:40:45  nigels
    *** empty log message ***

    Revision 1.9  2002/10/09 15:09:38  nigels
    Added RCS Id and Log tags


*/

#include <glt/glu.h>
#include <glt/matrix.h>

#include <math/matrix4.h>

#include <cassert>
#include <iosfwd>
using namespace std;

GltOrtho::GltOrtho()
: _left(-1.0), _right(1.0), _bottom(-1.0), _top(1.0), _zNear(-1.0), _zFar(1.0)
{
}

GltOrtho::~GltOrtho()
{
}

void
GltOrtho::set() const
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(_left,_right,_bottom,_top,_zNear,_zFar);
    glMatrixMode(GL_MODELVIEW);
}

void
GltOrtho::set(int width,int height)
{
    GLdouble x = 1.0;
    GLdouble y = 1.0;

    if (width!=0 && height!=0)
    {
        if (width>height)
            x = GLdouble(width)/GLdouble(height);
        else
            y = GLdouble(height)/GLdouble(width);
    }

    _left   = -x;
    _right  =  x;
    _bottom = -y;
    _top    =  y;
    set();
}

void
GltOrtho::mult() const
{
    glMatrixMode(GL_PROJECTION);
    glOrtho(_left,_right,_bottom,_top,_zNear,_zFar);
    glMatrixMode(GL_MODELVIEW);
}

GLdouble &GltOrtho::left()   { return _left;   }
GLdouble &GltOrtho::right()  { return _right;  }
GLdouble &GltOrtho::bottom() { return _bottom; }
GLdouble &GltOrtho::top()    { return _top;    }
GLdouble &GltOrtho::zNear()  { return _zNear;  }
GLdouble &GltOrtho::zFar()   { return _zFar;   }

const GLdouble &GltOrtho::left()   const { return _left;   }
const GLdouble &GltOrtho::right()  const { return _right;  }
const GLdouble &GltOrtho::bottom() const { return _bottom; }
const GLdouble &GltOrtho::top()    const { return _top;    }
const GLdouble &GltOrtho::zNear()  const { return _zNear;  }
const GLdouble &GltOrtho::zFar()   const { return _zFar;   }

bool
GltOrtho::tile(GltOrtho &frust,const int dx,const int dy,const int n) const
{
    if (dx<1 || dy<1 || n<0 || n>=dx*dy)
    {
        frust = *this;
        return false;
    }

    frust._zNear = 1;
    frust._zFar  = -1;

    const int px = n%dx;
    const int py = n/dx;

    const GLdouble width  = (_right-_left)/dx;
    const GLdouble height = (_top-_bottom)/dy;

    assert(py<=dy);

    if (py<dy)
    {
        frust._left = frust._right  = _left   + width*px;
        frust._top  = frust._bottom = _bottom + height*py;

        frust._right += width;
        frust._top   += height;

        return true;
    }
    else
        return false;
}

///////////////////////////

GltFrustum::GltFrustum()
: _left(-1.0), _right(1.0), _bottom(-1.0), _top(1.0), _zNear(1.0), _zFar(10.0)
{
}

GltFrustum::~GltFrustum()
{
}

void
GltFrustum::set() const
{
    assert(_zNear>0.0);
    assert(_zFar>0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(_left,_right,_bottom,_top,_zNear,_zFar);
    glMatrixMode(GL_MODELVIEW);
}

void
GltFrustum::set(int width,int height)
{
    GLdouble x = 1.0;
    GLdouble y = 1.0;

    if (width!=0 && height!=0)
    {
        if (width>height)
            x = GLdouble(width)/GLdouble(height);
        else
            y = GLdouble(height)/GLdouble(width);
    }

    _left   = -x;
    _right  =  x;
    _bottom = -y;
    _top    =  y;
    set();
}

void
GltFrustum::mult() const
{
    assert(_zNear>0.0);
    assert(_zFar>0.0);

    glMatrixMode(GL_PROJECTION);
    glFrustum(_left,_right,_bottom,_top,_zNear,_zFar);
    glMatrixMode(GL_MODELVIEW);
}

GLdouble &GltFrustum::left()   { return _left;   }
GLdouble &GltFrustum::right()  { return _right;  }
GLdouble &GltFrustum::bottom() { return _bottom; }
GLdouble &GltFrustum::top()    { return _top;    }
GLdouble &GltFrustum::zNear()  { return _zNear;  }
GLdouble &GltFrustum::zFar()   { return _zFar;   }

const GLdouble &GltFrustum::left()   const { return _left;   }
const GLdouble &GltFrustum::right()  const { return _right;  }
const GLdouble &GltFrustum::bottom() const { return _bottom; }
const GLdouble &GltFrustum::top()    const { return _top;    }
const GLdouble &GltFrustum::zNear()  const { return _zNear;  }
const GLdouble &GltFrustum::zFar()   const { return _zFar;   }

bool
GltFrustum::tile(GltFrustum &frust,const int dx,const int dy,const int n) const
{
    if (dx<1 || dy<1 || n<0 || n>=dx*dy)
    {
        frust = *this;
        return false;
    }

    const int px = n%dx;
    const int py = n/dx;

    const GLdouble width  = (_right-_left)/dx;
    const GLdouble height = (_top-_bottom)/dy;

    assert(py<=dy);

    if (py<dy)
    {
        frust._left = frust._right  = _left   + width*px;
        frust._top  = frust._bottom = _bottom + height*py;

        frust._right += width;
        frust._top   += height;

        frust._zNear = _zNear;
        frust._zFar  = _zFar;

        return true;
    }
    else
        return false;
}

///////////////////////////

GltPerspective::GltPerspective()
: _fovy(60.0), _aspect(4/3), _zNear(1.0), _zFar(10.0), _rotX(0.0), _rotY(0.0)
{
}

GltPerspective::~GltPerspective()
{
}

void
GltPerspective::set() const
{
    assert(_zNear>0.0);
    assert(_zFar>0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(_fovy,_aspect,_zNear,_zFar);
    glMatrixMode(GL_MODELVIEW);
}

void
GltPerspective::set(int width,int height)
{
    _aspect = GLdouble(width)/GLdouble(height);
    set();
}

void
GltPerspective::mult() const
{
    assert(_zNear>0.0);
    assert(_zFar>0.0);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(_fovy,_aspect,_zNear,_zFar);
    glMatrixMode(GL_MODELVIEW);
}

void
GltPerspective::mult(int width,int height)
{
    _aspect = GLdouble(width)/GLdouble(height);
    mult();
}

GLdouble &GltPerspective::fovy()   { return _fovy;   }
GLdouble &GltPerspective::aspect() { return _aspect;  }
GLdouble &GltPerspective::zNear()  { return _zNear;  }
GLdouble &GltPerspective::zFar()   { return _zFar;   }

const GLdouble &GltPerspective::fovy()   const { return _fovy;   }
const GLdouble &GltPerspective::aspect() const { return _aspect; }
const GLdouble &GltPerspective::zNear()  const { return _zNear;  }
const GLdouble &GltPerspective::zFar()   const { return _zFar;   }

bool
GltPerspective::tile(GltPerspective &persp,const int dx,const int dy,const int n) const
{
    if (dx<1 || dy<1 || n<0 || n>=dx*dy)
    {
        persp = *this;
        return false;
    }

//    const int px = n%dx;
//   const int py = n/dx;

    persp._fovy   = _fovy/dy;
    persp._aspect = _aspect;
    persp._zNear  = _zNear;
    persp._zFar   = _zFar;

//    persp._rotX   = (0.5*(1.0 - dy) + py)*persp._fovy;
//    persp._rotY   = (0.5*(1.0 - dx) + px)*persp._fovy*_aspect;
//  persp._yRot   = 0.5*(dx-1)*dy;
    return true;
}

