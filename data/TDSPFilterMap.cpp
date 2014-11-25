/** TDSP *******************************************************************
                          TDSPFilterMap.cpp  -  description
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

#include "TDSPFilterMap.h"

ClassImpQ(TDSPFilterMap);

TDSPFilterMap::TDSPFilterMap(char *name) : TDSPOperator(name) {
  fSynthesisBank = NULL;
  fAnalysisBank  = NULL;

  fSignal        = new TDSPSignal("output1"); // The Output Signal
  fSignal->SetTitle("the output signal of the linear filter");

  fInverseSignal = new TDSPSignal("output2"); // The Inverse Output Signal
  fInverseSignal->SetTitle("the output signal of the inverse filter");


  // Add the Signals
  
  Signal_Update           = AddSignal("Update(TDSPSignal*)");
  Signal_InverseUpdate    = AddSignal("InverseUpdate(TDSPSignal*)");
  
  // Add the Slots
  
  Slot_Update             = AddSlot("Update(TDSPSignal*)");
  Slot_InverseUpdate      = AddSlot("InverseUpdate(TDSPSignal*)");


}

void TDSPFilterMap::_cleanSynthesisBank() {
  if (fSynthesisBank&&(_isMySynthesisBank())) delete fSynthesisBank;
  _disableMySynthesisBank();
  fSynthesisBank = NULL;
}

void TDSPFilterMap::_cleanAnalysisBank() {
  if (fAnalysisBank &&(_isMyAnalysisBank()))  delete fAnalysisBank;
  _disableMyAnalysisBank();
  fAnalysisBank = NULL;
}

void TDSPFilterMap::_createMySynthesisBank() {
  _cleanSynthesisBank();
  fSynthesisBank = new TDSPFilterBank(); 
  _enableMySynthesisBank();
}

void TDSPFilterMap::_createMyAnalysisBank() {
  _cleanAnalysisBank();
  fAnalysisBank = new TDSPFilterBank(); 
  _enableMyAnalysisBank();
}

void TDSPFilterMap::_provideBanks() {
  if (!fSynthesisBank) 
    _createMySynthesisBank();  
  if (!fAnalysisBank) 
    _createMyAnalysisBank();  
}

void TDSPFilterMap::SetAnalysisBank(TDSPFilterBank *f) {
  if (f!=fAnalysisBank) {
    _cleanAnalysisBank();
    fAnalysisBank = f;
  }
}
void TDSPFilterMap::SetSynthesisBank(TDSPFilterBank *f) {
  if (f!=fSynthesisBank) {
    _cleanSynthesisBank();
    fSynthesisBank = f;
  }
}

TDSPFilterMap::~TDSPFilterMap(){
  _cleanSynthesisBank();
  _cleanAnalysisBank();
}

TDSPVector* TDSPFilterMap::Analyze(TDSPVector *input, TDSPVector *output) {
  if (fAnalysisBank) {
    output = fAnalysisBank->Analyze(input,output);
    return output;
  }
  
  Error("Analyze","No analysis filterbank connected to the object (name:%s)",GetName());
  return NULL;
}

TDSPVector* TDSPFilterMap::Synthesize(TDSPVector *input, TDSPVector *output) {
  if (fSynthesisBank) {
    output=fSynthesisBank->Synthesize(input,output);
    return output;
  }
  Error("Synthesize","No synthesis filterbank connected to the object (name:%s)",GetName());
  return NULL;
}

// Get/Set the filtermatrix directly

TDSPMatrix*  TDSPFilterMap::GetMatrix() {
  if (fSynthesisBank) return fSynthesisBank->GetMatrix();
  Error("GetMatrix","Synthesis filterbank not given !");
  return NULL;
}

void         TDSPFilterMap::SetMatrix(TDSPMatrix *a) {
  if (fSynthesisBank) fSynthesisBank->SetMatrix(a);
  Error("SetMatrix","Synthesis filterbank not given !");
}

TDSPMatrix*  TDSPFilterMap::GetInverseMatrix() {
  if (fAnalysisBank) return fAnalysisBank->GetMatrix();
  Error("GetInverseMatrix","Analysis filterbank not given !");
  return NULL;
}

void         TDSPFilterMap::SetInverseMatrix(TDSPMatrix *a) {
  if (fAnalysisBank) fAnalysisBank->SetMatrix(a);
  Error("SetInverseMatrix","Analysis filterbank not given !");
}


void         TDSPFilterMap::SetNumFilters(UInt_t n) {
  _provideBanks();
  fSynthesisBank->SetNum(n);
  fAnalysisBank->SetNum(n);
}


UInt_t TDSPFilterMap::GetNumFilters() {
  if (!fSynthesisBank) {
    Error("GetNumFilters","Synthesis bank not given !");
    return 0;
  }
  UInt_t sf = fSynthesisBank->GetNum();

  if (!fAnalysisBank) {
    Error("GetNumFilters","Analysis bank not given !");
    return 0;
  }

  if (sf!=fAnalysisBank->GetNum()) {
    Error("GetNumFilters","Synthesis bank and Analysis have different number of filters !");
  }

  return sf;
  
}


void     TDSPFilterMap::SetFreqPad(UInt_t s) {
  _provideBanks();
  fSynthesisBank->SetFreqPad(s); 
  fAnalysisBank->SetFreqPad(s);
}

UInt_t   TDSPFilterMap::GetFreqPad() {
  if (!fSynthesisBank) {
    Error("GetFreqPad","Synthesis filterbank no yet initialized !");
    return 0;
  }
  UInt_t f = fSynthesisBank->GetFreqPad();

  if (!fAnalysisBank) {
    Error("GetFreqPad","Analysis filterbank no yet initialized !");
    return 0;
  }

  if (f!=fAnalysisBank->GetFreqPad()) {
    Error("GetFreqPad","Frequency spacings of synthesis(%d) and analysis(%d) bank differ !",
	  f,fAnalysisBank->GetFreqPad());
    return 0;
  }
  return f;
}

Double_t   TDSPFilterMap::GetBandWidthEfficiency() {
  if (!fSynthesisBank) {
    Error("GetBandWidthEfficiency","Synthesis filterbank no yet initialized !");
    return 0;
  }
  Double_t f = fSynthesisBank->GetBandWidthEfficiency();

  if (!fAnalysisBank) {
    Error("GetBandWidthEfficiency","Analysis filterbank no yet initialized !");
    return 0;
  }

  if (TMath::Abs(f-fAnalysisBank->GetBandWidthEfficiency())>1.e-10) {
    Error("GetBandWidthEfficiency","BandWidthEfficiency's of the synthesis(%f) and the analysis(%f) bank differ !",
	  f,fAnalysisBank->GetBandWidthEfficiency());
    return 0;
  }
  return f;
}

void   TDSPFilterMap::SetSubSamplings(UInt_t s) {

  _provideBanks();
  fSynthesisBank->SetSubSamplings(s); 
  fAnalysisBank->SetSubSamplings(s);

}

UInt_t TDSPFilterMap::GetSubSamplings() {
  if (!fSynthesisBank) {
    Error("GetSubSamplings","Synthesis bank not given !");
    return 0;
  }

  UInt_t sf = fSynthesisBank->GetSubSamplings();

  if (!fAnalysisBank) {
    Error("GetSubSamplings","Analysis bank not given !");
    return 0;
  }

  if (sf!=fAnalysisBank->GetSubSamplings()) {
    Error("GetSubSamplings","Synthesis bank and Analysis have different up/down-samplings !");
  }
  
  return sf;
  
}


void    TDSPFilterMap::Activate(UInt_t s) {
  if (!fSynthesisBank) 
    Error("Activate","Synthesis bank not given !");
  else
    fSynthesisBank->Activate(s);

  if (!fAnalysisBank) 
    Error("Activate","Analysis bank not given !");
  else
    fAnalysisBank->Activate(s);
}

void         TDSPFilterMap::Deactivate(UInt_t s) {
  if (!fSynthesisBank) 
    Error("Deactivate","Synthesis bank not given !");
  else
    fSynthesisBank->Deactivate(s);
  
  if (!fAnalysisBank) 
    Error("Deactivate","Analysis bank not given !");
  else
    fAnalysisBank->Deactivate(s);
}

UInt_t       TDSPFilterMap::GetNumActiveSynthesisFilters() {
  if (!fSynthesisBank) {
    Error("GetNumActiveSynthesisFilter","Synthesis bank not given !");
    return 0;
  }
  return fSynthesisBank->GetNumActiveFilters();
}

UInt_t       TDSPFilterMap::GetNumActiveAnalysisFilters() {
  if (!fAnalysisBank) {
    Error("GetNumActiveAnalysisFilter","Analysis bank not given !");
    return 0;
  }
  return fAnalysisBank->GetNumActiveFilters();
}

TDSPVector*  TDSPFilterMap::GetSynthesisFilter(UInt_t s) {
  if (!fSynthesisBank) {
    Error("GetSynthesisFilter","Synthesis bank not given !");
    return NULL;
  }
  return fSynthesisBank->GetFilter(s);
}

TDSPVector*  TDSPFilterMap::GetAnalysisFilter(UInt_t s) {
  if (!fAnalysisBank) {
    Error("GetAnalysisFilter","Synthesis bank not given !");
    return NULL;
  }
  return fAnalysisBank->GetFilter(s);
}

void         TDSPFilterMap::LoadAndInitFromProtoType(Option_t *method, char *fname) {

  UInt_t pos;

  // Create the Banks if not given

  _provideBanks();

  TString opt = method;


  // Init the Synthesis bank

  fSynthesisBank->LoadAndInitFromProtoType(opt,fname);


  // Init the Analysis bank

  if ((pos = opt.Index("idft"))!= kNPOS) // dft<->idft 
    opt.Replace(pos,4,"dft");
  else
    if ((pos = opt.Index("dft"))!= kNPOS) // dft<->idft 
      opt.Replace(pos,3,"idft");
  

  fAnalysisBank->LoadAndInitFromProtoType( opt+" receiver reverse",fname);

  // make non-causility

  Info("LoadAndInitFromProtoType","Time shifting the filterbank by -%d (non-causality)!",
       fSynthesisBank->GetZero()+fAnalysisBank->GetLength()-1);

  fAnalysisBank->SetZero(fSynthesisBank->GetZero()+fAnalysisBank->GetLength()-1);
}


void    TDSPFilterMap::Print() {

  cout << "Synthesis Filter Bank:" << endl << " ";
  if (fSynthesisBank)
    fSynthesisBank->Print();
  else 
    cout << "(not yet defined)" << endl;

  cout << "Analysis Filter Bank:" << endl << " ";
  if (fAnalysisBank)
    fAnalysisBank->Print();
  else 
    cout << "(not yet defined)" << endl;

}

// void TDSPFilterMap::SetOption(Option_t *option) {
  
//   TString opt = option;
//   opt.ToLower();
  
//   Ssiz_t pos;
  
//   if ((pos = opt.Index("matchedfilter"))!= kNPOS) {
//     SetMode(1);
//     opt.Remove(pos,13);
//   }

//   if ((pos = opt.Index("mf"))!= kNPOS) {
//     SetMode(1);
//     opt.Remove(pos,2);
//   }
  
//   if ((pos = opt.Index("corr"))!= kNPOS) {
//     SetMode(2);
//     opt.Remove(pos,4);
//   }


//   if (opt.Strip().Length()) 
//     Error("SetOption","Unknown Option(s) \"%s\" !!",opt.Strip().Data());
  
// }
