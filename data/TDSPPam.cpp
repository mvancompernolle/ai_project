/** TDSP *******************************************************************
                          TDSPPam.cpp  -  description
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
#include "TDSPPam.h"

ClassImpQ(TDSPPam)



TDSPPam::TDSPPam(char   *name, 
		 Int_t   num,
		 UInt_t  opt) : TDSPSymbolGenerator(name, num, opt){
  // Book for num symbols
  //
  BookSymbols(num);
  InitSymbols();
}


void TDSPPam::InitSymbols() {
    
  // Pam fNumSymbols : m modulo 2 should be zero !! 
  //
  if (fNumSymbols%2) {
    Error("BookSymbols",
	  "Number of Symbols (%d) must be even !",fNumSymbols);
  }
  
  
  // Normalisation factor, so that mean signal power is "1"
  //
  Double_t A = TMath::Sqrt(3.0/((Double_t)(fNumSymbols*fNumSymbols)-1.0));
      
  Int_t i=0;
  
  for(Int_t   x=-(fNumSymbols-1);x<=(fNumSymbols-1);x+=2) {
    
    fSymbols[i] = A*x;
    
    // Flat props for Generation
    //
    fProbs[i] = 1;
    
    // Init the Sigmas for Detection
    fDetectSigmas[i] = 1;
    
    i++;
  }  
  
  NormalizeProbs();
  
}

TDSPPam::~TDSPPam(){
}


Double_t TDSPPam::SER(Double_t SNR) {

  return 2*(fNumSymbols-1)/fNumSymbols*TDSPMath::QSqrt(6*SNR/(fNumSymbols*fNumSymbols-1));
  
}


