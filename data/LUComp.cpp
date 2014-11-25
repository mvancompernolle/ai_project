/** TDSP *******************************************************************
                         LUComp.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 2002
    copyright            : (C) 2002 by Peter Jung
    email                : jung@hhi.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


// This implementation is based Matpack 1.0 and adapted to TDSP
//

#include <TMath.h>
#include <TVectorD.h>
#include <TDSPVector.h>
#include <TDSPMatrix.h>


/*------------------------------------------------------------------------------*
| implementation of the LU decomposition for the double             cludcomp.cc |
| precision complex matrix class of MatPack.                                    |
|                                                                               |
| MatPack Libary Release 1.0                                                    |
| Copyright (C) 1990-1995 by Berndt M. Gammel                                   |
|                                                                               |
| Permission to use, copy, modify, and distribute this software and its         |
| documentation for any non-commercial purpose and without fee is hereby        |
| granted, provided that the above copyright notice appear in all copies        |
| and that both that copyright notice and this permission notice appear         |
| in supporting documentation. This software is provided "as is" without        |
| express or implied warranty.                                                  |
|                                                                               |
*------------------------------------------------------------------------------*/

void TDSPMatrix::LUDecompose(TArrayI& Index, Int_t &d) {
  
  
  //
  //  Calculates the LU decomposition of the complex (double) matrix A, overwriting
  //  the old contents. The vector containing the permutations is returned in
  //  Index. This vector must be allocated (and deleted) by the user and must
  //  have the same index range (offset) like the matrix A. The sign of the
  //  permutations is returnd in d. This will be neccessary if you compute the
  //  determinant.
  //
  // Optimizations:
  //   -- avoid call to index operator that optimizes very badely
  //
  
  
  int imax = 0;
  double big,temp;
  TComplex sum,dum;

  // lowest and highest column index of the matrix
  int lo = Clo();
  int hi = Chi();

  // columns and rows must have the same range
  if (Rlo() != lo || Rhi() != hi ) {
    Error("LUDecompose", "rows and columns don't have the same range (%d,%d,%d,&d)",
	  Rlo(),Rhi(),Clo(),Chi());
    return;
  }

  // allocate auxilliary vector
  TVectorD V(lo,hi);

  // sign of permutations
  d = 1;
  
  for (register Int_t ii = lo; ii <= hi; ++ii) {
    big = 0.0;
    for (register Int_t j = lo; j <= hi; ++j)
      if ((temp = TMath::Abs(Element(ii,j).fRe) + TMath::Abs(Element(ii,j).fIm) ) > big) big = temp;

    if (big == 0.0) {
      Error("LUDecompose","Matrix is singular !");
      return;
    }
    V(ii) = 1.0 / big;
  }
  
  for (register Int_t j = lo; j <= hi; ++j) {
    for (register Int_t i = lo; i < j; ++i) {
      sum = Element(i,j);
      for (register Int_t k = lo; k < i; ++k)
	sum -= Element(i,k) * Element(k,j);
      Element(i,j) = sum;
    }
    
    big = 0.0;
    for (register Int_t i1 = j; i1 <= hi; ++i1) {
      sum = Element(i1,j);
      for (register Int_t k = lo; k < j; ++k)
	sum -= Element(i1,k) * Element(k,j);
      Element(i1,j) = sum;
      if ( (temp = V(i1) * ( TMath::Abs(sum.fRe) + TMath::Abs(sum.fIm) )) >= big) {
	big = temp;
	imax = i1;
      }
    }
    
    
    // interchange rows if neccessary
    if (j != imax) {
      
      for (register Int_t k = lo; k <= hi; ++k)
	Swap(imax,k, j,k);
      
      // sign of determinant changes
      d = -d;
      V(imax) = V(j);
    }

    Index[j] = imax;
    
    // instead we could set a[j][j] to a tiny value, say 1e-20,
    // and continue the computation
    if (Element(j,j) == 0.0) {
      Error("LUDecompose","Matrix is singular !");
      return;
    }
    
    if (j != hi) {
      dum = 1.0 / Element(j,j);
      for (register Int_t i = j+1; i <= hi; ++i) 
	Element(i,j) *= dum;
    }
  }
}

//----------------------------------------------------------------------------//

void TDSPMatrix::LUBacksubst(TArrayI& Index, TDSPVector& B) {

  //
  //  Do the backsubstitution for a complex right hand b side using
  //  the complex LU decomposed matrix A as retuned by Decompose().
  //
  // Optimizations:
  //   -- avoid call to index operator that optimizes very badely
  //
  
  Int_t    ii,ip;
  TComplex sum;

  // lowest and highest column index of the matrix
  Int_t lo = Clo();
  Int_t hi = Chi();

  // columns and rows must have the same range
  if (Rlo() != lo || Rhi() != hi || B.Lo() != lo || B.Hi() != hi) {
    Error("LUBacksubst","non conformant matrix (%d,%d,%d,%d) or vector (%d,%d)",
	  Rlo(),Rhi(),Clo(),Chi(),B.Lo(),B.Hi());
    return;
  }
  
  ii = lo-1;
  for (register Int_t i = lo; i <= hi; ++i) {
    
    ip = Index[i];
    sum = B.Element(ip);
    B.Element(ip) = B.Element(i);
    if (ii >= lo)
      for (register Int_t j = ii; j <= i-1; ++j)
	sum -= Element(i,j)*B.Element(j);
    else if (sum != 0.0)
      ii = i;
    B.Element(i) = sum;
  }

  for (register Int_t i1 = hi; i1 >= lo; i1--) {
    sum = B.Element(i1);
    for (register Int_t j = i1+1; j <= hi; ++j)
      sum -= Element(i1,j) * B.Element(j);
    B.Element(i1) = sum / Element(i1,i1);
  }
}

