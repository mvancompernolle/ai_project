/** TDSP *******************************************************************
                          TDSPConvolution.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2001
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

#include "TDSPSignal.h"
#include "TDSPConvolution.h"

ClassImpQ(TDSPConvolution)


TDSPConvolution::TDSPConvolution(char *name) : TDSPOutput1(name) {
  
  ResponseFunction = NULL;
  fConvOptions     = kTDSPVector_Conv_MatchB;
  fBlockLen        = 0; // No use of blocks

  // Add the Slots
  Slot_Response           = AddSlot("Response(TDSPVector*)");
  Slot_StochasticResponse = AddSlot("Response(TDSPStochasticSignal*)");
  
  // Add signals
  AddSignal("Response(TDSPVector*)");
}


TDSPConvolution::~TDSPConvolution() {
}

void TDSPConvolution::SetOption(Option_t *option) {
  
  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  if ((pos = opt.Index("noblock"))!= kNPOS) {
    fConvOptions&=!kTDSPVector_Conv_UseBlockLen;
    opt.Remove(pos,7);
  }
  if ((pos = opt.Index("block"))!= kNPOS) {
    fConvOptions|=kTDSPVector_Conv_UseBlockLen;
    opt.Remove(pos,5);
  }
  
}

// TDSPSignal* TDSPConvolution::Conv(TDSPSignal *input) {
  
//   if (ResponseFunction)                // Static Response Function
//     ResponseFunction->Conv(input,fSignal,fConvOptions,fBlockLen);
//   else 
//     if (StochasticResponseFunction)    // Stochastic Response Function
//       StochasticResponseFunction->Conv(input,fSignal,fConvOptions);
//     else {
//       Error("Conv","No Response Function given !!");
//       return NULL;
//     }

//   return fSignal;

// }



TDSPVector* TDSPConvolution::Conv(TDSPVector *input) {
  
  if (ResponseFunction)                // Static Response Function
    ResponseFunction->Conv(input,fSignal,fConvOptions,fBlockLen);
  else 
    if (StochasticResponseFunction)    // Stochastic Response Function
      StochasticResponseFunction->Conv(input,fSignal,fConvOptions);
    else {
      Error("Conv","No Response Function given !!");
      return NULL;
    }

  return fSignal;

}


