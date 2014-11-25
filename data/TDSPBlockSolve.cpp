/** TDSP *******************************************************************
                          TDSPBlockSolve.cpp  -  description
                             -------------------
    begin                : Mon Mar 18 2002
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

#include "TDSPBlockSolve.h"

ClassImpQ(TDSPBlockSolve)

TDSPBlockSolve::TDSPBlockSolve(char *name) : TDSPOutput1(name) {
  fSolver = NULL;
}
TDSPBlockSolve::~TDSPBlockSolve(){
}

void TDSPBlockSolve::Print() {
  
  TDSPOutput1::Print();
  if (fSolver)
    fSolver->Print();
  else
    cout << "No solver connected ..." << endl;

}

TDSPSignal* TDSPBlockSolve::Solve(TDSPSignal *input) {
  if (!fSolver) {
    Error("Solve","No External Solver set !");
    return NULL;
  }

  Int_t len  = input->GetLen();
  Int_t inum = fSolver->GetNumInputs();
  Int_t onum = fSolver->GetNumOutputs();
  Double_t r = fSolver->Redundancy();
  if (len%inum) {
    Error("Solve","Input signal length (%d) is not a multiple of the solver's input dimension (%d)",
	  len,inum);
    return NULL;
  }
  
  Int_t times = len/inum;
  
  // Set the Length of the Output Signal

  fSignal->SetLen(times*onum);

  // Create an Alias to the input and output signals

  TDSPVector *ia = input->Alias(NULL,inum);
  TDSPVector *oa = fSignal->Alias(NULL,onum);
    
  for(Int_t i=0;i<times;i++) {

    // Prepare this Block

    fSolver->SetX(oa);
    fSolver->SetY(ia);
    // Solve this Block
    
    if (!fSolver->Solve()) 
      Error("Solve","No Solution found for Block %d!",i);
    
    // Next Block

    ia->MoveVec(inum);
    oa->MoveVec(onum);

  }

  // Delete the Aliases

  delete ia;
  delete oa;

  // Final Configurations

  fSignal->Configure(input);
  fSignal->SetSamplingRate(input->GetSamplingRate()*r);

  
  return fSignal;

}
