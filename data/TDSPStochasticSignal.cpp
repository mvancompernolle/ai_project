/** TDSP *******************************************************************
                          TDSPStochasticSignal.cpp  -  description
                             -------------------
    begin                : Mon Dec 10 2001
    copyright            : (C) 2001 by Peter Jung
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

#include "TDSPStochasticSignal.h"

ClassImp(TDSPStochasticSignal)

eStochasticType gStochasticSignal_Default=kNoChange;

TDSPStochasticSignal::TDSPStochasticSignal(Int_t len) : TDSPSignal(len) {
  fAllocedNumMeans= 0;
  fRepresentation = -1;
  fMode           = gStochasticSignal_Default;
  fSigmas         = new TComplex[1];  // at least 
  fSigmas[0]      = 1;
  fFifoFilter     = NULL;
  fVecChanged  = kFALSE;
  fARMAInput      = new TDSPSignal();
}

TDSPStochasticSignal::~TDSPStochasticSignal(){
}



//////////////////////////////////////////////////////////
//
// Simple Stochastic Convolutions
//
// a[ 0..na(i)-1]   = channel response (could be timedependent - index i - via SetRepresentation() )
// b[ 0..nb     ]   = input signal
// c[ i = 0..nc-1 ] = Sum_(j=0..M-1) ( C(a[i,j])*b[i-j] )
//
// M=min[na,i]
//
// nc  = na+nb-1  
// nc  = na          ( opt==kTDSPSignal_Conv_Cut )
// C() = Conjugate   ( opt==kTDSPSignal_Conv_Conj ) 
// C() = 1        
// 
/////////////////////////////////////////////////////////

void TDSPStochasticSignal::Conv(TDSPVector*           si_b,  // input vector
				TDSPVector*           si_c,  // output vector
				Int_t                 opt) {
  
  register Int_t k,j;

  // Configure the output Signal and base of the input signal
  //
  si_c->Configure(si_b);

  // Get the Rate of the input Signal 
  //
  Double_t rate = si_b->GetSamplingRate();

  Int_t     nb = si_b->GetLen();
  TComplex  *b = si_b->GetVec();
  TComplex  *c = NULL;

    
  // The minimal output signal length if the stochastic signal is
  // nothing 

  Int_t    nc=nb;
    
  Int_t    kk;

  TComplex tmp,tmp1;

  for(k=0;k<nc;k++) {
    

    // Accessing time index k of the output signal "si_c"
    // - set the "si_a" to this time
    SetRepresentation(rate);
    if (fVecChanged||(!k)) {
  
      // fVecChanged is true only if the size or      
      // the reference to the data has been changed
      
      // recalculate the size of the output vector
      // depending on the options
      
      nc = opt&kTDSPVector_Conv_MatchA ? Num : (opt&kTDSPVector_Conv_MatchB ? nb : Num+nb-1);
      
      // adjust the size of the output vector and set the position. In the case of
      // small changes this is fast. If response function get much more larger then it was
      // before and the input signal is not very long, this results in long copy actions !!
      //
      
      si_c->SetLen(nc);
      c = si_c->GetVec();

    }
    
    if (k<Num) kk=k; else kk=Num-1;
    tmp=0;
    for(j=0;j<=kk;j++) {
      tmp1=fVec[j];
      if (opt&kTDSPVector_Conv_Conj) tmp1.fIm*=-1;
      tmp+=tmp1*b[k-j];
    }
    c[k]=tmp;
  }

  
}

// Takes the current signal as the model for further
// Signal representations

void TDSPStochasticSignal::SetAsModel() { 
  
  if (fVec != fMeans) {
    
    // Delete the old Model if the size has been changed
    //
    if (fMeans) {
      if (fAllocedNum!=fAllocedNumMeans) {
	delete [] fMeans;
	fMeans = NULL;
      }
    }
    
    // Swap Signal and Model
    //
    TComplex *tmp = fMeans;
    fMeans        = fVec;
    fVec       = tmp;

    // Reserve Memory for the signal representation itself
    //
    if (!fVec) {
      fVec = new TComplex[fAllocedNum];      
      fVecChanged = kTRUE;
    }

    fAllocedNumMeans = fAllocedNum;
  }
}


void TDSPStochasticSignal::Reset() {

  switch(fMode) {
  case kFilterGauss:
  case kFilterRayleigh:
    //if (fFifoFilter) fFifoFilter->Reset();
    break;
  default:
    break;
  }
  
  ResetRating();

}
