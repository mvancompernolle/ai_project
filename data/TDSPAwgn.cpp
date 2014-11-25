/** TDSP *******************************************************************
                          TDSPAwgn.cpp  -  description
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

#include "TDSPAwgn.h"

ClassImpQ(TDSPAwgn)



TDSPAwgn::TDSPAwgn(char *name) : TDSPOutput1(name){
  fNoiseLevel = 0;
  fEnergy     = 0;
  fInEnergy   = 0;
  fTime       = 0;
}
TDSPAwgn::~TDSPAwgn(){
}

TDSPSignal* TDSPAwgn::Update(TDSPSignal *input) {

  // !! This we have to change later on !!
  // 
  Reset();
  
  
  // Add the Noise
  //
  
  fInEnergy     += input->Energy();

  TDSPSignal *output=GetSignal();
  output->Configure(input);
  output->RandNC(input,fNoiseLevel*TMath::Sqrt(input->GetSamplingRate())); // that is sigma^2_n = sigma^2_N / epsilon


  // Update the Output Signal Energy
  //
  
  fEnergy       += output->Energy(); // that is x^*x * epsilon
  fTime         += output->GetTime(); // that is N * epsilon 
  

  Emit(Signal_NewData,output);

  return output;

}


void TDSPAwgn::Reset() {

  fInEnergy    = 0;
  fEnergy      = 0;
  fTime        = 0;
  
  Emit("Reset()");
}
