/** TDSP *******************************************************************
                          TDSPSymbolGenerator.cpp  -  description
                             -------------------
    begin                : Tue Nov 20 2001
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

#include <TROOT.h>
#include <TDSPMath.h>
#include "TDSPSymbolGenerator.h"
#include "TDSPStatistics.h"

ClassImpQ(TDSPSymbolGenerator)

// Dump Options
//
#define DUMP_NUMSYMBOLS    0x1
#define DUMP_DEFAULT       DUMP_NUMSYMBOLS
#define DUMP_DEFAULT_BATCH 0x0

#define CORRECTION_INTERVAL_NOVALUE 99999999999

#define TRACK_REFERENCE_PHASE        0x1
#define IDEAL_PHASE                  0x4
#define EXPECT_REFERENCE_SIGNAL      0x2
#define DEBUG_CORRECTION             0x8
#define CPEC                         0x10
#define DEBUG_SYMBOL                 0x20
#define DECISION_SIGNAL              0x40

Double_t log10e=TMath::Log10(TMath::E());
TDSPSymbolGenerator::TDSPSymbolGenerator(char *name, 
					 Int_t num,
					 UInt_t opt) : TDSPOutput1(name) {

  // Default Options

  fOptions = opt;
  
  // Add my Slots
  
  Slot_Generate        = AddSlot("Generate()");
  Slot_ReferenceUpdate = AddSlot("ReferenceUpdate(TDSPSignal*)");


  _recalc_distances=kTRUE;
  
  fProbs = NULL;
  fDetectSigmas = NULL;
  fDetectProbs = NULL;
  fISymbols = NULL;
  fSymbols = NULL;
  fYields = NULL;

  fAnalyseGrid=0;
  
  fAnalysed=kTRUE;

  fInEfficiency = 0;
  fdInEfficiency = 0;

  if (gROOT->IsBatch()) 
    fDumpOptions = DUMP_DEFAULT_BATCH;
  else
    fDumpOptions = DUMP_DEFAULT;
      
  // Global Correction (multiplicativ)

  fGlobalCorrection=1.0;

  // Tracking of corrections (disabled by default !)

  fCorrection=1.0;
  fCorrectionTrackingInterval=CORRECTION_INTERVAL_NOVALUE;
  fCorrectionTrackingIndex=CORRECTION_INTERVAL_NOVALUE;
  fReferenceSignal=NULL;
      
  fSignal->SetTitle("the generated signal");
  fDecisionSignal=NULL;
}

TDSPSymbolGenerator::~TDSPSymbolGenerator(){
  if (fProbs)             delete [] fProbs;
  if (fDetectSigmas)      delete [] fDetectSigmas;
  if (fDetectProbs)       delete [] fDetectProbs;
  if (fISymbols)          delete [] fISymbols;
  if (fSymbols)           delete [] fSymbols;
  
  if (fYields)            delete [] fYields;
  if (fDecisionSignal)    delete fDecisionSignal;

}

TDSPSignal* TDSPSymbolGenerator::GetDecisionSignal() {
  if (!fDecisionSignal)
    Error("GetDecisionSignal","Not used !");
  return fDecisionSignal;
}


void TDSPSymbolGenerator::BookSymbols(Int_t num) {
  
  fNumSymbols=num;

  // Reserve Probs for Generation
  //
  if (!(fProbs = new Double_t[fNumSymbols])) {
    Error("Booksymbols","Can not alloc fProbs[0..%d]",fNumSymbols-1);
    return;
  }
  
  // Reserve Probs for Detection
  //
  if (!(fDetectProbs  = new Double_t[fNumSymbols])) {
    Error("Booksymbols","Can not alloc fDetectProbs[0..%d]",fNumSymbols-1);
    return;
  }

  // Reserve the Sigmas for Detection
  //
  if (!(fDetectSigmas = new Double_t[fNumSymbols])) {
    Error("Booksymbols","Can not alloc fDetectSigmas[0..%d]",fNumSymbols-1);
    return;
  }

  // The Symbols itself
  //
  if (!(fSymbols = new TComplex[fNumSymbols])) {
    Error("Booksymbols","Can not alloc fSymbols[0..%d]",fNumSymbols-1);
    return;
  }

  // Place for the yields
  //
  if (!(fYields  = new Int_t[fNumSymbols*(fNumSymbols+1)])) {
    Error("Booksymbols","Can not alloc fYields[0..%d]",fNumSymbols*(fNumSymbols+1)-1);
    return;
  }
  
}


void TDSPSymbolGenerator::NormalizeProbs() {

  
  Double_t sum=0;
  
  // Calculate the sum
  //
  
  for(Int_t i=0;i<fNumSymbols;++i) {
    sum+=fProbs[i];
  }
  
  if (!sum) {
    
    Error("NormalizeProbs","Can not normalize the probs because sum of probs is zero !!");

  } else {
    
    // Normalize by the sum
    //
    
    for(Int_t i=0;i<fNumSymbols;++i) {
      fProbs[i] /= sum;
    }
  }
 
}


void TDSPSymbolGenerator::SetOption(Option_t *option) {

  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  if ((pos = opt.Index("dump1"))!= kNPOS) {
    opt.Remove(pos,5);
    fDumpOptions|=DUMP_NUMSYMBOLS;
  }

  if ((pos = opt.Index("dump0"))!= kNPOS) {
    opt.Remove(pos,5);
    fDumpOptions&=(!DUMP_NUMSYMBOLS);
  } 
  
  if ((pos = opt.Index("noreferencephase"))!=kNPOS) {
    opt.Remove(pos,16);
    fOptions&=(!TRACK_REFERENCE_PHASE);
  }

  if ((pos = opt.Index("noidealphase"))!=kNPOS) {
    opt.Remove(pos,12);
    fOptions&=(!IDEAL_PHASE);
  }

  if ((pos = opt.Index("idealphase"))!=kNPOS) {
    opt.Remove(pos,10);
    fOptions|=IDEAL_PHASE;
  }
  if ((pos = opt.Index("cpec"))!=kNPOS) {
    opt.Remove(pos,4);
    fOptions|=CPEC;
  }
  if ((pos = opt.Index("debugcorrection"))!=kNPOS) {
    opt.Remove(pos,15);
    fOptions|=DEBUG_CORRECTION;
  }
  if ((pos = opt.Index("debugsymbol"))!=kNPOS) {
    opt.Remove(pos,11);
    fOptions|=DEBUG_SYMBOL;
  }
  if ((pos = opt.Index("decisionsignal"))!=kNPOS) {
    opt.Remove(pos,14);
    fOptions|=DECISION_SIGNAL;
  }
  if (opt.Length())
    Error("SetOption","Unknown Option(s) %s !!",opt.Data());

}

void TDSPSymbolGenerator::DumpStep() {
  if (fDumpOptions&DUMP_NUMSYMBOLS) {
    cout << "processing : " << fNumAnalysed << "(" << fAnalyseGrid << ") symbols = ";
    cout << Double_t(fNumAnalysed)/fAnalyseGrid*100.0 << "%          log10(SER)=";
    if (fNumAnalysed) {
      Double_t feps = 0;
      if ((feps=Double_t(fMisIdentified)/Double_t(fNumAnalysed)))
	cout << TMath::Log10(feps) << " +- " <<  log10e/TMath::Sqrt(Double_t(fMisIdentified));   
      else cout << "(none)";
    } 
    cout << "            \r" << flush;
    
  }
}

// Generate 
//
void TDSPSymbolGenerator::Generate() {
  
  TDSPSignal *signal = GetSignal();

  TComplex   *sigvec = signal->GetVec();
  Int_t       num    = signal->GetLen();
  
  GenerateSymbols(num,sigvec);
  fNumAnalysed=0;
  DumpStep();
  Emit(Signal_NewData,signal);
  
  Int_t origSignalLength = GetSignalLength();
  
  while(!fAnalysed) {
    // Adjust the SignalLength
    //
    Int_t rest = fAnalyseGrid-fNumAnalysed;
    if (rest<origSignalLength) ReduceSignalLength(rest); 
    
    // Emit
    //
    DumpStep();
    Emit(Signal_NewData,signal);

  }  
  // Restore original SignalLength
  
  SetSignalLength(origSignalLength);

}


// Detect Symbols
//

TDSPSignal* TDSPSymbolGenerator::Update(TDSPSignal *input) {


  TComplex correctedsymbol, decision;

  // Check the Sampling Rates of input and output
  //
  if (input->GetSamplingRate() != GetSignal()->GetSamplingRate()) {
    Error("Update","Sampling rates of transmitted (%f) and received (%f) signals differ !",
	  GetSignal()->GetSamplingRate(),input->GetSamplingRate());
    return NULL;
  }

  // the  received Signal
  Int_t     ilen = input->GetLen();
  TComplex *ivec = input->GetVec();

  // the transmitted signal
  Int_t     olen = fSignal->GetLen();
  TComplex *ovec = fSignal->GetVec();
  
  // the reference signal
  TComplex *rvec = NULL;

  // the decision based signal
  TComplex *dvec = NULL;
  if (fOptions&DECISION_SIGNAL) {
    if (!fDecisionSignal) fDecisionSignal = new TDSPSignal();
    if (!fDecisionSignal)
      Error("Update","Failed to alloc memory for the decision signal - disabled!");
    else {
      fDecisionSignal->SetLen(ilen);
      dvec = fDecisionSignal->GetVec();
    }
  }

  // check the reference signal
  if (fOptions&EXPECT_REFERENCE_SIGNAL) {
    if (!fReferenceSignal) {
      Error("Update","No reference signal given !");
      return NULL;
    } 
    if (fReferenceSignal->GetLen()!=ilen) {
      Error("Update","Length of the measured signal(%d) and the reference signal(%d) differ!",ilen,
	    fReferenceSignal->GetLen());
      return NULL;
    }
    if (!(rvec=fReferenceSignal->GetVec())) {
      Error("Update","Vector of the reference signal seems to be not alloced (is NULL)");
      return NULL;
    }
    

  } else {

    if (fOptions&TRACK_REFERENCE_PHASE) {
      Error("Update","Misconfiguration - can not track the reference phase without using the reference signal!");
      return NULL;
    }

  }

  // Check that the correction tracking intervall was setup correctly

  if (fOptions&( TRACK_REFERENCE_PHASE | IDEAL_PHASE | CPEC )) {
    if (fCorrectionTrackingInterval==CORRECTION_INTERVAL_NOVALUE) {
      Error("Update","No value for \"Correction Tracking Interval\" specified. Disabling Correction!");
      return NULL;
    } else {
      if (fOptions&DEBUG_CORRECTION)
	cout << "debug:correction: tracking-intervall=" << fCorrectionTrackingInterval << endl;
      if (fCorrectionTrackingIndex==CORRECTION_INTERVAL_NOVALUE) {
	fCorrectionTrackingIndex=fCorrectionTrackingInterval;
	Info("Update","Resetting Correction Tracking Index to %d",fCorrectionTrackingIndex);
      }
    }
  }

  // Burst length should be agree
  //
  if (ilen!=olen) {
    Error("Update","Length of transmitted (%d) and received (%d) signals differ !",
	  olen, ilen);
    return NULL;
  }

  if (olen!=fNumISymbols) {
    Error("Update","Length of the transmit signal (%d) has been changed (old=%d) !! Aborting ...",
	  olen, fNumISymbols);
    return NULL;
  }
 
  for(register Int_t i=0;i<fNumISymbols;++i) {

    // Reset Counter etc. if they are already Analysed
    //
    if (fAnalysed) Analyse_Reset();  

    // Global correction of the actual symbol
    //
    correctedsymbol = ivec[i]*fGlobalCorrection;
    
    // Do we use that symbol as a tracking symbol/pilot symbol ?

    if (++fCorrectionTrackingIndex>=fCorrectionTrackingInterval) {
      fCorrectionTrackingIndex=0;
      if (fOptions&TRACK_REFERENCE_PHASE) {
	// tracking the phase
	if (Bool_t(rvec[i])&&Bool_t(ovec[i])) { 
	  fCorrection =ovec[i]/rvec[i];
	  fCorrection/=Abs(fCorrection);
	  if (fOptions&DEBUG_CORRECTION)
	    cout << "debug:correction: phases(sent,reference,received,correction)=" << 
	      ovec[i].Phase() << ", " << 
	      rvec[i].Phase() << ", " << 
	      ivec[i].Phase() << ", " << 
	      fCorrection.Phase() << endl;
	} else {
	  if (!rvec[i]) Info("Update","Can't track the ref. phase at i=%d because ref. signal is zero - using the old phase",i);
	  if (!ovec[i]) Info("Update","Can't track the ref. phase at i=%d because sent signal is zero - using the old phase",i);
	}
      }

      // Ideal Phase Correction
       
      if (fOptions&IDEAL_PHASE) {
	if (Bool_t(correctedsymbol)&&Bool_t(ovec[i])) { 
	  fCorrection =ovec[i]/correctedsymbol;
	  fCorrection/=Abs(fCorrection);
	  if (fOptions&DEBUG_CORRECTION)
	    cout << "debug:correction: phases(sent,received,correction)=" << 
	      ovec[i].Phase()         << ", " << 
	      correctedsymbol.Phase() << ", " << 
	      fCorrection.Phase() << endl;
	} else {
	  if (!Bool_t(correctedsymbol)) 
	    Info("Update","Can't set ideal phase at i=%d because measured signal is zero - using the old phase",i);
	  if (!Bool_t(ovec[i])) 
	    Info("Update","Can't set ideal phase at i=%d because sent signal is zero - using the old phase",i);
	}
      }

      // Common Phase Error Correction
      //
      if (fOptions&CPEC) {
	
	// Calculate the averaged phase over the next symbols
	//
	Int_t        N=fCorrectionTrackingInterval;
	if (i+N>ilen) {
	  Error("Update","input signal len (%d) does not match CPEC correction interval(%d) at i=%d - cutting",ilen,i,N);
	  N=ilen-i;
	}
	Int_t        n=0;
	Double_t phase=0;
	TComplex   tmp;
	for(register Int_t f=0;f<N;++f) {
	  tmp = ivec[i+f]*fGlobalCorrection;
	  if (Bool_t(tmp)) { 
	    phase += tmp.Phase();
	    ++n;
	  } else {
	    Info("Update","Can't use phase at i=%d for CPEC because measured signal is zero",i);
	  }
	}
	phase/=n;
	fCorrection=TComplex(TMath::Cos(-phase),TMath::Sin(-phase));
	if (fOptions&DEBUG_CORRECTION)
	  cout << "debug:correction CPEC_phase=" << 
	    fCorrection.Phase() << endl;
	
      }

    }

    // Correct the symbol
    //
    correctedsymbol *= fCorrection;

    // Save that symbol into fDecisionSignal
    
    if (dvec) dvec[i] = correctedsymbol;

    // make hard decision

    decision         = DetectSymbol(correctedsymbol);

    // Try to identify the Symbol
    //

    Analyse_Symbol(// the sended data
		   fISymbols[i],          // the original signal indices vector
		   ovec+i,                // the original signal vector
		   
		   // the received data
		   decision,              // the detected signal indices vector
		   &correctedsymbol       // the received signal vector
		   );
    

    TComplex cc=ovec[i]/correctedsymbol;
   
    if (fOptions&(DEBUG_SYMBOL | DEBUG_CORRECTION)) {

      // Symbol-wise debugging
      
      if (fOptions&DEBUG_SYMBOL) {
	cout << 
	  "debug:symbol: (sent,rec,corr,dec)=" <<
	  ovec[i] << ", " << ivec[i]  << ", " << correctedsymbol << ", " << fSymbols[decision] << endl;
	cout << 
	  "debug:symbol: (phase-diff)       =";
	if (Bool_t(ovec[i])) cout << Phase(correctedsymbol/ovec[i]); else cout <<  "(none)";
	cout << endl;
	cout << 
	  "debug:symbol: (sent-corr)^2      =" << ::Abs2(ovec[i] - correctedsymbol) << endl;
	cout << 
	  "debug:symbol: (ave.noise+int+po.)= " << GetIntNoisePower() << endl;
      }

      // General Debugging 

      cout << "debug: (enter=continue debug, c=continue normal)" << flush;
      Char_t c=getchar();
      switch(c) {
      case 'c' :
      case 'C' :
	fOptions &= !(DEBUG_SYMBOL | DEBUG_CORRECTION);
	break;
      }
    }
    
    // Analyse and determine parameters
    //
    if (fNumAnalysed==fAnalyseGrid) {
      // Analyse a partition of the received symbols
      //
      Analyse_Finish();
    }
    

  }

  // Reset the reference signal

  fReferenceSignal=NULL;

  // at the moment no return value

  return NULL;

}



void TDSPSymbolGenerator::Analyse_Reset() {
  // Reset the yiels
  Int_t    num2 = fNumSymbols*fNumSymbols;
  for(register Int_t i=0;i<num2;i++) fYields[i] = 0;
  fMisIdentified                 = 0;
  fNoisePower                    = 0;
  fSignalPower                   = 0;  
  fReceivedPower                 = 0;  
  fAttentuation                  = 0;
  fNumAnalysed                   = 0;
  fAnalysed                      = kFALSE;
  fCorrectionTrackingIndex       = fCorrectionTrackingInterval;
  fCorrection                    = 1;
}

void TDSPSymbolGenerator::Analyse_Symbol(Int_t     sent_i,
					TComplex  *sent_c,
					Int_t     rec_i,
					TComplex  *rec_c) {
  
  TComplex tmpc;
  
  // Count the Yields 
  //
  fYields[sent_i*fNumSymbols+rec_i+1]++;
  
  // Count the misidentifications
  // (=trace of yields)
  //
  if (sent_i != rec_i) ++fMisIdentified;
  
  // Count the noise+interference power
  //
  tmpc         = *rec_c-*sent_c;
  fNoisePower += tmpc.fRe*tmpc.fRe + tmpc.fIm*tmpc.fIm;
  
  // Count the transmit signal power
  //
  fSignalPower += sent_c->fRe*sent_c->fRe + sent_c->fIm*sent_c->fIm;

  // Count the received signal power
  //
  fReceivedPower += rec_c->fRe*rec_c->fRe + rec_c->fIm*rec_c->fIm;

  // Count the attentuation
  //
  fAttentuation +=  *sent_c % *rec_c;

  fNumAnalysed++;

 
}


void TDSPSymbolGenerator::Analyse_Finish() {
  
  
  // Calculate the inefficieny
  //
  fInEfficiency  = fMisIdentified;
  fInEfficiency /= (Double_t)fNumAnalysed;


  if (fInEfficiency) {
    fLogSER        = TMath::Log10(fInEfficiency);
    fdInEfficiency = TMath::Sqrt(Double_t(fMisIdentified))/fNumAnalysed;  
    fdLogSER       = log10e/TMath::Sqrt(Double_t(fMisIdentified));
  } else {
    fLogSER        = 0;
    fdInEfficiency = 0;
    fdLogSER       = 0;
  }
 
  fAnalysed = kTRUE;
}

void TDSPSymbolGenerator::SetAnalyseGrid(UInt_t a) { 
  fAnalyseGrid = a;
}

void TDSPSymbolGenerator::ReduceSignalLength(Int_t num) {
  if (num>fNumISymbolsAlloc) {
    Error("ReduceSignalLength","%d is greater than alloced length %d", num, fNumISymbolsAlloc);
  } else {
    fNumISymbols = num;
    GetSignal()->SetLen(num);
  }
}
 
void TDSPSymbolGenerator::SetSignalLength(Int_t num) { 
  // This should be optimized later !!
  //
  if (fISymbols)       delete [] fISymbols;
  fNumISymbols      = num;
  fNumISymbolsAlloc = num;
  fISymbols         = new Int_t[num];

  GetSignal()->SetMinLen(num);
  GetSignal()->SetLen(num);
  
  if (!fAnalyseGrid) SetAnalyseGrid(num);
 
}

Double_t TDSPSymbolGenerator::SER(Double_t SNR) { 
  Error("SER","can not evaluated SER, because its a base class!");
  return 1; 
}

UInt_t   TDSPSymbolGenerator::EstimateNumSymbols(Double_t dError,           // the Error
						 UInt_t   d,                // the Default Value
						 eEstimateNumSymbols method,// the Method
						 UInt_t   Min               // Minimal Number
						 ) {
  return TDSPStatistics::EstimateNumSymbols(fInEfficiency,dError,d,method,Min);

}
UInt_t   TDSPSymbolGenerator::EstimateNumSymbols(Double_t dError,// the Error
						 UInt_t   d,     // the Default Value
						 char *method,   // the Method
						 UInt_t   Min    // Minimal Number
						 ) {
  return TDSPStatistics::EstimateNumSymbols(fInEfficiency,dError,d,method,Min);

}



void TDSPSymbolGenerator::_calc_distances() {
  
  Double_t dist,mean=0,min = 99999999;
  Double_t qq=0,qmean=0;
  Double_t var=0,qvar=0,diff2;
  


  for(register Int_t i=0;i<fNumSymbols;++i) {
    TComplex sym = fSymbols[i];
    for(register Int_t j=0;j<i;++j) {
      dist = Dist(sym,fSymbols[j]);
      if (dist<min) min=dist; // The Minimum
      mean+=dist;             // The Mean
      qmean+=dist*TDSPMath::Q(dist);
      qq+=TDSPMath::Q(dist);
    }
  }

  // The Means

  fMinDistance  = min;
  fMeanDistance = ( mean/=0.5*fNumSymbols*(fNumSymbols-1) );
  fQWeightedDistance = ( qmean/=qq );

  
  { for(register Int_t i=0;i<fNumSymbols;++i) {
    TComplex sym = fSymbols[i];
    for(register Int_t j=0;j<i;++j) {
      dist  = Dist(sym,fSymbols[j]);
      diff2 = (dist-mean);
      diff2*=diff2;
      var  += diff2;    
      qvar += diff2*TDSPMath::Q(dist);
    }
  } }

  // The Variances

  fVarianceDistance  = ( var/=(fNumSymbols*(fNumSymbols/2-1)-1) );
  fQWeightedVarianceDistance = ( qvar/=qq );
  

  _recalc_distances = kFALSE;

}

void TDSPSymbolGenerator::_check_distances() {
  if (_recalc_distances) _calc_distances();
}


Double_t TDSPSymbolGenerator::GetDistance(Option_t *option) {


  _check_distances();

  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  if ((pos = opt.Index("min"))!= kNPOS) 
    return fMinDistance;

  if ((pos = opt.Index("mean"))!= kNPOS) 
    return fMeanDistance;

  if ((pos = opt.Index("var"))!= kNPOS) 
    return fVarianceDistance;
  
  if ((pos = opt.Index("qmean"))!= kNPOS) 
    return fQWeightedDistance;

  if ((pos = opt.Index("qvar"))!= kNPOS) 
    return fQWeightedVarianceDistance;
  
  Error("GetDistance","Unknown Distance (\"%s\") ... ", opt.Data());

  return 0;

}

#include <TDSPPsk.h>
#include <TDSPQam.h>
#include <TDSPFastBpsk.h>
#include <TDSPFastQpsk.h>

TDSPSymbolGenerator* Modulator(Option_t *option, Int_t param) {

  
  TDSPSymbolGenerator *modu = NULL;

  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  // Fast BPSK-implementation
  //
  if ((pos = opt.Index("bpsk"))!= kNPOS) {
    opt.Remove(pos,4);
    modu = new TDSPFastBpsk();
    goto setoption;
  } 

  // Fast QPSK-implementation
  //
  if ((pos = opt.Index("qpsk"))!= kNPOS) {
    opt.Remove(pos,4);
    modu = new TDSPFastQpsk();
    goto setoption;
  } 
  // General PSK-implementation
  //
  if ((pos = opt.Index("psk"))!= kNPOS) {
    opt.Remove(pos,3);
    modu = new TDSPPsk(NULL,param);
    goto setoption;
  }

  // General qam-implementation
  //
  if ((pos = opt.Index("qam"))!= kNPOS) {
    opt.Remove(pos,3);
    modu = new TDSPQam(NULL,param);
    goto setoption;
  }
   
 setoption:
  
  if (modu)  
    modu->SetOption(opt.Strip().Data());
  else 
    Error("Modulator","Error in initializing modulator \"%s\"",option);
  return modu;
  
}