//----------------------------------------------------------------------------//

void TDSPMatrix::LUBacksubst(TArrayI& Index, TDSPMatrix& B) {
  //
  //  Do the backsubstitution for many complex right hand sides stored
  //  in the columns of the matrix B using the complex LU decomposed
  //  matrix A as returned by Decompose().
  //
  // Optimizations:
  //   -- avoid call to index operator that optimizes very badely
  //
  
  
  Int_t ii,ip;
  TComplex sum;

  // lowest and highest column index of the matrix
  Int_t lo = Clo(),
    hi = Chi(),
    bclo = B.Clo(),
    bchi = B.Chi();
  
  // all columns and rows must have the same range
  if (Rlo() != lo || Rhi() != hi || B.Rlo() != lo || B.Rhi() != hi) {
    Error("LUBacksubst","non conformant matrices (%d,%d,%d,%d) and (%d,%d,%d,%d)",
	  Rlo(),Rhi(),Clo(),Chi(),B.Rlo(),B.Rhi(),B.Clo(),B.Chi());
    return;
  }

  
  for (register Int_t k = bclo; k <= bchi; ++k) {
    
    ii = lo-1;
    for (register Int_t i = lo; i <= hi; ++i) {

      ip = Index[i];
      sum = B.Element(ip,k);
      B.Element(ip,k) = B.Element(i,k);
      if (ii >= lo)
	for (register Int_t j = ii; j <= i-1; ++j)
	  sum -= Element(i,j)*B.Element(j,k);
      else if (sum != 0.0)
	ii = i;
      B.Element(i,k) = sum;
    }
    
    for (register Int_t i1 = hi; i1 >= lo; --i1) {
      sum = B.Element(i1,k);
      for (register Int_t j = i1+1; j <= hi; ++j)
	sum -= Element(i1,j) * B.Element(j,k);
      B.Element(i1,k) = sum / Element(i1,i1);
    }
  }
}
//----------------------------------------------------------------------------//

void TDSPMatrix::LUSolveLinear(TDSPVector& b) {
  
  //
  // Solve the linear equation system A*x=b for x. The result x overwrites
  // the vector b. A will be destroyed during the calculation.
  //
  
  int sgn;
  int lo = Clo();
  int hi = Chi();
  
  // all columns and rows must have the same range
  if (Rlo() != lo || Rhi() != hi || b.Lo() != lo || b.Hi() != hi)
    Error("LUSolveLinear","non conformant matrix (%d,%d,%d,%d) or vector (%d,%d)",
	  Rlo(),Rhi(),Clo(),Chi(),b.Lo(),b.Hi());
  
  // allocate a permutation table
  TArrayI idx(hi-lo+1);
  
  // solve the equation
  LUDecompose(idx,sgn);   // lu-decomposition: result overwrites A
  LUBacksubst(idx,b);     // backsubstitution: result overwrites b
}
//----------------------------------------------------------------------------//

void TDSPMatrix::LUSolveLinear(TDSPMatrix& B) {
  //
  // Solve the linear equation system A * X = B with many right hand sides for X.
  // X = A.Inverse * B  is returned in B overwriting the previous contents.
  // The matrix A will be destroyed during the calculation.
  //
  
  int sgn;
  int lo = Clo();
  int hi = Chi();
  
  // columns and rows must have the same range
  if (Rlo() != lo || Rhi() != hi || B.Rlo() != lo || B.Rhi() != hi)
    Error("LUSolveLinear","nonconformant matrices (%d,%d,%d,%d) and (%d,%d,%d,%d)",
	  Rlo(),Rhi(),Clo(),Chi(),B.Rlo(),B.Rhi(),B.Clo(),B.Chi());
  
  // allocate a permutation table
  TArrayI idx(hi-lo+1);
  
  // solve the equation
  LUDecompose(idx,sgn);   // lu-decomposition: result overwrites A
  LUBacksubst(idx,B);     // backsubstitution: result overwrites B
}

//----------------------------------------------------------------------------//

TDSPMatrix* TDSPMatrix::Inverse (TDSPMatrix *Y) {
  //
  // Returns the inverse of this matrix. If the matrix is not square
  // or if the matrix turns out to be singular an error will result.
  //
  
  if (!Y) Y= new TDSPMatrix();
  Y->SetSize(fRows,fRows);
  TDSPMatrix *LU = Dup();   // copy of this matrix - protect from overwriting

  Y->Unit();           // make unit matrix (diagonal 1)
  LU->LUSolveLinear(*Y); // create inverse in Y

  delete LU;

  return Y;
}

//----------------------------------------------------------------------------//

TComplex TDSPMatrix::Det() {
  //
  // Returns the determinant of this matrix. The LU decomposition is
  // generated and the determinant is computed as the product of the
  // diagonal values. If the matrix is not square or if the matrix
  // turns out to be singular an error will result.
  //

  if (!IsSquare()) return ComplexZero;
  
  Int_t     sgn;
  TComplex  det;
  
  // allocate a permutation table
  TArrayI idx(fRows);
  
  // generate a copy of this matrix
  TDSPMatrix *A = Dup();
  
  // compute the LU decomposition
  A->LUDecompose(idx,sgn);
  
  // sign from permutations in the decomposition
  det = sgn;
  
  // product the diagonal elements
  for (register Int_t j = 0; j < fCols; ++j) det *= A->Element(j,j);

  delete A;
  
  return det;
}

