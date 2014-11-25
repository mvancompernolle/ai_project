#include "dlist.h"

/*! \file
    \ingroup Node
*/

#if !defined(NDEBUG)
#include <iostream>
#endif

#include <cassert>
using namespace std;

GltDisplayList::GltDisplayList()
: _list(-1)
{
}

GltDisplayList::~GltDisplayList()
{
    #if !defined(NDEBUG)
    if (defined())
        cerr << "WARNING: Potential OpenGL display list leak (" << this << ")" << endl;
    #endif

    reset();
}

bool
GltDisplayList::defined() const
{
    return _list!=-1;
}

void
GltDisplayList::draw() const
{
    if (_list!=-1)
        glCallList(_list);
}

void
GltDisplayList::reset()
{
    if (_list!=-1)
    {
        glDeleteLists(_list,1);
        _list = -1;
    }
}

void
GltDisplayList::newList(GLenum mode)
{
    if (_list==-1)
        _list = glGenLists(1);

    assert(_list!=-1);

    glNewList(_list,mode);
}

void
GltDisplayList::endList()
{
    glEndList();
}
