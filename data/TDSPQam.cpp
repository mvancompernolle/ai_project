/** TDSP *******************************************************************
                          TDSPQam.cpp  -  description
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
#include "TDSPQam.h"

ClassImpQ(TDSPQam)



TDSPQam::TDSPQam(char   *name, 
		 Int_t   num,
		 UInt_t  opt) : TDSPSymbolGenerator(name, num, opt){
  // Book for num symbols
  //
  BookSymbols(num);
  InitSymbols();
}


void TDSPQam::InitSymbols() {
  
  
  Int_t    m     = (Int_t)TMath::Sqrt((Double_t)fNumSymbols);
  
  // QAM box len m : m modulo 2 should be zero !! 
  //
  if (m%2) {
    Error("BookSymbols",
	  "Correcting QAM Box len  of \"%s\" from %d to %d",
	  GetName(),m,m-m%2);
    m = m-m%2;
  }

  
  // QAM Box len m : m^2=fNumSymbols
  //
  if (m*m!=fNumSymbols) {
    
    Error("BookSymbols",
	  "Correcting number of symbols of \"%s\" from %d to %d",
	  GetName(),fNumSymbols,m*m);
    
    TDSPSymbolGenerator::BookSymbols(m*m);

  }

  // Normalisation factor, so that mean signal power is "1"
  //
  Double_t A = TMath::Sqrt(1.5/(m*m-1));
  
  // A=sqrt(3/2/(m-1)/(m+1));
  // c = A*((2*min(m-1,floor(m*rand(1,L)))+1-m)+i*(2*min(m-1,floor(m*rand(1,L)))+1-m));
  //
  //-(M-1) ... M-1

  Int_t i=0;

  for(Int_t   x=-(m-1);x<=(m-1);x+=2) {
    for(Int_t y=-(m-1);y<=(m-1);y+=2)   {
      
      fSymbols[i] = A*TComplex(x,y);
      
      // Flat props for Generation
      //
      fProbs[i] = 1;
      
      // Init the Sigmas for Detection
      fDetectSigmas[i] = 1;
      
      i++;
    }  
  }
  
  
  NormalizeProbs();
  
}

TDSPQam::~TDSPQam(){
}


Double_t TDSPQam::SER(Double_t SNR) {

  Int_t f2 = (Int_t)(TMath::Sqrt((Double_t)fNumSymbols));
  return 2*(1-1/f2)*TDSPMath::QSqrt(SNR*3.0/(fNumSymbols-1));

}


