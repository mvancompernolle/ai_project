/** TDSP *******************************************************************
                          TmxArray.cpp  -  description
                             -------------------
    begin                : Tue Jan 8 2002
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

#ifdef WIN32
# include "../../config.h"
#else
# include "config.h"
#endif

#include <Riostream.h>
#include <TComplex.h>

#ifdef HAVE_MATLAB
# define V5_COMPAT
# include <engine.h>
#else
typedef void Engine;
typedef void mxArray;
#endif

#include "TmxArray.h"

ClassImp(TmxArray)



TmxArray::TmxArray(){
  fmxArray = NULL;
  fMymx    = kTRUE;
#ifndef HAVE_MATLAB
  Warning("TmxArray","not compiled with matlab !");
#endif
}
TmxArray::TmxArray(const char *name){
  fmxArray = NULL;
  fMymx    = kTRUE;
#ifndef HAVE_MATLAB
  Warning("TmxArray","not compiled with matlab !");
#endif
  TNamed::SetName(name);
}

TmxArray::TmxArray(void *mx){
  fmxArray = NULL;
  fMymx    = kTRUE;
#ifndef HAVE_MATLAB
  Warning("TmxArray","not compiled with matlab !");
#else
  Setmx(mx);
#endif
}


TmxArray::~TmxArray(){
  Clean();
}

void TmxArray::Setmx(void *mx) {
  if (mx==fmxArray) return;
  if (!mx) 
    Error("Setmx","mxArray is NULL-Pointer");
  else {
    fmxArray = mx;
    fMymx    = kFALSE;
  }
}
void TmxArray::SetName(const char *n) { 
#ifdef HAVE_MATLAB
  if (fmxArray) mxSetName((mxArray*)fmxArray,n);
  else Error("SetName","Array not assigned!");
#endif
  TNamed::SetName(n);
}

Int_t TmxArray::GetN() {
#ifdef HAVE_MATLAB
  if (fmxArray) return mxGetN((mxArray*)fmxArray);
  else Error("GetN","Array not assigned!");
#endif
  return 0;
}

Int_t TmxArray::GetM() {
#ifdef HAVE_MATLAB
  if (fmxArray) return mxGetM((mxArray*)fmxArray);
  else Error("GetM","Array not assigned!");
#endif
  return 0;
}
const char *TmxArray::GetName() {
#ifdef HAVE_MATLAB
  if (fmxArray) return mxGetName((mxArray*)fmxArray);
  else Info("GetName","Array not assigned - returning TNamed::GetName() !");
#endif
  return TNamed::GetName();
}

void TmxArray::Clean() {
#ifdef HAVE_MATLAB
//   if (fmxArray&&fMymx) mxDestroyArray((mxArray*)fmxArray);
#endif
  fMymx    = kTRUE;
  fmxArray = NULL;
}

void TmxArray::Detach() {
  if (!fmxArray) 
    Error("Detach","Can not detach mxArray because I have no mxArray (fmxArray=NULL)!");
  else
    fMymx = kFALSE;
}

void TmxArray::Set(TDSPVector* s) {
  
#ifdef HAVE_MATLAB

  Int_t l     = s->GetLen();
  TComplex* v = s->GetVec();

  if ((!fmxArray)||(mxGetN((mxArray*)fmxArray) != l)) { 
    Clean();
    fmxArray = (void*)mxCreateDoubleMatrix(1, l, mxCOMPLEX); // a column vector
  }
  

  const char *name  = TNamed::GetName();
  
  SetName(name);
  
  Double_t *real = mxGetPr((mxArray*)fmxArray);
  Double_t *imag = mxGetPi((mxArray*)fmxArray);

  // Copy the vector

  for(register Int_t i=0;i<l;i++) {
    real[i] = v[i].fRe;
    imag[i] = v[i].fIm;
  }
#endif
}

void TmxArray::Set(TDSPMatrix* s) {

#ifdef HAVE_MATLAB
  
  Int_t rows     = s->GetRows();
  Int_t cols     = s->GetCols();
  Int_t len      = s->GetLen();

  TComplex*    v = s->GetVec();

  if ((!fmxArray)||
      ((mxGetN((mxArray*)fmxArray) != cols)||
       (mxGetM((mxArray*)fmxArray) != rows))) {
    
    Clean();
    fmxArray = mxCreateDoubleMatrix(rows, cols, mxCOMPLEX);
     
  }

  const char *name  = TNamed::GetName();
  
  SetName(name);
  
  Double_t *real = mxGetPr((mxArray*)fmxArray);
  Double_t *imag = mxGetPi((mxArray*)fmxArray);

  // Copy the vector

  register Int_t j=0,k=0;
  for(register Int_t i=0;i<len;i++) {
    real[j] = v[i].fRe;
    imag[j] = v[i].fIm;    
    if ((i+1)%cols) j+=rows;else j=++k;
  }
#endif
}


Bool_t      TmxArray::IsString() {
  
#ifdef HAVE_MATLAB
  if (!fmxArray) { Error("IsString","Array not assigned!");return kFALSE;};
  return (mxIsChar((mxArray*)fmxArray)==1);
#else
  return kFALSE;
#endif
}

char*       TmxArray::GetString(char *buf, Int_t buflen) {

#ifdef HAVE_MATLAB 
  if  (!IsString()) { Error("GetString","Array is not a matlab string!");return NULL;};
  
  // Get the real length of the String
  
  Int_t len = (mxGetM((mxArray*)fmxArray)*mxGetN((mxArray*)fmxArray))+1;

  if ((len>buflen)&&(buflen)) {
    Info("GetString","Returning truncated string (from len %d to %d)!",len,buflen);
    len = buflen;
  }

  if (!buf) buf = (char*)malloc(len*sizeof(char));
  
  if (mxGetString((mxArray*)fmxArray, buf, len)) 
    Info("GetString","String truncated by mxGetString!");
  
  return buf;
#else
  return NULL;
#endif
}

TDSPVector* TmxArray::Get(TDSPVector *s) {

#ifdef HAVE_MATLAB
  if (!fmxArray) { Error("Get","Array not assigned!");return NULL;};
  if (!s) s = new TDSPVector();
  
  Int_t N = GetN();
  s->SetLen(N);
  
  TComplex *v    = s->GetVec();
  Double_t *real = mxGetPr((mxArray*)fmxArray);
  Double_t *imag = mxGetPi((mxArray*)fmxArray);
  
  // Copy the vector
  if (imag) {
    for(register Int_t i=0;i<N;i++) {
      v->fRe     = *(real++);
      (v++)->fIm = *(imag++);
    }
  } else {
    for(register Int_t i=0;i<N;i++) {
      v->fRe     = *(real++);
      (v++)->fIm = 0;
    }
  }

  return s;
#else
  return NULL;
#endif
}

TDSPVector* TmxArray::GetRow(Int_t r, TDSPVector *s) {

#ifdef HAVE_MATLAB

  if (!fmxArray) { Error("GetRow","Array not assigned!");return NULL;};
  if (!s) s = new TDSPVector();
  
  Int_t M = GetM(); // number of rows
  Int_t N = GetN(); // number of columns
  if (r>=M) {
    Error("GetRow","row %d out of range 0..%d !",r,M-1);
    return NULL;
  }

  s->SetLen(N);
  
  TComplex *v    = s->GetVec();
  Double_t *real = mxGetPr((mxArray*)fmxArray)+r;
  Double_t *imag = mxGetPi((mxArray*)fmxArray);
  
  // Copy the row
  if (imag) {
    imag+=r;
    for(register Int_t i=0;i<N;i++) {
      v->fRe     = *real; real+=M;
      (v++)->fIm = *imag; imag+=M;
    }
  } else {
    for(register Int_t i=0;i<N;i++) {
      v->fRe     = *real;real+=M;
      (v++)->fIm = 0;
    }
  }

  return s;
#else
  return NULL;
#endif
}

TDSPVector* TmxArray::GetColumn(Int_t c, TDSPVector *s) {

#ifdef HAVE_MATLAB

  if (!fmxArray) { Error("GetColumn","Array not assigned!");return NULL;};
  if (!s) s = new TDSPVector();
  
  Int_t N = GetN(); // number of columns
  Int_t M = GetM(); // number of rows

  if (c>=N) {
    Error("GetColumn","column %d out of range 0..%d !",c,N-1);
    return NULL;
  }

  s->SetLen(M);
  
  TComplex *v    = s->GetVec();
  Double_t *real = mxGetPr((mxArray*)fmxArray)+c*M;
  Double_t *imag = mxGetPi((mxArray*)fmxArray);
  
  // Copy the row
  if (imag) {
    imag+=c*M;
    for(register Int_t i=0;i<M;i++) {
      v->fRe     = *(real++);
      (v++)->fIm = *(imag++);
    }
  } else {
    for(register Int_t i=0;i<M;i++) {
      v->fRe     = *(real++);
      (v++)->fIm = 0;
    }
  }

  return s;
#else
  return NULL;
#endif
}

TDSPMatrix* TmxArray::Get(TDSPMatrix *s) {

#ifdef HAVE_MATLAB

  if (!fmxArray) { Error("Get","Array not assigned!");return NULL;};
  if (!s) s = new TDSPMatrix();
  
  Int_t N = GetN();
  Int_t M = GetM();
  s->SetSize(M,N);
  Int_t L = s->GetLen();

  TComplex *v1    = s->GetVec();
  Double_t *real = mxGetPr((mxArray*)fmxArray);
  Double_t *imag = mxGetPi((mxArray*)fmxArray);
  TComplex *v    = v1;
  
  // Copy the vector
  if (imag) {
    for(register Int_t i=0;i<L;) {
      v->fRe     = *(real++);
      v->fIm     = *(imag++);
#ifdef LINALG_MAT_ROWWISE
      if ((++i)%M) v+=N; else v=++v1;
#endif
    }
  } else {
    for(register Int_t i=0;i<L;) {
      v->fRe     = *(real++);
      v->fIm = 0;
#ifdef LINALG_MAT_ROWWISE
      if ((++i)%M) v+=N; else v=++v1;
#endif
    }
  }

  return s;
#else
  return NULL;
#endif
}


TComplex TmxArray::Element(Int_t index) {

#ifdef HAVE_MATLAB

  if (fmxArray) {
    Double_t *real = mxGetPr((mxArray*)fmxArray);
    Double_t *imag = mxGetPi((mxArray*)fmxArray);
    if (imag) 
      return TComplex(real[index],imag[index]);
    else 
      return TComplex(real[index],0);
    
  } else {
    Error("Element","Array not assigned");
    return TComplex(0,0);
  }
#else 
  return TComplex(0,0);
#endif
}

void    TmxArray::Print() {

  if (!fmxArray) { Error("Print","Array not assigned!");return;};
  if (IsString()) {
    char *s = GetString();
    if (s) {
      cout << s << endl;
      delete s;
    }
    return;
  }
  Int_t N = GetN();
  Int_t M = GetM();
  TComplex tmp;
  for(Int_t n=0;n<N;++n) {
    tmp= operator()(n);
    cout << tmp;
  }
  cout << endl;

}
