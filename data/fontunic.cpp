#include "fontunic.h"

/*! \file
    \ingroup GLT

    $Id: fontunic.cpp,v 1.22 2003/08/05 08:03:42 nigels Exp $

    $Log: fontunic.cpp,v $
    Revision 1.22  2003/08/05 08:03:42  nigels
    *** empty log message ***

    Revision 1.20  2003/06/26 09:58:08  nigels
    *** empty log message ***

    Revision 1.19  2003/06/02 07:05:12  nigels
    Tweak things for gcc 3.0.1

    Revision 1.18  2002/11/27 00:57:28  nigels
    expand

    Revision 1.17  2002/11/07 15:40:45  nigels
    *** empty log message ***

    Revision 1.16  2002/10/09 15:09:38  nigels
    Added RCS Id and Log tags


*/

#include <iosfwd>
#include <string>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <vector>
#include <iostream>
using namespace std;

#include <misc/compress.h>
#include <math/real.h>

GltFontUnicode::GltFontUnicode(void *data)
: GltFontUnicodeCache(3000), _data(NULL)
{
    _hStep = 16;
    _vStep = 16;
    init(data);
}

GltFontUnicode::~GltFontUnicode()
{
    init(NULL);
}

void
GltFontUnicode::init(void *data)
{
    clear();

    if (!data)
        return;

    int compressed;
    void *font = getHeader((const byte *) data,compressed);

    string tmp;

    if (compressed)
    {
        decompress(tmp,font);
        font = (void *) tmp.data();
    }

    //

    byte *p = (byte *) font;

    uint32 size = 0;
    int i = 0;
    for (; i<0x10000; i++)
    {
        _index[i] = size;
        size += *(p++);
    }
    _index[i] = size;

    //

    _data = new byte [size];
    memcpy(_data,p,size);

    _init = true;
}

void
GltFontUnicode::clear()
{
    if (_data)
    {
        delete [] _data;
        _data = NULL;
        memset(_index,0,sizeof(_index));
    }

    GltFontUnicodeCache::clear();

    _init = false;
}

bool
GltFontUnicode::print(const GltChar ch) const
{
    assert(_init);

    if (_init)
    {
        ((GltFontUnicodeCache *)this)->draw(ch);
        return true;
    }
    else
        return false;
}

bool
GltFontUnicode::print(const GltString &str) const
{
    assert(_init);

    if (_init)
    {
        bool ok = true;

        glPushAttrib(GL_CURRENT_BIT);
        for (uint32 i=0; i<str.length(); i++)
            ok = ok && print(str[i]);
        glPopAttrib();

        // Advance OpenGL raster position to next line
        glBitmap(0,0,0,0,0,-GLfloat(_vStep),NULL);

        return ok;
    }
    else
        return false;
}

int
GltFontUnicode::width(const GltChar ch) const
{
    assert(_init);

    if (_init)
        return (_index[ch+1] - _index[ch])>>1;
    else
        return 0;
}

void
GltFontUnicode::OnDraw(const uint32 seed) const
{
    assert(_init);

    if (_init)
    {
        const wchar_t ch = wchar_t(seed);

        if (_data)
        {
            const int w = width(ch);

            assert(w==0 || w==8 || w==16);

            if (w>0)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glBitmap(w,16,0,15,GLfloat(w),0,(GLubyte *) _data + _index[ch]);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////

// Create variable-length header for font data

bool
GltFontUnicode::makeHeader(string &header,const int compressed)
{
    // 11 characters is big enough for integer 2^32 + \0

    char buffer[5+11];
    sprintf(buffer,"GLTU %u",compressed);
    header = buffer;
    header += '\0';

    return true;
}

// Decode variable-length header for texture data

void *
GltFontUnicode::getHeader(const void * const data,int &compressed)
{
    const char * const h = (const char * const) data;

    if (h[0]=='G' && h[1]=='L' && h[2]=='T' && h[3]=='U' && h[4]==' ')
    {
        if (sscanf(h+5,"%i",&compressed)==1)
            return (void *) (h + strlen(h) + 1);
    }

    return NULL;
}

#if 0

bool
GltFontUnicode::writeSrc(ostream &os)
{
    if (!_size)
        return false;

    os << "/* This source file written by GltFontUnicode::writeSrc()    */" << endl;
    os << "/* Refer to GltFontUnicode Documentation                     */" << endl;
    os << "/*                                                           */" << endl;
    os << "/* GNU Unicode Font         http://czyborra.com/unifont/     */" << endl;
    os << "/* GLT OpenGL Integeration: http://www.nigels.com/glt/       */" << endl;
    os << endl;

    os << "unsigned int unifontSize = " << _size << ';' << endl << endl;

    os << "unsigned char unifontData[] = " << endl;
    bin2src(os,(unsigned char *) _data,_size);
    os << endl;

    os << "unsigned char unifontIndex[] = " << endl;
    bin2src(os,(unsigned char *) _index,sizeof(_index));
    os << endl;

    return false;
}

bool
GltFontUnicode::writeAsm(ostream &os)
{
    if (!_size)
        return false;

    os << "/* This source file written by GltFontUnicode::writeAsm()    */" << endl;
    os << "/* Refer to GltFontUnicode Documentation                     */" << endl;
    os << "/*                                                           */" << endl;
    os << "/* GNU Unicode Font         http://czyborra.com/unifont/     */" << endl;
    os << "/* GLT OpenGL Integeration: http://www.nigels.com/glt/       */" << endl;
    os << endl;

    os << ".globl _unifontSize\n.balign 4\n_unifontSize:\n";
    bin2asm(os,(unsigned char *) &_size,sizeof(_size));
    os << endl;

    os << ".globl _unifontData\n.balign 4\n_unifontData:\n";
    bin2asm(os,(unsigned char *) _data,_size);
    os << endl;

    os << ".globl _unifontIndex\n.balign 4\n_unifontIndex:\n";
    bin2asm(os,(unsigned char *) _index,sizeof(_index));
    os << endl;

    return false;
}

#endif
