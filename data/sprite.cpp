#include "sprite.h"

/*! \file
    \ingroup Node
    \todo    Init from GLT image
*/

#include <glt/viewport.h>
#include <glt/texture.h>
#include <glt/error.h>
#include <glt/rgb.h>

#include <misc/string.h>
#include <misc/image.h>

#include <fstream>
using namespace std;

GltSprite::GltSprite()
: _nx(0), _ny(0), _width(0), _height(0), _blend(false)
{
    color() = white;
}

GltSprite::~GltSprite()
{
    clear();
}

bool
GltSprite::init(const string &filename,const bool mipmap)
{
    if (!filename.size())
        return false;

    uint32 width;
    uint32 height;
    string data;

    ifstream is(filename.c_str(),ios::binary);
    readStream(is,data);

    string image;
    if (decode(width,height,image,data))
        return init(width,height,image,mipmap);

    return false;
}

bool
GltSprite::init(const void *buffer,const bool mipmap)
{
    uint32 format,components,width,height;
    string data;

    if (GltTexture::decodeImage(buffer,format,components,width,height,data))
        return init(width,height,data,mipmap);

    return false;
}

bool
GltSprite::init(const GLsizei width,const GLsizei height,const string &image,const bool mipmap)
{
    const GLsizei channels = image.size()/(width*height);
    return init(width,height,reinterpret_cast<const byte *>(&image[0]),channels,mipmap);
}

bool
GltSprite::init(const GLsizei width,const GLsizei height,const byte *image,const GLsizei channels,const bool mipmap)
{
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTextureSize);

    assert(maxTextureSize>0);

    uint32 tileWidth  = MIN(lowerPowerOf2(width) ,maxTextureSize);
    uint32 tileHeight = MIN(lowerPowerOf2(height),maxTextureSize);

    #if 0 // Limit texture size for debugging
    const uint32 testSize = 256;
    tileWidth  = MIN(tileWidth,testSize);
    tileHeight = MIN(tileHeight,testSize);
    #endif

    _nx = width /tileWidth;
    _ny = height/tileHeight;

    if ((_nx*tileWidth)!=uint32(width))
        _nx++;

    if ((_ny*tileHeight)!=uint32(height))
        _ny++;

    _width = width;
    _height = height;

    //

    clear();

    _texture.resize(_nx*_ny);

    uint32 k=0;
    for (uint32 j=0; j<_ny; j++)
    {
        for (uint32 i=0; i<_nx; i++)
        {
            // TODO - Handle right and top tiles
            GltTexturePtr tile(new GltTexture());
            tile->setWrap(GL_CLAMP,GL_CLAMP);
            tile->setRowLength(width);

            // Determine location of bottom left pixel
            // of texture tile
            const GLubyte *pixels = image;

            if (i+1<_nx)
                pixels += channels*(i*tileWidth);
            else
                pixels += channels*(width-tileWidth);

            if (j+1<_ny)
                pixels += channels*(j*tileHeight*width);
            else
                pixels += channels*(height-tileHeight)*width;

            tile->init(tileWidth,tileHeight,pixels,channels,mipmap);
            _texture[k++] = tile;
        }
    }

    return true;
}

void
GltSprite::clear()
{
    // Clear and release each texture
    for (uint32 i=0; i<_texture.size(); i++)
        _texture[i]->clear();

    // Clear container of texture objects
    _texture.clear();
}
void
GltSprite::draw() const
{
    // Check that our textures are valid
    if (_texture.size()==0)
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

        glPushMatrix();
        transformation().glMultMatrix();

        // Current x,y position
        uint32 x=0;
        uint32 y=0;

        // Width and height of current texture
        GLsizei width  = 0;
        GLsizei height = 0;

        uint32 k=0;
        for (uint32 j=0; j<_ny; j++)
        {
            for (uint32 i=0; i<_nx; i++)
            {
                width  = _texture[k]->width();
                height = _texture[k]->height();
                _texture[k]->set();

                glPushMatrix();

                    glTranslatef(float(x),float(y),0);

                    const GLdouble l = (i+1==_nx ? 1.0 - (_width-x)/GLdouble(width) : 0.0);
                    const GLdouble r = 1.0;
                    const GLdouble b = (j+1==_ny ? 1.0 - (_height-y)/GLdouble(height) : 0.0);
                    const GLdouble t = 1.0;

                    const uint32 w = MIN(uint32(width), _width-x);
                    const uint32 h = MIN(uint32(height),_height-y);

                    glBegin(GL_POLYGON);
                        glTexCoord2d(l,b); glVertex2i(0,0);
                        glTexCoord2d(r,b); glVertex2i(w,0);
                        glTexCoord2d(r,t); glVertex2i(w,h);
                        glTexCoord2d(l,t); glVertex2i(0,h);
                    glEnd();

                glPopMatrix();

                x += width;
                k++;
            }

            x = 0;
            y += height;
        }

        glPopMatrix();

    glPopAttrib();

    GLERROR;
}

bool   GltSprite::empty()  const { return _texture.size()==0; }
uint32 GltSprite::width()  const { return _width;             }
uint32 GltSprite::height() const { return _height;            }

      bool &GltSprite::blend()       { return _blend; }
const bool &GltSprite::blend() const { return _blend; }

