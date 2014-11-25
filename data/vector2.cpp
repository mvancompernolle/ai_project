#include "vector2.h"

/*! \file
    \ingroup Math

    $Id: vector2.cpp,v 1.1 2003/07/22 03:56:26 nigels Exp $

    $Log: vector2.cpp,v $
    Revision 1.1  2003/07/22 03:56:26  nigels
    *** empty log message ***

    Revision 1.20  2003/03/06 12:30:30  nigels
    *** empty log message ***

*/

#include "matrix4.h"
#include "vector3.h"
#include "vector4.h"

#include <glt/gl.h>
#include <glt/glu.h>

#include <misc/string.h>
#include <cassert>
#include <cmath>

#include <iostream>
#include <algorithm>
using namespace std;

Vector2::Vector2()
{
    _vector[0] = _vector[1] = 0.0;
}

Vector2::Vector2(const Vector2 &v)
{
    _vector[0] = v._vector[0];
    _vector[1] = v._vector[1];
}

Vector2::Vector2(const Vector3 &v)
{
    _vector[0] = v.x();
    _vector[1] = v.y();
}

Vector2::Vector2(const Vector4 &v)
{
    _vector[0] = v.x();
    _vector[1] = v.y();
}

Vector2::Vector2(const real x, const real y)
{
    _vector[0] = x;
    _vector[1] = y;
}

Vector2::Vector2(const float *v)
{
    _vector[0] = v[0];
    _vector[1] = v[1];
}

Vector2::Vector2(const double *v)
{
    _vector[0] = v[0];
    _vector[1] = v[1];
}

Vector2::Vector2(const string &str)
{
    #ifndef NDEBUG
    const int n =
    #endif
        atoc(str,atof,"+-eE.0123456789",_vector+0,_vector+2);

    assert(n==2);
}

const real &
Vector2::operator[](const int i) const
{
    assert(i>=0 && i<2);
    return _vector[i];
}

real &
Vector2::operator[](const int i)
{
    assert(i>=0 && i<2);
    return _vector[i];
}

Vector2::operator real *()
{
    return (real *) _vector;
}

Vector2::operator const real *() const
{
    return (real *) _vector;
}


      real &Vector2::x()       { return _vector[0]; }
const real &Vector2::x() const { return _vector[0]; }
      real &Vector2::y()       { return _vector[1]; }
const real &Vector2::y() const { return _vector[1]; }

      real &Vector2::s()       { return _vector[0]; }
const real &Vector2::s() const { return _vector[0]; }
      real &Vector2::t()       { return _vector[1]; }
const real &Vector2::t() const { return _vector[1]; }

bool
Vector2::operator==(const Vector2 &v) const
{
    return
        _vector[0] == v[0] &&
        _vector[1] == v[1];
}

bool
Vector2::operator!=(const Vector2 &v) const
{
    return
        _vector[0] != v[0] ||
        _vector[1] != v[1];
}

bool
Vector2::operator==(const real &a) const
{
    return
        _vector[0] == a &&
        _vector[1] == a;
}

bool
Vector2::operator< (const Vector2 &v) const
{
    if (x()!=v.x())
        return x()<v.x();
    else
        return y()<v.y();
}

bool
Vector2::operator> (const Vector2 &v) const
{
    if (x()!=v.x())
        return x()>v.x();
    else
        return y()>v.y();
}

ostream &
Vector2::writePov(ostream &os) const
{
    os << "< ";
    os << x() << ',';
    os << y() << ",0 >";

    return os;
}

Vector2
Vector2::operator-() const
{
    return Vector2(-_vector[0], -_vector[1]);
}

void
Vector2::scale(const real &x)
{
    _vector[0] *= x;
    _vector[1] *= x;
}

void
Vector2::scale(const Vector2 &x)
{
    _vector[0] *= x._vector[0];
    _vector[1] *= x._vector[1];
}

void
Vector2::normalize()
{
    const real mag = sqrt(norm());

    if (mag == 0.0)
        return;

    const real magInv = 1.0/mag;

    _vector[0] *= magInv;
    _vector[1] *= magInv;
}

