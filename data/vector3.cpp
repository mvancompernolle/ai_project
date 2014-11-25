#include "vector3.h"

/*! \file
    \ingroup Math

    $Id: vector3.cpp,v 1.21 2003/07/22 03:58:06 nigels Exp $

    $Log: vector3.cpp,v $
    Revision 1.21  2003/07/22 03:58:06  nigels
    Vector -> Vector3
    CLAMP -> clamp
    LERP -> lerp

    Revision 1.20  2003/03/06 12:30:30  nigels
    *** empty log message ***

*/

#include "matrix4.h"
#include "vector2.h"
#include "vector4.h"

#include "matrix4.h"

#include <glt/gl.h>
#include <glt/glu.h>
#include <glt/viewport.h>

#include <misc/string.h>
#include <cassert>
#include <cmath>

#include <iostream>
#include <algorithm>
using namespace std;

const Vector3 VectorX(1.0,0.0,0.0);
const Vector3 VectorY(0.0,1.0,0.0);
const Vector3 VectorZ(0.0,0.0,1.0);
const Vector3 Vector0(0.0,0.0,0.0);
const Vector3 Vector1(1.0,1.0,1.0);

Vector3::Vector3()
{
    _vector[0] = _vector[1] = _vector[2] = 0.0;
}

Vector3::Vector3(const Vector2 &v)
{
    _vector[0] = v.x();
    _vector[1] = v.y();
    _vector[2] = 0.0;
}

Vector3::Vector3(const Vector3 &v)
{
    _vector[0] = v._vector[0];
    _vector[1] = v._vector[1];
    _vector[2] = v._vector[2];
}

Vector3::Vector3(const Vector4 &v)
{
    _vector[0] = v.x();
    _vector[1] = v.y();
    _vector[2] = v.z();
}

Vector3::Vector3(const real x, const real y, const real z)
{
    _vector[0] = x;
    _vector[1] = y;
    _vector[2] = z;
}

Vector3::Vector3(const float *v)
{
    _vector[0] = v[0];
    _vector[1] = v[1];
    _vector[2] = v[2];
}

Vector3::Vector3(const double *v)
{
    _vector[0] = v[0];
    _vector[1] = v[1];
    _vector[2] = v[2];
}

Vector3::Vector3(const string &str)
{
    #ifndef NDEBUG
    const int n =
    #endif
        atoc(str,atof,"+-eE.0123456789",_vector+0,_vector+3);

    assert(n==3);
}

const real &
Vector3::operator[](const int i) const
{
    assert(i>=0 && i<3);
    return _vector[i];
}

real &
Vector3::operator[](const int i)
{
    assert(i>=0 && i<3);
    return _vector[i];
}

Vector3::operator real *()
{
    return (real *) _vector;
}

Vector3::operator const real *() const
{
    return (real *) _vector;
}


      real &Vector3::x()       { return _vector[0]; }
const real &Vector3::x() const { return _vector[0]; }
      real &Vector3::y()       { return _vector[1]; }
const real &Vector3::y() const { return _vector[1]; }
      real &Vector3::z()       { return _vector[2]; }
const real &Vector3::z() const { return _vector[2]; }

bool
Vector3::operator==(const Vector3 &v) const
{
    return
        _vector[0] == v[0] &&
        _vector[1] == v[1] &&
        _vector[2] == v[2];
}

bool
Vector3::operator!=(const Vector3 &v) const
{
    return
        _vector[0] != v[0] ||
        _vector[1] != v[1] ||
        _vector[2] != v[2];
}

bool
Vector3::operator==(const real &a) const
{
    return
        _vector[0] == a &&
        _vector[1] == a &&
        _vector[2] == a;
}

bool
Vector3::operator< (const Vector3 &v) const
{
    if (x()!=v.x()) return x()<v.x();
    if (y()!=v.y()) return y()<v.y();
    return z()<v.z();
}

