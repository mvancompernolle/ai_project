//
// $Id: VectorData.cpp,v 1.1 2002/05/20 05:11:26 knicewar Exp $
// 
// Copyright Keith Nicewarner.  All rights reserved.  See COPYING file
// in top source directory.
//
// Non-inline implementation of the VectorData classes.
// 

#include <string.h>
#include "VectorData.h"
#include "Vector.h"



using namespace std;
using namespace LinAlg;



VectorData::VectorData(unsigned const n,
		       double   const initVal):
  _numRefs(0)
{
#ifdef DEBUG
  cout << __PRETTY_FUNCTION__ << ": (" << n << ", " << initVal << ")" << endl;
#endif
  register double *p   = _ptr;
  register double *top = p + n;
  while (p < top)
  {
    *p++ = initVal;
  }
}

VectorData::VectorData(unsigned const   n,
		       double   const * initVal):
  _numRefs(0)
{
#ifdef DEBUG
  cout << __PRETTY_FUNCTION__ << ": (" << n << ", " << initVal << ")" << endl;
#endif
  memcpy(_ptr, initVal, n*sizeof(double));
}

VectorData::VectorData(Vector const & v):
  _numRefs(0)
{
#ifdef DEBUG
  cout << __PRETTY_FUNCTION__ << ": (" << &v << ")" << endl;
#endif
  if (v.data())
  {
    memcpy(_ptr, v.data()->_ptr, v.length()*sizeof(double));
  }
}
