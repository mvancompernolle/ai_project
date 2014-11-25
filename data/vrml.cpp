#include "vrml.h"

/*! \file
    \ingroup Mesh

    $Id: vrml.cpp,v 1.6 2003/05/31 11:38:27 nigels Exp $

    $Log: vrml.cpp,v $
    Revision 1.6  2003/05/31 11:38:27  nigels
    *** empty log message ***

    Revision 1.5  2003/03/06 12:31:14  nigels
    *** empty log message ***

*/

#include <glt/gl.h>
#include <glt/rgb.h>
#include <glt/error.h>
#include <glt/material.h>

using namespace std;

IndexedFaceSet::IndexedFaceSet()
: normalPerVertex(true),
  colorPerVertex(true),
  ccw(true),
  convex(true),
  solid(true),
  creaseAngle(0.0)
{
}

IndexedFaceSet::~IndexedFaceSet()
{
}

void
IndexedFaceSet::draw() const
{
}

//

Material::Material()
: ambientIntensity(0.2f),
  diffuseColor(grey80),
  emissiveColor(black),
  shininess(0.2f),
  specularColor(black),
  transparency(0.0f)
{
}

Material::~Material()
{
}

void
Material::draw() const
{
}

//

Shape::Shape()
{
}

Shape::~Shape()
{
}

void
Shape::draw() const
{
    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LIGHTING_BIT);
    glPushMatrix();

        transformation().glMultMatrix();

        if (material.get())
        {
            GLERROR

            glDisable(GL_COLOR_MATERIAL);

            GltMaterial mat(GL_FRONT_AND_BACK);

            mat.ambient().red() = mat.ambient().green() = mat.ambient().blue() = material->ambientIntensity;
            mat.diffuse()       = material->diffuseColor;
            mat.specular()      = material->specularColor;
            mat.emission()      = material->emissiveColor;
            mat.shininess()     = material->shininess;

            if (material->transparency!=0.0f)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

                mat.diffuse().alpha() = 1.0f - material->transparency;
            }

            mat.set();

            GLERROR
        }

        if (faces.get())
        {
            if (faces->solid)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);

            // TODO - Complete debug range checking, quad handling

            glBegin(GL_TRIANGLES);
            for (uint32 i=0; i+2<faces->coordIndex.size(); i+=4)
            {
                const int index[3] =
                {
                        faces->coordIndex[i],
                        faces->coordIndex[i+1],
                        faces->coordIndex[i+2]
                };

                if (index<0)
                    continue;

                assert(index[0]<faces->coord.size());
                assert(index[1]<faces->coord.size());
                assert(index[2]<faces->coord.size());

                const Vector &a = faces->coord[index[0]];
                const Vector &b = faces->coord[index[1]];
                const Vector &c = faces->coord[index[2]];

                if (faces->normal.size())
                {
                    const Vector &na = faces->normal[index[0]];
                    const Vector &nb = faces->normal[index[1]];
                    const Vector &nc = faces->normal[index[2]];

                    na.glNormal();
                     a.glVertex();
                    nb.glNormal();
                     b.glVertex();
                    nc.glNormal();
                     c.glVertex();
                }
                else
                {
                    xProduct(b-a,c-a).glNormal();
                    a.glVertex();
                    b.glVertex();
                    c.glVertex();
                }
            }
            glEnd();
        }

    glPopMatrix();
    glPopAttrib();
}

void
Shape::boundingBox(BoundingBox &box) const
{
    box.reset();

    if (faces.get())
    {
        vector<Vector> p = faces->coord;
        transformPoints(p);
        box += p;
    }
}

//

Transform::Transform()
: center(0,0,0),
  rotation(0,0,1,0),
  scale(1,1,1),
  scaleOrientation(0,0,1,0),
  translation(Vector0),
  bboxCenter(0,0,0),
  bboxSize(-1,-1,-1)
{
}

Transform::~Transform()
{
}

void
Transform::draw() const
{
}
