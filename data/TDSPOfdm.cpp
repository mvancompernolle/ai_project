/** TDSP *******************************************************************
                          TDSPOfdm.cpp  -  description
                             -------------------
    begin                : Fri Nov 23 2001
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

#include "TDSPOfdm.h"

ClassImpQ(TDSPOfdm)

TDSPOfdm::TDSPOfdm(char *name) : TDSPBlockfft(name) {
  
  // Add Slots 
  AddSlot("Receive(TDSPSignal*)");
  AddSlot("Transmit(TDSPSignal*)");
  
  // Add the Signals
  AddSignal("Received(TDSPSignal*)");
  AddSignal("Transmitted(TDSPSignal*)");
  
  
  fFFT_Signal->SetTitle("the received signal (FFT)");
  fIFFT_Signal->SetTitle("the transmit signal (IFFT)");

  
  fAutoEqualization=kFALSE;
  fEqualization=NULL;

}

TDSPOfdm::~TDSPOfdm(){
}

void TDSPOfdm::SetOption(Option_t *option) {
  
  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  if ((pos = opt.Index("autoequalization"))!=kNPOS) {
    fAutoEqualization=kTRUE;
    opt.Remove(pos,17);
  }

  if ((pos = opt.Index("noautoequalization"))!=kNPOS) {
    fAutoEqualization=kFALSE;
    opt.Remove(pos,19);
  }

  if (opt.Strip().Length()) 
    Error("SetOption","Unknown Option(s) \"%s\" !!",opt.Strip().Data());
  
}

void TDSPOfdm::SetEqualization(TDSPVector *e) {
  fEqualization = e->Copy(fEqualization);
}


void TDSPOfdm::EqualizeMe(TDSPVector *in) {
  if (!in) in=fFFT_Signal;

  if (!in) {
    Error("EqualizeMe","No vector for equalization given ...");  
    return;
  }

  TDSPVector *e = GetEqualization();

  if (!e) {
    Error("EqualizeMe","Have no equalization vector !");
    return;
  }
  Int_t       e_len = e->GetLen();
  Int_t       i_len = in->GetLen();

  if (!e_len) {
    Error("EqualizeMe","Equalization vector has zero length");
    return;
  }

  if (i_len%e_len) {
    Warning("EqualizeMe","input signal length(%d) isnt a mult. of equ.-blocklen(%d) - truncating to %d!",
	    i_len,e_len,i_len=(i_len/e_len)*e_len);
  }

  TComplex *e_vec = e->GetVec();
  TComplex *i_vec = in->GetVec();
  Int_t j=0;
  for(register Int_t i=0;i<i_len;++i) { 
    *i_vec++ *= e_vec[j++]; if (j==e_len) j=0;
  }

}


TDSPSignal* TDSPOfdm::Receive(TDSPSignal *in) {
  TDSPSignal *o = FFT(in);
  if (fAutoEqualization) EqualizeMe(o);
  Received(o);
  return      o;
}


TDSPSignal* TDSPOfdm::Transmit(TDSPSignal *in) {
  TDSPSignal *o = IFFT(in);
  Transmitted(o);
  return      o;
}


Bool_t  TDSPOfdm::CalcEqualization(TDSPVector *r) {

  Bool_t equalization_ok = kTRUE;

  if (!r) {
    Error("CalcEqualization","Got a channel response = NULL ");
    return kFALSE;
  }

  Int_t r_len = r->GetLen();
  Int_t     N = GetNumCarriers();

  if (!r_len) {
    Error("CalcEqualization","Got a channel response of zero length !!");
    return kFALSE;
  }

  if (r_len>GetCyclicPrefix()) {
    Warning("CalcEqualization","Channel Length (%d) exceeds cyclic prefix (%d) !!",
	    r_len,GetCyclicPrefix());
  }

  if (r_len>N) r_len = N;


  TDSPVector H(N);
  H.Zeros();
  
  // ::fft(r,GetNumCarriers(),&H); (this is not yet implemented ... 
  for (register Int_t n=0;n<r_len;++n) H.Element(n)=r->Element(n);
  ::fft(&H,N,&H);
  H*=TMath::Sqrt(Double_t(N));

  for(register Int_t i = 0;i<H.GetLen();++i) {
    if (H.Element(i)) H.Element(i)=1./H.Element(i);
    else {
      Error("CalcEqualization"," H(f) has a zero pol at carrier %d (0..%d) -  using 1/H(f)=99.",i,H.GetLen()-1);
      H.Element(i)=99.;
      equalization_ok = kFALSE;
    }
  }
  SetEqualization(&H);

  return equalization_ok;
}
