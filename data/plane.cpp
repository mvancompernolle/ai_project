#include "plane.h"

/*! \file
    \ingroup Math

    $Id: plane.cpp,v 1.10 2003/03/06 12:29:58 nigels Exp $

    $Log: plane.cpp,v $
    Revision 1.10  2003/03/06 12:29:58  nigels
    *** empty log message ***

*/

#include "matrix4.h"
#include "bbox.h"

Plane::Plane()
: _pos(Vector0), _direction(VectorZ), _d(0.0)
{
}

Plane::Plane(const Vector &pos,const Vector &dir)
: _pos(pos), _direction(dir), _d(0.0)
{
    _d = - (_pos*_direction);
}

const Vector &Plane::direction() const { return _direction; }
      Vector &Plane::direction()       { return _direction; }

double  Plane::d() const { return _d; }
double &Plane::d()       { return _d; }

bool
Plane::operator==(const Plane &plane) const
{
    return (plane.direction()==_direction && plane.d()==_d );
}

void
Plane::transform(const Matrix &trans)
{
    _pos       = trans * _pos;
    _direction = trans * _direction - trans * Vector0;
    _d = - (_pos*_direction);
}

void
Plane::flip()
{
    _direction  = _direction * -1;
    _d         *= -1;
}

real
Plane::dist(const Vector &pos) const
{
    return (pos*_direction)+_d;
}

bool
Plane::inside(const Vector &pos) const
{
    return dist(pos)>=0.0;
}

real
Plane::dist(const BoundingBox &box) const
{
    // Algorithm from Real-Time Rendering book
    // Page 311 Plane/Box Intersection Detection

    // vMin and vMax are corners of the axis-aligned
    // box in the direction of the plane normal.

    // Returns 0.0 iff plane touches box

    Vector vMin,vMax;

    for (int i=0; i<3; i++)
        if (_direction[i]>=0.0)
        {
            vMin[i] = box.min()[i];
            vMax[i] = box.max()[i];
        }
        else
        {   vMin[i] = box.max()[i];
            vMax[i] = box.min()[i];
        }

    const real vMinDist = dist(vMin);

    if (vMinDist>0.0)
        return vMinDist;

    const real vMaxDist = dist(vMax);

    if (vMaxDist>=0.0)
        return 0.0;

    return vMaxDist;
}

real
Plane::intersect(const Vector &p0,const Vector &p1) const
{
    const real d0 = dist(p0);
    const real d1 = dist(p1);

    return d0/(d0-d1);
}

Vector
Plane::intersectPosition(const Vector &p0,const Vector &p1) const
{
    const real t = intersect(p0,p1);
    return p0 + (p1-p0)*t;
}
