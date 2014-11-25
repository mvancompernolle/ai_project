#include "colmap.h"

/*! \file
    \ingroup GLT

    $Id: colmap.cpp,v 1.13 2003/07/22 03:39:19 nigels Exp $

    $Log: colmap.cpp,v $
    Revision 1.13  2003/07/22 03:39:19  nigels
    CLAMP -> clamp
    LERP -> lerp

    Revision 1.12  2002/11/27 00:57:28  nigels
    expand

    Revision 1.11  2002/11/07 15:40:44  nigels
    *** empty log message ***

    Revision 1.10  2002/10/09 15:09:38  nigels
    Added RCS Id and Log tags


*/

#include <glt/rgb.h>

#include <cassert>
using namespace std;

GltColorMap::GltColorMap()
{
}

GltColorMap::~GltColorMap()
{
}

void
GltColorMap::addEntry(const real t,const GltColor &color)
{
    if (_map.size()==0)
    {
        _map.push_back(make_pair(t,color));
        return;
    }

    if (t<start())
    {
        _map.insert(_map.begin(),make_pair(t,color));
        return;
    }

    if (t>=end())
    {
        _map.push_back(make_pair(t,color));
        return;
    }

    iterator i,j;
    i = _map.begin();
    j = i; j++;

    while (i!=_map.end() && j!=_map.end())
        if (i->first<=t && j->first>t)
        {
            _map.insert(i,make_pair(t,color));
            break;
        }
        else
        {
            i++;
            j++;
        }

    assert(0);
}

void
GltColorMap::addEntry(const real t,const GltColor &color,const real alpha)
{
    GltColor col(color);
    col.alpha() = alpha;
    addEntry(t,col);
}

void
GltColorMap::clear()
{
    _map.clear();
}

GltColor
GltColorMap::lookup(const real t) const
{
    if (_map.size()==0)
        return black;

    if (t<=start())
        return _map.front().second;

    if (t>=end())
        return _map.back().second;

    const_iterator i,j;
    i = _map.begin();
    j = i; j++;

    while (i!=_map.end() && j!=_map.end())
        if (i->first<=t && j->first>=t)
            return lerp(i->second,j->second,(t-i->first)/(j->first-i->first));
        else
        {
            i++;
            j++;
        }

    assert(0);
    return black;
}

real
GltColorMap::lookup(const int i) const
{
    assert(i>=0);
    assert(i<size());

    const_iterator j = _map.begin();
    for (int k=0; k<i; k++)
        j++;

    return j->first;
}

const int    GltColorMap::size()     const { return _map.size();           }
const real   GltColorMap::start()    const { return _map.front().first;    }
const real   GltColorMap::end()      const { return _map.back().first;     }
const real   GltColorMap::duration() const { return end() - start();       }

GltColorMap &GltColorMap::operator+=(const real x)
{
    iterator i = _map.begin();
    for (; i!=_map.end(); i++)
        i->first += x;
    return *this;
}

GltColorMap &GltColorMap::operator-=(const real x)
{
    iterator i = _map.begin();
    for (; i!=_map.end(); i++)
        i->first -= x;
    return *this;
}

GltColorMap &GltColorMap::operator*=(const real x)
{
    iterator i = _map.begin();
    for (; i!=_map.end(); i++)
        i->first *= x;
    return *this;
}

GltColorMap &GltColorMap::operator/=(const real x)
{
    iterator i = _map.begin();
    for (; i!=_map.end(); i++)
        i->first /= x;
    return *this;
}

