#include "shape.h"

/*! \file
    \ingroup Node
*/

using namespace std;

//////////////

GltShape::GltShape(const bool solid)
: _solid(solid),
  _visible(true),
  _convexity(1),
  _inheritColor(false),
  _inheritSolid(false),
  _expired(false)
{
}

GltShape::~GltShape()
{
}

GltFieldPtr
GltShape::settings()
{
    GltFields *root = new GltFields(name());

    root->add(_visible       ,"visible"       );
    root->add(_solid         ,"solid"         );
    root->add(_name          ,"name"          );
    root->add(_color         ,"color"         );
    root->add(_transformation,"transformation");

    return root;
}

GLdouble GltShape::volume() const                       { return 0.0;      }
string   GltShape::description() const                  { return name();   }
bool     GltShape::povrayExport(ostream &os) const      { return false;    }

Matrix    &GltShape::transformation() { return _transformation; }
GltColor  &GltShape::color()          { return _color;          }
bool      &GltShape::solid()          { return _solid;          }
bool      &GltShape::visible()        { return _visible;        }
string    &GltShape::name()           { return _name;           }
uint32    &GltShape::convexity()      { return _convexity;      }
bool      &GltShape::inheritColor()   { return _inheritColor;   }
bool      &GltShape::inheritSolid()   { return _inheritSolid;   }
bool      &GltShape::expired()        { return _expired;        }

const Matrix    &GltShape::transformation() const { return _transformation; }
const GltColor  &GltShape::color()          const { return _color;          }
const bool      &GltShape::solid()          const { return _solid;          }
const bool      &GltShape::visible()        const { return _visible;        }
const string    &GltShape::name()           const { return _name;           }
const uint32    &GltShape::convexity()      const { return _convexity;      }
const bool      &GltShape::inheritColor()   const { return _inheritColor;   }
const bool      &GltShape::inheritSolid()   const { return _inheritSolid;   }
const bool      &GltShape::expired()        const { return _expired;        }

void
GltShape::flushExpired()
{
}

void
GltShape::boundingBox(BoundingBox &box) const
{
    // By default, the bounding box is
    // undefined.

    box.reset();
}

bool
GltShape::inside(const Vector &pos) const
{
    return false;
}

void
GltShape::glColor() const
{
    if (!_inheritColor)
        _color.glColor();
}

void
GltShape::transformPoints(vector<Vector> &v) const
{
    for (uint32 i=0; i<v.size(); i++)
        v[i] = _transformation * v[i];
}

bool
GltShape::intersects(const GltShape &shape) const
{
    BoundingBox box1,box2;

    boundingBox(box1);
    if (!box1.defined())
        return false;

    shape.boundingBox(box2);
    if (!box2.defined())
        return false;

    return box1.intersects(box2);
}

