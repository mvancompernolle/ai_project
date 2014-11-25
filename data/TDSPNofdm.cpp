/** TDSP *******************************************************************
                          TDSPNofdm.cpp  -  description
                             -------------------
    begin                : Fri Nov 23 2001
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


#include <TFile.h>
#include <TDSPfft.h>
#include "TDSPNofdm.h"
#include "../Matlab/TMatlab.h"

ClassImpQ(TDSPNofdm)

#define OPTION_DEFAULT           0x0
#define OPTION_SHOWPULSE         0x1
#define OPTION_ANALYSECORR       0x2
#define OPTION_ANALYSEMAPPING    0x4
#define OPTION_DEBUG_CORRELATION 0x8 
#define OPTION_DEBUG_MAPPING     0x10 
#define OPTION_WILSON_PHASE      0x12

TDSPNofdm::TDSPNofdm(char *name) : TDSPBlockMap(name) {

  // Add Delegations

  Delegation_Transmitted = AddDelegation("Transmitted(TDSPSignal*)");
  Delegation_Received    = AddDelegation("Received(TDSPSignal*)");

  Delegation_Transmit    = AddDelegation("Transmit(TDSPSignal*)");
  Delegation_Receive     = AddDelegation("Receive(TDSPSignal*)");

  
  // Give them a meaning as a slot
  //
  Connect(Delegation_Transmit,
	  ClassName(),
	  this,
	  Slot_Update);


  Connect(Delegation_Receive,
	  ClassName(),
	  this,
	  Slot_InverseUpdate);

  // Give them a meaning as a signal
  //
  Connect(Signal_Update,
	  ClassName(),
	  this,
	  Delegation_Transmitted);

  Connect(Signal_InverseUpdate,
	  ClassName(),
	  this,
	  Delegation_Received);


  
  fSignal->SetTitle("the transmit signal");
  fInverseSignal->SetTitle("the received signal");

  // Set Some Defaults
  //
  SetLength(88);
  SetTimePad(11);
  SetFreqPad(11);

  // Check for a global matlab engine
  //
  
#ifndef HAVE_MATLAB
  Error("TDSPNofdm","Implementation only avaiable with Matlab+Peters Gabor ToolBox");
#endif

  fTransmitMatrix = NULL;
  fReceiverMatrix = NULL;
  fCorrelation    = NULL;
  fMapping        = NULL;
  fMappingPart1   = NULL;

  fNoiseCorrelation    = NULL;
  fMatrix = NULL;
  fPseudoInverseMatrix = NULL;
  fChannel = NULL;
  fDual=kTRUE;

  // Default Mode
 
  fPulseName="gaussnk";
  fPulseNumParams = 0;
  fPulseParams[0]=1.;
  fOptions = OPTION_DEFAULT;
  fMode = 0; // not specified
  SetOption("tight");
  fHistogram_all=0;
  fHistogram_diag=0;
  fHistogram_nondiag=0;
  fHistogram_corr_all=0;
  fHistogram_corr_diag=0;
  fHistogram_corr_nondiag=0;
  // Flags

  _recalc_correlations = kTRUE;
  _recalc_mapping = kTRUE;

}
TDSPNofdm::~TDSPNofdm(){
  if (fMatrix)              delete fMatrix;fMatrix=NULL;
  if (fPseudoInverseMatrix) delete fPseudoInverseMatrix;fPseudoInverseMatrix=NULL;
  if (fCorrelation)         delete fCorrelation;
  if (fMapping)             delete fMapping;
}

// Appends a String of the form start+",param1,param2,...,paramn"
// (if start="" then returns "param1,param2,...,paramn"
//
TString& TDSPNofdm::_build_pulse_param_string(TString& start) {
 
  Bool_t WasEmpty = start.IsNull();
  for(Int_t i=0;i<fPulseNumParams;++i) {
    if (i) start+=",";
    else
      if (!WasEmpty) start+=",";
    
    start+=fPulseParams[i]; 
  }
  return start;
}

void TDSPNofdm::CreatePulses() {
 
  // Check for a global matlab engine
  //
  
#ifdef HAVE_MATLAB

  TMatlab *m = TMatlab::GlobalEngine();
  if (fLength%fFreqPad) 
    Error("CreatePulses","!! fLength must be a multiple of fFreqPad !!");
  else {
    Info("CreatePulses","Creating function set (T=%d,F=%d,L=%d) ...",fTimePad,fFreqPad,fLength);
    m->EvalString("clear;");
    m->EvalString("T=%d;F=%d;L=%d;",fTimePad,fFreqPad,fLength);
    
    TString  p1;p1+= fLength;
    TString& pulseparamstring = _build_pulse_param_string(p1); // first parameter is always L

    Info("CreatePulses","Using pulse=\"%s\" with params=(%s)",fPulseName.Data(),pulseparamstring.Data());

    if (!m->EvalString("my_pulse=%s(%s);",fPulseName.Data(),pulseparamstring.Data())) {
      Error("CreatePulses","can not execute : my_pulse=%s(%s);",fPulseName.Data(),pulseparamstring.Data());
    }
    if (!m->Eval("Gamma=GenerateFrame(my_pulse,T,F,'power');"))
      Error("CreatePulses","Do you have Peters Gabor Toolbox ?");
    else {


      if (fTight) {

	// Generating a tight frame or ONB Basis
	
	Info("CreatePulses","Creating tight window ...");
	m->EvalString("my_pulse=TightWindow(Gamma*Gamma',transpose(my_pulse));");
	Info("CreatePulses","Creating tight function set ...");
	m->Eval("Gamma=GenerateFrame(my_pulse,T,F,'power');");
	Info("CreatePulses","Calculating tight dual set ...");
	m->EvalString("GStar=pinv(Gamma);");

      } else {
	
	if (fDual) {
	  
	  // Generating the dual basis

	  Info("CreatePulses","Calculating dual set ...");
	  m->EvalString("GStar=pinv(Gamma);");

	} else {

	  // Using the same basis at the receiver

	  Info("CreatePulses","Using pulse as dual pulse ...");
	  m->EvalString("GStar=Gamma';");
	  
	  Double_t eps=Double_t(fLength)/Double_t(fTimePad*fFreqPad);
	  if (eps<1.) {
	    // bfdm - like - perform the right normalization of the receiver
	    // pulses in GStar
	    m->EvalString("GStar=GStar*%f;",eps);
	    Info("TDSPNofdm","Manually adapting norm2(g)=epsilon=%f - check ...",eps);
	    m->Eval("trace(GStar'*GStar)/(L*L/T/F)");
	  }
	}
	
      }
      
      m->Eval(      "ReportFrame(Gamma,GStar,T,F);");
      
      if (fOptions&OPTION_SHOWPULSE) {
	Info("CreatePulses","Showing pulse number 0 from (0..%d)",fNumCarriers*fNumTime-1);
	m->Eval("figure(1);ShowAtom(Gamma(:,%d), GStar(%d,:),%d,%d,'');",
		1,1,fTimePad,fFreqPad);	
      }


      fTransmitMatrix = new TDSPMatrix();
      fReceiverMatrix = new TDSPMatrix();

      *fTransmitMatrix = *(m->Get("Gamma")); // Transmit Filter
      *fReceiverMatrix = *(m->Get("GStar")); // Receiver Filter
      
      SetOption("bfdm"); // Default mode

    }
  }

  TDSPBlockMap::Init();

#endif
}

// Free's the an NOFDM-Mode
//
void TDSPNofdm::_free_mode(Int_t mode) {

  switch(mode) {
  case 0:
    // No Mode set
    break;
  case 1:
    // Nothing to clean up because the used Matrices coming
    // from extern routines
    fMatrix         = NULL;
    fPseudoInverseMatrix  = NULL;
    break;
  case 2:
    fMatrix         = NULL;
    if (f_PseudoInverseMatrix) delete f_PseudoInverseMatrix;
    fPseudoInverseMatrix=f_PseudoInverseMatrix=NULL;
    break;
  default:
    Error("_free_mode","Unknown situation (mode=%d)",mode);
    break;
  }
}

// Setup's a new NOFDM-Mode
//
void TDSPNofdm::_new_mode(Int_t mode) {
  
  switch(mode) {
  
  case 0 :
    // no Mode to set
    break;
  case 1 :
    // Receiver based on biorthogonality
    //      - using the fTransmitMatrix as fMatrix and 
    //        fReceiverMatrix as fPseudoInverseMatrix

    if (!fTransmitMatrix) {
      Error("_new_mode","Transmit matrix not yet calculated !!");
      return;
    }
    SetMatrix(fTransmitMatrix);
    if (!fReceiverMatrix) {
      Error("_new_mode","Receiver matrix not yet calculated !!");
      return;
    }
    SetInverseMatrix(fReceiverMatrix);
    break;

  case 2 :
    // Correlating Receiver
    //      - using fTransmitMatrix as fMatrix and 
    //        (fChannel*fTransmitMatrix)' as fInverseMatrix
    //        Note : that is the same (H Gamma)' = (H Gamma)'Gamma G' = (Gamma' H Gamma)'G'
    //               because Gamma G' = Id !!

    if (!fTransmitMatrix) {
      Error("_new_mode","Transmit matrix not yet calculated !!");
      return;
    }

    SetMatrix(fTransmitMatrix);

    if (fChannel) {
      
      TDSPMatrix *f=fTransmitMatrix->LeftApply(fChannel);
      SetInverseMatrix(f_PseudoInverseMatrix=f->Adjoint());
      delete f;
    } else {
      SetInverseMatrix(f_PseudoInverseMatrix=fTransmitMatrix->Adjoint());
    }

    break;
  default:
    Error("_new_mode","Unknown situation (mode=%d)",mode);
    break;
  }
}

void TDSPNofdm::SetMode(Int_t mode) {
  _free_mode(fMode);
  _new_mode( fMode = mode);
}

void TDSPNofdm::SetOption(Option_t *option) {
  
  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;

  if ((pos = opt.Index("wilsonphase"))!=kNPOS) {
    fOptions|=OPTION_WILSON_PHASE;
    opt.Remove(pos,11);
  }

  if ((pos = opt.Index("debugcorrelation"))!=kNPOS) {
    fOptions|=OPTION_DEBUG_CORRELATION;
    opt.Remove(pos,16);
  }
  
  if ((pos = opt.Index("analysecorr"))!=kNPOS) {
    fOptions|=OPTION_ANALYSECORR;
    opt.Remove(pos,11);
  }
  if ((pos = opt.Index("analysemapping"))!=kNPOS) {
    fOptions|=OPTION_ANALYSEMAPPING;
    opt.Remove(pos,14);
  }
  if ((pos = opt.Index("bfdm"))!= kNPOS) {
    SetMode(1);
    opt.Remove(pos,4);
  }
  
  if ((pos = opt.Index("corr"))!= kNPOS) {
    SetMode(2);
    opt.Remove(pos,4);
  } 
  if ((pos = opt.Index("nondual"))!= kNPOS) {
    fDual=kFALSE;
    fTight=kFALSE;
    opt.Remove(pos,7);
  }
  if ((pos = opt.Index("dual"))!= kNPOS) {
    fDual=kTRUE;
    fTight=kFALSE;
    opt.Remove(pos,4);
  }
  if ((pos = opt.Index("nontight"))!= kNPOS) { // the same as "dual"
    fDual=kTRUE;
    fTight=kFALSE;
    opt.Remove(pos,8);
  }
  if ((pos = opt.Index("tight"))!= kNPOS) {
    fTight=kTRUE;
    fDual=kFALSE;
    opt.Remove(pos,5);
  }

  if ((pos = opt.Index("noshowpulse"))!=kNPOS) {
    fOptions&=(!OPTION_SHOWPULSE);
    opt.Remove(pos,11);
  }

  if ((pos = opt.Index("showpulse"))!=kNPOS) {
    fOptions|=OPTION_SHOWPULSE;
    opt.Remove(pos,9);
  }

  if (opt.Strip().Length()) 
    Error("SetOption","Unknown Option(s) \"%s\" !!",opt.Strip().Data());
  
}


void TDSPNofdm::SetChannel(TDSPMatrix *c) {
  fChannel = c;
  SetMode(fMode); // Set the Mode Again to update all other matrices
  _recalc_correlations = kTRUE;
  _recalc_mapping      = kTRUE;
}

void TDSPNofdm::_calc_mapping() {
  
  // Calculate the complete mapping G^* H Gamma = H_{kl,mn}

  if (!fTransmitMatrix) {
    Error("_calc_mapping","No transmitter matrix known !!");
    return;
  }

  if (!fReceiverMatrix) {
    Error("_calc_mapping","No receiver matrix known !!");
    return;
  }

  if (fChannel) {
    
    fMappingPart1 = fChannel->RightApply(fTransmitMatrix,fMappingPart1);

  } else {
    
    Info("_calc_mapping","No channel given...  Assume a identity !");
    fMappingPart1 = fTransmitMatrix->Copy(fMappingPart1);
    
  }

  fMapping      = fReceiverMatrix->RightApply(fMappingPart1, fMapping);

  _recalc_mapping = kFALSE;
}

void TDSPNofdm::_calc_correlations() {


  // Clean Up from the run before ...

  if (fCorrelation) delete fCorrelation;
  if (fNoiseCorrelation) delete fNoiseCorrelation;

  // Some Checks !!
  
  if (!fTransmitMatrix) {
    Error("_calc_correlation","No transmitter matrix known !!");
    return;
  }

  if (!fReceiverMatrix) {
    Error("_calc_correlation","No receiver matrix known !!");
    return;
  }

  
  switch(fMode) {

  case 0:
    break;

  case 1:
  case 2:

    {
      
      // Calculate the Correlation Matrix of the input and 
      // output signals for a CORRELATING RECEIVER
      //
      // A=(H Gamma)' H Gamma
      // this is in expanded form :
      //
      // A=C'QC  with  Q=G'G and C=Gamma' H Gamma  
      //
      
      //////////////////////////////////////////////////
      //
      // (1) Calculate C 
      //
      
      TDSPMatrix *C  = NULL;
      
      
      // Info("_calc_correlations","Calculating noise correlation...");
      if (fChannel) {
	
	// If the Channel H is known ...
	// C = Gamma' H Gamma  
	// 
	if (!(C =  fChannel->AdjointTransform(fTransmitMatrix))) {  // C = Gamma' H Gamma 
	  Error("_calc_correlation","Error in calculation of C=Gamma' H Gamma - aborting");
	  return;
	}
	
	
	// Calculate the Noise Covariance
	//
	// NN=mm' where : m=(HGamma)'n = Gamma'H'n
	// mm'=Gamma' H' nn' H Gamma = sigma2 * Gamma' H' H Gamma     with nn'=sigma2*I
	// 
	// for sigma2=1 it is NN=Gamma' H' H Gamma
	
	TDSPMatrix *HH    = fChannel->AdjointRightApply(fChannel);
	if (!HH) {
	  Error("_calc_correlations","Error in calculation of H'H - aborting!");
	  return;
	}
	fNoiseCorrelation = HH->AdjointTransform(fTransmitMatrix);
	delete HH;
	if (!fNoiseCorrelation) {
	  Error("_calc_correlations","Error in calculation of Gamma'(H'H)Gamma - aborting!");
	  return;
	}
      } else {
	
	Info("_calc_correlations","No channel given...  Assume a identity !");
	
	// If the Channel H is unknown ...
	// C = Gamma' Gamma  
	// (H=1)
	
	C = fTransmitMatrix->AdjointRightApply(fTransmitMatrix);
	
	// This is the same as the noise - correlation
	
	fNoiseCorrelation = C->Dup();
	
      }
      // Info("_calc_correlations","Calculating total correlation...");

      //////////////////////////////////////////////////
      //
      // (2) Calculate Q 
      //
      
      TDSPMatrix *Q  =  fReceiverMatrix->AdjointLeftApply(fReceiverMatrix);   // Q = (G')(G')'
      
      //////////////////////////////////////////////////
      //
      // (3) Calculate A
      //
      if (!(fCorrelation    =  Q->AdjointTransform(C))) { // A = C'QC
	Error("_calc_correlation","Error in calculation of A=C'QC - aborting");
	delete C;
	delete Q;
	return;
      }
      
      
      
      
      // Delete and Cleanup
      
      delete Q;
      delete C;
    }
    
    break;
    

  default:
    Error("_calc_correlations","Unknown situation ... - aborting (mode=%d)",fMode);
    return;
    
  }
  

  _recalc_correlations = kFALSE;

}

void  TDSPNofdm::CalcCorrelationFromChannel(TDSPMatrix* channel) {
  SetChannel(channel);
  _calc_correlations();
  if (fOptions&OPTION_ANALYSECORR) {
    Int_t dim        = fCorrelation->GetRows();
    Double_t diag    = fCorrelation->Trace();
    Double_t all     = fCorrelation->Sum();
    Double_t nondiag = all-diag;
    if (!fHistogram_all)     fHistogram_corr_all     = new TH1F("Histogram_corr_all","all",
								100,0,3.5);
    if (!fHistogram_diag)    fHistogram_corr_diag    = new TH1F("Histogram_corr_diag","mean diagonal",
								100,0,2.5);
    if (!fHistogram_nondiag) fHistogram_corr_nondiag = new TH1F("Histogram_corr_nondiag","mean nondiagonal",
								100,-1.1,2);
    fHistogram_corr_all->Fill(        all/dim);
    fHistogram_corr_diag->Fill(      diag/dim);
    fHistogram_corr_nondiag->Fill(nondiag/dim);
  }
  Correlation(fCorrelation); // Emit this correlation
  
}

void  TDSPNofdm::CalcMappingFromChannel(TDSPMatrix* channel) {
  SetChannel(channel);
  _calc_mapping();
  if (fOptions&OPTION_ANALYSEMAPPING) {
    Int_t dim        = fMapping->GetRows();
    Double_t diag    = fMapping->Trace2();
    Double_t all     = fMapping->Sum2();
    Double_t nondiag = all-diag;
    if (!fHistogram_all)     fHistogram_all     = new TH1F("Histogram_all","all",
							   100,0,3.5);
    if (!fHistogram_diag)    fHistogram_diag    = new TH1F("Histogram_diag","mean diagonal",
							   100,0,2.5);
    if (!fHistogram_nondiag) fHistogram_nondiag = new TH1F("Histogram_nondiag","mean nondiagonal",
							   100,-0.1,1.2);
    fHistogram_all->Fill(        all/dim);
    fHistogram_diag->Fill(      diag/dim);
    fHistogram_nondiag->Fill(nondiag/dim);
  }

  Mapping(fMapping); // Emit this full mapping
  MappingPart1(fMappingPart1); // Emit partial mapping 1
  
}

void  TDSPNofdm::SetTransmitMatrix(TDSPMatrix *h) {
  if (fTransmitMatrix) delete fTransmitMatrix;
  fTransmitMatrix=h;
}
void  TDSPNofdm::SetReceiverMatrix(TDSPMatrix *h) {
  if (fReceiverMatrix) delete fReceiverMatrix;
  fReceiverMatrix=h;
}

void  TDSPNofdm::SetPulseParam(Int_t num, Double_t v) { 
  if (fPulseNumParams<num+1) fPulseNumParams=num+1;
  fPulseParams[num]=v;
};

Double_t TDSPNofdm::GetPulseParam(Int_t num) { 
  if (num>fPulseNumParams) {
    Error("GetPulseParam","Pulse parameter no.%d not yet set!", num);
    return 0;
  }
  return fPulseParams[num];
};

#include "tleaf_helpers.h"

void TDSPNofdm::LoadPulses(char *fname, char *transmitname, char *receivername) {
  Info("LoadPulses","Loading from \"%s\"",fname);
  TFile f(fname);
  SetTransmitMatrix((TDSPMatrix*)f.Get(transmitname));
  SetReceiverMatrix((TDSPMatrix*)f.Get(receivername));
  f.Close();
  fPulseFile=fname;
}


void TDSPNofdm::SavePulses(char *fname, char *transmitname, char *receivername) {

  
  Info("SavePulses","Saving to \"%s\"",fname);
  TFile f(fname,"RECREATE");
  
  Info("SavePulses","Saving transmit matrix under the key \"%s\".",transmitname);
  GetTransmitMatrix()->Write(transmitname);
  Info("SavePulses","Saving receiver matrix under the key \"%s\".",receivername);
  GetReceiverMatrix()->Write(receivername);

  TDirectory *settings = f.mkdir("Settings","Settings for the TDSPNofdm object");
  settings->cd();
  _writeleafI("TimePad","discrete time pad (sometimes referred as \"a\" or \"T\")",      fTimePad);
  _writeleafI("FreqPad","discrete frequency pad (sometimes referred as \"b\" or \"F\")", fFreqPad);
  _writeleafI("Length" ,"length of the pulses (sometimes referred as \"L\")",            fLength); 
  _writeleafI("Tight"  ,"tight",                                                         fTight);
  _writeleafI("Dual"   ,"Dual",                                                          fDual);
  _writeleafC("PulseName","The name of the pulse used",                                  (char*)fPulseName.Data());
  f.Close();
  fPulseFile=fname;
}



TDSPVector* TDSPNofdm::GetTransmitPulse(TDSPVector *p) {
  p = fTransmitMatrix->GetColumn(0,p);
  return p;
}

TDSPVector* TDSPNofdm::GetReceiverPulse(TDSPVector *p) {
  p = fReceiverMatrix->GetRow(0,p);
  p->AdjointMe();
  return p;
}

void        TDSPNofdm::Draw() {
  TCanvas *ca = new TCanvas("NOFDM","Nofdm Pulses", 800,480);
  ca->Divide(2,1);
  TDSPVector *gamma = GetTransmitPulse();
  TDSPVector *fgamma=fft(gamma);
  TDSPVector    *g = GetReceiverPulse();
  TDSPVector   *fg = fft(gamma);

  gamma->FFTShiftMe();
  fgamma->FFTShiftMe();

  g->FFTShiftMe();
  fg->FFTShiftMe();

  ca->cd(1);
  g->Draw("real ac");
  gamma->Draw("real a");

  ca->cd(2);
  fg->Draw("real ac");
  fgamma->Draw("real a");
  gPad->Update();

  delete g;
  delete gamma;
  delete fg;
  delete fgamma;
}

void        TDSPNofdm::Print() {


  TDSPVector *gamma = GetTransmitPulse();
  TDSPVector *fgamma=fft(gamma);
 
  TDSPVector    *g = GetReceiverPulse();
  TDSPVector   *fg = fft(gamma);

  gamma->FFTShiftMe();
  fgamma->FFTShiftMe();
  
  g->FFTShiftMe();
  fg->FFTShiftMe();

  Double_t sgamma2t=gamma->CMoment2();
  Double_t sgamma2f=fgamma->CMoment2();
  
  Double_t sg2t=g->CMoment2();
  Double_t sg2f=fg->CMoment2();
  

  TDSPBlockMap::Print();
  cout << "Pulse file      : " << fPulseFile << endl;
  cout << "Transmit pulse gamma ------" << endl;
  cout << "|gamma|^2       : " <<  gamma->Norm2()  << endl;
  cout << "sigma^2_t       : " <<  sgamma2t << endl;
  cout << "sigma^2_f       : " <<  sgamma2f << endl;
  cout << "sigma_t/sigma_f : " <<  TMath::Sqrt(sgamma2t/sgamma2f) << endl;
  cout << "Receiver pulse g ----------" << endl;
  cout << "|g|^2           : " <<  g->Norm2() << endl;
  cout << "sigma^2_t       : " <<  sg2t << endl;
  cout << "sigma^2_f       : " <<  sg2f << endl;
  cout << "sigma_t/sigma_f : " <<  TMath::Sqrt(sg2t/sg2f) << endl;
  
  delete g;
  delete gamma;
  delete fg;
  delete fgamma;
  
}

void        TDSPNofdm::Correlation(TDSPMatrix*c) {
  // if (fOptions&OPTION_DEBUG_CORRELATION) {
//     c->Print();
//     TCanvas *dw = DebugGetWindow("Correlation Matrix");
//     dw->Clear();
//     dw->cd();
//     c->Draw("hist box");
//     dw->Update();    
//     if (!DebugWaitToContinue()) fOptions&=!(OPTION_DEBUG_CORRELATION);
//   }
  Emit("Correlation(TDSPMatrix*)",c);
}

void        TDSPNofdm::Mapping(TDSPMatrix*c) {
  // if (fOptions&OPTION_DEBUG_MAPPING) {
//     c->Print();
//     TCanvas *dw = DebugGetWindow("Correlation Matrix");
//     dw->Clear();
//     dw->cd();
//     c->Draw("hist box");
//     dw->Update();    
//     if (!DebugWaitToContinue()) fOptions&=!(OPTION_DEBUG_MAPPING);
//   }
  Emit("Mapping(TDSPMatrix*)",c);
}
void        TDSPNofdm::MappingPart1(TDSPMatrix*c) {
  Emit("MappingPart1(TDSPMatrix*)",c);
}
