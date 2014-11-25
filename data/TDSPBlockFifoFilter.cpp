/** TDSP *******************************************************************
                          TDSPBlockFifoFilter.cpp  -  description
                             -------------------
    begin                : Thu Jan 10 2002
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

#include "TDSPBlockFifoFilter.h"

ClassImpQ(TDSPBlockFifoFilter)

TDSPBlockFifoFilter::TDSPBlockFifoFilter(char *name) {
  fInput  = new TDSPBlockFifo();
  fOutput = new TDSPBlockFifo();
  fFilter = NULL;
  fWidth  = 0;
}
TDSPBlockFifoFilter::~TDSPBlockFifoFilter(){
}

void TDSPBlockFifoFilter::ConfigFifos() {
  if ((fWidth)&&(fFilter)) {
    fInput->SetSize( fWidth, fFilter->GetFIR()->GetLen()-1);
    fOutput->SetSize(fWidth, fFilter->GetIIR()->GetLen()-1);
  }
}

void TDSPBlockFifoFilter::SetFilter(TDSPFilter *f) {
  fFilter = f;
  ConfigFifos();

}
void TDSPBlockFifoFilter::SetWidth(Int_t i) {
  fWidth = i;
  ConfigFifos();
}


TDSPSignal* TDSPBlockFifoFilter::filter(TDSPSignal *input, TDSPSignal *output) {

  // If the width of Fifo's is not already set
  // then set it to the length of the first incoming signal
  //

  if (!fWidth) 
    SetWidth(input->GetLen());
  
  // Put the Signal into the input-fifo
  //
  fInput->Shift(input);
  
  // Make place in the output-fifo
  //
  fOutput->Shift();

  // Some Shortcuts for input-fifo parameters

  Int_t in_ind       = fInput->Back(fInput->GetIndex()); // that is the current signal
  Int_t in_depth     = fInput->GetDepth();
  TComplex *in_block = fInput->GetBlock();


  // Some Shortcuts for output-fifo parameters

  Int_t out_ind       = fOutput->Back(fOutput->GetIndex()); // that is the current signal
  Int_t out_depth     = fOutput->GetDepth();
  TComplex *out_block = fOutput->GetBlock();

  // pointer to fir-filter 
  //
  TComplex *fir_vec   = fFilter->GetFIR()->GetVec();

  // pointer to iir-filter 
  //
  TComplex *iir_vec   = fFilter->GetIIR()->GetVec();
  
  // Pointer to the result

  register TComplex* ovec = fOutput->GetBlock()+out_ind*fWidth;

  // Loop through the cached input signals
  // in the fifo and apply the FIR
  //
  register Int_t i=0;
  for(;i<=in_depth;i++) {
    register TComplex* vec  = &in_block[in_ind*fWidth];
    // Loop over 1 input signal
    //
    if (i) 
      for(register Int_t j=0;j<fWidth;j++)
	ovec[j] += vec[j]*fir_vec[i];
    else 
      for(register Int_t j=0;j<fWidth;j++)
	ovec[j]  = vec[j]*fir_vec[0];

    // Back Signal in the Fifo
    //
    in_ind  = fInput->Back(in_ind);
  }


  // Loop through the cached output signals
  // in the fifo and apply the IIR
  //
  for(i=1;i<=out_depth;i++) {

    // Back Signal in the Fifo
    //
    out_ind  = fOutput->Back(out_ind);

    register TComplex* vec  = &out_block[out_ind*fWidth];
    // Loop over 1 output signal
    //
    for(register Int_t j=0;j<fWidth;j++)
      ovec[j] -= vec[j]*iir_vec[i];
    
  }
  
  // Normalize
  TComplex norm = *iir_vec;
  if (norm!=1.0) {
    // Loop over 1 output signal
    //
    for(register Int_t j=0;j<fWidth;j++)
      ovec[j] /= norm;
  }

  output->SetVec(ovec,fWidth);
  
  return output;

}

