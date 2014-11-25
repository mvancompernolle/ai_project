/** TDSP *******************************************************************
                          TDSPHarmonicGenerator.cpp  -  description
                             -------------------
    begin                : Mon Sep 10 2001
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


#include "TDSPHarmonicGenerator.h"

ClassImpQ(TDSPHarmonicGenerator)


TDSPHarmonicGenerator::TDSPHarmonicGenerator(char *name) : TDSPOutput1(name){
 
  Spectrum  = new TDSPSpectrum();
  RegisterSignalsAndSlots();

}
TDSPHarmonicGenerator::~TDSPHarmonicGenerator(){
}



void TDSPHarmonicGenerator::Fill(){
  register Int_t i,j;
  
  TDSPSignal *Signal = GetSignal();
  TComplex   *signal = Signal->GetVec();
  Int_t       num    = Signal->GetLen();

  Int_t     Num      = Spectrum->GetNum();
  TComplex *freq     = Spectrum->GetFrequencies();
  TComplex *amp      = Spectrum->GetAmplitudes();

  TComplex c,c1;
  
  for(i=0;i<num;i++) {
    c=0;
    for(j=0;j<Num;j++) {
      c1=Exp(freq[j]*i);
      c+=amp[j]*c1;
    }
    signal[i]=c;
  }
}


