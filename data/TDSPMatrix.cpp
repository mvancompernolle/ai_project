/** TDSP *******************************************************************
                          TDSPMatrix.cpp  -  description
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


#include <TError.h>
#include "TDSPMatrix.h"
#include "dl_atlas.h"

ClassImp(TDSPMatrix)


#ifdef WIN32
# undef DL_ATLAS
#endif


TDSPMatrix::TDSPMatrix(){
  fZTitle = NULL;
  fCols   = 0;
  fRows   = 0;
}


TDSPMatrix::TDSPMatrix(Int_t rows, Int_t cols) : TDSPVector() {
  fZTitle = NULL;
  SetSize(rows,cols);
}
void TDSPMatrix::SetZTitle(char *tit) {
  if (fZTitle) delete fZTitle;
  fZTitle = new char[strlen(tit)+1];
  strcpy(fZTitle,tit);
}

void TDSPMatrix::SetLen(Int_t num, Bool_t savedata) {
  if (Num!=num) SetSize(1,num);
}

void TDSPMatrix::SetSize(Int_t rows, Int_t cols) {
  if (rows<0) {
    Error("SetSize","Rows can not be negative (%d)", rows);
    return;
  }
  if (cols<0) {
    Error("SetSize","Columns can not be negative (%d)", cols);
    return;
  }
  TDSPVector::SetLen(rows*cols);
  fRows = rows;
  fCols = cols;
}

void TDSPMatrix::SetRows(UInt_t r, Bool_t savedate) {
  fRows = r;
  if (fCols) TDSPVector::SetLen(r*fCols,savedate);
}

void TDSPMatrix::SetCols(UInt_t c, Bool_t savedate) {
  fCols = c;
  if (fRows) TDSPVector::SetLen(c*fRows,savedate);
}

TDSPMatrix::~TDSPMatrix(){
}
//______________________________________________________________________________
// void TDSPVector::Streamer(TBuffer &R__b)
// {
//   // Stream an object of class TDSPVector.
  
//   UInt_t R__s, R__c;
//   if (R__b.IsReading()) {
//     Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
//     TObject::Streamer(R__b);
//     R__b >> fMinAlloc;
//     R__b >> fAllocHighScale;
//     R__b >> fAllocLowScale;
//     R__b >> Num;
//     SetLen(Num);
//     R__b >> fPos;
//     //R__b >> fAllocedNum;
//     R__b >> fSamplingRate;
//     R__b >> fZero;
//     for(Int_t i=0;i<Num;++i) fVec[i].Streamer(R__b);
    
//     R__b.CheckByteCount(R__s, R__c, TDSPVector::IsA());
//   } else {
//     if (fVec) {
//       R__c = R__b.WriteVersion(TDSPVector::IsA(), kTRUE);
//       TObject::Streamer(R__b);
//       R__b << fMinAlloc;
//       R__b << fAllocHighScale;
//       R__b << fAllocLowScale;
//       R__b << Num;
//       R__b << fPos;
//       //R__b << fAllocedNum;
//       R__b << fSamplingRate;
//       R__b << fZero;
//       for(Int_t i=0;i<Num;++i) fVec[i].Streamer(R__b);
//       R__b.SetByteCount(R__c, kTRUE);
//     } else 
//       Error("Streamer","Streaming of unalloced vectors not yet implemented");
//   }
// }


istream &operator>>(istream &is,TDSPMatrix &z) {

  cerr << "Not ready implemented - see " << __FILE__ << endl;
  cout << "Enter a list of complex numbers :" << endl;
 
  z.SetLen(404);
  TComplex  *c = z.GetVec();
  Int_t i = 0;
  while(1) {
    is >> c[i];
    if (is.fail()) {is.clear();break;}
    i++;
  }
  cout << i << " values read." << endl;
  z.SetLen(i);
  return ( is );
}

ostream &operator<<(ostream &os,TDSPMatrix &z) {
  Int_t      i;
  Int_t      j = z.GetLen();
  TComplex  *c = z.GetVec();
    
  for(i=0;i<j;i++) {
    os << c[i];
    if ((!((i+1)%z.fCols))&&(i+1!=j)) os << endl;
  }
  return ( os );
}

istream &operator>>(istream &is,TDSPMatrix *z) {
  if (z)
    is >> *z;
  else 
    Error("operator>>","Can not input to (TDSPMatrix*)0x0");
  return ( is );
}

ostream &operator<<(ostream &os,TDSPMatrix *z) {
  if (z) 
    os << *z;
  else 
    os << "(TDSPMatrix*)0x0";
  return ( os );
}

void TDSPMatrix::Fourier() {
  TComplexBase tmp;
  Double_t norm1 = _twopi/fRows; 
  Double_t norm2 = 1/TMath::Sqrt((Double_t)fRows);
  for(register Int_t i = 0; i<Num;++i) {
    tmp  = ((i/fRows)*(i%fRows));
    tmp *= norm1; 
    fVec[i] = TComplex(TMath::Cos(tmp),TMath::Sin(tmp));
    fVec[i]*=norm2;
  }
}

void TDSPMatrix::Hilbert() {

  TComplex *ep = fVec;
  for(register UInt_t i=0; i < fRows; ++i)
    for(register UInt_t j=0; j <fCols ; ++j)
      *ep++ = 1./(i+j+1);
}

void TDSPMatrix::Delta(TComplex diag) {

  Int_t min=TMath::Min(fRows,fCols);
  Zeros();
  for(register int i=0; i < min; ++i)
    Element(i,i)=diag;
}

Bool_t TDSPMatrix::IsHermitian() {
  
  if (!IsSquare()) return kFALSE;

  for(register UInt_t i=0;i<fCols;++i) 
    for(register UInt_t j=0;j<=i;++j) {
      
      if ((i==j)&&(Element(j,i).fIm)) return kFALSE;
      if (Element(j,i).fIm!=-Element(i,j).fIm) return kFALSE;
      if (Element(j,i).fRe!= Element(i,j).fRe) return kFALSE;

    }
  
  return kTRUE;

}

/////////////////////////////////////////////////////////////////////////////////////
//
// Matrix Multiplication out =  (this*) * inp 
//

TDSPMatrix* TDSPMatrix::AdjointRightApply( TDSPMatrix *inp, TDSPMatrix *out) {
  
  ///Not yet optimized
  
  TDSPMatrix *b = Adjoint();
  out = b->RightApply(inp,out);
  delete b;
  return out;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Matrix Multiplication out =  inp * (this*)
//

TDSPMatrix* TDSPMatrix::AdjointLeftApply( TDSPMatrix *inp, TDSPMatrix *out) {
  
  ///Not yet optimized
  
  TDSPMatrix *b = Adjoint();
  out = b->LeftApply(inp,out);
  delete b;
  return out;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Matrix Multiplication out = this * inp
//
TDSPMatrix* TDSPMatrix::RightApply(TDSPMatrix *inp, TDSPMatrix *out) {

  if (!out) out = new TDSPMatrix();
  
  
  Int_t iRows     = inp->GetRows();
  Int_t iCols     = inp->GetCols();
  Int_t n_stacks  = iRows/fCols; 

  if (iRows%fCols) {
    Error("RightApply","A's rows (%d) must be multiple of this's cols (%d) in product (this)*A=%dx%d*%dx%d.", 
	  iRows,fCols,fRows,fCols,iRows,iCols);
    return NULL;
  }

  out->SetSize(n_stacks*fRows,iCols);
  
#ifdef LINALG_INDEX_OPT
# ifdef LINALG_MAT_ROWWISE
  
  register TComplex *ovec = out->GetVec();
  TComplex *invec         = inp->GetVec();
  //
  // C=AB
  //
  while(n_stacks--) { // Loop over row-Stacked matrixes
 

    TComplex* matrix         = fVec; // The Top Left of the Matrix A
    register TComplex* iivec = 0; 
    

#ifdef DL_ATLAS
    if (fATLAS) {
      //
      // Calculate C = alpha*A*B+beta*C
      //   
# ifdef  TCOMPLEX_USE_DOUBLE
      dl_cblas_zgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
		     fRows,iCols,fCols,  
		     &ComplexOne, 
		     fVec,  fCols,  // alpha*A
		     invec, iCols,         // B
		     &ComplexZero, 
		     ovec,  iCols);   // beta*
# else
      dl_cblas_cgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
		     fRows,iCols,fCols,  
		     &ComplexOne, 
		     fVec,  fCols,  // alpha*A
		     invec, iCols,         // B
		     &ComplexZero, 
		     ovec,  iCols);   // beta*
# endif
      //
      // next block
      //
      invec+=iCols*fRows;
      
    } else {

#endif
      
      // Computing C=A*B ( where A is the this )
      for(register UInt_t rows   = fRows; rows; --rows) { // Loop over Rows of the output matrix C
	
	// At the Beginning we are already (row-wise) at Column 1 of B
	TComplex *ivec=invec;
	
	for(register UInt_t cols = iCols; cols; --cols) { //  Loop over Cols of the output matrix C
	  
	  iivec = ivec;                              // The top row of the moment. column in Matrix B
	  register TComplex* imatrix=matrix;         // the left side of moment. row of Matrix A
	  
	  // Handle output matrix C(row,cols)
	  //
	  *ovec=0;
	  for(register UInt_t  l = fCols;    l; --l) { 
	    *ovec += *iivec * *imatrix++;
	    iivec += iCols;
	  }
	  ++ovec;      // Next Entry in the Matrix C
	  ++ivec;      // Next Col in the Matrix B
	}    
	matrix+=fCols; // Next Row in the Matrix A
      }
      
      
      // Next Block
      
      invec  =iivec+1;    
#ifdef DL_ATLAS
    }
#endif
  }
  
# else
#   warning "Column-wise <TDSPMatrix::RightApply> not yet implemented" 
  Error("RightApply"," Column-wise not yet implemented!");
  return NULL;
# endif
#else
# warning "!! Using UNOPTIMIZED version of <TDSPMatrix::RightApply> !!"
  //
  // Un-optimized version 
  //
  Int_t stack_row = 0;
  while(stack_row<iRows) { // Loop over row-Stacked matrixes
    for(register UInt_t row=0;row<fRows; ++row)  { // Loop over Rows of the output matrix out
      for(register UInt_t col=0;col<iCols;++col) { //  Loop over Cols of the output matrix out
	out->Element(stack_row + row,col)=0;
	for(register UInt_t  l=0;l<fCols;++l) 
	  out->Element(stack_row + row,col) += Element(row,l) * inp->Element(l,col); // Adjoint multiplication
      }
    }
    stack_row+=fCols;
  }
#endif
  return out;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Matrix-Vector Multiplication out =  this * inp 
//

TDSPVector* TDSPMatrix::RightApply(TDSPVector *inp, TDSPVector *out) {

  if (!out) out = new TDSPVector();
  
  Int_t iLen = inp->GetLen();

  if (iLen%fCols) {
    Error("RightApply","Input Signal Length (%d) must be multiple of matrix cols (%d).", iLen,fCols);
    return NULL;
  }

  out->SetLen((iLen/fCols)*fRows);
  
  register TComplex *ovec = out->GetVec();
  TComplex *ivec          = inp->GetVec();
  
  
#ifdef DL_ATLAS

  if (fATLAS) {
    while(iLen) {
      //
      // Calculate C = alpha*A*B+beta*C
      //   
# ifdef  TCOMPLEX_USE_DOUBLE
      dl_cblas_zgemv(CblasRowMajor,CblasNoTrans,
		     fRows,fCols,  
		     &ComplexOne, fVec,fCols,  // alpha*A
		     ivec, 1,             // x
		     &ComplexZero, ovec,1);      // beta*y
# else
      dl_cblas_cgemv(CblasRowMajor,CblasNoTrans,
		     fRows,fCols, 
		     &ComplexOne, fVec,fCols,  // alpha*A
		     ivec, 1,             // B
		     &ComplexZero, ovec,1);      // beta*
# endif
      ivec+=fCols;
      ovec+=fRows;
      iLen-=fCols;
    }
  } else {

#endif
    
    while(iLen) {
      register TComplex* matrix = fVec;
      for(register UInt_t rows   = fRows; rows; --rows) {
	
	register TComplex* iivec  = ivec;
	*ovec=0;
	
	for(register UInt_t cols = fCols; cols; --cols)
	  *ovec += *iivec++ * *matrix++;
	
	++ovec;
      }
    
      // Next Block
      
      ivec+=fCols;
      iLen-=fCols;
    }

#ifdef DL_ATLAS
  }
#endif
  
  return out;
}

////////////////////////////////////////////////////////////////////
//
// Matrix-Vector Multiplication out =  (this*) * inp 
//

TDSPVector* TDSPMatrix::AdjointRightApply(TDSPVector *inp, TDSPVector *out) {

#ifndef WIN32
# warning "TDSPMatrix::AdjointRightApply ist falsch !!"
#endif

  Warning("AdjointRightApply","routine ist noch falsch ...!!");

  if (!out) out = new TDSPVector();
  
  Int_t iLen = inp->GetLen();

  if (iLen%fCols) {
    Error("AdjointRightApply","Input Signal Length (%d) must be multiple of matrix cols (%d).", iLen,fCols);
    return NULL;
  }

  out->SetLen((iLen/fCols)*fRows);
  
  register TComplex *ovec = out->GetVec();
  TComplex *ivec          = inp->GetVec();
  
  while(iLen) {
    register TComplex* matrix = fVec;
    for(register UInt_t rows   = fRows; rows; --rows) {

      register TComplex* iivec  = ivec;
      *ovec=0;

      for(register UInt_t cols = fCols; cols; --cols)
	*ovec += *matrix++ % *iivec++;
      
      ++ovec;
    }
    
    // Next Block
    
    ivec+=fCols;
    iLen-=fCols;
    
  }
  
  return out;
}

////////////////////////////////////////////////////////////////////
//
// Matrix-Vector Multiplication out =  (this^T) * inp = inp^T * this = inp * this
//

TDSPVector* TDSPMatrix::TransposedRightApply(TDSPVector *inp, TDSPVector *out) {
  return LeftApply(inp,out);
}

////////////////////////////////////////////////////////////////////
//
// Matrix-Vector Multiplication out =  inp * (this^T) = this * inp^T = this * imp
//

TDSPVector* TDSPMatrix::TransposedLeftApply(TDSPVector *inp, TDSPVector *out) {
  return RightApply(inp,out);
}
////////////////////////////////////////////////////////////////////
//
// Matrix-Vector Multiplication out =  inp * this 
//

TDSPVector* TDSPMatrix::LeftApply(TDSPVector *inp, TDSPVector *out) {

#ifdef DL_ATLAS
# warning  "TDSPVector* TDSPMatrix::LeftApply(TDSPVector *inp, TDSPVector *out) NOT USING ATLAS"
#endif

  if (!out) out = new TDSPVector();
  
  Int_t iLen = inp->GetLen();

  if (iLen%fRows) {
    Error("LeftApply","Input Vector Length (%d) must be multiple of matrix rows (%d).", iLen,fRows);
    return NULL;
  }

  out->SetLen((iLen/fRows)*fCols);
  
  register TComplex *ovec = out->GetVec();
  TComplex *ivec          = inp->GetVec();
  
  while(iLen) {
    for(register UInt_t cols = 0; cols<fCols; cols++) {
      register TComplex* iivec  = ivec;
      register TComplex* matrix = fVec + cols;
      *ovec=0;
      for(register UInt_t rows = fRows; rows; rows--) {
	*ovec += *iivec++ * *matrix;
	matrix+=fCols;
      }
      ovec++;
    }
    
    // Next Block
    
    ivec+=fRows;
    iLen-=fRows;
    
  }
  
  return out;
}

////////////////////////////////////////////////////////////////////
//
// Matrix-Vector Multiplication out =  inp * (this*) 
//

TDSPVector* TDSPMatrix::AdjointLeftApply(TDSPVector *inp, TDSPVector *out) {

  if (!out) out = new TDSPVector();
  
  Int_t iLen = inp->GetLen();

  if (iLen%fRows) {
    Error("AdjointLeftApply","Input Signal Length (%d) must be multiple of matrix rows (%d).", iLen,fRows);
    return NULL;
  }

  out->SetLen((iLen/fRows)*fCols);
  
  register TComplex *ovec = out->GetVec();
  TComplex *ivec          = inp->GetVec();
  
  while(iLen) {
    for(register UInt_t cols = 0; cols<fCols; cols++) {
      register TComplex* iivec  = ivec;
      register TComplex* matrix = fVec + cols;
      *ovec=0;
      for(register UInt_t rows = fRows; rows; rows--) {
	*ovec += *matrix++ % *iivec++;
	matrix+=fCols;
      }
      ovec++;
    }
    
    // Next Block
    
    ivec+=fRows;
    iLen-=fRows;
    
  }
  
  return out;
}

TComplex* TDSPMatrix::MoveVec(Int_t m, Int_t n) {
#ifdef LINALG_MAT_ROWWISE
  return fVec+=n+m*fCols;
#else
  return fVec+=m+n*fRows;
#endif
}


void    TDSPMatrix::Print() {
  cout << (*this) << endl;
}
void    TDSPMatrix::Input() {
  cin  >> (*this);
}

TH2* TDSPMatrix::Draw(Option_t *option, 
		      Double_t dx, Double_t xoff,
		      Double_t dy, Double_t yoff,
		      TH2 *h) {

  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;

  if ((pos = opt.Index("hist"))!= kNPOS) {

    opt.Remove(pos,4);

    if (!h) {
      //if (fHistogram2) delete fHistogram2;
      TString name = GetName();
      name+="_htemp";
      //fHistogram2 =  
      h = new TH2F(name.Data(),GetTitle(),
		   fRows,xoff-0.5*dx,xoff+fRows*dx-0.5*dx,
		   fCols,yoff-0.5*dy,yoff+fCols*dy-0.5*dy
		   );
    }
    
    Ssiz_t posr = opt.Index("real");
    if (posr!=kNPOS) opt.Remove(posr,4);

    Ssiz_t posl = opt.Index("imag");
    if (posl!=kNPOS) opt.Remove(posl,4);

    Ssiz_t posa = opt.Index("abs");
    if (posa!=kNPOS) opt.Remove(posa,3);

    Ssiz_t posp = opt.Index("phase");
    if (posp!=kNPOS) opt.Remove(posp,5);

    // Default - Draw Abs

    if ((posr==kNPOS)&&(posl==kNPOS)&&(posp==kNPOS)) posa=1;

    if (posa!=kNPOS) 
      // Draw the ABS
      for(UInt_t x=0;x<fRows;x++) for(UInt_t y=0;y<fCols;y++)
	h->SetBinContent(x+1,y+1,
		::Abs(Element(x,y)));
    else 
      
      if (posr!=kNPOS) 
	// Draw the Real Part
	for(UInt_t x=0;x<fRows;x++) for(UInt_t y=0;y<fCols;y++)
	  h->SetBinContent(x+1,y+1,
		  Element(x,y).fRe);
    
      else 
	if (posl!=kNPOS) 
	  // Draw the Imag Part
	for(UInt_t x=0;x<fRows;x++) for(UInt_t y=0;y<fCols;y++)
	  h->SetBinContent(x+1,y+1,
		  Element(x,y).fIm);
    
	else 
	  if (posp!=kNPOS) 
	    // Draw the Phase
	    for(UInt_t x=0;x<fRows;x++) for(UInt_t y=0;y<fCols;y++)
	      h->SetBinContent(x+1,y+1,
		      Element(x,y).Phase());
    
    
    h->Draw(opt);
    if (fXTitle) h->GetXaxis()->SetTitle(fXTitle);
    if (fYTitle) h->GetYaxis()->SetTitle(fYTitle);
    if (fZTitle) h->GetZaxis()->SetTitle(fZTitle);
    return h;
  } else {
    TDSPVector::Draw(option,dx,xoff);
    return NULL;
  }
}


TDSPVector* TDSPMatrix::GetColumn(Int_t c, TDSPVector *out) {
  
  if (c>=fCols) {
    Error("GetColumn","column index %d exceeds limit of %d!",c,fCols);
    return NULL;
  }
  
  if (!out) out=new TDSPVector();
  out->SetLen(fRows);

#ifdef LINALG_INDEX_OPT
  TComplex * vo = out->GetVec();
# ifdef LINALG_MAT_ROWWISE
  TComplex * vi = fVec+c;
# else 
  TComplex * vi = fVec+c*fRows;
# endif
#endif

  for(register UInt_t i=0;i<fRows;++i) {
#ifdef LINALG_INDEX_OPT
# ifdef LINALG_MAT_ROWWISE
    *vo++=*vi;vi+=fCols;
# else
    *vo++=*vi++;
# endif
#else
# warning "unoptimized version of TDSPMatrix::GetColumn" 
    out->Element(i) = Element(i,c);
#endif
  }

  return out;
}

TDSPVector * TDSPMatrix::GetRow(Int_t r, TDSPVector *out) {
  
  if (r>=fRows) {
    Error("GetRow","row index %d exceeds limit of %d!",r,fRows);
    return NULL;
  }

  if (!out) out=new TDSPVector();
  out->SetLen(fCols);

#ifdef LINALG_INDEX_OPT
  TComplex * vo = out->GetVec();
# ifdef LINALG_MAT_ROWWISE
  TComplex * vi = fVec+r*fCols;
# else 
  TComplex * vi = fVec+r;
# endif
#endif

  for(register UInt_t i=0;i<fCols;++i) {
#ifdef LINALG_INDEX_OPT
# ifdef LINALG_MAT_ROWWISE
    *vo++=*vi++;
# else
    *vo++=*vi;vi+=fRows;
# endif
#else
# warning "unoptimized version of TDSPMatrix::GetRow" 
    out->Element(i) = Element(r,i);
#endif
  }

  return out;
}

void TDSPMatrix::SetColumn(UInt_t c, TDSPVector *in, UInt_t ifrom, UInt_t ito) {
  
  if (!ito) ito=in->GetLen()-1;
  if (ito<ifrom) {
    Error("SetColumn","Reverse SetColumn not yet implemented - abort!");
    return;
  }

  if (ito-ifrom+1>fRows) {
    Info("SetColumn","Vector part (%d..%d) does not match matrix's fRows (%d) - using (%d..%d)!",
	 ifrom,ito,fRows,ifrom,ifrom+fRows-1);
    ito=ifrom+fRows-1;
  }

#ifdef LINALG_INDEX_OPT
  TComplex * vo = in->GetVec()+ifrom;
# ifdef LINALG_MAT_ROWWISE
  TComplex * vi = fVec+c;
# else 
  TComplex * vi = fVec+c*fRows;
# endif
#endif

  for(register UInt_t i=ifrom;i<=ito;++i) {
#ifdef LINALG_INDEX_OPT
# ifdef LINALG_MAT_ROWWISE
    *vi   = *vo++;vi+=fCols;
# else
    *vi++ = *vo++;
# endif
#else
# warning "unoptimized version of TDSPMatrix::SetColumn" 
    Element(i-ifrom,c) = in->Element(i);
#endif
  }
}

void TDSPMatrix::SetRow(UInt_t r, TDSPVector *in, UInt_t ifrom, UInt_t ito) {

  if (!ito) ito=in->GetLen()-1;
  if (ito<ifrom) {
    Error("SetRow","Reverse SetRow not yet implemented - abort!");
    return;
  }
  
  if (ito-ifrom+1>fCols) {
    Warning("SetRow","Vector part (%d..%d) does not match matrix's fCols (%d) - using (%d..%d)!",
	    ifrom,ito,fCols,ifrom,ifrom+fCols-1);
    ito=ifrom+fCols-1;
  }

#ifdef LINALG_INDEX_OPT
  TComplex * vo = in->GetVec()+ifrom;
# ifdef LINALG_MAT_ROWWISE
  TComplex * vi = fVec+r*fCols;
# else 
  TComplex * vi = fVec+r;
# endif
#endif

  for(register UInt_t i=ifrom;i<=ito;++i) {
#ifdef LINALG_INDEX_OPT
# ifdef LINALG_MAT_ROWWISE
    *vi++ = *vo++;
# else
    *vi   = *vo++;vi+=fRows;
# endif
#else
# warning "unoptimized version of TDSPMatrix::SetRow" 
    Element(r,i-ifrom) = in->Element(i);
#endif
  }
}

#ifdef HAVE_MATLAB
# include "../Matlab/TmxArray.h"
TDSPMatrix& TDSPMatrix::operator=(TmxArray& m) { 
  m.Get(this); 
  return (*this);
}
#endif

TDSPMatrix* TDSPMatrix::ReversedColumns(TDSPMatrix *r) {
  if (!r) r = new TDSPMatrix();
  r->SetSize(fRows,fCols);
  for(UInt_t m=0;m<fRows;++m) {
    UInt_t n1=fCols;
    for(UInt_t n=0;n<fCols;++n)
      r->Element(m,--n1) = Element(m,n);
  }
  return r;
}

TDSPMatrix* TDSPMatrix::ReversedRows(TDSPMatrix *r) {
  if (!r) r = new TDSPMatrix();
  r->SetSize(fRows,fCols);
  for(UInt_t n=0;n<fCols;++n) {
    UInt_t m1=fRows;
    for(UInt_t m=0;m<fRows;++m)
      r->Element(--m1,n) = Element(m,n);
  }
  return r;
}

void        TDSPMatrix::ReverseColumns() {
  UInt_t col2=fCols/2;
  for(UInt_t m=0;m<fRows;++m) {
    UInt_t n1=fCols;
    for(UInt_t n=0;n<col2;++n)
      Swap(m,--n1,m,n);
  }
}

void        TDSPMatrix::ReverseRows() {
  UInt_t row2=fRows/2;
  for(UInt_t n=0;n<fCols;++n) {
    UInt_t m1=fRows;
    for(UInt_t m=0;m<row2;++m)
      Swap(--m1,n,m,n);
  }
}

// Convolutes "inp" with the rows of "this" to "out" 
//
// out(i)=sum_(j<=i) (this)_ji inp(i-j)
// In a LTV-Filter operation the columns (i) are is the time and 
// the rows (j) are the delays
//

void        TDSPMatrix::RowConvolute(TComplex* inp, TComplex *out, 
				     Int_t N,
				     Bool_t cyclic) {
#ifndef WIN32
# warning "TDSPMatrix::RowConvolute unoptimized !"
#endif  
  
  // If N is not specified (=0) loop over the whole Matrix-Columns (in
  // LTV-filter operations this is the time-axis and the Rows are the
  // the delayed coefficients
  //
  if (!N) N=fCols;
  
  // Check that we are not process more data that the matrix
  // is prepared for ...

  if (N>fCols) {
    Error("RowConvolute","Number of indexes(%d) to process exceeds matrix columns (%d)",
	  N,fCols);
    return;
  }
  
  Int_t numdelays=1; 

  // We can assume the input signal as a cyclic infinite vector ...
  // or not ...
  if (cyclic) {
    if (N<fRows) {
      Error("RowConvolute","Vector length (%d) too small for matrix columns (matrix size= %d x %d)",
	    N,fRows,fCols);
      return;
    }
    // Loop over the time slots (the columns)
    for(register Int_t time=0;time<N;++time) {
      // Loop over the delay slots (the rows)
      register Int_t delay = 0;
      for(;delay<numdelays;++delay) 
	out[time] += Element(delay,time)*inp[time-delay];
      for(;delay<fRows;++delay)  // the cyclic part
	out[time] += Element(delay,time)*inp[N-delay];
      // Update the maximal possible delay 
      if (numdelays<fRows) ++numdelays;

    }
  } else {
    // Loop over the time slots (the columns)
    for(register Int_t time=0;time<N;++time) {
      // Loop over the delay slots (the rows)
      for(register Int_t delay = 0;delay<numdelays;++delay) 
	out[time] += Element(delay,time)*inp[time-delay];
      // Update the maximal possible delay 
      if (numdelays<fRows) ++numdelays;
    }
  }
}

// Convolutes "inp" with the rows of "this" to "out" 
//
// out(i)=sum_(j<=i) (this)_ji inp(i-j)
// In a LTV-Filter operation the columns (i) are is the time and 
// the rows (j) are the delays
//

TDSPVector* TDSPMatrix::RowConvolute(TDSPVector*inp, 
				     TDSPVector*out, 
				     Bool_t cyclic,
				     Bool_t zerosbefore
				     ) {

  if (!out) out   = new TDSPVector();

  Int_t ilen      = inp->GetLen();
  out->SetLen(ilen);
  if (zerosbefore) out->Zeros();

  /* make blocks in terms of the maximal possible time 
     (in filtering operations the time-axis is here in the columns) */
  
  Int_t blocks=ilen/fCols;
  Int_t   rest=ilen%fCols;

  TComplex *ivec = inp->GetVec();
  TComplex *ovec = out->GetVec();

  /* loop over blocks to repeat ... */
  
  for(Int_t block=0;block<blocks;++block) {
    RowConvolute(ivec, 
		 ovec,
		 fCols,
		 cyclic
		 );
    ivec+=fCols;
    ovec+=fCols;
  }
  if (rest)
    /* perform the remaining samples ! */
    RowConvolute(ivec, 
		 ovec,
		 rest,
		 cyclic
		 );
  return out;
}


