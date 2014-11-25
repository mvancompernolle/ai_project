/** TDSP *******************************************************************
                          TDSPCorrelationFilter.cpp  -  description
                             -------------------
    begin                : Mon Jan 21 2002
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

#include <TGraph.h>
#include "../Matlab/TMatlab.h"
#include "../Matlab/TmxArray.h"
#include "TDSPCorrelationFilter.h"


ClassImp(TDSPCorrelationFilter)


TDSPCorrelationFilter::TDSPCorrelationFilter() : TDSPFilter() {
  
  fCorrelation = new TDSPSignal();
  fAR     = fMA = 0;
  fMethod = kCorrFilterProny;
  fNorm   = 0;
  fOptions = kCorrFilterNormalize;
}
TDSPCorrelationFilter::~TDSPCorrelationFilter(){
}

void TDSPCorrelationFilter::Init(Double_t MaxTime, Int_t Entries) {

  
  fCorrelation->SetLen(Entries);
  TComplex *c = fCorrelation->GetVec();
  SetRate(Entries/MaxTime);
  for(register Int_t i=0;i<Entries;i++) c[i] = Correlation(i/fRate);
  
  switch(fMethod) {

  case kCorrFilterYule:
  case kCorrFilterProny:
    {
      fNormalized = kFALSE;

#ifdef HAVE_MATLAB
      
      TMatlab *_Matlab = TMatlab::GlobalEngine();

      // Create an mxArray from "fCorrelation"
      // named c
      TmxArray *cmx = new TmxArray("c");
      cmx->Set(fCorrelation);
      
      // Put the Matlab Vector "cmx" in the running
      // Matlab Engine
      //
      _Matlab->Put(cmx);
      switch(fMethod) {
      case kCorrFilterProny:
	{
	  if (!fAR) fAR = 1; // Defaults
	  if (!fMA) fMA = 3; // Defaults
	  if (!_Matlab->EvalString("[a,b] = prony(c,%d,%d);",fAR,fMA)) 
	    Error(  "Init","Error executing <prony> in matlab (Signal Processing Toolbox?)");
	  else {
	    if (!_Matlab->EvalString("d = filternorm(a,b,2);")) 
	      Error("Init","Error executing <filternorm> in matlab (Signal Processing Toolbox?)");
	    else {
	      TmxArray *amx = _Matlab->Get("a");
	      TmxArray *bmx = _Matlab->Get("b");
	      TmxArray *dmx = _Matlab->Get("d");
	      FIR(amx->Get());
	      IIR(bmx->Get());
	      fNorm   = (*dmx)(0).fRe;
	      delete amx;
	      delete bmx;
	    }
	  }
	}
	break;
	  
	
       case kCorrFilterYule:
	{
	  if (!fAR) fAR = 20; // Defaults
	  fMA = 0;            // must be zero in Yule equations !!
	  if (!_Matlab->EvalString("[a] = aryule(c,%d);",fAR))
	    Error(  "Init","Error executing <aryule> in matlab (Signal Processing Toolbox?)");
	  else {
	    if (!_Matlab->EvalString("d = filternorm(a,1,2);")) 
	      Error("Init","Error executing <filternorm> in matlab (Signal Processing Toolbox?)");
	    else { 
	      TmxArray *amx = _Matlab->Get("a");
	      TmxArray *dmx = _Matlab->Get("d");
	      FIR(amx->Get());
	      IIR(NULL);
	      fNorm   = (*dmx)(0).fRe;
	      delete amx;
	    }
	  }
	}
	break;
      }
      
      delete cmx;
#else
      Error("Init","Implementation only avaiable with Matlab+Signal Processing ToolBox");
#endif
    }
    
    break;
    
  default:
    
    Error("Init","Unknown or unimplemented Method");
    break;
    
  }

  // Normalize the Filter ?
  //
  if (fOptions&kCorrFilterNormalize) Normalize();
  
}
// Because the is somehow the base class implementation
// set this correlation to a simple delay(0)

Double_t TDSPCorrelationFilter::Correlation(Double_t delay) {
  return (delay) ? 0.0 : 1.0;  
}

void TDSPCorrelationFilter::Test() {


  if (!fFIR) {
    Error("Test","No FIR-part of the Filter given !");
    return;
  } 

  if ((!fCorrelation)||(!fCorrelation->GetLen())) {
    Error("Test","Use Init() to initialize the correlation !");
    return;
  }

  TDSPVector* TestInput  = new TDSPVector(100000);
  TDSPVector* TestOutput = new TDSPVector(); 

  TestInput->RandNC();

  filter(TestInput,TestOutput);
  TDSPVector* AutoCorrelation = TestOutput->XCorr(-fCorrelation->GetLen(),kXCorrCoeff);

  AutoCorrelation->Draw("ac real",1.0/fRate);
  
  fCorrelation->Draw("c real--",1.0/fRate);

  cout << "Filter amplification is : " << TestOutput->Power()/TestInput->Power() << endl;
  
  delete TestInput;
  delete TestOutput;
  delete AutoCorrelation;
}
