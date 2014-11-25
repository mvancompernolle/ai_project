#include "raster.h"

/*! \file
    \ingroup GLT

    $Id: raster.cpp,v 1.7 2002/11/27 00:57:28 nigels Exp $

    $Log: raster.cpp,v $
    Revision 1.7  2002/11/27 00:57:28  nigels
    expand

    Revision 1.6  2002/11/07 15:40:45  nigels
    *** empty log message ***

    Revision 1.5  2002/10/09 15:09:38  nigels
    Added RCS Id and Log tags


*/

GltRasterPos::GltRasterPos(const bool getIt)
{
    if (getIt)
        get();
    else
        _pos[0] = _pos[1] = _pos[2] = _pos[3] = 0.0;
}

GltRasterPos::~GltRasterPos()
{
}

void
GltRasterPos::get()
{
    glGetDoublev(GL_CURRENT_RASTER_POSITION,_pos);
}

void
GltRasterPos::set()
{
    glRasterPos4dv(_pos);
}

void
GltRasterPos::set(int x,int y)
{
    _pos[0] = x;
    _pos[1] = y;
    _pos[2] = 0.0;
    _pos[3] = 0.0;
    set();
}

GLdouble &GltRasterPos::x() { return _pos[0]; }
GLdouble &GltRasterPos::y() { return _pos[1]; }
GLdouble &GltRasterPos::z() { return _pos[2]; }
GLdouble &GltRasterPos::w() { return _pos[3]; }

const GLdouble &GltRasterPos::x() const { return _pos[0]; }
const GLdouble &GltRasterPos::y() const { return _pos[1]; }
const GLdouble &GltRasterPos::z() const { return _pos[2]; }
const GLdouble &GltRasterPos::w() const { return _pos[3]; }
