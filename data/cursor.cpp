#include "cursor.h"

/*! \file
    \ingroup GLT

    $Id: cursor.cpp,v 1.11 2003/06/02 07:04:28 nigels Exp $

    $Log: cursor.cpp,v $
    Revision 1.11  2003/06/02 07:04:28  nigels
    Tweak things for gcc 3.0.1

    Revision 1.10  2003/02/07 09:57:10  nigels
    Added GltChar and GltString

    Revision 1.9  2002/11/27 00:57:28  nigels
    expand

    Revision 1.8  2002/11/07 15:40:44  nigels
    *** empty log message ***

    Revision 1.7  2002/10/09 15:09:38  nigels
    Added RCS Id and Log tags


*/

#include <glt/font.h>
#include <glt/viewport.h>
#include <glt/error.h>

using namespace std;

GltCursor::GltCursor(const GltFont &font,const GltViewport &area)
: _font(font), _area(area) // , _y(0)
{
    GLERROR

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glPushAttrib(GL_VIEWPORT_BIT);

        GLERROR

        _area.set();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0,GLdouble(area.width()),GLdouble(area.height()-1),-1.0,-1.0,1.0);

        GLERROR

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        GLERROR

//      _y = 0;
        glRasterPos2i(0,0);

        GLERROR
}

GltCursor::~GltCursor()
{
    GLERROR

    glPopAttrib();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    GLERROR
}

void
GltCursor::print(const GltString &str)
{
    GltString s;
    dos2unix(s,str);

    GltString::size_type i=0;
    GltString::size_type j=0;

    do
    {
        j = s.find('\n',i);
        if (j==GltString::npos)
        {
            printLine(s.substr(i));
            return;
        }

        printLine(s.substr(i,j-i));
        i = j+1;

    } while (j!=GltString::npos);
}

void
GltCursor::printLine(const GltString &str)
{
    GLERROR

    // We set the raster position before drawing
    // any text because we want to apply the
    // current color.

//  glRasterPos2i(0,_y);
    _font.print(str);
//  _y += _font.vStep();
//  glRasterPos2i(0,_y);

    // We set the raster position after drawing
    // so that other routines can make use of the
    // current position.

    GLERROR
}

