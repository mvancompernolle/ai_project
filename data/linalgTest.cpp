//
// $Id: linalgTest.cpp,v 1.4 2003/07/14 17:35:15 knicewar Exp $
// 
// Copyright Keith Nicewarner.  All rights reserved.  See COPYING file
// in top source directory.
// 
// Run all Matrix classes through their courses to verify proper
// operation.
// 

#include <iostream>
#include <stdexcept>
#include <math.h>
#include "Vector.h"
#include "Matrix.h"
#include "MatrixSVD.h"



using namespace std;
using namespace LinAlg;



unsigned maxLength =   10;
double   maxValue  =  100;
double   minValue  = -100;

inline
bool similar(double const & x,
             double const & y)
{
  return (fabs(x - y) < FLT_EPSILON);
}

inline
unsigned randomLength()
{
  return unsigned(maxLength*(rand()/(double)RAND_MAX));
}

inline
double randomValue()
{
  return (minValue + (maxValue - minValue)*(rand()/(double)RAND_MAX));
}

bool vectorTest()
{
  bool status = true;

  //
  // Test construction.
  //
  {
    unsigned length = randomLength();
    Vector v(length);
    if (v.length() != length)
    {
      cout << "vectorTest: Uninitialized constructor failed "
           << "to create correct size." << endl;
      status = false;
    }
  }

  {
    unsigned length = randomLength();
    double   value  = randomValue();
    Vector v(length, value);
    if (v.length() != length)
    {
      cout << "vectorTest: Scalar constructor failed to create correct size."
           << endl;
      status = false;
    }
    for (unsigned i = 0; i < v.length(); ++i)
    {
      if (!similar(v[i], value))
      {
        cout << "vectorTest: Scalar constructor failed to initialze "
             << "data correctly." << endl;
        status = false;
        break;
      }
    }
  }

  double   data[]   = {1, 2, 3, 4, 5};
  unsigned dataSize = sizeof(data)/sizeof(double);

  Vector v(dataSize, data);
  if (v.length() != dataSize)
  {
    cout << "vectorTest: Array constructor failed to create correct size."
         << endl;
    status = false;
  }
  for (unsigned i = 0; i < v.length(); ++i)
  {
    if (!similar(v[i], data[i]))
    {
      cout << "vectorTest: Array constructor failed to initialze "
           << "data correctly." << endl;
      status = false;
      break;
    }
  }

  Vector copy(v);
  if (copy.length() != dataSize)
  {
    cout << "vectorTest: Copy constructor failed to create correct size."
         << endl;
    status = false;
  }
  for (unsigned i = 0; i < copy.length(); ++i)
  {
    if (!similar(copy[i], data[i]))
    {
      cout << "vectorTest: Copy constructor failed to initialze "
           << "data correctly." << endl;
      status = false;
      break;
    }
  }

  Vector assign;
  assign = v;
  if (assign.length() != dataSize)
  {
    cout << "vectorTest: Assign operator failed to create correct size."
         << endl;
    status = false;
  }
  for (unsigned i = 0; i < assign.length(); ++i)
  {
    if (!similar(assign[i], data[i]))
    {
      cout << "vectorTest: Assign operator failed to initialze "
           << "data correctly." << endl;
      status = false;
      break;
    }
  }

  //
  // Test arithmetic operators.
  //
  Vector a(dataSize, data);
  a += v;
  for (unsigned i = 0; i < a.length(); ++i)
  {
    if (!similar(a[i], 2*data[i]))
    {
      cout << "vectorTest: Operator += failed." << endl;
      status = false;
      break;
    }
  }

  a -= v;
  for (unsigned i = 0; i < a.length(); ++i)
  {
    if (!similar(a[i], data[i]))
    {
      cout << "vectorTest: Operator -= failed." << endl;
      status = false;
      break;
    }
  }

  a *= 3;
  for (unsigned i = 0; i < a.length(); ++i)
  {
    if (!similar(a[i], 3*data[i]))
    {
      cout << "vectorTest: Operator *= failed." << endl;
      status = false;
      break;
    }
  }

  a /= 3;
  for (unsigned i = 0; i < a.length(); ++i)
  {
    if (!similar(a[i], data[i]))
    {
      cout << "vectorTest: Operator /= failed." << endl;
      status = false;
      break;
    }
  }

  Vector neg = -a;
  for (unsigned i = 0; i < neg.length(); ++i)
  {
    if (!similar(neg[i], -data[i]))
    {
      cout << "vectorTest: Operator - failed." << endl;
      status = false;
      break;
    }
  }

  Vector sum = a + v;
  for (unsigned i = 0; i < sum.length(); ++i)
  {
    if (!similar(sum[i], 2*data[i]))
    {
      cout << "vectorTest: Vector + Vector failed." << endl;
      status = false;
      break;
    }
  }

  Vector dif = sum - v;
  for (unsigned i = 0; i < dif.length(); ++i)
  {
    if (!similar(dif[i], data[i]))
    {
      cout << "vectorTest: Vector - Vector failed." << endl;
      status = false;
      break;
    }
  }

  //
  // Test dot product.
  //
  if (!similar(a*v, 55))
  {
    cout << "vectorTest: Vector * Vector failed." << endl;
    status = false;
  }

  //
  // Test scaling.
  //
  Vector dbl = v*2;
  for (unsigned i = 0; i < dbl.length(); ++i)
  {
    if (!similar(dbl[i], 2*data[i]))
    {
      cout << "vectorTest: Vector * double failed." << endl;
      status = false;
      break;
    }
  }

  Vector haf = dbl/2;
  for (unsigned i = 0; i < haf.length(); ++i)
  {
    if (!similar(haf[i], data[i]))
    {
      cout << "vectorTest: Vector / double failed." << endl;
      status = false;
      break;
    }
  }

  //
  // Test norm functions.
  //
  if (!similar(v.norm(0), 5))
  {
    cout << "vectorTest: norm(0) failed." << endl;
    cout << "  Expected " << 5 << ", but got " << v.norm(0) << endl;
    status = false;
  }
  if (!similar(v.norm(1), 15))
  {
    cout << "vectorTest: norm(1) failed." << endl;
    cout << "  Expected " << 15 << ", but got " << v.norm(1) << endl;
    status = false;
  }
  if (!similar(v.norm(2), sqrt(55.0)))
  {
    cout << "vectorTest: norm(2) failed." << endl;
    cout << "  Expected " << sqrt(55.0) << ", but got " << v.norm(2) << endl;
    status = false;
  }
  if (!similar(v.norm(3), pow(225.0, 1/3.0)))
  {
    cout << "vectorTest: norm(3) failed." << endl;
    cout << "  Expected " << pow(225.0, 1/3.0)
         << ", but got " << v.norm(3) << endl;
    status = false;
  }
  if (!similar(v.norm(4), pow(979.0, 1/4.0)))
  {
    cout << "vectorTest: norm(4) failed." << endl;
    cout << "  Expected " << pow(979.0, 1/4.0)
         << ", but got " << v.norm(4) << endl;
    status = false;
  }
  if (!similar(v.magnitude(), sqrt(55.0)))
  {
    cout << "vectorTest: magnitude() failed." << endl;
    cout << "  Expected " << sqrt(55.0) << ", but got "
         << v.magnitude() << endl;
    status = false;
  }
  if (!similar(v.distanceTo(v + Vector(v.length(), 2)), sqrt(20.0)))
  {
    cout << "vectorTest: distanceTo() failed." << endl;
    cout << "  Expected " << sqrt(20.0) << ", but got "
         << v.distanceTo(v + Vector(v.length(), 2)) << endl;
    status = false;
  }

  //
  // Test sub-vector operation.
  //
  {
    SubVector subv = v.subVector(1, 4);
    for (unsigned i = 0; i < subv.length(); ++i)
    {
      if (subv[i] != data[i+1])
      {
        cout << "vectorTest: subVector(int, int) failed." << endl;
        cout << "  Expected " << data[i+1] << ", but got " << subv[i] << endl;
        status = false;
        break;
      }
    }
  }
  {
    SubVector subv = v.subVector(Range<int>(1, 4));
    for (unsigned i = 0; i < subv.length(); ++i)
    {
      if (subv[i] != data[i+1])
      {
        cout << "vectorTest: subVector(Range) failed." << endl;
        cout << "  Expected " << data[i+1] << ", but got " << subv[i] << endl;
        status = false;
        break;
      }
    }
  }
  a.subVector(1, 4) *= 2;
  for (unsigned i = 0; i < a.length(); ++i)
  {
    if ((i >= 1) && (i <= 4))
    {
      if (!similar(a[i], 2*data[i]))
      {
        cout << "vectorTest: subVector modifier failed." << endl;
        cout << "  Expected " << 2*data[i] << ", but got " << a[i] << endl;
        status = false;
        break;
      }
    }
    else if (!similar(a[i], data[i]))
    {
      cout << "vectorTest: subVector modifier failed." << endl;
      cout << "  Expected " << data[i] << ", but got " << a[i] << endl;
      status = false;
      break;
    }
  }

  return status;
}

