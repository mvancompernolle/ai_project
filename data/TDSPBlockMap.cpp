/** TDSP *******************************************************************
                          TDSPBlockMap.cpp  -  description
                             -------------------
    begin                : Tue Feb 19 2002
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

#include "TDSPBlockMap.h"

ClassImpQ(TDSPBlockMap);

TDSPBlockMap::TDSPBlockMap(char *name) : TDSPOperator(name) {
  fMatrix = NULL;
  fPseudoInverseMatrix = NULL;

  fMAMode = kMatrixApplyReal;        // How to apply the Matrix
  fMAModeInverse = kMatrixApplyReal; // How to apply the Inverse Matrix

  fSignal        = new TDSPSignal("output1"); // The Output Signal
  fSignal->SetTitle("the output signal of the linear mapping");

  fInverseSignal = new TDSPSignal("output2"); // The Inverse Output Signal
  fInverseSignal->SetTitle("the output signal of the inverse linear mapping");

  // Add the Signals
  
  Signal_Update           = AddSignal("Update(TDSPSignal*)");
  Signal_InverseUpdate    = AddSignal("InverseUpdate(TDSPSignal*)");
  
  // Add the Slots
  
  Slot_Update             = AddSlot("Update(TDSPSignal*)");
  Slot_InverseUpdate      = AddSlot("InverseUpdate(TDSPSignal*)");


}
TDSPBlockMap::~TDSPBlockMap(){
}

TDSPSignal* TDSPBlockMap::Apply(TDSPSignal *input) {
  if (fMatrix) {
    fMatrix->Apply(input,fSignal,fMAMode);
    fSignal->Configure(input);
    fSignal->SetSamplingRate(input->GetSamplingRate()*fMatrix->Redundancy());
    return fSignal;
  }
  
  Error("Apply","No matrix connected to the object (name:%s)",GetName());
  return NULL;
    
}

TDSPSignal* TDSPBlockMap::InverseApply(TDSPSignal *input) {
  if (fPseudoInverseMatrix) {
    fPseudoInverseMatrix->Apply(input,fInverseSignal,fMAModeInverse);
    fInverseSignal->Configure(input);
    fInverseSignal->SetSamplingRate(input->GetSamplingRate()*fPseudoInverseMatrix->Redundancy());
    return fInverseSignal;
  }
  Error("InverseApply","No inverse matrix connected to the object (name:%s)",GetName());
  return NULL;
}


Int_t       TDSPBlockMap::GetBlockLen() {

  if (fMatrix) return fMatrix->GetCols();
  return 0;
}

Double_t    TDSPBlockMap::TestError(UInt_t num) {
  
  Double_t dist = 0;
  TDSPSignal TestSignal(GetBlockLen());
  TDSPSignal Result1;
  TDSPSignal Result2;
  
  for(UInt_t i=0;i<num;i++) {
    TestSignal.RandNC();
    fMatrix->RightApply(&TestSignal,&Result1);
    fPseudoInverseMatrix->RightApply(&Result1,&Result2);
    dist+=Dist(TestSignal,Result2);
  }
  
  return dist/((Double_t)(num*GetBlockLen()));
}
