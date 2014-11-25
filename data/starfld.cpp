#include "starfld.h"

/*! \file
    \ingroup Node
*/

#include <glt/rgb.h>

#include <node/line.h>
#include <node/interp.h>

#include <math/random.h>

GltStarField::GltStarField(const int n,const int layers)
{
    double duration = 60.0;

    for (int i=0; i<layers; i++)
    {
        GltLine *stars = new GltLine();
        stars->solid() = false;
        stars->color() = white;

        GltRandomSphere<> pos(3.0);

        for (int j=0; j<n; j++)
                stars->points().push_back(pos.rand());

        GltInterpolatorRotation *interp = new GltInterpolatorRotation(stars->transformation());
        interp->axis() = VectorY;
        interp->start(duration,true);

        push_back(interp);
        push_back(stars);

        // Each subsequent 'layer' is faster
        duration *= 0.7;
    }
}

GltStarField::~GltStarField()
{
}

