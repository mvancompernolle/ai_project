/** TDSP *******************************************************************
                          TDSPFilterBank.cpp  -  description
                             -------------------
    begin                : Mon Okt 14 2002
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
#include <TFile.h>
#include <TDSPFifo.h>
#include <TDSPMath.h>
#include <MultiRate.h>
#include "TDSPFilterBank.h"

ClassImp(TDSPFilterBank)

#ifndef LINALG_MAT_ROWWISE
# warning "filterbank for column wise matrices not yet implemented"
#endif

TDSPFilterBank::TDSPFilterBank(UInt_t N, UInt_t L){
  fFilterBank = NULL;
  fNumActive=0;
  SetSize(N,L);
  fZero = 0;
#ifndef LINALG_MAT_ROWWISE
  Error("TDSPFilterBank","Not yet implemented in column wise matrix compilation !!");
  return NULL;
#endif
  fMethodStr="(not specified)";
}

TDSPFilterBank::~TDSPFilterBank(){
  _CleanMe();
}

void TDSPFilterBank::_CleanMe() {
  if (fFilterBank) delete fFilterBank;
  _disableMyMatrix();
}


Bool_t TDSPFilterBank::_IsReady() {
  if (!fFilterBank) return kFALSE;
  return kTRUE;
}

void   TDSPFilterBank::_reset_options() {
  SetFilterOption(0); // Default options
  fNumActive=GetNum();
}

Bool_t TDSPFilterBank::IsActive(UInt_t n) { 
  if (n<GetNum())
    return !(fFilterOptions[n]&0x1);
  Error("IsActive","n=%d exceeds number of filters = %d!",n,GetNum());
  return kFALSE;
}

void   TDSPFilterBank::Activate(UInt_t n) { 
  if (n<GetNum()) {
    if (!IsActive(n)) {++fNumActive;fFilterOptions[n]&=!(0x1);};
  } else 
    Error("Activate","n=%d exceeds number of filters = %d!",n,GetNum());
}
 
void   TDSPFilterBank::Deactivate(UInt_t n) { 
  if (n<GetNum()) {
    if (IsActive(n)) {fFilterOptions[n]|=0x1;--fNumActive;};
  } else 
    Error("Deactivate","n=%d exceeds number of filters = %d!",n,GetNum());
}

UInt_t TDSPFilterBank::GetLength() {
  if (_IsReady()) return fFilterBank->GetCols();
  Warning("GetLength","filterbank to yet set !!");
  return 0;
}

UInt_t TDSPFilterBank::GetNum() {
  if (_IsReady()) return fFilterBank->GetRows();
  Warning("GetNum","filterbank to yet set !!");
  return 0;
}

void   TDSPFilterBank::SetNum(UInt_t n) {
  if (_IsReady()) fFilterBank->SetRows(n);
  else {
    fFilterBank = new TDSPMatrix(n,0);
    _enableMyMatrix();
  }

  fSubSamplings.Set(n);
  fFilterOptions.Set(n);
  _reset_options();
}

void   TDSPFilterBank::SetLength(UInt_t l) {
  if (_IsReady()) fFilterBank->SetCols(l);
  else {
    fFilterBank = new TDSPMatrix(0,l);
    _enableMyMatrix();
    _reset_options();
  }
}

void   TDSPFilterBank::LoadMatrix(char *fname, char *key) {
  TFile f(fname);
  SetMatrix((TDSPMatrix*)f.Get(key));
  f.Close();
}

void   TDSPFilterBank::SetMatrix(TDSPMatrix *a) {
  if (a!=fFilterBank) {
    _CleanMe();
    fFilterBank = a;
    _reset_options();
  }
}

void TDSPFilterBank::SetSize(UInt_t N, UInt_t L) {

  if (_IsReady()) {
    fFilterBank->SetSize(N,L);
  } else {
    fFilterBank  = new TDSPMatrix(N,L);
    _enableMyMatrix();
  }


  fSubSamplings.Set(N);
  SetSubSamplings(N); // Default = critical Subsamplings

  fFilterOptions.Set(N);
  _reset_options();

}

void TDSPFilterBank::SetFilter(UInt_t n, TDSPVector *f) {
  if (!_IsReady()) {
    Error("SetFilter","filterbank not yet initialized!");
    return;
  }
  if (!f) {
    Error("SetFilter","filter is empty !");
    return;
  }
  
  if (n>=GetNum()) {
    Error("SetFilter","filter number %d is not in the range 0..%d",
	  n, GetNum()-1);
    return;
  }
   
  if ((UInt_t)f->GetLen()!=GetLength()) {
    Error("SetFilter","filter length(%d) does not match the filterbank length(%d)",
	  f->GetLen(),GetLength());
    return;
  }

  if (f->GetSamplingRate()!=1.0) 
    Warning("SetFilter","Samplingrate(%f) ignored!!", f->GetSamplingRate());

  // Now set the filter !!

  fFilterBank->SetRow(n,f);
}

TDSPVector* TDSPFilterBank::GetFilter(UInt_t n, TDSPVector *f) {
  if (fFilterBank) 
    return fFilterBank->GetRow(n,f);
  
  Error("GetFilter","filterbank not yet initialized! initialized!");
  return NULL;
}

void TDSPFilterBank::Print() {
  cout << "OBJ: TDSPFilterBank" << endl;
  cout << " method        = " << fMethodStr << endl;
  cout << " filter-matrix = ";
  if (fFilterBank) {
    cout << "\"num of filters\"x\"filterlength\" = " << GetLength() << "x" << GetNum() << endl;
    cout << " Subsamplings   = ";
    if (IsCommonSubSampling()) cout << GetSubSamplings();
    else 
      cout << "(not common subsamplings)";
  } else 
    cout << "(not yet set !)";

  cout << endl;
  cout << " active fil.   = ";
  UInt_t n= GetNum();

  for(UInt_t i=0;i<n;++i) if (IsActive(i)) cout << i << " ";
  cout << endl;
  cout << " inactive fil. = ";
  
  for(UInt_t j=0;j<n;++j) if (!IsActive(j)) cout << j << " ";
  cout << endl;

  if (_isDFT()) 
    cout << " frequency pad = " << fFreqPad << endl;
  
  cout << " Bandwidth-eff = " << GetBandWidthEfficiency() << endl;
}

// TH2* TDSPFilterBank::Draw(Option_t *opt) {
//   if (!_IsReady()) {
//     Error("Draw","filterbank not yet initialized !");
//     return NULL;
//   }
//   return fFilterBank->Draw(opt);
// }
void TDSPFilterBank::SetFilterOption(UInt_t opt) {
  
  if (!_IsReady()) {
    Error("SetFilterOption","filterbank not yet initialized !");
    return;
  }
  UInt_t n = GetNum();
  for(UInt_t i=0;i<n;++i) fFilterOptions[i] = opt;
}

void TDSPFilterBank::SetFilterOption(UInt_t n, UInt_t opt) {
  
  if (!_IsReady()) {
    Error("SetFilterOption","filterbank not yet initialized !");
    return;
  }

  if (n>=GetNum()) {
    Error("SetFilterOption","filter number (%d) out of range (0..%d)!",
	  n, GetNum()-1);
    return;
  }
  
  fFilterOptions[n] = opt;
}


UInt_t TDSPFilterBank::GetFilterOption(UInt_t n) {
  
  if (!_IsReady()) {
    Error("GetFilterOption","filterbank not yet initialized !");
    return 0;
  }

  if (n>=GetNum()) {
    Error("GetFilterOption","filter number (%d) out of range (0..%d)!",
	  n, GetNum()-1);
    return 0;
  }
  
  return fFilterOptions[n];
}



void TDSPFilterBank::SetSubSampling(UInt_t n, UInt_t T) {
  if (!_IsReady()) {
    Error("SetSubSampling","filterbank not yet initialized !");
    return;
  }

  if (n>=GetNum()) {
    Error("SetSubSampling","filter number (%d) out of range (0..%d)!",
	  n, GetNum()-1);
    return;
  }
  
  fSubSamplings[n] = T;

  // Update min-max 

  _MinMaxSubSampling();
}

void TDSPFilterBank::SetSubSamplings(UInt_t T) {
  if (!_IsReady()) {
    Error("SetSubSamplings","filterbank not yet initialized !");
    return;
  }
  UInt_t n = GetNum();
  for(register UInt_t i=0;i<n;++i) 
    fSubSamplings[i] = T;

  // Update min-max 


  fMinSubSampling=fMaxSubSampling=T;

}

UInt_t TDSPFilterBank::GetSubSampling(UInt_t n) {
  if (!_IsReady()) {
    Error("GetSubSampling","filterbank not yet initialized !");
    return 0;
  }
  
  if (n>=GetNum()) {
    Error("GetSubSampling","filter number (%d) out of range (0..%d)!",
	  n, GetNum()-1);
    return 0;
  }
  
  return (UInt_t)(fSubSamplings[n]);
}

void TDSPFilterBank::_MinMaxSubSampling() {
  
  UInt_t   n = GetNum();
  if (n) {
    Int_t max = fSubSamplings[0];
    Int_t min = fSubSamplings[0];
    for(register UInt_t i=1;i<n;++i) {
      if (fSubSamplings[i]>max) max=fSubSamplings[i];
      if (fSubSamplings[i]<min) min=fSubSamplings[i];
    }
  }
  
  Error("_MinMaxSubSamplings","number of filters in the filterbank is zero !!");
}


Bool_t TDSPFilterBank::IsCommonSubSampling() {

  if (!_IsReady()) {
    Error("IsCommonSubSampling","filterbank not yet initialized !");
    return 0;
  }
  
  UInt_t n  = GetNum();
  UInt_t ii = (UInt_t)(fSubSamplings[0]);
  
  for(register UInt_t i=1;i<n;++i) 
    if (ii!=(UInt_t)(fSubSamplings[i])) return kFALSE;
  
  return kTRUE;
  
}

UInt_t TDSPFilterBank::GetSubSamplings() {
  if (!IsCommonSubSampling()) {
    Error("GetSubSamplings","filterbank has not a common subsampling factor !");
    return 0;
  }
  return (UInt_t)(fSubSamplings[0]);
}


// Signal Synthesis
//

TDSPVector *TDSPFilterBank::Synthesize(TDSPVector *input, TDSPVector *output) {

  if (!output) output = new TDSPVector();
  
  Int_t     ilen = input->GetLen();  // The length of the input vector
  Int_t   L      = GetLength();      // The common length of the filters
  Int_t   N      = GetNum();         // The Number of filters
  
  if (ilen%fNumActive) 
    Warning("Synthesize","input signal length(%d) is not a multiple of the number of active filters (%d)!",
	    ilen,fNumActive);

  output->SetLen((ilen/fNumActive-1)*fMaxSubSampling+L);  // really numactive ? 
  output->Zeros();
  
  if (fFilterBank->GetSamplingRate()!=1.) 
    Warning("Synthesize","filterbank samplingrate (%f) ignored !!",fFilterBank->GetSamplingRate());

#ifdef LINALG_MAT_ROWWISE
  
  TComplex *fil  = fFilterBank->GetVec(); // The pointer onto the first filter (1. row of the matrix)

  for(Int_t i=0;i<N;++i) {                // Loop over the filter
    if (IsActive(i)) 
      UpSampling(input,
		 fSubSamplings[i],  // The Upsampling factor of that filter
		 fil, L,fZero,     // the reference to the filter, its length and its zero-point
		 output,           // result vector
		 fNumActive, i,    // parallel/serial-conversion (stream i of N streams)
		 MULTIRATE_SAMPLING_RAWMODE
		 );            
    fil+=L;                               // next filter
  }
#else
  Error("Synthesize","column wise filterbank not yet implemented!");
#endif
  
  // Set the sampling-Rate of the output-signal

  output->SetSamplingRate(input->GetSamplingRate()*fMaxSubSampling/fNumActive);
  return output;

}

// Signal Analysis
//

TDSPVector *TDSPFilterBank::Analyze(TDSPVector *input, TDSPVector *output) {

  if (!output) output = new TDSPVector();
  
  Int_t     ilen = input->GetLen();  // The length of the input vector
  Int_t   L      = GetLength();      // The common length of the filters
  Int_t   N      = GetNum();         // The Number of filters
  
  output->SetLen(((ilen-L)/fMinSubSampling+1)*fNumActive); // really fNumActive ??
  output->Zeros();
  
  if (fFilterBank->GetSamplingRate()!=1.) 
    Warning("Analyze","filterbank samplingrate (%f) ignored !!",fFilterBank->GetSamplingRate()); 


#ifdef LINALG_MAT_ROWWISE
  TComplex *fil =fFilterBank->GetVec();
  for(Int_t i=0;i<N;++i) { // Loop over the filter
    if (IsActive(i)) 
      DownSampling(input,
		   fSubSamplings[i],// The downsampling factor of that filter
		   fil,L,fZero,    // the reference to the filter, its length and its zero-point
		   output,         // result vector
		   fNumActive,i,   // serial/parallel-conversion (stream i of N streams) 
		   MULTIRATE_SAMPLING_RAWMODE
		   );
    fil+=L; // next filter
  }
#else
  Error("Analyze","column wise filterbank not yet implemented!");
#endif

  // Set the sampling-Rate of the output-signal
  output->SetSamplingRate(input->GetSamplingRate()/fMinSubSampling*fNumActive);

  return output;

}
 

// Load the prototyp filter from the file "fname" under the key "key"
// and call InitFromProtoTyp

void   TDSPFilterBank::LoadAndInitFromProtoType(Option_t* method,
						char *fname,
						char *key) {
  TString opt = method;
  opt.ToLower();
  
  Ssiz_t pos;
  
  // Load an TDSPNofdm-like Matrix 

  if ((pos = opt.Index("nofdm"))!= kNPOS) {
    opt.Remove(pos,5);
    Info("LoadAndInitFromProtoType","Loading \"%s\" ...", fname);
    
    TDSPMatrix *m1;    // Temporary matrix
    TDSPVector *proto; // Temporary vector for the prototyp
    
    TFile f(fname);
    
    if ((pos = opt.Index("receiver"))!=kNPOS) {
      opt.Remove(pos,8);
      // Expect the receiver matrix
      //
      if (!key) key="ReceiverMatrix";
      m1 = (TDSPMatrix*)f.Get(key);
      proto = m1->GetRow(0); 
      Info("LoadAndInitFromProtoType","using %s(0,:) as prototype", key);
    } else {
      // Expect the transmitter matrix
      //
      if (!key) key="TransmitMatrix";
      m1 = (TDSPMatrix*)f.Get(key);
      proto = m1->GetColumn();
      Info("LoadAndInitFromProtoType","using %s(:,0) as prototype", key);
    }
    
    delete m1;
    
    proto->FFTShiftMe();

    if ((pos = opt.Index("reverseprototype"))!= kNPOS) {
      opt.Remove(pos,16);
      Info("LoadAndInitFromProtoType","time-reversing prototype (only)", key);
      proto->Reverse();
    }

    // Now init the filterbank with this prototyp
    //

    InitFromProtoType(opt,proto);
    delete proto;
    return;
  }

  if (opt.Strip().Length()) 
    Error("LoadAndInitFromProtoType","Unknown specifications \"%s\" !!",opt.Strip().Data());
  
}

// Init the filters from a prototyp filter
//

void   TDSPFilterBank::InitFromProtoType(Option_t* method,
					 TDSPVector *proto) {

  
  TString opt = method;
  opt.ToLower();
  
  Ssiz_t pos;
  
  Bool_t conj    = kTRUE;
  Bool_t reverse = kFALSE;

  if ((pos = opt.Index("reverse"))!=kNPOS) {
    opt.Remove(pos,7);
    reverse=kTRUE;
  }

  if ((pos = opt.Index("idft"))!=kNPOS) {
    opt.Remove(pos,1); // remove here only the "i" from "idft"
    conj = kFALSE; 
  }

  if ((pos = opt.Index("dft"))!= kNPOS) {
    opt.Remove(pos,3);

    // Init the filterbank as a DFT-Filterbank
    //

    if (conj) {
      Info("InitFromProtoType","Initialize the filterbank as a DFT filterbank");
      fMethodStr = "dft";
    } else {
      Info("InitFromProtoType","Initialize the filterbank as an inverse DFT filterbank");
      fMethodStr = "idft";
    }
    if (!proto) {
      Error("InitFromProtoType","%s-Filterbank initialisation failed - no prototype given !",fMethodStr.Data());
      return;
    }

    UInt_t len = proto->GetLen();
    
    if (!len) {
      Error("InitFromProtoType","%s-Filterbank initialisation failed - prototype has zero length!",fMethodStr.Data());
      return;
    }

    _enableDFT();

    if (!fFreqPad) 
      Warning("InitFromProtoType","OOps - FreqPad is zero - really ?");

    SetLength(len);

    UInt_t num = GetNum();
    
    // Loop over the filters
    //
    TDSPVector ModulatedProtoType;
    if (reverse) 
      Info("InitFromProtoType","Time reversing the whole bank");

    for(UInt_t i=0;i<num;++i) {
      proto->Modulate((conj ? -1. : 1.)*2.*Pi*i*fFreqPad/Double_t(len), 
		      0,0,0,
		      &ModulatedProtoType);
      if (reverse) ModulatedProtoType.Reverse();
      SetFilter(i,&ModulatedProtoType);
    }
  }
  if (opt.Strip().Length()) 
    Error("InitFromProtoType","Unknown method \"%s\" !!",opt.Strip().Data());
  


}

Double_t       TDSPFilterBank::GetBandWidthEfficiency() { 
  UInt_t up = GetSubSamplings();
  if (!up) {
    Error("GetBandWidthEfficiency","subsamplings are zero !");
    return 0;
  }
  return Double_t(fNumActive)/Double_t(up); 
}


TDSPMatrix*    TDSPFilterBank::GetDualMatrix(Option_t *method) {

  if (!_IsReady()) {
    Error("GetDualMatrix","Filterbank not yet initialized !");
    return NULL;
  }

  TDSPMatrix* pinv1 = fFilterBank->PseudoInverse();
  TDSPMatrix* pinv2 = pinv1->Transposed();
  pinv2->ReverseColumns();
  delete pinv1;
  return pinv2;

}
