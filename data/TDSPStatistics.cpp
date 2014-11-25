/** TDSP *******************************************************************
                          TDSPStatistics.cpp  -  description
                             -------------------
    begin                : Wed Feb 6 2002
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

#include "TDSPStatistics.h"
#include "TDSPSymbolGenerator.h"
#include "TDSPPsk.h"
#include "TDSPQam.h"
#include "TDSPPam.h"

ClassImp(TDSPStochasticProcess)
ClassImp(TDSPStochasticScalar)
ClassImp(TDSPStatistics)



TDSPStochasticProcess::TDSPStochasticProcess() { 
  fMode = kNoChange; 
  fSigmas         = NULL;
  fMeans          = NULL;

};    

TDSPStochasticProcess::~TDSPStochasticProcess(){
  if (fMeans)  delete [] fMeans;
  if (fSigmas) delete [] fSigmas;
}



TDSPStochasticScalar::TDSPStochasticScalar() { 
  fValue      = 0;
  fFifoFilter = NULL;
    
  fSigmas         = new TComplex[1];  // at least 
  fSigmas->fIm    = fSigmas->fRe  = 1/TMath::Sqrt(2.0);

  fMeans          = new TComplex[1];  // at least 
  fMeans->fRe     = fMeans->fIm   = 0;

};    

TDSPStochasticScalar::~TDSPStochasticScalar(){
  SetMode(kNoChange); // Properly cleanups
}

void TDSPStochasticScalar::SetMode(eStochasticType mode) {
  
  if (mode!=fMode) {
    // Do some special cleanings
    //
    switch(fMode) {
    default:
      break;
      
    }

    // Do some special initialisations
    //
    switch(mode) {
    default:
      break;
    }
  }

  TDSPStochasticProcess::SetMode(mode);
}


Double_t TDSPStochasticProcess::SER(Double_t snr,
				    TDSPSymbolGenerator *g) {

  UInt_t    num = g->GetNumSymbols();
  Double_t fnum = (Double_t)num;

  ////////////////////////////////////////////////////
  //
  // SER depends on the stochastic model
  //
  switch(fMode) {
    
  case kNoChange:
    return g->SER(snr);
    break;

  default:

    if (g->InheritsFrom(TDSPPsk::Class())) {
      
      //////////////////////////////////////////////////////
      //
      // n-Psk-Modulations
      //
      switch(fMode) {
      
      case kRayleigh:       // UnCorrelated Rayleigh
      case kFilterRayleigh: // Correlated Rayleigh

	////////////////////////////////////////////////////
	//
	// Rayleigh Process 
	//
	// -->    s' = a * s + n
	// "a" is rayleigh distributed p(a)=1/s^2*exp(-a*a/(2*s^2))
	// the mean snr is
	// 2*s^2*snr where 2*s^2=E[a^2]
	  
	switch(num) {
	case 2: // BPSK
	  if (fSigmas->fRe!=fSigmas->fIm) {
	    Error("SER","fSigmas[0].fRe <> fSigmas[0].fIm - this case not implemented!!");
	    return 0;
	  }
	  return 0.5*(1-TMath::Sqrt(snr/(1+snr)));
	  break;

	case 4: // QPSK 
	  if (fSigmas->fRe!=fSigmas->fIm) {
	    Error("SER","fSigmas[0].fRe <> fSigmas[0].fIm - this case not implemented!!");
	    return 0;
	  }
	  return 1-TMath::Sqrt(snr/(2+snr));
	  break;
	  
	default:
	  Error("SER","%d-Psk SER-Model not implemented!",num);
	  break;
	  
	}
	break;

     
      default:
	Error("SER","Distribution (%d) not yet implemented",fMode);
	return 0;
	break;
      }
      
      
    }

    if (g->InheritsFrom(TDSPQam::Class())) {
      
      //////////////////////////////////////////////////////
      //
      // Qam-n-Modulations
      //
      switch(fMode) {
	
      case kRayleigh:       // UnCorrelated Rayleigh
      case kFilterRayleigh: // Correlated Rayleigh
	
	////////////////////////////////////////////////////
	//
	// Rayleigh Process 
	//
	// -->    s' = a * s + n
	// "a" is rayleigh distributed p(a)=1/s^2*exp(-a*a/(2*s^2))
	// the mean snr is
	// 2*s^2*snr where 2*s^2=E[a^2]
	
	if (fSigmas->fRe!=fSigmas->fIm) {
	  Error("SER","fSigmas[0].fRe <> fSigmas[0].fIm - this case not implemented!!");
	  return 0;
	}
	
	return (1.0-1.0/TMath::Sqrt(fnum))*(1-TMath::Sqrt(3*snr/(2.0*(fnum-1.0)+3.0*snr)));
	
	break;

     
      default:
	Error("SER","Distribution (%d) not yet implemented",fMode);
	return 0;
	break;
      }
    }
      
    if (g->InheritsFrom(TDSPPam::Class())) {
      
      //////////////////////////////////////////////////////
      //
      // Pam-n-Modulations
      //
      switch(fMode) {
	
      case kRayleigh:       // UnCorrelated Rayleigh
      case kFilterRayleigh: // Correlated Rayleigh
	
	////////////////////////////////////////////////////
	//
	// Rayleigh Process 
	//
	// -->    s' = a * s + n
	// "a" is rayleigh distributed p(a)=1/s^2*exp(-a*a/(2*s^2))
	// the mean snr is
	// 2*s^2*snr where 2*s^2=E[a^2]
	
	if (fSigmas->fRe!=fSigmas->fIm) {
	  Error("SER","fSigmas[0].fRe <> fSigmas[0].fIm - this case not implemented!!");
	  return 0;
	}
	
	return (fnum-1.0)/fnum*(1-TMath::Sqrt(6.0*snr/(2.0*(fnum*fnum-1.0)+6.0*snr)));
	
	break;

      
      default:
	Error("SER","Distribution (%d) not yet implemented",fMode);
	return 0;
	break;
      }
      
      
    }
    break;
  }
  
  Error("SER","Unknown Modulation Type");
  return 0;
}

void TDSPStochasticScalar::Reset() {

  switch(fMode) {
  case kFilterGauss:
  case kFilterRayleigh:
    if (fFifoFilter) fFifoFilter->Reset();
    break;
  default:
    break;
  }
  
  ResetRating();

}

UInt_t TDSPStatistics::EstimateNumSymbols(Double_t SER,                // the SER
					  Double_t dError,             // the Error
					  UInt_t Default,              // the Default Value
					  eEstimateNumSymbols method,  // the Method
					  UInt_t Min                   // Minimum Number
					  ) {
  
  switch(method) {
    
  case kEstimateSERlog://default
    if (SER) Default = (UInt_t)(pow(TMath::Log10(TMath::E())/(dError*TMath::Log10(SER)),2));
    break;
    
  case kEstimatedSERlog:
    if (SER) Default = (UInt_t)(TMath::Log10(TMath::E())/(SER*dError*dError));
    break;
    
  case kEstimateSER: 
    if (SER) Default = (UInt_t)(1/(SER*dError*dError));
    break;
  }

  return (Default<Min) ? Min : Default;

}
// Estimates the needed Number of Symbols
//
// method = ""        is default
//          "SER"     dError=dSER/SER=const. 
//          "SERLOG"  dError=dLogSER=const. 
//          "LOG"     dError=dLogSER/dLogSER=const.(default)
//

UInt_t   TDSPStatistics::EstimateNumSymbols(Double_t SER,    // the SER
					    Double_t dError, // the Error
					    UInt_t Default,  // the Default Value (for SER=0)
					    char *method,    // the Method
					    UInt_t Min       // Minimum Number
					    ) {
  TString opt = method;
  opt.ToLower();
  Ssiz_t pos;

  if ((pos = opt.Index("serlog"))!= kNPOS) {   
    return EstimateNumSymbols(SER,dError,Default,kEstimatedSERlog,Min);
  }
  if ((pos = opt.Index("ser"))!= kNPOS) {    
    return EstimateNumSymbols(SER,dError,Default,kEstimateSER,Min);
  }
  if ((pos = opt.Index("log"))!= kNPOS) {    
    return EstimateNumSymbols(SER,dError,Default,kEstimateSERlog,Min);
  }

  Error("EstimateNumSymbols","Unknown method (=\"%s\") for estimation of the number of symbols!");

  return 0;

}