void    TDSPMatrix::FFTShiftMe(Int_t dim) {
  switch(dim) {
    
  case 1: // Row-Wise 
  case 'R':
  case 'r':
    {
      Int_t l = fCols/2;
      if (fCols%2) { // Odd number of Columns
#ifndef WIN32
# warning "TDSPMatrix::FFTShift for odd rows/cols not optimized!"
#endif
	TDSPMatrix _tmp(fRows,fCols);
	Copy(&_tmp);
	for(register Int_t row=0;row<fRows;++row) {
	  for(register Int_t i=0;i<=l;++i) 
	    Element(row,i+l) = _tmp.Element(row,i);
	  for(register Int_t j=0;j<l;++j)
	    Element(row,j)   = _tmp.Element(row,j+l+1);
	}
      } else {       // Even number of Columns
	for(register Int_t row=0;row<fRows;++row)
	  for(register Int_t i=0;i<l;++i)
	    Swap(row,i+l,row,i);
      }
	
    }
    break;
  case 2: // column-Wise
  case 'C':
  case 'c':
    {
      Int_t l = fRows/2;
      if (fRows%2) { // Odd number of Rows
	
	TDSPMatrix _tmp(fRows,fCols);
	Copy(&_tmp);
	for(register Int_t col=0;col<fCols;++col) {
	  for(register Int_t i=0;i<=l;++i) 
	    Element(i+l,col) = _tmp.Element(i,col);
	  for(register Int_t j=0;j<l;++j)
	    Element(j,col)   = _tmp.Element(j+l+1,col);
	}
      } else {       // Even number of Rows
	for(register Int_t col=0;col<fCols;++col)
	  for(register Int_t i=0;i<l;++i)
	    Swap(i+l,col,i,col);
      }
    }
    break;
  default:
    Error("FFTShift","Unknown Option !");
    break;
    
  }
}



