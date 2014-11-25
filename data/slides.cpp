#include "slides.h"

#include <glt/rgb.h>
#include <glt/error.h>
#include <glt/viewport.h>

using namespace std;

/*! \file
    \ingroup Node
*/

GltSlides::GltSlides()
: _preload(false),
  _current(0),
  _cacheSize(5),
  _autoScale(true),
  _preserveAspectRatio(true),
  _blend(true)
{
    color() = white;
}

GltSlides::~GltSlides()
{
    clear();
}

void
GltSlides::clear()
{
    for (uint32 i=0; i<_preloadSprite.size(); i++)
        _preloadSprite[i].clear();
    _preloadSprite.clear();
    _slideCache.clear();
}

void
GltSlides::init()
{
    clear();

    // Read all the files and initialise sprites

    if (_preload)
    {
        _preloadSprite.resize(_slideFilename.size());
        for (uint32 i=0; i<_slideFilename.size(); i++)
        {
            _preloadSprite[i].inheritColor() = true;
            _preloadSprite[i].blend() = _blend;

            if (_slideFilename[i].size())
                _preloadSprite[i].init(_slideFilename[i],false);
            else
                _preloadSprite[i].init(_slideBuffer[i],false);
        }
    }
}

void
GltSlides::draw() const
{
    GLERROR

    // Get the current viewport from OpenGL
    GltViewport viewport(true);

    // Give up if viewport is zero width or height
    if (viewport.width()==0 || viewport.height()==0)
        return;

    glColor();

    glPushAttrib(GL_LIGHTING_BIT|GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0,viewport.width(),0,viewport.height(),-1,1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        if (_current>=0 && _current<_slideFilename.size())
        {
            if (_preload)
                _preloadSprite[_current].draw();
            else
            {
                GltSprite *slide = _slideCache.find(_current);

                if (!slide)
                {
                    if (_slideCache.size()>=_cacheSize)
                        _slideCache.pop_back();

                    _slideCache.insert(_current,GltSprite());
                    slide = _slideCache.find(_current);

                    if (slide)
                    {
                        slide->inheritColor() = true;
                        slide->blend() = _blend;

                        if (_slideFilename[_current].size())
                            slide->init(_slideFilename[_current],false);
                        else
                            slide->init(_slideBuffer[_current],false);
                    }
                }

                if (slide)
                {
                    if (_autoScale)
                    {
                        // Get the current viewport from OpenGL
                        GltViewport viewport(true);

                        // Give up if viewport is zero width or height
                        if (slide->width() && slide->height() && viewport.width() && viewport.height())
                        {
                            const real sx = viewport.width()/real(slide->width());
                            const real sy = viewport.height()/real(slide->height());

                            if (_preserveAspectRatio)
                            {
                                const real cxSlide = slide->width()*0.5;
                                const real cySlide = slide->height()*0.5;

                                const real cxViewport = viewport.width()*0.5;
                                const real cyViewport = viewport.height()*0.5;

                                slide->transformation() =
                                    matrixTranslate(-cxSlide,-cySlide) *
                                    matrixScale(MIN(sx,sy)) *
                                    matrixTranslate(cxViewport,cyViewport);
                            }
                            else
                                slide->transformation() = matrixScale(sx,sy);
                        }
                    }

                    slide->draw();
                }
            }
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

    glPopAttrib();

    GLERROR;
}

void GltSlides::next()
{
    const uint32 s = size();
    if (s)
        _current = (_current+s+1)%s;
}

void GltSlides::prev()
{
    const uint32 s = size();
    if (s)
        _current = (_current+s-1)%s;
}

void GltSlides::push_back(const string &filename)
{
    _slideFilename.push_back(filename);
    _slideBuffer.push_back(NULL);
}

void GltSlides::push_back(const void *buffer)
{
    _slideFilename.push_back(string());
    _slideBuffer.push_back(buffer);
}

      uint32 &GltSlides::current()       { return _current; }
const uint32 &GltSlides::current() const { return _current; }

const uint32  GltSlides::size() const    { return _slideFilename.size(); }

      uint32 &GltSlides::cacheSize()       { return _cacheSize; }
const uint32 &GltSlides::cacheSize() const { return _cacheSize; }
