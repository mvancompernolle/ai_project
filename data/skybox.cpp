#include "skybox.h"

/*! \file
    \ingroup Node
*/

#include <math/matrix4.h>
#include <math/umatrix.h>

#include <glt/rgb.h>
#include <glt/glu.h>
#include <glt/error.h>
#include <glt/viewport.h>

GltSkyBox::GltSkyBox()
: _fov(65.0)
{
    color() = white;

    _positiveX.setWrap(GL_CLAMP,GL_CLAMP);
    _negativeX.setWrap(GL_CLAMP,GL_CLAMP);
    _positiveY.setWrap(GL_CLAMP,GL_CLAMP);
    _negativeY.setWrap(GL_CLAMP,GL_CLAMP);
    _positiveZ.setWrap(GL_CLAMP,GL_CLAMP);
    _negativeZ.setWrap(GL_CLAMP,GL_CLAMP);
}

GltSkyBox::~GltSkyBox()
{
    clear();
}

void
GltSkyBox::clear()
{
    _positiveX.clear();
    _negativeX.clear();
    _positiveY.clear();
    _negativeY.clear();
    _positiveZ.clear();
    _negativeZ.clear();
}

void
GltSkyBox::draw() const
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

    // Estimate vertical angle based on viewers distance from
    // on-screen window.  TODO - Make use of maximum screen size...

//  const double alpha = atan(double(viewport.height())/1600.0)*M_DEG_PI;
//  gluPerspective(alpha,double(viewport.width())/double(viewport.height()),0.1,5.0);

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

    //
    //
    //

        GLERROR

        glPushAttrib(GL_ENABLE_BIT|GL_POLYGON_BIT);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glEnable(GL_CULL_FACE);

            if (solid())
            {
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                glColor();
                glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
            }
            else
            {
                white.glColor();
                glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
            }

            // +X

            _positiveX.set();
            glBegin(GL_QUADS);
                glTexCoord2f(0,0);  glVertex3f( 1,-1,-1);
                glTexCoord2f(1,0);  glVertex3f( 1,-1, 1);
                glTexCoord2f(1,1);  glVertex3f( 1, 1, 1);
                glTexCoord2f(0,1);  glVertex3f( 1, 1,-1);
            glEnd();

            // -X

            _negativeX.set();
            glBegin(GL_QUADS);
                glTexCoord2f(0,0);  glVertex3f(-1,-1, 1);
                glTexCoord2f(1,0);  glVertex3f(-1,-1,-1);
                glTexCoord2f(1,1);  glVertex3f(-1, 1,-1);
                glTexCoord2f(0,1);  glVertex3f(-1, 1, 1);
            glEnd();

            // +Y

            _positiveY.set();
            glBegin(GL_QUADS);
                glTexCoord2f(0,0);  glVertex3f(-1, 1,-1);
                glTexCoord2f(1,0);  glVertex3f( 1, 1,-1);
                glTexCoord2f(1,1);  glVertex3f( 1, 1, 1);
                glTexCoord2f(0,1);  glVertex3f(-1, 1, 1);
            glEnd();

            // -Y

            _negativeY.set();
            glBegin(GL_QUADS);
                glTexCoord2f(0,0);  glVertex3f(-1,-1, 1);
                glTexCoord2f(1,0);  glVertex3f( 1,-1, 1);
                glTexCoord2f(1,1);  glVertex3f( 1,-1,-1);
                glTexCoord2f(0,1);  glVertex3f(-1,-1,-1);
            glEnd();

            // +Z

            _positiveZ.set();
            glBegin(GL_QUADS);
                glTexCoord2f(0,0);  glVertex3f( 1,-1, 1);
                glTexCoord2f(1,0);  glVertex3f(-1,-1, 1);
                glTexCoord2f(1,1);  glVertex3f(-1, 1, 1);
                glTexCoord2f(0,1);  glVertex3f( 1, 1, 1);
            glEnd();

            // -Z

            _negativeZ.set();
            glBegin(GL_QUADS);
                glTexCoord2f(0,0);  glVertex3f(-1,-1,-1);
                glTexCoord2f(1,0);  glVertex3f( 1,-1,-1);
                glTexCoord2f(1,1);  glVertex3f( 1, 1,-1);
                glTexCoord2f(0,1);  glVertex3f(-1, 1,-1);
            glEnd();

        glPopAttrib();

        GLERROR

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    GLERROR
}

GltTexture &GltSkyBox::positiveX() { return _positiveX; }
GltTexture &GltSkyBox::negativeX() { return _negativeX; }
GltTexture &GltSkyBox::positiveY() { return _positiveY; }
GltTexture &GltSkyBox::negativeY() { return _negativeY; }
GltTexture &GltSkyBox::positiveZ() { return _positiveZ; }
GltTexture &GltSkyBox::negativeZ() { return _negativeZ; }

      GLdouble &GltSkyBox::fov()        { return _fov; }
const GLdouble  GltSkyBox::fov() const  { return _fov; }
