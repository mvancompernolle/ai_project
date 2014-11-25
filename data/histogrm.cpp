#include "histogrm.h"

/*! \file
    \ingroup Node
*/

#include <glt/viewport.h>
#include <glt/error.h>
#include <glt/glu.h>
#include <glt/rgb.h>

#include <node/fields.h>

#include <cmath>
using namespace std;

GltHistogram::GltHistogram()
:
    _min(0.0),
    _max(1.0),
    _size(0),
    _cumulative(true),
    _drawLine(true),
    _samples(0),
    _outlyers(0)
{
    color() = yellow;
    name() = "histogram";
}

GltHistogram::~GltHistogram()
{
}

GltFieldPtr
GltHistogram::settings()
{
    GltFields *root = new GltFields(name());

    root->add(visible()    ,"display"         );
    root->add(_drawLine    ,"line"            );
    root->add(color()      ,"color"           );
    root->add(_min     ,1.0,"min"             );
    root->add(_max     ,1.0,"max"             );
    root->add(_size        ,"size"            );
    root->add(_cumulative  ,"cumulative"      );
    root->add(new GltFieldFunc<GltHistogram>(*this,&GltHistogram::reset,"reset",true));

    return root;
}

void
GltHistogram::draw() const
{
    if (!visible() || _min==_max || _min>_max || !_count.size())
        return;

    const unsigned int max = maxCount();

    GLERROR;

    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // Use the current OpenGL viewport for
    // viewing system

    GltViewport viewport(true);
    gluOrtho2D(0,_count.size(),0,MAX(max,1));

    GLERROR;

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glPushMatrix();
        glScaled(0.99,0.99,1.0);

        glColor();

        if (_drawLine)
        {
            glTranslatef(0.5,0.0,0.0);
            glBegin(GL_LINE_STRIP);
            for (uint32 i=0; i<_count.size(); i++)
                glVertex2i(i,_count[i]);
            glEnd();
        }
        else
        {
            glBegin(GL_LINE_STRIP);
            glVertex2i(0,0);
            for (uint32 i=0; i<_count.size(); i++)
            {
                glVertex2i(i,_count[i]);
                glVertex2i(i+1,_count[i]);
            }
            glVertex2i(_count.size(),0);
            glEnd();
        }

        glPopMatrix();

    glPopAttrib();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    GLERROR;
}

void
GltHistogram::reset()
{
    _size = MAX(_size,0);
    _count.clear();
    _count.resize(_size);
    for (int i=0; i<_size; i++)
        _count[i] = 0;
    _samples = 0;
    _outlyers = 0;
}

void
GltHistogram::add(const double x)
{
    add((int) floor(double(x-_min)/double(_max-_min)*double(_count.size())));
}

void
GltHistogram::add(const int n)
{
    if (int(_count.size())!=_size)
        reset();

    _samples++;

    if (n>=0 && n<(int)_count.size())
    {
        _count[n]++;

        if (_samples==(unsigned int)~0)
        {
            _samples >>= 1;
            for (uint32 i=0; i<_count.size(); i++)
                _count[i] >>= 1;
        }
    }
    else
        _outlyers++;
}

bool   &GltHistogram::drawLine()   { return _drawLine; }
double &GltHistogram::min()        { return _min; }
double &GltHistogram::max()        { return _max; }
int    &GltHistogram::size()       { return _size; }
bool   &GltHistogram::cumulative() { return _cumulative; }

unsigned int GltHistogram::samples()  const { return _samples;  }
unsigned int GltHistogram::outlyers() const { return _outlyers; }

unsigned int GltHistogram::minCount() const
{
    unsigned int min = ~0;

    for (uint32 i=0; i<_count.size(); i++)
        min = MIN(min,_count[i]);

    return min;
}

unsigned int GltHistogram::maxCount() const
{
    unsigned int max = 0;

    for (uint32 i=0; i<_count.size(); i++)
        max = MAX(max,_count[i]);

    return max;
}

