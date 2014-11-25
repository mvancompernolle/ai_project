//
// $Id: svdTest.cpp,v 1.7 2005/01/19 07:39:15 knicewar Exp $
// 
// Copyright Keith Nicewarner.  All rights reserved.  See COPYING file
// in top source directory.
// 
// Rigorous random testing of SVD operations.
// 

#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "MatrixSVD.h"



using namespace std;
using namespace LinAlg;



class RandomizeElement
{
public:
  double maxElement;

  RandomizeElement(double maxElement_):
    maxElement(maxElement_)
  {
  }

  void operator() (double & x)
  {
    x = (maxElement*(2*(rand()/(double)RAND_MAX) - 1));
  }
};

class MaxMagFinder
{
public:
  double max;

  MaxMagFinder():
    max(0)
  {
  }

  void operator() (double & x)
  {
    double mag = fabs(x);
    if (mag > max)
    {
      max = mag;
    }
  }
};


#ifdef VXWORKS
extern "C"
int svdTest()
#else
int main()
#endif
{
  try
  {
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    // Without this, I get a "missing locale facet" exception!
    cout.imbue(locale::empty());
#endif

    srand((unsigned)time(0));

    unsigned maxRows    = 10;
    unsigned maxCols    = 10;
    double   maxElement = 1e3;
    unsigned numIterations = 100000;

    RandomizeElement randomizer(maxElement);

    cout << "Performing " << numIterations
         << " random SVD tests (this could take a while) ..." << endl;
    double maxSvdResidual = 0;
    double maxInvResidual = 0;
    for (unsigned i = 0; i < numIterations; ++i)
    {
      unsigned numRows;
      unsigned numCols;
      do
      {
        numRows = 1 + unsigned(maxRows*(rand()/(double)RAND_MAX));
        numCols = 1 + unsigned(maxCols*(rand()/(double)RAND_MAX));
      }
      while (numRows < numCols);

      Matrix m(numRows, numCols);

      visitMatrix(m, randomizer);

      MatrixSVD svd(m);
      Matrix svdResidual = (svd.U()*svd.W()*transpose(svd.V()) - m);

      Matrix minv = m.inverse();
      Matrix invResidual = (minv*m*minv - minv);

      MaxMagFinder maxMagFinder;
      visitMatrix(svdResidual, maxMagFinder);
      if (maxMagFinder.max > maxSvdResidual)
      {
        maxSvdResidual = maxMagFinder.max;
      }
      visitMatrix(invResidual, maxMagFinder);
      if (maxMagFinder.max > maxInvResidual)
      {
        maxInvResidual = maxMagFinder.max;
      }
    }
    cout << "max SVD residual = " << maxSvdResidual << endl;
    cout << "max inv residual = " << maxInvResidual << endl;
    if ((maxSvdResidual > 0.1) ||
        (maxInvResidual > 0.1))
    {
      cout << "WARNING: This is bad!" << endl;
    }
	else
    {
      cout << "(Small numbers are good, so this is acceptable.)" << endl;
    }
  }
  catch (exception const & ex)
  {
    cerr << "Unhandled exception: " << ex.what() << endl;
    return -1;
  }
  catch (...)
  {
    cerr << "Unhandled exception!" << endl;
    return -1;
  }

  return 0;
}
