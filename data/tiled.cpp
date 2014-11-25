#include "tiled.h"

/*! \file
    \ingroup Node
*/

#include <glt/viewport.h>
#include <glt/texture.h>
#include <glt/error.h>
#include <glt/rgb.h>

GltTextureViewport::GltTextureViewport(const GltTexture &texture,const bool tile,const bool blend)
: _texture(texture), _tile(tile), _blend(blend)
{
    color() = white;
}

GltTextureViewport::~GltTextureViewport()
{
}

void
GltTextureViewport::draw() const
{
    // Check that our texture is valid
    if (_texture.id()==0)
        return;

    // Get the current viewport from OpenGL
    GltViewport viewport(true);

    // Give up if viewport is zero width or height
    if (viewport.width()==0 || viewport.height()==0)
        return;

    GLERROR

    glPushAttrib(GL_ENABLE_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        if (_blend)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        }

        glColor();

        _texture.set();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0,viewport.width(),0,viewport.height(),-1,1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        if (_tile)
        {
            const GLdouble iw = GLdouble(viewport.width()) /GLdouble(_texture.width());
            const GLdouble ih = GLdouble(viewport.height())/GLdouble(_texture.height());

            glBegin(GL_POLYGON);
                glTexCoord2d( 0, 0);    glVertex2i(0,               0);
                glTexCoord2d(iw, 0);    glVertex2i(viewport.width(),0);
                glTexCoord2d(iw,ih);    glVertex2i(viewport.width(),viewport.height());
                glTexCoord2d( 0,ih);    glVertex2i(0,               viewport.height());
            glEnd();
        }
        else
        {
            glBegin(GL_POLYGON);
                glTexCoord2i(0,0);      glVertex2i(0,               0);
                glTexCoord2i(1,0);      glVertex2i(_texture.width(),0);
                glTexCoord2i(1,1);      glVertex2i(_texture.width(),_texture.height());
                glTexCoord2i(0,1);      glVertex2i(0,               _texture.height());
            glEnd();
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

    glPopAttrib();

    GLERROR;
}
