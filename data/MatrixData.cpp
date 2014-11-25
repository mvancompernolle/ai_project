//
// $Id: MatrixData.cpp,v 1.2 2002/05/20 05:11:26 knicewar Exp $
// 
// Copyright Keith Nicewarner.  All rights reserved.  See COPYING file
// in top source directory.
//
// Non-inline implementation of the MatrixData classes.
// 

#include "Matrix.h"



using namespace std;
using namespace LinAlg;



MatrixData::MatrixData(unsigned const nr,
		       unsigned const nc,
		       double   const initVal)
{
#ifdef DEBUG
  cout << __PRETTY_FUNCTION__ << ": (" << initVal << ')' << endl;
#endif
  _numRefs = 0;
  _rinc    = nc;
  register double *p   = _ptr;
  register double *top = p + nr*nc;
  while (p < top)
  {
    *p++ = initVal;
  }
}

MatrixData::MatrixData(unsigned const   nr,
		       unsigned const   nc,
		       double   const * initVal)
{
#ifdef DEBUG
  cout << __PRETTY_FUNCTION__ << ": (" << initVal << ')' << endl;
#endif
  _numRefs = 0;
  _rinc    = nc;
  register double *p   = _ptr;
  register double *top = p + nr*nc;
  while (p < top)
  {
    *p++ = *initVal++;
  }
}

MatrixData::MatrixData(unsigned const    nr,
		       unsigned const    nc,
		       double   const ** initVal)
{
#ifdef DEBUG
  cout << __PRETTY_FUNCTION__ << ": (" << initVal << ')' << endl;
#endif
  _numRefs = 0;
  _rinc    = nc;

  register       double  *dp   = _ptr;
  register const double  *sp;
  register const double  *stop;
  register const double **sr   = initVal;
  register const double **rtop = sr + nr;
  while (sr < rtop)
  {
    sp   = *sr++;
    stop = sp + _rinc;
    while (sp < stop)
    {
      *dp++ = *sp++;
    }
  }
}

MatrixData::MatrixData(Matrix const & m)
{
#ifdef DEBUG
  cout << __PRETTY_FUNCTION__ << ": (" << &m << ')' << endl;
#endif
  _numRefs = 0;
  _rinc    = m.numCols();

  register double  *dp   = _ptr;
  register double  *sp;
  register double  *stop;
  register double **sr   = m.rptr();
  register double **rtop = sr + m.numRows();
  while (sr < rtop)
  {
    sp   = *sr++;
    stop = sp + _rinc;
    while (sp < stop)
    {
      *dp++ = *sp++;
    }
  }
}