bool
Vector3::operator> (const Vector3 &v) const
{
    if (x()!=v.x()) return x()>v.x();
    if (y()!=v.y()) return y()>v.y();
    return z()>v.z();
}

ostream &
Vector3::writePov(ostream &os) const
{
    os << "< ";
    os << x() << ',';
    os << y() << ',';
    os << z() << " >";

    return os;
}

Vector3
Vector3::operator-() const
{
    return Vector3(-_vector[0], -_vector[1], -_vector[2]);
}

void
Vector3::scale(const real &x)
{
    _vector[0] *= x;
    _vector[1] *= x;
    _vector[2] *= x;
}

void
Vector3::scale(const Vector3 &x)
{
    _vector[0] *= x._vector[0];
    _vector[1] *= x._vector[1];
    _vector[2] *= x._vector[2];
}

void
Vector3::normalize()
{
    const real mag = sqrt(norm());

    if (mag == 0.0)
        return;

    const real magInv = 1.0/mag;

    _vector[0] *= magInv;
    _vector[1] *= magInv;
    _vector[2] *= magInv;
}

void
Vector3::abs()
{
    _vector[0] = fabs(_vector[0]);
    _vector[1] = fabs(_vector[1]);
    _vector[2] = fabs(_vector[2]);
}

int
Vector3::dominant() const
{
    const real x = fabs(_vector[0]);
    const real y = fabs(_vector[1]);
    const real z = fabs(_vector[2]);

    if (x>y && x>z)
        return 0;
    else
        if (y>z)
            return 1;
        else
            return 2;
}

real
Vector3::norm() const
{
    return ((*this)*(*this));
}

real
Vector3::length() const
{
    return sqrt((*this)*(*this));
}

bool
Vector3::project(const Matrix &model,const Matrix &proj,const GltViewport &view)
{
    // TODO - No need to copy from x,y,z if real is GLdouble

    GLdouble x,y,z;
    GLint ret = gluProject(_vector[0],_vector[1],_vector[2],model,proj,view,&x,&y,&z);

    _vector[0] = x;
    _vector[1] = y;
    _vector[2] = z;

    return ret==GL_TRUE;
}

Vector3 &
Vector3::operator=(const Vector3 &x)
{
    _vector[0] = x[0];
    _vector[1] = x[1];
    _vector[2] = x[2];

    return *this;
}

Vector3 &
Vector3::operator=(const float *x)
{
    _vector[0] = x[0];
    _vector[1] = x[1];
    _vector[2] = x[2];

    return *this;
}

Vector3 &
Vector3::operator=(const double *x)
{
    _vector[0] = x[0];
    _vector[1] = x[1];
    _vector[2] = x[2];

    return *this;
}

real
Vector3::operator*(const Vector3 &x) const
{
    return _vector[0]*x[0] + _vector[1]*x[1] + _vector[2]*x[2];
}

Vector3 &
Vector3::operator+=(const Vector3 &v)
{
    _vector[0] += v[0];
    _vector[1] += v[1];
    _vector[2] += v[2];

    return *this;
}

Vector3 &
Vector3::operator-=(const Vector3 &x)
{
    _vector[0] -= x[0];
    _vector[1] -= x[1];
    _vector[2] -= x[2];

    return *this;
}

Vector3 &
Vector3::operator*=(const real &x)
{
    _vector[0] *= x;
    _vector[1] *= x;
    _vector[2] *= x;

    return *this;
}

Vector3 &
Vector3::operator*=(const Matrix &m)
{
    return operator=(m*(*this));
}

real
Vector3::dist(const Vector3 &x) const
{
    return ((*this)-x).norm();
}

Vector3 &
Vector3::vmin(const Vector3 &v)
{
    _vector[0] = MIN(_vector[0],v[0]);
    _vector[1] = MIN(_vector[1],v[1]);
    _vector[2] = MIN(_vector[2],v[2]);
    return *this;
}

