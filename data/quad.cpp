#include "quad.h"

/*! \file
    \ingroup Mesh

    $Id: quad.cpp,v 1.9 2003/05/10 17:01:35 nigels Exp $

    $Log: quad.cpp,v $
    Revision 1.9  2003/05/10 17:01:35  nigels
    Tidy and document

    Revision 1.8  2003/03/06 12:31:14  nigels
    *** empty log message ***

*/

#include <iostream>
using namespace std;

Quad::Quad()
: _normal(Vector0)
{
    _vertex[0] = _vertex[1] = _vertex[2] = _vertex[3] = NULL;
}

Quad::Quad(Point *a,Point *b,Point *c,Point *d)
{
    _vertex[0] = a;
    _vertex[1] = b;
    _vertex[2] = c;
    _vertex[3] = d;

    _normal = planeNormal(*a,*b,*c);
}

Point *
Quad::operator[](int index)
{
    return (index>=0 && index<=3 ? _vertex[index] : (Point *) NULL);
}

const Point *
Quad::operator[](int index) const
{
    return (index>=0 && index<=3 ? _vertex[index] : (Point *) NULL);
}

      Vector &Quad::normal()       { return _normal; }
const Vector &Quad::normal() const { return _normal; }

Plane
Quad::plane() const
{
    return Plane(*_vertex[0],_normal);
}

bool
Quad::operator==(const Quad &quad) const
{
    const Vector &a = *_vertex[0];
    const Vector &b = *_vertex[1];
    const Vector &c = *_vertex[2];
    const Vector &d = *_vertex[3];

    const Vector &w = *quad._vertex[0];
    const Vector &x = *quad._vertex[1];
    const Vector &y = *quad._vertex[2];
    const Vector &z = *quad._vertex[3];

    if (a==w)
    {
        if (c!=y)
            return false;
        if (b==x && d==z)
            return true;
        if (b==z && d==x)
            return true;
        return false;
    }

    if (a==x)
    {
        if (c!=z)
            return false;
        if (b==y && d==w)
            return true;
        if (b==w && d==y)
            return true;
        return false;
    }

    if (a==y)
    {
        if (c!=w)
            return false;
        if (b==z && d==x)
            return true;
        if (b==x && d==z)
            return true;
        return false;
    }

    if (a==z)
    {
        if (c!=x)
            return false;
        if (b==w && d==y)
            return true;
        if (b==y && d==w)
            return true;
        return false;
    }

    return false;
}

bool
Quad::operator!=(const Quad &quad) const
{
    return !operator==(quad);
}

void
Quad::glDraw() const
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
        _vertex[3]->texture().glTexCoord();
        _vertex[3]->glVertex();
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
        _vertex[3]->normal() .glNormal();
        _vertex[3]->texture().glTexCoord();
        _vertex[3]->glVertex();
    }
}

ostream &
Quad::writePOV(ostream &os) const
{
    // TODO

    return os;
}
