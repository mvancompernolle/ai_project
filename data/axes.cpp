#include "axes.h"

/*! \file
    \ingroup GlutMaster
*/

#include <glt/matrix.h>
#include <glt/rgb.h>

#include <glutm/shape.h>
#include <glutm/glut.h>

GlutAxes::GlutAxes(const double size)
{
    GltShape *cylinder;
    GltShape *cone;

    const double radius = 0.05;
    const double length = 0.8;

    // Add the component shapes to the container.
    // Smart pointers will ensure that these nodes
    // are deleted when no longer in use.

    push_back(cylinder = new GlutCylinder(true,radius,length));
    push_back(cone = new GlutCone(true,radius*2.0,1.0-length));

    // Position the cone at the end of the cylinder

    cone->transformation() *= matrixTranslate(VectorZ*length);

    // All colours are taken from OpenGL state,
    // rather than the individual nodes

    inheritColor() = true;

    cylinder->inheritColor() = true;
    cone    ->inheritColor() = true;

    // Scale the whole axes by the desired size

    transformation() *= matrixScale(size);
}

GlutAxes::~GlutAxes()
{
}

void
GlutAxes::draw() const
{
    if (!visible())
        return;

    // X Axes

    glPushMatrix();
        glRotateY(90.0);
        red.glColor();
        GltShapes::draw();
    glPopMatrix();

    // Y Axes

    glPushMatrix();
        glRotateX(-90.0);
        green.glColor();
        GltShapes::draw();
    glPopMatrix();

    // Z Axes

    blue.glColor();
    GltShapes::draw();
}
