#include "shapes.h"

/*! \file
    \ingroup Node
*/

#include <math/vector3.h>
#include <math/matrix4.h>

#include <cstdio>
#include <iostream>
using namespace std;

///////////////////////////// GlutShapes ///////////////////////////////////////

GltShapes::GltShapes()
{
    reserve(3);         // Conserve memory
}

GltShapes::~GltShapes()
{
}

void
GltShapes::draw() const
{
    if (!visible())
        return;

    glPushMatrix();

        glColor();
        transformation().glMultMatrix();

        for (uint32 i=0; i<size(); i++)
            if ((*this)[i].get())
                (*this)[i]->draw();

    glPopMatrix();
}

void
GltShapes::boundingBox(BoundingBox &box) const
{
    box.reset();

    for (uint32 i=0; i<size(); i++)
        if ((*this)[i].get())
        {
            BoundingBox tmp;
            (*this)[i]->boundingBox(tmp);
            box += tmp;
        }
}

string GltShapes::description() const
{
    char buffer[1024];
    sprintf(buffer,"Shape container with %d elements.",int(size()));
    return buffer;
}

bool
GltShapes::povrayExport(ostream &os) const
{
    os << "union { " << endl;

    for (uint32 i=0; i<size(); i++)
        (*this)[i]->povrayExport(os);

    transformation().writePov(os) << endl;
    os << "}" << endl;

    return true;
}

void
GltShapes::flushExpired()
{
    for (uint32 i=0; i<size(); )
    {
        if ((*this)[i]->expired())
            erase(begin()+i);
        else
        {
            (*this)[i]->flushExpired();
            i++;
        }
    }
}
