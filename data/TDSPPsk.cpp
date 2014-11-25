/** TDSP *******************************************************************
                          TDSPPsk.cpp  -  description
                             -------------------
    begin                : Mon Nov 19 2001
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

#include "TDSPMath.h"
#include "TDSPPsk.h"

ClassImpQ(TDSPPsk)



TDSPPsk::TDSPPsk(char   *name, 
		 Int_t   num,
		 UInt_t  opt) : TDSPSymbolGenerator(name, num, opt){

  // Book for num symbols
  //
  BookSymbols(num);
  InitSymbols();
}


void TDSPPsk::InitSymbols() {
  
  Double_t pi2 = 2*TMath::Pi();
    
  for(Int_t i=0;i<fNumSymbols;++i) {
    // Book the PSK-Symbols
    //
    Double_t t=i*pi2;
    t/=fNumSymbols;
    Double_t cosine = TMath::Cos(t);
    Double_t   sine = TMath::Sin(t);
    
    // not nice

    if (TMath::Abs(cosine)<1.e-15) cosine=0;
    if (  TMath::Abs(sine)<1.e-15)   sine=0;

    fSymbols[i] = TComplex(cosine,sine);

    // Flat props for Generation
    //
    fProbs[i] = 1;
    
    // Init the Sigmas for Detection
    fDetectSigmas[i] = 1;
  }  
  
  NormalizeProbs();
  
}

TDSPPsk::~TDSPPsk(){
}


Double_t TDSPPsk::SER(Double_t SNR) {

  switch(fNumSymbols) {
    
  case 2:
    // Proakis "Digital Comm."(4.ed.) p. 256 Eq. 5.2-5 
    //   or p. 270 Eq. 5.2-61
    //
    return TDSPMath::QSqrt(2*SNR);
  case 4:
    // Proakis "Digital Comm."(4.ed.) p. 270 Eq. 5.2-61
    //
    return 2*TDSPMath::Q(TMath::Sqrt(2*SNR)*TMath::Sin(TMath::Pi()/fNumSymbols));
  default:
    Error("SER","for fNumSymbols>4 not yet implemented!");
    return 0;
    break;
    
  }

}