Vector3 &
Vector3::vmax(const Vector3 &v)
{
    _vector[0] = MAX(_vector[0],v[0]);
    _vector[1] = MAX(_vector[1],v[1]);
    _vector[2] = MAX(_vector[2],v[2]);
    return *this;
}

///////////////////////// OpenGL

#include <glt/gl.h>

void
Vector3::glVertex() const
{
    #ifdef GLT_FAST_FLOAT
    glVertex3fv(_vector);
    #else
    glVertex3dv(_vector);
    #endif
}

void
Vector3::glNormal() const
{
    #ifdef GLT_FAST_FLOAT
    glNormal3fv(_vector);
    #else
    glNormal3dv(_vector);
    #endif
}

void
Vector3::glColor() const
{
    #ifdef GLT_FAST_FLOAT
    glColor3fv(_vector);
    #else
    glColor3dv(_vector);
    #endif
}

void
Vector3::glTexCoord() const
{
    #ifdef GLT_FAST_FLOAT
    glTexCoord3fv(_vector);
    #else
    glTexCoord3dv(_vector);
    #endif
}

////////////////////////// Friends

/*!
    \brief Output vector to stream
    \ingroup Math
*/

ostream &
operator<<(ostream &os, const Vector3 &x)
{
    os << x[0] << '\t';
    os << x[1] << '\t';
    os << x[2];

    return os;
}

/*!
    \brief Input vector from stream
    \ingroup Math
*/

istream &
operator>>(istream &is, Vector3 &x)
{
    is >> x[0];
    is >> x[1];
    is >> x[2];

    return is;
}

/*!
    \brief Vector scale
    \ingroup Math
*/

Vector3
operator*(const real x, const Vector3 &v)
{
    return v*x;
}

/*!
    \brief Vector scale
    \ingroup Math
*/

Vector3
operator*(const Vector3 &v, const real x)
{
    return Vector3( x*v._vector[0], x*v._vector[1], x*v._vector[2]);
}

/*!
    \brief Vector inverse scale
    \ingroup Math
*/

Vector3
operator/(const Vector3 &v, const real x)
{
    assert(x != 0.0);
    const real inv = 1.0/x;
    return v*inv;
}

/*!
    \brief Vector addition
    \ingroup Math
*/

Vector3
operator+(const Vector3 &v1, const Vector3 &v2)
{
    return Vector3(v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2]);
}

/*!
    \brief Vector difference
    \ingroup Math
*/

Vector3
operator-(const Vector3 &v1, const Vector3 &v2)
{
    return Vector3(v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]);
}

/*!
    \brief Vector cross product
    \ingroup Math
*/

Vector3
xProduct(const Vector3 &v1, const Vector3 &v2)
{
    return Vector3(v1[1]*v2[2] - v1[2]*v2[1],
                  v1[2]*v2[0] - v1[0]*v2[2],
                  v1[0]*v2[1] - v1[1]*v2[0]);
}

/*!
    \brief Calculate normal from three points in plane
    \ingroup Math
*/

Vector3
planeNormal(const Vector3 &v1, const Vector3 &v2,const Vector3 &v3)
{
    return xProduct(v2-v1,v3-v1);
}

/*!
    \brief Location on unit sphere
    \ingroup Math
*/

Vector3
polar(const real lat,const real longitude)
{
    return
        Vector3(
            cos(lat*M_PI_DEG) * cos(longitude*M_PI_DEG),
            sin(lat*M_PI_DEG),
            cos(lat*M_PI_DEG) * sin(longitude*M_PI_DEG)
        );
}

/*!
    \brief Create orthogonal co-ordinate system, based on a
    \ingroup Math
*/

void orthogonalSystem(Vector3 &a,Vector3 &b,Vector3 &c)
{
    a.normalize();

    if (fabs(a.z())>0.8)
    {
        b = xProduct(a,VectorY);
        c = xProduct(a,b);
    }
    else
    {
        b = xProduct(a,VectorZ);
        c = xProduct(a,b);
    }

    b.normalize();
    c.normalize();
}