void
Vector2::abs()
{
    _vector[0] = fabs(_vector[0]);
    _vector[1] = fabs(_vector[1]);
}

int
Vector2::dominant() const
{
    const real x = fabs(_vector[0]);
    const real y = fabs(_vector[1]);

    return (x>y ? 0 : 1);
}

real
Vector2::norm() const
{
    return ((*this)*(*this));
}

real
Vector2::length() const
{
    return sqrt((*this)*(*this));
}

Vector2 &
Vector2::operator=(const Vector2 &x)
{
    _vector[0] = x[0];
    _vector[1] = x[1];

    return *this;
}

Vector2 &
Vector2::operator=(const float *x)
{
    _vector[0] = x[0];
    _vector[1] = x[1];

    return *this;
}

Vector2 &
Vector2::operator=(const double *x)
{
    _vector[0] = x[0];
    _vector[1] = x[1];

    return *this;
}

real
Vector2::operator*(const Vector2 &x) const
{
    return _vector[0]*x[0] + _vector[1]*x[1];
}

Vector2 &
Vector2::operator+=(const Vector2 &v)
{
    _vector[0] += v[0];
    _vector[1] += v[1];

    return *this;
}

Vector2 &
Vector2::operator-=(const Vector2 &x)
{
    _vector[0] -= x[0];
    _vector[1] -= x[1];

    return *this;
}

Vector2 &
Vector2::operator*=(const real &x)
{
    _vector[0] *= x;
    _vector[1] *= x;

    return *this;
}

Vector2 &
Vector2::operator*=(const Matrix &m)
{
    return operator=(m*(*this));
}

real
Vector2::dist(const Vector2 &x) const
{
    return ((*this)-x).norm();
}

Vector2 &
Vector2::vmin(const Vector2 &v)
{
    _vector[0] = MIN(_vector[0],v[0]);
    _vector[1] = MIN(_vector[1],v[1]);
    return *this;
}

Vector2 &
Vector2::vmax(const Vector2 &v)
{
    _vector[0] = MAX(_vector[0],v[0]);
    _vector[1] = MAX(_vector[1],v[1]);
    return *this;
}

///////////////////////// OpenGL

#include <glt/gl.h>

void
Vector2::glVertex() const
{
    #ifdef GLT_FAST_FLOAT
    glVertex2fv(_vector);
    #else
    glVertex2dv(_vector);
    #endif
}

void
Vector2::glTexCoord() const
{
    #ifdef GLT_FAST_FLOAT
    glTexCoord2fv(_vector);
    #else
    glTexCoord2dv(_vector);
    #endif
}

////////////////////////// Friends

/*!
    \brief Output vector to stream
    \ingroup Math
*/

ostream &
operator<<(ostream &os, const Vector2 &x)
{
    os << x[0] << '\t';
    os << x[1];

    return os;
}

/*!
    \brief Input vector from stream
    \ingroup Math
*/

istream &
operator>>(istream &is, Vector2 &x)
{
    is >> x[0];
    is >> x[1];

    return is;
}

/*!
    \brief Vector scale
    \ingroup Math
*/

Vector2
operator*(const real x, const Vector2 &v)
{
    return v*x;
}

/*!
    \brief Vector scale
    \ingroup Math
*/

Vector2
operator*(const Vector2 &v, const real x)
{
    return Vector2( x*v._vector[0], x*v._vector[1]);
}

/*!
    \brief Vector inverse scale
    \ingroup Math
*/

Vector2
operator/(const Vector2 &v, const real x)
{
    assert(x != 0.0);
    const real inv = 1.0/x;
    return v*inv;
}

/*!
    \brief Vector addition
    \ingroup Math
*/

Vector2
operator+(const Vector2 &v1, const Vector2 &v2)
{
    return Vector2(v1[0] + v2[0], v1[1] + v2[1]);
}

/*!
    \brief Vector difference
    \ingroup Math
*/

Vector2
operator-(const Vector2 &v1, const Vector2 &v2)
{
    return Vector2(v1[0] - v2[0], v1[1] - v2[1]);
}