void TDSPMatrix::SetDiag(TComplex a) {
  Int_t minMN = TMath::Min(fRows,fCols);
  for(register Int_t i=0;i<minMN;++i)
    Element(i,i)=a;
}

void TDSPMatrix::AddDiag(TComplex a) {
  Int_t minMN = TMath::Min(fRows,fCols);
  for(register Int_t i=0;i<minMN;++i)
    Element(i,i)+=a;
}

void TDSPMatrix::MultDiag(TComplex a) {
  Int_t minMN = TMath::Min(fRows,fCols);
  for(register Int_t i=0;i<minMN;++i)
    Element(i,i)*=a;
}

void TDSPMatrix::SetNonDiag(TComplex a) {
  for(register Int_t iRow=0;iRow<fRows;++iRow)
    for(register Int_t iCol=0;iCol<fCols;++iCol) 
      if (iRow==iCol) continue; else
	Element(iRow,iCol)=a;
}

void TDSPMatrix::SetDiag(TDSPVector *a) {
  Int_t minMN = TMath::Min(fRows,fCols);

  if ((fStrict)&&(minMN!=a->GetLen())) {
    switch(fStrict) {
    case kStrictWarning:
      Warning("SetDiag","vector length(%d) and diag length(%d) differ - using minimum(%d)!\n",
	      a->GetLen(),minMN,TMath::Min(minMN,a->GetLen()));
      break;
    case kStrictNone:
      Error(  "SetDiag","vector length(%d) and diag length(%d) differ - abort (StrictMode enabled)!\n",
	      a->GetLen(),minMN);
      return;
    }
  }
  minMN       = TMath::Min(minMN,a->GetLen());
  for(register Int_t i=0;i<minMN;++i)
    Element(i,i)=a->Element(i);
}

