#include "point.h"

/*! \file
    \ingroup Math

    $Id: point.cpp,v 1.8 2003/03/06 12:29:59 nigels Exp $

    $Log: point.cpp,v $
    Revision 1.8  2003/03/06 12:29:59  nigels
    *** empty log message ***

*/

#include "matrix4.h"

Point::Point()
: Vector(Vector0), _normal(Vector0), _texture(Vector0)
{
}

Point::Point(const Vector &pos,const Vector &normal)
: Vector(pos), _normal(normal), _texture(Vector0)
{
}

Point::Point(const Vector &pos,const Vector &normal,const Vector &texture)
: Vector(pos), _normal(normal), _texture(texture)
{
}

Point::Point(const double &x,const double &y,const double &z)
: Vector(x,y,z), _normal(Vector0), _texture(Vector0)
{
}

      Vector &Point::position()       { return *this; }
const Vector &Point::position() const { return *this; }

      Vector &Point::normal()         { return _normal; }
const Vector &Point::normal() const   { return _normal; }

      Vector &Point::texture()        { return _texture; }
const Vector &Point::texture() const  { return _texture; }

bool
Point::operator==(const Point &vector) const
{
    if (this==&vector)
        return true;

    return
        Vector::operator==(vector) &&
        _normal ==vector._normal   &&
        _texture==vector._texture;
}

bool
Point::operator!=(const Point &vector) const
{
    if (this==&vector)
        return false;

    return
        Vector::operator!=(vector) ||
        _texture!=vector._texture  ||
        _normal!=vector._normal;
}


void
Point::transform(const Matrix &trans)
{
    position() = trans * position();
    normal()  = trans * normal() - trans * Vector0;
}
