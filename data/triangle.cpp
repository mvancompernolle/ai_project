#include "triangle.h"

/*! \file
    \ingroup Mesh

    $Id: triangle.cpp,v 1.8 2003/03/06 12:31:14 nigels Exp $

    $Log: triangle.cpp,v $
    Revision 1.8  2003/03/06 12:31:14  nigels
    *** empty log message ***

*/

#include <iostream>
using namespace std;

Triangle::Triangle()
: _normal(Vector0)
{
    _vertex[0] = _vertex[1] = _vertex[2] = NULL;
}

Triangle::Triangle(Point *a,Point *b,Point *c)
{
    _vertex[0] = a;
    _vertex[1] = b;
    _vertex[2] = c;

    _normal = planeNormal(*a,*b,*c);
}

Point *
Triangle::operator[](int index)
{
    return (index>=0 && index<=2 ? _vertex[index] : (Point *) NULL);
}

const Point *
Triangle::operator[](int index) const
{
    return (index>=0 && index<=2 ? _vertex[index] : (Point *) NULL);
}

      Vector &Triangle::normal()       { return _normal; }
const Vector &Triangle::normal() const { return _normal; }

Plane
Triangle::plane() const
{
    return Plane(*_vertex[0],_normal);
}

bool
Triangle::operator==(const Triangle &tri) const
{
    const Vector &a = *_vertex[0];
    const Vector &b = *_vertex[1];
    const Vector &c = *_vertex[2];

    const Vector &x = *tri._vertex[0];
    const Vector &y = *tri._vertex[1];
    const Vector &z = *tri._vertex[2];

    if (a==x)
    {
        if (b==y && c==z)
            return true;

        if (b==z && c==y)
            return true;

        return false;
    }

    if (a==y)
    {
        if (b==x && c==z)
            return true;

        if (b==z && c==x)
            return true;

        return false;
    }

    if (a==z)
    {
        if (b==y && c==x)
            return true;

        if (b==x && c==y)
            return true;

        return false;
    }

    return false;
}

bool
Triangle::operator!=(const Triangle &tri) const
{
    const Vector &a = *_vertex[0];
    const Vector &b = *_vertex[1];
    const Vector &c = *_vertex[2];

    const Vector &x = *tri._vertex[0];
    const Vector &y = *tri._vertex[1];
    const Vector &z = *tri._vertex[2];

    if (a!=x && a!=y && a!=z)
        return true;

    if (b!=x && b!=y && b!=z)
        return true;

    if (c!=x && c!=y && c!=z)
        return true;

    return false;
}

void
Triangle::glDraw() const
{
    if (_vertex[0]->normal()==Vector0)
    {
        _normal.glNormal();
        _vertex[0]->texture().glTexCoord();
        _vertex[0]->glVertex();
        _vertex[1]->texture().glTexCoord();
        _vertex[1]->glVertex();
        _vertex[2]->texture().glTexCoord();
        _vertex[2]->glVertex();
    }
    else
    {
        _vertex[0]->normal() .glNormal();
        _vertex[0]->texture().glTexCoord();
        _vertex[0]->glVertex();
        _vertex[1]->normal() .glNormal();
        _vertex[1]->texture().glTexCoord();
        _vertex[1]->glVertex();
        _vertex[2]->normal() .glNormal();
        _vertex[2]->texture().glTexCoord();
        _vertex[2]->glVertex();
    }
}

ostream &
Triangle::writePOV(ostream &os) const
{
    if (_vertex[0]->normal()==Vector0)
    {
      os << "triangle { ";

          os << "< ";
          os << (*this)[0]->x() << ',';
          os << (*this)[0]->y() << ',';
          os << (*this)[0]->z();
          os << " >,";

          os << "< ";
          os << (*this)[1]->x() << ',';
          os << (*this)[1]->y() << ',';
          os << (*this)[1]->z();
          os << " >,";

          os << "< ";
          os << (*this)[2]->x() << ',';
          os << (*this)[2]->y() << ',';
          os << (*this)[2]->z();
          os << " >";

      os << "} " << endl;
    }
    else
    {
      os << "smooth_triangle { ";

          os << "< ";
          os << (*this)[0]->x() << ',';
          os << (*this)[0]->y() << ',';
          os << (*this)[0]->z();
          os << " >, ";

          os << "< ";
          os << (*this)[0]->normal().x() << ',';
          os << (*this)[0]->normal().y() << ',';
          os << (*this)[0]->normal().z();
          os << " >, ";

          os << "< ";
          os << (*this)[1]->x() << ',';
          os << (*this)[1]->y() << ',';
          os << (*this)[1]->z();
          os << " >, ";

          os << "< ";
          os << (*this)[1]->normal().x() << ',';
          os << (*this)[1]->normal().y() << ',';
          os << (*this)[1]->normal().z();
          os << " >, ";

          os << "< ";
          os << (*this)[2]->x() << ',';
          os << (*this)[2]->y() << ',';
          os << (*this)[2]->z();
          os << " >, ";

          os << "< ";
          os << (*this)[2]->normal().x() << ',';
          os << (*this)[2]->normal().y() << ',';
          os << (*this)[2]->normal().z();
          os << " > ";

        os << "} " << endl;
    }
    return os;
}
