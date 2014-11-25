/** TDSP *******************************************************************
                          TDSPChop.cpp  -  description
                             -------------------
    begin                : Tue Nov 13 2001
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

#include "TDSPChop.h"

ClassImpQ(TDSPChop)

TDSPChop::TDSPChop(char *name) : TDSPOutput1(name) {
  fSignalLength = 0; // that means auto-synchonize with input signallength
  fFilled = 0;

}
TDSPChop::~TDSPChop(){
}

TDSPSignal* TDSPChop::Update(TDSPSignal *input) {
  
  Int_t inputlen,outputlen;
  TDSPSignal *Signal = GetSignal();
  TComplex *odata    = NULL;
  TComplex *idata    = input->GetVec();

  // Get the input signal len
  //
  inputlen  = input->GetLen();
   
  // adjust output signal len
  //
  if (!fSignalLength)
    outputlen = inputlen; // Auto synchonisation to the input signal
  else 
    outputlen = fSignalLength; // fixed length

  // Processing now the input signal
  // Starting at index 0

  Int_t index = 0;

  // (1)
  // Do we have some rest data ?
  // Fill it up and Emit these data
  if (fFilled) {
    
    // Make sure that we are working on our
    // intern data block
    Signal->ResetSignal();
    odata = Signal->GetVec();

    // Fill it up
    while(fFilled==outputlen) {
      odata[fFilled++] = idata[index++]; 
      inputlen--;
    }
    // Emit
    Emit(Signal_NewData,Signal);

    // Cache is empty now
    fFilled=0;
  }

  // (2)
  // step by step pop the data burst from the input
  // signal if the outputlen is lower or equal to the input
  // length

  while(inputlen>=outputlen) {

    // Set the Pointer direct from the input signal partition
    // to the output signal

    Signal->SetVec(idata+index);

    index   +=outputlen;
    inputlen-=outputlen;

    // Emit "new data" so that listeners can act these data burst
    //

    Emit(Signal_NewData,Signal);

  }
  
  // (3)
  // save the rest of the input data into the cache
  //

  if (inputlen) {
    // Make sure that we are working on our
    // intern data block
    Signal->ResetSignal();

    // Get this adress if do not have it
    // already
    if (!odata) odata = Signal->GetVec();
    
    // Copy
    //
    while(inputlen) {
      odata[fFilled++] = idata[index++];
      inputlen--;
    }
  }

  // At the moment no return type

  return NULL;
  
}
