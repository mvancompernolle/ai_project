#include "skyspher.h"

/*! \file
    \brief   OpenGL Sky Sphere Node
    \ingroup Node
*/

#include <glt/gl.h>
#include <glt/glu.h>

#include <glt/viewport.h>
#include <glt/error.h>
#include <glt/rgb.h>

template<class T>
void cosTable(T *val,const T min,const T max,const uint32 n)
{
    assert(val);
    const T delta = (max-min)/n;
    T j = min;
    uint32 i;
    for (i=0; i<n-1; i++,j+=delta)
        val[i] = static_cast<T>(cos(j));
    val[i] = static_cast<T>(cos(max));
}

template<class T>
void sinTable(T *val,const T min,const T max,const uint32 n)
{
    assert(val);
    const T delta = (max-min)/n;
    T j = min;
    uint32 i;
    for (i=0; i<n-1; i++,j+=delta)
        val[i] = static_cast<T>(sin(j));
    val[i] = static_cast<T>(sin(max));
}


GltSkySphere::GltSkySphere()
: _slices(18), _fov(65.0)
{
}

GltSkySphere::~GltSkySphere()
{
}

void
GltSkySphere::draw() const
{
    if (!visible())
        return;

    GLERROR

    GltViewport viewport(true);

    //
    // Setup perspective camera mode,
    // orthogonal doesn't really work...
    //

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    gluPerspective(_fov,double(viewport.width())/double(viewport.height()), 0.1, 200.0);

    //
    // Twiddle the current modelview matrix
    // to cancel out translation and scale.
    //

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    Matrix      matrix(GL_MODELVIEW_MATRIX);

    // No translation

    matrix[12] = 0.0;
    matrix[13] = 0.0;
    matrix[14] = 0.0;

    // No scale

    const real sf = sqrt( sqr(matrix[0]) + sqr(matrix[1]) + sqr(matrix[2]) );
    matrix *= matrixScale(1.0/sf);

    //

    matrix.glLoadMatrix();
    transformation().glMultMatrix();

    //
    //
    //

        GLERROR

        glPushAttrib(GL_ENABLE_BIT|GL_POLYGON_BIT|GL_LIGHTING_BIT);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_CULL_FACE);

            if (solid())
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
                glShadeModel(GL_SMOOTH);
            }
            else
            {
                glDisable(GL_BLEND);
                glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
                glShadeModel(GL_FLAT);
            }

            //

            drawSphere(_map,_slices);

            //

        glPopAttrib();

        GLERROR

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    GLERROR
}

void
GltSkySphere::drawSphere(const GltColorMap &colmap,const int slices)
{
    if (colmap.size()<2)
        return;

    glBegin(GL_QUADS);

    float *x = new float[slices];
    float *z = new float[slices];
    sinTable<float>(x,0.0f,float(M_2PI),slices);
    cosTable<float>(z,0.0f,float(M_2PI),slices);

    for (int i=0; i<colmap.size()-1; i++)
    {
        const real a1 = colmap.lookup(i);
        const real a2 = colmap.lookup(i+1);

        const float y1 = (float) cos(a1);
        const float y2 = (float) cos(a2);
        const float s1 = (float) sin(a1);
        const float s2 = (float) sin(a2);

        const GltColor c1 = colmap.lookup(a1);
        const GltColor c2 = colmap.lookup(a2);

        for (int j=0; j<slices-1; j++)
        {
            const float &x1 = x[j];
            const float &x2 = x[j+1];
            const float &z1 = z[j];
            const float &z2 = z[j+1];

            c1.glColor();
            glVertex3f(x1*s1,y1,z1*s1);
            glVertex3f(x2*s1,y1,z2*s1);
            c2.glColor();
            glVertex3f(x2*s2,y2,z2*s2);
            glVertex3f(x1*s2,y2,z1*s2);
        }
    }

    delete [] x;
    delete [] z;

    glEnd();
}

void
GltSkySphere::clear()
{
    _map.clear();
}

   GltColorMap &GltSkySphere::map()       { return _map; }

      GLdouble &GltSkySphere::fov()       { return _fov; }
const GLdouble  GltSkySphere::fov() const { return _fov; }