bool matrixTest()
{
  bool status = true;

  //
  // Test construction.
  //
  {
    unsigned numRows = randomLength();
    unsigned numCols = randomLength();
    Matrix m(numRows, numCols);
    if ((m.numRows() != numRows) ||
        (m.numCols() != numCols))
    {
      cout << "matrixTest: Uninitialized constructor failed "
           << "to create correct size." << endl;
      status = false;
    }
  }

  {
    unsigned numRows = randomLength();
    unsigned numCols = randomLength();
    double   value   = randomValue();
    Matrix m(numRows, numCols, value);
    if ((m.numRows() != numRows) ||
        (m.numCols() != numCols))
    {
      cout << "matrixTest: Scalar constructor failed to create correct size."
           << endl;
      status = false;
    }
    for (unsigned r = 0; r < m.numRows(); ++r)
    {
      for (unsigned c = 0; c < m.numCols(); ++c)
      {
        if (!similar(m[r][c], value))
        {
          cout << "matrixTest: Scalar constructor failed to initialze "
               << "data correctly." << endl;
          status = false;
          break;
        }
      }
      if (!status)
      {
        break;
      }
    }
  }

  double data[] = { 1,  2,  3,
                    4,  5,  6,
                    7,  8,  9,
		   10, 11, 12,
		   13, 14, 15,
		   16, 17, 18 };
  unsigned numRows = 6;
  unsigned numCols = 3;
  Matrix m(numRows, numCols, data);

  if ((m.numRows() != numRows) ||
      (m.numCols() != numCols))
  {
    cout << "matrixTest: Array constructor failed to create correct size."
         << endl;
    status = false;
  }

  unsigned i;
  unsigned r;
  unsigned c;
  i = 0;
  for (r = 0; r < m.numRows(); ++r)
  {
    for (c = 0; i < m.numCols(); ++c)
    {
      if (!similar(m[r][c], data[i++]))
      {
        cout << "matrixTest: Array constructor failed to initialze "
             << "data correctly." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  Matrix copy(m);
  if ((copy.numRows() != numRows) ||
      (copy.numCols() != numCols))
  {
    cout << "matrixTest: Copy constructor failed to create correct size."
         << endl;
    status = false;
  }
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(copy[r][c], data[i++]))
      {
        cout << "matrixTest: Copy constructor failed to initialze "
             << "data correctly." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  Matrix assign;
  assign = m;
  if ((assign.numRows() != numRows) ||
      (assign.numCols() != numCols))
  {
    cout << "matrixTest: Assign operator failed to create correct size."
         << endl;
    status = false;
  }
  i = 0;
  for (r = 0; r < assign.numRows(); ++r)
  {
    for (c = 0; i < assign.numCols(); ++c)
    {
      if (!similar(assign[r][c], data[i++]))
      {
        cout << "matrixTest: Assign operator failed to initialze "
             << "data correctly." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  //
  // Test arithmetic operators.
  //
  Matrix a(numRows, numCols, data);
  a += m;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(a[r][c], 2*data[i++]))
      {
        cout << "matrixTest: Operator += failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  a -= m;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(a[r][c], data[i++]))
      {
        cout << "matrixTest: Operator -= failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  a *= 3;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(a[r][c], 3*data[i++]))
      {
        cout << "matrixTest: Operator *= failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  a /= 3;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(a[r][c], data[i++]))
      {
        cout << "matrixTest: Operator /= failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  Matrix neg = -a;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(neg[r][c], -data[i++]))
      {
        cout << "matrixTest: Operator - failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  Matrix sum = a + m;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(sum[r][c], 2*data[i++]))
      {
        cout << "matrixTest: Matrix + Matrix failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  Matrix dif = sum - m;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(dif[r][c], data[i++]))
      {
        cout << "matrixTest: Matrix - Matrix failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  //
  // Test scaling.
  //
  Matrix dbl = m*2;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(dbl[r][c], 2*data[i++]))
      {
        cout << "matrixTest: Matrix * double failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  Matrix haf = dbl/2;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(haf[r][c], data[i++]))
      {
        cout << "matrixTest: Matrix / double failed." << endl;
        status = false;
        break;
      }
    }
  }

  Matrix pls = m + 2;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(pls[r][c], data[i++] + 2))
      {
        cout << "matrixTest: Matrix + double failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }

  Matrix min = m - 2;
  i = 0;
  for (r = 0; r < copy.numRows(); ++r)
  {
    for (c = 0; i < copy.numCols(); ++c)
    {
      if (!similar(min[r][c], data[i++] - 2))
      {
        cout << "matrixTest: Matrix - double failed." << endl;
        status = false;
        break;
      }
    }
    if (!status)
    {
      break;
    }
  }


  //
  // TODO: Test sub-matrix operation.
  //

  return status;
}


#ifdef VXWORKS
extern "C"
int linalgTest()
#else
int main()
#endif
{
  int status = 0;

  try
  {
    bool vectorStatus = vectorTest();
    cout << "Vector class: " << (vectorStatus ? "PASSED" : "FAILED") << endl;
    if (!vectorStatus)
    {
      status = -1;
    }

    bool matrixStatus = matrixTest();
    cout << "Matrix class: " << (matrixStatus ? "PASSED" : "FAILED") << endl;
    if (!matrixStatus)
    {
      status = -1;
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

  return status;
}
