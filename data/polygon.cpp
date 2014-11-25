#include "polygon.h"

/*! \file
    \ingroup Node
*/

#if 0

using namespace std;

///////////////////////////// GlutShapes ///////////////////////////////////////

GltPolygon::GltPolygon()
{
}

GltPolygon::~GltPolygon()
{
}

void
GltPolygon::draw() const
{
    glPushMatrix();
        transformation().glMultMatrix();

//      for (int i=0; i<size(); i++)
//          if ((*this)[i].get())
//              (*this)[i]->draw();

    glPopMatrix();
}

bool
GltPolygon::boundingBox(Vector &min,Vector &max) const
{
/*
    bool firstBox = true;

    for (int i=0; i<size(); i++)
        if ((*this)[i].get())
        {
            Vector tmin,tmax;

            if (!(*this)[i]->boundingBox(tmin,tmax))
                return false;
            else
            {
                if (firstBox)
                {
                    min = tmin;
                    max = tmax;
                    firstBox = false;
                }
                else
                {
                    min.vmin(tmin);
                    max.vmax(tmax);
                }

            }
        }

    return !firstBox;
*/
    return false;
}

string GltPolygon::description() const
{
    char buffer[1024];
    sprintf(buffer,"Convex polygon.");
    return buffer;
}

#endif
