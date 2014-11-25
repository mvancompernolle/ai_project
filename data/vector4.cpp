#include "vector4.h"

/*! \file
    \ingroup Math
    \todo    Complete w.r.t Vector3

    $Id: vector4.cpp,v 1.7 2003/07/22 03:58:06 nigels Exp $

    $Log: vector4.cpp,v $
    Revision 1.7  2003/07/22 03:58:06  nigels
    Vector -> Vector3
    CLAMP -> clamp
    LERP -> lerp

    Revision 1.6  2003/03/06 12:30:51  nigels
    *** empty log message ***

*/

#include <misc/string.h>

#include "matrix4.h"
#include "vector2.h"
#include "vector3.h"

#include <cassert>
#include <cmath>

#include <iostream>
#include <algorithm>
using namespace std;

Vector4::Vector4()
{
    _vector[0] = _vector[1] = _vector[2] = _vector[3] = 0.0;
}

Vector4::Vector4(const Vector2 &v)
{
    _vector[0] = v.x();
    _vector[1] = v.y();
    _vector[2] = _vector[3] = 0.0;
}

Vector4::Vector4(const Vector3 &v)
{
    _vector[0] = v.x();
    _vector[1] = v.y();
    _vector[2] = v.z();
    _vector[3] = 0.0;
}

Vector4::Vector4(const Vector4 &v)
{
    _vector[0] = v._vector[0];
    _vector[1] = v._vector[1];
    _vector[2] = v._vector[2];
    _vector[3] = v._vector[3];
}

Vector4::Vector4(const real x, const real y, const real z,const real w)
{
    _vector[0] = x;
    _vector[1] = y;
    _vector[2] = z;
    _vector[3] = w;
}

Vector4::Vector4(const float *v)
{
    _vector[0] = v[0];
    _vector[1] = v[1];
    _vector[2] = v[2];
    _vector[3] = v[3];
}

Vector4::Vector4(const double *v)
{
    _vector[0] = v[0];
    _vector[1] = v[1];
    _vector[2] = v[2];
    _vector[3] = v[3];
}

Vector4::Vector4(const string &str)
{
    #ifndef NDEBUG
    const int n =
    #endif
        atoc(str,atof,"+-eE.0123456789",_vector+0,_vector+4);

    assert(n==4);
}

const real &
Vector4::operator[](const int i) const
{
    assert(i>=0 && i<4);
    return _vector[i];
}

real &
Vector4::operator[](const int i)
{
    assert(i>=0 && i<4);
    return _vector[i];
}

Vector4::operator real *()
{
    return (real *) _vector;
}

Vector4::operator const real *() const
{
    return (real *) _vector;
}

      real &Vector4::x()       { return _vector[0]; }
const real &Vector4::x() const { return _vector[0]; }
      real &Vector4::y()       { return _vector[1]; }
const real &Vector4::y() const { return _vector[1]; }
      real &Vector4::z()       { return _vector[2]; }
const real &Vector4::z() const { return _vector[2]; }
      real &Vector4::w()       { return _vector[3]; }
const real &Vector4::w() const { return _vector[3]; }

///////////////////////// OpenGL

#include <glt/gl.h>

void
Vector4::glVertex() const
{
    #ifdef GLT_FAST_FLOAT
    glVertex4fv(_vector);
    #else
    glVertex4dv(_vector);
    #endif
}

void
Vector4::glNormal() const
{
    #ifdef GLT_FAST_FLOAT
    glNormal3fv(_vector);
    #else
    glNormal3dv(_vector);
    #endif
}

void
Vector4::glColor() const
{
    #ifdef GLT_FAST_FLOAT
    glColor4fv(_vector);
    #else
    glColor4dv(_vector);
    #endif
}

void
Vector4::glTexCoord() const
{
    #ifdef GLT_FAST_FLOAT
    glTexCoord4fv(_vector);
    #else
    glTexCoord4dv(_vector);
    #endif
}

////////////////////////// Friends

/*!
    \brief Output vector to stream
    \ingroup Math
*/

ostream &
operator<<(ostream &os, const Vector4 &x)
{
    os << x[0] << '\t';
    os << x[1] << '\t';
    os << x[2] << '\t';
    os << x[3];

    return os;
}

/*!
    \brief Input vector from stream
    \ingroup Math
*/

istream &
operator>>(istream &is, Vector4 &x)
{
    is >> x[0];
    is >> x[1];
    is >> x[2];
    is >> x[3];

    return is;
}

