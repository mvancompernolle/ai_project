/** TDSP *******************************************************************
                          TDSPFifo.cpp  -  description
                             -------------------
    begin                : Tue Feb 12 2002
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

#include "TDSPFifo.h"

ClassImp(TDSPFifo)

TDSPFifo::TDSPFifo(Int_t depth){
  fDepth = 0;
  fFill  = 0;
  fIndex = 0;
  fBlock = 0;  
  if (depth) SetDepth(depth);
}
TDSPFifo::~TDSPFifo(){
}

void TDSPFifo::Reset() {
  fFill  = 0;
  fIndex = 0;
}

void TDSPFifo::SetDepth(Int_t depth) { 
  if (depth!=fDepth) {
    fDepth=depth;
    if (fBlock) delete [] fBlock;
    fBlock = new TComplex[fDepth+1];
  }
}

// Makes the Dot-Product between this fifo content and a vector
//
TComplex  TDSPFifo::Dot(TDSPVector *fir, Bool_t conj) {
  TComplex *fir_vec = fir->GetVec();
  if (fir_vec)
    return Dot(fir_vec,conj, TMath::Min(fDepth,fir->GetLen()));
  Error("Dot","fit vector not yet alloced !");
  return 0;
}

// Makes the Dot-Product between this fifo and an another fifo
//
TComplex  TDSPFifo::Dot(TDSPFifo *f, Bool_t conj) {
  Int_t     len     = TMath::Min(fDepth,f->GetDepth());
  TComplex      tmp = 0;
  if (conj) 
    for(register Int_t j=0;j<len;++j) tmp += operator()(-j) % f->operator()(-j);
  else 
    for(register Int_t j=0;j<len;++j) tmp += operator()(-j) * f->operator()(-j);
  return tmp;
}

// Fill the fifo by a vector
//
void TDSPFifo::FillFrom(TDSPVector *in) {
  Int_t      l=in->GetLen();
  TComplex *iv=in->GetVec();
  for(register Int_t i=l-fDepth;i<l;++i)
    Shift(iv[i]);
}

// Fill from the fifo to a vector
//
TDSPVector* TDSPFifo::FillTo(TDSPVector *out) {
  if (!out) out=new TDSPVector();
  out->SetLen(fDepth);
  TComplex *ov=out->GetVec();
  for(register Int_t i=0;i<fDepth;++i)
    ov[i] = operator()(i);
  return out;
}

// Shift the in-vector through the fifo. Save the 
// output of the fifo in the out vector

TDSPVector* TDSPFifo::Shift(TDSPVector*in, TDSPVector*out) {
  if (!out) out=new TDSPVector();
  Int_t l=in->GetLen();
  out->SetLen(l);
  TComplex *iv=in->GetVec();
  TComplex *ov=out->GetVec();
  for(register Int_t i=0;i<l;++i)
    ov[i]=Shift(iv[i]);
  return out;
}

TComplex  filter(TDSPFifo *inp,
		 TDSPFifo *out,
		 TDSPVector *fir,
		 TDSPVector *iir) {

  TComplex     tmp,norm;
  Int_t        len;

  // Check the Ratings of the Coefficients - they must be equal
  //
  
  Double_t     rate = fir->GetSamplingRate();
  if (iir) if (rate!=iir->GetSamplingRate()) {
    Error("filter","Sampling Rates of FIR- and IIR-Filters differ !\n");
    return ComplexZero;
  }
  
  // FIR part
  //
  Int_t              fir_len = fir->GetLen();
  register TComplex *fir_vec = fir->GetVec();

  // IIR part
  //
  Int_t     iir_len;
  register TComplex *iir_vec;
  
  if (iir) {
    iir_len = iir->GetLen();
    iir_vec = iir->GetVec();
    norm    = iir_vec[0];
  } else {
    iir_len = 0;
    iir_vec = NULL;
    norm    = 1; 
  } 

  Int_t fill  = inp->GetFill();

  len=fir_len<fill ? fir_len : fill; 
  tmp=inp->operator()(-1)*fir_vec[0]; // the newest entry
  for(register Int_t i=1;i<len;i++) tmp += inp->operator()(-1-i)*fir_vec[i];

  fill  = out->GetFill();fill++;
  len=iir_len<fill ? iir_len : fill; 
  for(register Int_t j=1;j<len;j++) tmp -= (*out)(-j)*iir_vec[j];
  if ((norm.fRe!=1.0)||(norm.fIm!=0.0)) 
    tmp/=norm;
  
  return tmp;
}

TDSPVector *filter(TDSPFifo   *inp,
		   TDSPVector *out,
		   TDSPVector *fir,
		   TDSPVector *iir) {
  
  TComplex     tmp,norm;
  Int_t        len;

  // Check the Ratings of the Coefficients - they must be equal
  //
  
  Double_t     rate = fir->GetSamplingRate();
  if (iir) if (rate!=iir->GetSamplingRate()) {
    Error("filter","Sampling Rates of FIR- and IIR-Filters differ !\n");
    return NULL;
  }

  // Input Signal
  //
  Int_t                 iLen = inp->GetDepth();

  // Output Signal
  //
  out->SetLen(iLen);
  register TComplex    *ovec = out->GetVec();

  // Configure the Output Signal on the Base of the FIR Signal
  //
  out->Configure(fir);

  // FIR part
  //
  Int_t              fir_len = fir->GetLen();
  register TComplex *fir_vec = fir->GetVec();

  // IIR part
  //
  Int_t     iir_len;
  register TComplex *iir_vec;
  
  if (iir) {
    iir_len = iir->GetLen();
    iir_vec = iir->GetVec();
    norm    = iir_vec[0];
  } else {
    iir_len = 0;
    iir_vec = NULL;
    norm    = 1; 
  } 

  register Int_t j,i;
 
  if ((norm.fRe==1.0)&&(norm.fIm==0.0)) {
    for(i=0;i<iLen;i++) {
      len=fir_len<i ? fir_len : i+1; 
      tmp=inp->operator()(i)*fir_vec[0];
      for(j=1;j<len;j++) tmp += inp->operator()(i-j)*fir_vec[j];
      len=iir_len<i ? iir_len : i+1; 
      for(j=1;j<len;j++) tmp -= ovec[i-j]*iir_vec[j];
      ovec[i]=tmp;
    }
  } else {
    for(i=0;i<iLen;i++) {
      len=fir_len<i ? fir_len : i+1; 
      tmp=inp->operator()(i)*fir_vec[0];
      for(j=1;j<len;j++) tmp += inp->operator()(i-j)*fir_vec[j];
      len=iir_len<i ? iir_len : i+1; 
      for(j=1;j<len;j++) tmp -= ovec[i-j]*iir_vec[j];
      ovec[i]=tmp/norm;
    }
  }

  return out;
}

istream &operator>>(istream &is,TDSPFifo &z) {


  cout << "Enter a list of complex numbers :" << endl;
 
  z.SetDepth(404);
  Int_t i = 0;
  while(1) {
    is >> z(i);
    if (is.fail()) {is.clear();break;}
    i++;
  }
  cout << i << " values read." << endl;
  z.SetDepth(i);
  return ( is );
}
ostream &operator<<(ostream &os,TDSPFifo &z) {
  Int_t      i;
  Int_t      j = z.GetDepth();
    
  for(i=0;i<j;i++) {
    if (i<z.GetFill()) os << z(i);
    else os << "   _______    ";
    if ((!((i+1)%z.fPerLine))&&(i+1!=j)) os << endl;
  }
  return ( os );
}

istream &operator>>(istream &is,TDSPFifo *z) {
  is >> *z;
  return ( is );
}
ostream &operator<<(ostream &os,TDSPFifo *z) {
  os << *z;
  return ( os );
}

void    TDSPFifo::Print() {
  cout << (*this) << endl;
}
void    TDSPFifo::Input() {
  cin  >> (*this);
}


void TDSPFifo::Draw(Option_t *option, Double_t dx, Double_t xoff) {
  
  TDSPVector *tmp = FillTo();
  tmp->Draw(option,dx,xoff);
  delete tmp;
}

TGraph*   TDSPFifo::Graph(Option_t*option, TGraph *in, Double_t dx, Double_t xoff) {
  TDSPVector *tmp = FillTo();
  TGraph *g = tmp->Graph(option,in,dx,xoff);
  delete tmp;
  return g;
}


// Number of Values to Print per Line

UInt_t TDSPFifo::fPerLine = 4;
