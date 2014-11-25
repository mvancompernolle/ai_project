#include "memhist.h"

#error OBSOLETE

/*! \file
    \brief   Classes for memory use analysis
    \ingroup Misc
*/

#ifndef DOXYGEN

#include <istream>
#include <ostream>
#include <cassert>
#include <iostream>     // Debugging
using namespace std;

MemoryEvent::MemoryEvent(void *ptr,size_t size)
: _ptr(ptr), _size(size), _new(true)
{
}

MemoryEvent::MemoryEvent(void *ptr)
: _ptr(ptr), _size(0), _new(false)
{
}

MemoryEventHistory::MemoryEventHistory()
{
}

MemoryEventHistory::MemoryEventHistory(istream &is)
{
    char   type;
    void  *ptr;
    size_t size;

    while (is.good())
    {
        is >> type;

        if (type=='+')
        {
            is >> ptr >> size;
            push_back(MemoryEvent(ptr,size));
        }

        if (type=='-')
        {
            is >> ptr;
            push_back(MemoryEvent(ptr));
        }

        type = 0;
    }
}

void
MemoryEventHistory::summary(ostream &os) const
{
    MemoryModel mem;
    for (list<MemoryEvent>::const_iterator i=begin(); i!=end(); i++)
        mem.apply(*i);

    os << "Address space: " << (void *) mem.minAddress() << " - " << (void *) mem.maxAddress() << endl;
    os << "Peak memory  : " << mem.maxMemory() << " bytes in " << mem.maxBlocks() << " blocks." << endl;
    os << "Used memory  : " << mem.memoryUsed() << " bytes in " << mem.blocks() << " blocks." << endl;
    os << "Allocations  : " << mem.numAllocations() << " blocks." << endl;
    os << "Releases     : " << mem.numFree() << " blocks, " << mem.numFreeNull() << " NULL, " << mem.numFreeInvalid() << " invalid." << endl;
}

//////////////////////////////////////////////////////////

MemoryModel::MemoryModel()
: _memoryUsed(0),
  _minAddress(0xffffffff),
  _maxAddress(0x00000000),
  _maxBlocks(0),
  _maxMemory(0),
  _numAllocations(0),
  _numFree(0),
  _numFreeNull(0),
  _numFreeInvalid(0)
{
}

void
MemoryModel::apply(const MemoryEvent &event)
{
    const size_t min = size_t(event._ptr);
    const size_t max = min + event._size;

    if (event._new)
    {
        assert(_allocated.find(min)==_allocated.end());

        _allocated.insert(make_pair(min,max));
        _memoryUsed += event._size;
        if (min<_minAddress) { _minAddress = min; _maxBlocks = blocks(); }
        if (max>_maxAddress) { _maxAddress = max; _maxBlocks = blocks(); }
        if (_memoryUsed>_maxMemory) { _maxMemory = _memoryUsed; }
        _numAllocations++;
    }
    else
    {
        _numFree++;

        if (min==0) // Ignore deletion of NULL pointer
        {
            _numFreeNull++;
            return;
        }

        std::map<size_t, size_t, std::less<size_t> >::iterator i = _allocated.find(min);
        if (i!=_allocated.end())
        {
            _memoryUsed -= i->second - i->first;
            _allocated.erase(i);

            assert(_allocated.find(min)==_allocated.end());
        }
        else
        {
            cout << (void *) min << endl;
            _numFreeInvalid++;
        }

        return;
    }
}

size_t MemoryModel::minAddress()   const { return _minAddress; }
size_t MemoryModel::maxAddress()   const { return _maxAddress; }
size_t MemoryModel::addressSpace() const { return _maxAddress - _minAddress; }
size_t MemoryModel::blocks()       const { return _allocated.size(); }
size_t MemoryModel::memoryUsed()   const { return _memoryUsed; }
size_t MemoryModel::maxBlocks()    const { return _maxBlocks; }
size_t MemoryModel::maxMemory()    const { return _maxMemory; }

size_t MemoryModel::numAllocations() const { return _numAllocations; }
size_t MemoryModel::numFree()        const { return _numFree; }
size_t MemoryModel::numFreeNull()    const { return _numFreeNull; }
size_t MemoryModel::numFreeInvalid() const { return _numFreeInvalid; }

#endif
