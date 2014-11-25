#include "fade.h"

/*! \file
    \ingroup Node
*/

#include <glt/glu.h>
#include <glt/rgb.h>
#include <glt/error.h>

//////////////////////// GltFadeViewport ///////////////////////////////////

GltFadeViewport::GltFadeViewport()
: _border(false), _borderColor(black)
{
    color().alpha() = 0.0;
}

GltFadeViewport::~GltFadeViewport()
{
}

          bool &GltFadeViewport::border()            { return _border;      }
    const bool &GltFadeViewport::border()      const { return _border;      }
      GltColor &GltFadeViewport::borderColor()       { return _borderColor; }
const GltColor &GltFadeViewport::borderColor() const { return _borderColor; }

void
GltFadeViewport::draw() const
{
    GLERROR

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // Use the current OpenGL viewport for
    // viewing system

    GltViewport viewport(true);

    gluOrtho2D(0,viewport.width(),0,viewport.height());

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        // Use desired viewport for fading

        if (color().alpha()!=0.0)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            color().glColor();
            viewport.drawQuad();
        }

        if (_border)
        {
            glDisable(GL_BLEND);
            _borderColor.glColor();
            viewport.drawLines();
        }

    glPopAttrib();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    GLERROR
}