void TDSPMatrix::AddDiag(TDSPVector *a) {
  Int_t minMN = TMath::Min(fRows,fCols);

  if ((fStrict)&&(minMN!=a->GetLen())) {
    switch(fStrict) {
    case kStrictWarning:
      Warning("AddDiag","vector length(%d) and diag length(%d) differ - using minimum(%d)!\n",
	      a->GetLen(),minMN,TMath::Min(minMN,a->GetLen()));
      break;
    case kStrictNone:
      Error(  "AddDiag","vector length(%d) and diag length(%d) differ - abort (StrictMode enabled)!\n",
	      a->GetLen(),minMN);
      return;
    }
  }
  minMN       = TMath::Min(minMN,a->GetLen());
  for(register Int_t i=0;i<minMN;++i)
    Element(i,i) += a->Element(i);
}


void TDSPMatrix::MultDiag(TDSPVector *a) {
  Int_t minMN = TMath::Min(fRows,fCols);

  if ((fStrict)&&(minMN!=a->GetLen())) {
    switch(fStrict) {
    case kStrictWarning:
      Warning("MultDiag","vector length(%d) and diag length(%d) differ - using minimum(%d)!\n",
	      a->GetLen(),minMN,TMath::Min(minMN,a->GetLen()));
      break;
    case kStrictNone:
      Error(  "MultDiag","vector length(%d) and diag length(%d) differ - abort (StrictMode enabled)!\n",
	      a->GetLen(),minMN);
      return;
    }
  }
  minMN       = TMath::Min(minMN,a->GetLen());
  for(register Int_t i=0;i<minMN;++i)
    Element(i,i) *= a->Element(i);
}
// extracts the mxn submatrix at (atRow,atCol)

