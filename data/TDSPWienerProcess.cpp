/** TDSP *******************************************************************
                          TDSPWienerProcess.cpp  -  description
                             -------------------
    begin                : Die Mai 21 2002
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
#include <TError.h>
#include "TDSPWienerProcess.h"

ClassImpQ(TDSPWienerProcess)

TDSPWienerProcess::TDSPWienerProcess(char *name) : TDSPOutput1(name){
  fBurstOff=0;
  fPhase   =0;
}
TDSPWienerProcess::~TDSPWienerProcess(){
}


// Multiply phases as a Wiener-Levi-Process with step-variance of the phase
// = 2*Pi*B onto a vector

TDSPVector*  TDSPWienerProcess::MultRandPhase(TDSPVector *in,			   // input vector
					      Double_t variance,		   // outer variance
					      Double_t burstlen,		   // Burstlen
					      Double_t burstoff,		   // Offset
					      Double_t Phase,                     // Start Phase
					      TDSPVector *v       	           // output vector
					      ) {			   
  
 
  if (!in) {::Error("RandWienerPhase","no input vector given !");return NULL;} 
  
  TComplex *fin = in->GetVec();
  
  if (!fin) {::Error("RandWienerPhase","input vector not allocated !");return NULL;}
  
  Int_t    lin           = in->GetLen();
  Double_t SamplingRate  = in->GetSamplingRate();
  

  if (!v) v = new TDSPVector();
  v->SetLen(lin);
  v->Configure(in);

  TComplex     *fVec     = v->GetVec();

  Double_t dd;
  Int_t    iBurstlen,iBurstoff;
  
  // Adjust length of input vector (lin) for the Samplingrate
  if (!burstlen) {
    burstlen=lin/SamplingRate;
    iBurstlen=lin;
  } else {
    dd=Double_t(burstlen)*SamplingRate;
    iBurstlen=TMath::Nint(dd);
    if (TMath::Abs(iBurstlen-dd)>1.0e-4) 
      ::Warning("MultRandPhase","burstlen %f * Samplingrate %f = %f is not an integer (using %d)!", 
		burstlen,SamplingRate,dd,iBurstlen);
  }

  // Adjust the offset for the Samplingrate
  dd=Double_t(burstoff)*SamplingRate;
  iBurstoff=TMath::Nint(dd);
  if (TMath::Abs(iBurstoff-dd)>1.0e-4) 
	  ::Warning("MultRandPhase","Burstoffset %f * Samplingrate %f = %f is not an integer (using %d)!", 
	    burstoff,SamplingRate,dd,iBurstoff);


  Double_t     sigma = TMath::Sqrt(variance/SamplingRate);
  //TComplex     phase = variance*(burstlen-burstoff)+Phase; 
  //TComplex     phase = gRandom->Gaus(0,TMath::Sqrt(variance*(burstlen-burstoff)))+Phase;  // initial phase 
  TComplex     phase = gRandom->Gaus(0,TMath::Sqrt(variance*(burstlen-burstoff)))+
    gRandom->Gaus(0,TMath::Sqrt(Phase));  // initial phase 
  TComplex  expphase;								// initial phasor
  
  Int_t            j  = -iBurstoff;
  //Double_t  sqrtPhase = TMath::Sqrt(Phase);
  
  for(register UInt_t i=0;i<lin;++i) {
    //if (!(j++%iBurstlen)) phase=Phase;
    if (!(j++%iBurstlen)) phase=gRandom->Gaus(0,TMath::Sqrt(Phase));
    expphase   = TComplex(TMath::Cos(phase),TMath::Sin(phase)); // create exp(I*phase)
    fVec[i]    = fin[i]*expphase;								// multiply the phasor
    phase     += gRandom->Gaus(0,sigma);				// next phase step
  }

  return v;

}


TDSPVector* TDSPWienerProcess::Apply(TDSPVector* input, TDSPVector *output) {
  return MultRandPhase(input,fVariance,fBurstlen,fBurstOff,fPhase,output);
}

TDSPSignal* TDSPWienerProcess::Update(TDSPSignal* input) {
   Apply(input,fSignal);
   Emit("Update(TDSPSignal*)",fSignal);
   return fSignal;  
}

