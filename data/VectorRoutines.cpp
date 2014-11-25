/** TDSP *******************************************************************
                          VectorRoutines.cpp  -  description
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

#include <TError.h>
#include <TDSPVector.h>
#include <TDSPMatrix.h>
#include <TDSPMath.h>

TDSPVector* Index(TComplex x, TComplex y, TComplex step, TDSPVector *result) {

#ifndef WIN32
# warning "Index(..) still contains some errors ..."
#endif

  if (step==0.0) {
    Error("Index","index step can not be zero !!");
    return NULL;
  }

  if (y==0.0){
    Int_t x1=x;
    if ((   Abs(x)==x1)&&
	(step==1.0)){
      y=x-1;
      x=0;
    } 
  }

  TComplex l1=(y-x)/step;
  l1=l1+1;
  if (l1.fIm) {
    Error("Index","can not build that index");
    return NULL;
  }

  Int_t    l2=l1;
  if (Abs(l1) != l2) {
    Error("Index","number of elements (%f) is not a positive integer", Abs(l1));
    return NULL;
  }
  
  if (!result) result = new TDSPVector();
  result->SetLen(l2);
  TComplex *v = result->GetVec();
  
  for(TComplex in=x;l2--;in+=step) 
    *v++ = in;
  
  return result;

}

TDSPVector* Rect(Int_t L, Int_t N, Double_t norm, TDSPVector *input) {

  norm/=TMath::Sqrt((Double_t)L);
  if (!input) input = new TDSPVector();

  input->SetLen(L);
  input->Zeros();

  TComplex *iv=input->GetVec();
  Int_t n1=(L-N)/2;
  for(register Int_t i=N;i>0;--i) 
    iv[n1++] = norm;
  return input;
}

TDSPVector* Rect_FFTShift(Int_t L, Int_t N, Double_t norm, TDSPVector *input) {

  norm/=TMath::Sqrt((Double_t)L);
  if (!input) input = new TDSPVector();

  input->SetLen(L);
  input->Zeros();

  TComplex *iv=input->GetVec();
  Int_t n1=(L-N)/2;
  for(register Int_t i=N;i>0;--i) 
    iv[n1++] = norm;
  
  input->FFTShiftMe();

  return input;
}

// GAUSSNK (Norbert Kaiblinger's Gauss function, modified by HGFei) 
// Author:     N.Kaiblinger, 20.Feb.1996 (kaibling@tyche.mat.univie.ac.at)
// 
// Copyright:  (c) NUHAG, Dept. Math., University of Vienna, Austria
//             Permission is granted to modify and re-distribute this
//             code in any manner as long as this notice is preserved.
//             All standard disclaimers apply.

TDSPVector* Gaussnk_FFTShift(Int_t L, Double_t norm, TDSPVector *input) {

  if (!input) input = new TDSPVector();

  input->SetLen(L);
  input->Zeros();

  TComplex *iv=input->GetVec();
  Double_t sqrtL = TMath::Sqrt((Double_t)L);

  for(register Int_t j=-3;j<4;++j)
    for(register Int_t i=0;i<L;++i) 
      iv[i] += TMath::Exp(-Pi*TMath::Power(i/sqrtL+j*sqrtL,2));

  norm/=input->Norm();

  (*input)*=norm;
  
  return input;
}

TDSPVector* Gaussnk(Int_t L, Double_t norm, TDSPVector *input) {
  input = Gaussnk_FFTShift(L,norm,input);
  input->FFTShiftMe();
  return input;
}



// Raised Cosine - written by Stefan Liggesmeier
//

TDSPVector* Rcos_FFTShift(Int_t L, Int_t N, Double_t rolloff, Double_t norm, TDSPVector *output) {
  
  
  if (!output) output = new TDSPVector();
  
  output->SetLen(L);
  output->Zeros();
  
  if (N%2 > 0) N+=1;
  for (int k=0; k<(N/2); k++) {
    // filling up to N/2-1
    if (k>(1+rolloff)/(2./N)) output->Element(k)=0;
    else {
      if (k>(1-rolloff)/(2./N)) 
	output->Element(k)=0.5*(1.+cos((Pi*(1./N)/rolloff)*(k-(1.-rolloff)/(2./N))));
      else 
	output->Element(k)=1.;
    }
    output->Element(N-k-1)=1.-output->Element(k);                // filling from N/2+1 up to N
    output->Element(L-k-1)=output->Element(k);                   // second slope too
    output->Element(L-N+k)=1.-output->Element(k);                // second slope too
  }

  // replace the samples (0.5 and 0.5) by the average of the
  // previous sample and 0.5 respectively by 0.5 and the following sample
  
  if (output->Element((N/2)-2) == 1.) {
    output->Element((N/2)-1)=1;
    output->Element((N/2))=1.-output->Element(N/2-1);
    output->Element(L-(N/2))=output->Element(N/2-1);
    output->Element(L-(N/2)-1)=1.-output->Element(N/2-1);
  } else {
    output->Element((N/2)-1)=(output->Element((N/2)-2)+0.5)/2;
    output->Element(N/2)=1.-output->Element((N/2)-1);
    output->Element(L-(N/2))=output->Element((N/2)-1);
    output->Element(L-(N/2)-1)=1.-output->Element((N/2)-1);
    // cout << "Index=" << (N/2) << endl;
    // cout << "output=" << output->Element((N/2)) << endl;getchar();
  }

  // for (Int_t i=0;i<L;i++) cout << "output=" << output->Element(i) << endl;

  output->Normalize(norm);
  return output;
}


TDSPVector* Rcos(Int_t L, Int_t N, Double_t rolloff, Double_t norm, TDSPVector *output) {
  output = Rcos_FFTShift(L,N,rolloff,norm, output);
  output->FFTShiftMe();
  return output;
}

TDSPVector* RandNC(UInt_t L, Double_t sigma) {
  TDSPVector* b = new TDSPVector(L);
  b->RandNC(sigma);
  return b;
}

// Vector of length L with "1" at one_pos and the rest is zero
TDSPVector*  Delta(UInt_t L, Int_t one_pos) {
  TDSPVector* b = new TDSPVector(L);
  b->Delta(one_pos);
  return b;
}