TDSPMatrix* TDSPMatrix::SubMatrix(Int_t atRow, Int_t atCol, 
				  Int_t m, Int_t n, TDSPMatrix*out) {
  
  if (m+atRow>fRows) {
    Error("SubMatrix","Submatrix %d x %d at row %d exceed dimensions %d x %d",
	  m,n,atRow,fRows,fCols);
    return NULL;
  }
  if (n+atCol>fCols) {
    Error("SubMatrix","Submatrix %d x %d at column %d exceed dimensions %d x %d",
	  m,n,atCol,fRows,fCols);
    return NULL;
  }
  
  if (!out) out = new TDSPMatrix();
  out->SetSize(m,n);
  for(register Int_t row=0;  row<m;++row) 
    for(register Int_t col=0;col<n;++col) 
      out->Element(row,col) = Element(row+atRow, col+atCol);
  return out;
}

TDSPMatrix* TDSPMatrix::TransposedSubMatrix(Int_t atRow, Int_t atCol, 
					    Int_t m, Int_t n, TDSPMatrix*out) {
  
  if (m+atRow>fRows) {
    Error("SubMatrix","Submatrix %d x %d at row %d exceed dimensions %d x %d",
	  m,n,atRow,fRows,fCols);
    return NULL;
  }
  if (n+atCol>fCols) {
    Error("SubMatrix","Submatrix %d x %d at column %d exceed dimensions %d x %d",
	  m,n,atCol,fRows,fCols);
    return NULL;
  }
  
  if (!out) out = new TDSPMatrix();
  out->SetSize(n,m);
  for(register Int_t row=0;  row<m;++row) 
    for(register Int_t col=0;col<n;++col) 
      out->Element(col,row) = Element(row+atRow, col+atCol);
  return out;
}




TComplex TDSPMatrix::Trace() {
  UInt_t m=TMath::Min(fCols,fRows);
  UInt_t j=0;
  TComplex tr=0;
  for(UInt_t i=0;i<m;++i) {
    tr+=fVec[j];
#ifdef LINALG_MAT_ROWWISE
    j+=fCols;
#else
    j+=fRows;
#endif
    ++j;
  }
  return tr;
}

Double_t TDSPMatrix::Trace2() {
  UInt_t m=TMath::Min(fCols,fRows);
  UInt_t j=0;
  TComplex tr=0;
  for(UInt_t i=0;i<m;++i) {
    tr+=::Abs2(fVec[j]);
#ifdef LINALG_MAT_ROWWISE
    j+=fCols;
#else
    j+=fRows;
#endif
    ++j;
  }
  return tr;
}


TH2    *TDSPMatrix::fHistogram2 = NULL;
