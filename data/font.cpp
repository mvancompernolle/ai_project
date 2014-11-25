#include "font.h"

/*! \file
    \ingroup GLT

    $Id: font.cpp,v 1.15 2003/06/02 07:04:43 nigels Exp $

    $Log: font.cpp,v $
    Revision 1.15  2003/06/02 07:04:43  nigels
    Tweak things for gcc 3.0.1

    Revision 1.14  2003/02/07 09:57:10  nigels
    Added GltChar and GltString

    Revision 1.13  2002/11/27 00:57:28  nigels
    expand

    Revision 1.12  2002/11/07 15:40:44  nigels
    *** empty log message ***

    Revision 1.11  2002/10/09 15:09:38  nigels
    Added RCS Id and Log tags


*/

#include <math/real.h>

#include <cstdio>
#include <cassert>
using namespace std;

GltFont::GltFont()
: _hStep(0), _vStep(0), _init(false)
{
}

GltFont::GltFont(const GltFont &)
{
    // Can't copy fonts
    assert(0);
}

GltFont &
GltFont::operator=(const GltFont &)
{
    // Can't copy fonts
    assert(0);
    return *this;
}

GltFont::~GltFont()
{
    _init = false;
}

void
GltFont::size(int &w,int &h,const GltString &str) const
{
    h = 0;
    w = 0;

    // Get the first line of text

    string::size_type i = 0;
    string::size_type j = str.find('\n',i);

    // Process each line of text

    while (i<str.size())
    {
        h++;

        int lWidth = 0;
        for (string::size_type k=i; k<j && k<str.size(); k++)
            lWidth += width(str[k]);

        //

        w = MAX(w,lWidth);

        //

        if (j==GltString::npos)
            break;

        i = j+1;
        j = str.find('\n',i);
    }

    //

    h *= _vStep;
}

int
GltFont::width(const GltChar ch) const
{
    return _hStep;
}

const int GltFont::hStep()  const { return _hStep; }
const int GltFont::vStep()  const { return _vStep; }

