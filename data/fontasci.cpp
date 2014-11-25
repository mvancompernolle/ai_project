#include "fontasci.h"

/*! \file
    \ingroup GLT

    $Id: fontasci.cpp,v 1.16 2003/02/07 09:57:10 nigels Exp $

    $Log: fontasci.cpp,v $
    Revision 1.16  2003/02/07 09:57:10  nigels
    Added GltChar and GltString

    Revision 1.15  2002/11/27 00:57:28  nigels
    expand

    Revision 1.14  2002/11/07 15:40:44  nigels
    *** empty log message ***

    Revision 1.13  2002/10/09 15:09:38  nigels
    Added RCS Id and Log tags


*/

#include <math/real.h>

#include <algorithm>
#include <cstdio>
#include <cassert>
using namespace std;

GltFontAscii::GltFontAscii(void *data)
: _width(0), _height(0), _listBase(-1)
{
    if (data)
        init(data);
}

GltFontAscii::~GltFontAscii()
{
    init(NULL);
}

void
GltFontAscii::init(void *data)
{
    clear();

    if (data)
    {
        void *font = getHeader(data,_width,_height);
        if (_width!=0 && _height!=0 && font)
        {
            _hStep = _width;
            _vStep = _height;
            compileLists(font);
            _init = true;
        }
    }
}

void
GltFontAscii::clear()
{
    _init = false;

    if (_listBase!=-1)
    {
        glDeleteLists(_listBase,256);
        _listBase = -1;
    }
}

void
GltFontAscii::compileLists(void *data)
{
    if (_listBase==-1 && data)
    {
        GLubyte *ptr = (GLubyte *) data;
        const int bytesPerChar = ((_width+7)>>3)*_height;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        _listBase = glGenLists(256);
        for (int i = 0; i < 255; i++)
        {
            glNewList(i+_listBase, GL_COMPILE);
            glBitmap(_width,_height,0,_height-1,hStep(),0,(GLubyte *) ptr);
            glEndList();
            ptr += bytesPerChar;
        }
    }
}

bool
GltFontAscii::print(const GltChar ch) const
{
    assert(_init);

    if (_init)
    {
        glCallList(_listBase + (ch&0xff));
        return true;
    }
    else
        return false;
}

bool
GltFontAscii::print(const GltString &str) const
{
    assert(_init);

    if (_init)
    {
        glPushAttrib(GL_LIST_BIT|GL_CURRENT_BIT);
            for (uint32 i=0; i<str.length(); i++)
                print(str[i]);
        glPopAttrib();

        // Advance OpenGL raster position to next line
        glBitmap(0,0,0,0,0,-_vStep,NULL);

        return true;
    }
    else
        return false;
}

////////////////////////////////////////////////////////////////////

// Create variable-length header for font data

bool
GltFontAscii::makeHeader
(
    string &header,
    const int            width,
    const int            height
)
{
    // 11 characters is big enough for integer 2^32 + \0

    char buffer[5+11+11];
    sprintf(buffer,"GLTF %u %u",width,height);
    header = buffer;
    header += '\0';

    return true;
}

// Decode variable-length header for texture data

void *
GltFontAscii::getHeader
(
    const void * const data,
    int            &width,
    int            &height
)
{
    const char * const h = (const char * const) data;

    if (h[0]=='G' && h[1]=='L' && h[2]=='T' && h[3]=='F' && h[4]==' ')
    {
        if (sscanf(h+5,"%i %i",&width,&height)==2)
            return (void *) (h + strlen(h) + 1);
    }

    return NULL;
}
