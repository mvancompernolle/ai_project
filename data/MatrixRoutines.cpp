/** TDSP *******************************************************************
                          MatrixRoutines.cpp  -  description
                             -------------------
    begin                : Mon Apr 29 2002
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

#include <TDSPVector.h>
#include <TDSPMatrix.h>

// This contains global matrix routines


// y=Ax+b

TDSPVector* Affine(TDSPMatrix *A, TDSPVector *x, TDSPVector *b, TDSPVector *result) {
  result=A->RightApply(x,result);
  if (b) (*result)+=*b;
  return result;
}

// Unit_Matrices

TDSPMatrix* UnitMatrix(Int_t nrows, Int_t ncols, TComplex diag) {
  if (!ncols) ncols=nrows;
  TDSPMatrix *b = new TDSPMatrix(nrows,ncols);
  b->Unit(diag);
  return b;
}

TDSPMatrix* FlatMatrix(Int_t nrows, Int_t ncols, TComplex fil) {
  if (!ncols) ncols=nrows;
  TDSPMatrix *b = new TDSPMatrix(nrows,ncols);
  b->Set(fil);
  return b;
}


TDSPMatrix* RowStack(TDSPVector *in, Int_t n, TDSPMatrix *r) {

#ifndef WIN32
# warning "RowStack is still unoptimized"
#endif
  
  if (!r) r = new TDSPMatrix();
  Int_t   m = in->GetLen();
  r->SetSize(n,m);
 

  for(register Int_t row=0;row<n;++row) { 
    TComplex *vec = in->GetVec();
    for(register Int_t col=0;col<m;++col)
      r->Element(row,col) = *(vec++); 
  }
  
  return r;

}

TDSPMatrix* ColumnStack(TDSPVector *in, Int_t m, TDSPMatrix *r) {

#ifndef WIN32
# warning "ColumnStack is still unoptimized"
#endif
  
  if (!r) r = new TDSPMatrix();
  Int_t   n = in->GetLen();
  r->SetSize(n,m);
 

  for(register Int_t col=0;col<m;++col) { 
    TComplex *vec = in->GetVec();
    for(register Int_t row=0;row<n;++row)
      r->Element(row,col) = *(vec++); 
  }
  
  return r;

}

TDSPMatrix* ConvMatrix(TDSPVector *in, Int_t m, Int_t n, TDSPMatrix *r, 
		       Bool_t cyclic, 
		       Int_t Offset /* offset in the vector in */
		       ) {

#ifndef WIN32
# warning "ConvMatrix is still unoptimized"
#endif
  
  if (!r) r = new TDSPMatrix();
  Int_t   l = in->GetLen()-Offset;

  if (l<0) { 
    Error("ConvMatrix","Offset in the input vector(%d) is outside the vector of length(%d)!",
	  Offset,in->GetLen());
    return NULL;
  }

  if (!m) m = l;
  if (!n) n = l;
  
  l=TMath::Min(l,n); /* we do not need more from the vector */

  r->SetSize(m,n);
  r->Zeros();
  
  TComplex    *vec = in->GetVec()+Offset;
  if (cyclic) {
    for(register Int_t row=0;row<m;++row) {
      register     Int_t col=row+n; /* +n make sure that (col--)%n is not negative */
      for(register Int_t i=0;  i<l;++i) {
	r->Element(row,(col--)%n) = vec[i];
      }
    }
  } else {
    for(register Int_t row=0;row<m;++row) {
      register     Int_t col=row;
      for(register Int_t i=0;  i<l;++i) {
	r->Element(row,col--) = vec[i];
	if (col<0) break;
      }
    }
  }
  return r;

}

TDSPMatrix* ConvMatrix(TDSPMatrix *in, Int_t m, Int_t n, TDSPMatrix *r, 
		       Bool_t cyclic, Int_t mOffset, Int_t nOffset) {

#ifndef WIN32
# warning "ConvMatrix is still unoptimized"
#endif
  
  Int_t  l1 = in->GetRows()-mOffset;
  Int_t  l2 = in->GetCols()-nOffset;
  if (l1<0)  { 
    Error("ConvMatrix","Row Offset in the input matrix(%d) is outside the Matrix of size(%d,%d)!",
	  mOffset,in->GetRows(),in->GetCols());
    return NULL;
  } 
  if (l2<0)  { 
    Error("ConvMatrix","Column Offset in the input matrix(%d) is outside the Matrix of size(%d,%d)!",
	  nOffset,in->GetRows(),in->GetCols());
    return NULL;
  }
  if (!m) m = l1;
  if (!n) n = l1;

  if (m>l1) {
    Error("ConvMatrix","Number rows in the output matrix (%d) is greater than of the input (%d)!",
	  m,l1);
    return NULL;
  }

  
  if (!r) r = new TDSPMatrix();

  l2=TMath::Min(l2,n); /* we do not need more from the vector */
  r->SetSize(m,n);
  r->Zeros();

  
  if (cyclic) {
    for(register Int_t row=0;row<m;++row) {
      register     Int_t col=row+n;
      for(register Int_t i=0;  i<l2;++i) {
	r->Element(row,(col--)%n) = in->Element(row+mOffset,i+nOffset);
      }
    }
  } else {
    for(register Int_t row=0;row<m;++row) {
      register     Int_t col=row;
      for(register Int_t i=0;  i<l2;++i) {
	r->Element(row,col--) = in->Element(row+mOffset,i+nOffset);
	if (col<0) break;
      }
    }
  }
  return r;

}

TDSPMatrix* Toeplitz(TDSPVector *in, 
		     Int_t m, Int_t n, 
		     TDSPMatrix *r,
		     Bool_t cyclic) {

#ifndef WIN32
# warning "Toeplitz is still unoptimized"
#endif

  if (!r) r = new TDSPMatrix();
  Int_t   l = in->GetLen();

  if (!m) m = l;
  if (!n) n = l;
  
  r->SetSize(m,n);
  r->Zeros();
 

  TComplex    *vec = in->GetVec();
  if (cyclic) {
    for(register   Int_t row=0;row<m;++row) { 
      register     Int_t col=row;
      for(register Int_t i=0;  i<l;++i) {
	r->Element(row,(col++)%n) = vec[i];
      }
    }
  } else {
    for(register Int_t row=0;row<m;++row) {
      register     Int_t col=row;
      for(register Int_t i=0;  i<l;++i) {
	if (col==n) break;
	r->Element(row,(col++)%n) = vec[i];
      }
    }

  }
  
  return r;

}

TDSPMatrix* RandNC(UInt_t M,UInt_t N, Double_t sigma) {
  TDSPMatrix* b = new TDSPMatrix(M,N);
  b->RandNC(sigma);
  return b;
}
