/** TDSP *******************************************************************
                          TDSPMultiEcho.cpp  -  description
                             -------------------
    begin                : Die Dez 10 2002
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

#include <Riostream.h>
#include <TRandom.h>
#include <TDSPMath.h>
#include <TPad.h>
#include "TDSPfft.h"
#include "TDSPMultiEcho.h"


#define MODELGROUP_STD     1

#define STD_ID             1

#define MODELGROUP_COST207 207

#define COST207_TU         1
#define COST207_RA         2
#define COST207_BU         3
#define COST207_HT         4


void TDSPSingleEcho::Print() {
  cout << "Std : min. tau = " << fMinTau << "탎 tau-Range=" << fTauRange;
  cout << "탎 Energy=" << fFraction*100. << endl;
}
void TDSPSingleEchoEXP::Print() {
  cout << "Exp. Decay : min. tau = " << fMinTau << "탎 tau-Range=" << fTauRange;
  cout << "탎 Energy=" << fFraction*100. << "% Decay=" << fDecay << endl;
}
Double_t TDSPSingleEcho::GetProbability(Double_t tau) {
  if (tau<fMinTau) return 0;
  if (tau>fMinTau+fTauRange) return 0;
  return 1./fTauRange*fFraction; // Default: uniform in fMinTau...fMinTau+fTauRange
}

Double_t TDSPSingleEchoEXP::GetProbability(Double_t tau) {
  if (tau<fMinTau) return 0;
  if (tau>fMinTau+fTauRange) return 0; 
  Double_t al=fDecay/(1.-TMath::Exp(-fDecay*fTauRange));
  return al*TMath::Exp(-fDecay*(tau-fMinTau))*fFraction; // Default: uniform in fMinTau...fMinTau+fTauRange
}

TGraph*  TDSPSingleEcho::Graph(TGraph *in, Double_t x1, Double_t x2, UInt_t num) {
  if (!in) in = new TGraph();
  in->SetName(GetName());
  in->SetTitle(GetTitle());
  in->Set(num);
  if (x1==-9999) x1=fMinTau-fTauRange*.1;
  if (x2==-9999) x2=fMinTau+fTauRange*1.1;
  Double_t dx=(x2-x1)/num;
  for(register UInt_t i=0;i<num;++i) 
    in->SetPoint(i,x1+dx,GetProbability(x1+dx));
  return in;
}


void TDSPSingleEcho::Draw(Option_t *o, Double_t x0, Double_t x1, UInt_t num) {
  TGraph *a = Graph(NULL,x0,x1,num);
  a->Draw(o);
  a->GetXaxis()->SetTitle("#tau / #mu s");
  gPad->Update();
}

void TDSPSingleEcho::ChangeRepresentation() {
  fPhase    = gRandom->Rndm()*2.*Pi; // Gleichverteilte Startphasen in (0..2pi)
  // Gleichverteilte Einfallswinkel in (0..pi) + Jakes-verteilte normierte Dopplerfrequenzen -1..1
  fDoppler  = TMath::Cos(gRandom->Rndm()*Pi); 
  //fTau=fMinTau+gRandom->Rndm()*fTauRange;
}

void TDSPSingleEchoEXP::ChangeRepresentation() {
  TDSPSingleEcho::ChangeRepresentation();
  Double_t al=fDecay/(1.-TMath::Exp(-fDecay*fTauRange));
  fTau=-1./fDecay*TMath::Log(1.-fDecay/al*gRandom->Rndm())+fMinTau;
}

TDSPMultiEcho::TDSPMultiEcho(char *name, char *opt, UInt_t numecho, Double_t sRate, Double_t maxdoppler) {
  fListOfEchos = new TList();
  fListOfEchos->SetOwner(); // Causes Tlist to manage the desctructor of its elements

  fTaus         = new TArrayD();
  fDiscreteTaus = new TArrayI();
  fDoppler      = new TArrayD();
  fPhase        = new TArrayD();
  fModelGroup  = 0; // No Model
  fNumEchos    = numecho;
  SetName(name);
  SetNumEchos(50);
  SetSymbolRate(sRate);
  SetMaxDoppler(maxdoppler);
  SetOverSampling(1);
  SetNumSlices(40);
  fFilter = new TDSPVector(1);
  fFilter->Element(0)=1.;
  SetCarrierFrequencyGHz(5.2); // 5.2GHz
  fHaveRepresentation=kFALSE;
  fHaveImpulseResponse=kFALSE;
  fRandomPhases=kTRUE;
  fImpulseResponse=NULL;
  fTransferFunction=NULL;
  fScatteringFunction=NULL;
  SetOption(opt);
}

TDSPMultiEcho::~TDSPMultiEcho(){
  if (fListOfEchos) delete fListOfEchos;
  if (fTaus) delete fTaus;
}

void TDSPMultiEcho::ls() {
  TIter next(fListOfEchos);
  while(TDSPSingleEcho *e = (TDSPSingleEcho*)next()) 
    e->Print();
}

void TDSPMultiEcho::Print() {
  cout << "OBJ: " << ClassName() << "  " << "name=" << GetName();
  cout << " title=" << GetTitle() << endl;
  cout << " Number of Echos    = " << fNumEchos << endl;
  cout << " Symbol Frequency   = " << GetSymbolRateMHz() << " MHz" << endl;
  cout << " Sample Frequency   = " << GetSymbolRateMHz()*fOverSampling << " MHz" << endl;
  cout << " Oversampling       = " << fOverSampling << endl;
  cout << " Max Doppler        = " << fMaxDoppler << "(v=" << GetSpeedKMH() << " kmh at f0=";
  cout << GetCarrierFrequencyMHz() << " MHz)" << endl;
  cout << " Symbol period      = " << 1./GetSymbolRateMHz() << " 탎" << endl;
  cout << " Sample period      = " << 1./GetSymbolRateMHz()/fOverSampling << " 탎" << endl;
  cout << " No. of time slices = " << fNumSlices << endl;
  cout << " Echo-pdf's         = ..." << endl;
  ls();
  if (!fRandomPhases) 
    cout<< " Ignoring Phases of the Echos..." << endl; 
}

void TDSPMultiEcho::SetOption(Option_t *o) {

  TString opt = o;
  opt.ToLower();

  Ssiz_t pos;

  UInt_t mg=0;
  UInt_t mo=0;
                        
  if ((pos = opt.Index("randomphases=yes"))!=kNPOS) {
    opt.Remove(pos,16);
    fRandomPhases=kTRUE;
  }
  if ((pos = opt.Index("randomphases=no"))!=kNPOS) {
    opt.Remove(pos,15);
    fRandomPhases=kFALSE;
  }

  if ((pos = opt.Index("std"))!= kNPOS) {
    opt.Remove(pos,3);
    mg = MODELGROUP_STD;
  }

  if ((pos = opt.Index("id"))!=kNPOS) {
    opt.Remove(pos,2);
    mo      = STD_ID;
    fRandomPhases=kFALSE;
    SetTitle("STD - Identity");
  }
 
  if ((pos = opt.Index("cost207"))!= kNPOS) {
    opt.Remove(pos,7);
    mg = MODELGROUP_COST207;
  }

  if ((pos = opt.Index("tu"))!=kNPOS) {
    opt.Remove(pos,2);
    mo      = COST207_TU;
    SetTitle("COST207 - Typical Urban  (non-hilly urban environment)");
  }
  if ((pos = opt.Index("ra"))!=kNPOS) {
    opt.Remove(pos,2);
    mo      = COST207_RA;
    SetTitle("COST207 - Rural Area     (non-hilly rural environment)");
  }
  if ((pos = opt.Index("bu"))!=kNPOS) {
    opt.Remove(pos,2);
    mo      = COST207_BU;
    SetTitle("COST207 - Bad Urban      (hilly urban environment)");
  }
  if ((pos = opt.Index("ht"))!=kNPOS) {
    opt.Remove(pos,2);
    mo      = COST207_HT;
    SetTitle("COST207 - Hilly Terrain  (hilly rural environment)");
  }
  
  if ((mg)&&(mo)) SetModel(mg,mo);

  if (opt.Strip().Length())
    Error("SetOption","Unknown Option(s) \"%s\" !!",opt.Strip().Data());
  
}


void TDSPMultiEcho::CleanModel() {
  fListOfEchos->Clear();
  if (fModelGroup) {
  }
}

void TDSPMultiEcho::SetNumEchos(UInt_t N) {
  fNumEchos = N;
  fTaus->Set(fNumEchos);
  fDoppler->Set(fNumEchos);
  fPhase->Set(fNumEchos);
}

void TDSPMultiEcho::SetModel(UInt_t mg, UInt_t mo) {

  if ((fModelGroup==mg)&&(fModel==mo)) return;
  CleanModel();

  fModelGroup=mg;
  fModel=mo;

  switch(fModelGroup) {
  case MODELGROUP_STD:
    switch(fModel) {
    case STD_ID : 
      // STD - Identity map ...
      Add(new TDSPSingleEcho(0,0.00000000001,1.));
      break;
    }
    break;
  case MODELGROUP_COST207:
    switch(fModel) {
    case COST207_TU :
      //COST207 - Typical Urban  (non-hilly urban environment)
      Add(new TDSPSingleEchoEXP(0 , 7,1,1));
      break;
    case COST207_RA :
      //COST207 - Rural Area     (non-hilly rural environment)
      Add(new TDSPSingleEchoEXP(0 ,.7,1,9.2));
      break;
    case COST207_BU :
      //COST207 - Bad Urban      (hilly urban environment)
      Add(new TDSPSingleEchoEXP(0 , 5,2./3.,1));
      Add(new TDSPSingleEchoEXP(5 , 5,1./3.,1));
      break;
    case COST207_HT :
      //COST207 - Hilly Terrain  (hilly rural environment)
      Add(new TDSPSingleEchoEXP(0 , 2,.88,3.5));
      Add(new TDSPSingleEchoEXP(15, 5,.12,1));
      break;
      
    default:
      Error("SetModel","Unkown COST207-Model (%d) !", fModel);
      return;
    }

    break;
  default:
    Error("SetModel","Modelgroup (%d) not implemented!",fModelGroup);
    return;
  }
 
  SetNumEchos(GetNumEchos());
  
}

Double_t TDSPMultiEcho::GetProbability(Double_t tau) {
  Double_t p=0;
  TIter next(fListOfEchos);
  while(TDSPSingleEcho *e = (TDSPSingleEcho*)next()) 
    p+=e->GetProbability(tau);
  return p;
}

Double_t TDSPMultiEcho::GetMinTau() {
  Double_t mintau=99999;
  TIter next(fListOfEchos);
  while(TDSPSingleEcho *e = (TDSPSingleEcho*)next()) 
    if (e->GetMinTau()<mintau) mintau=e->GetMinTau();
  return mintau;
}

Double_t TDSPMultiEcho::GetMinTauRange() {
  Double_t mintau=99999;
  TIter next(fListOfEchos);
  while(TDSPSingleEcho *e = (TDSPSingleEcho*)next()) 
    if (e->GetTauRange()<mintau) mintau=e->GetTauRange();
  return mintau;
}

Double_t TDSPMultiEcho::GetMaxTau() {
  Double_t maxtau=-99999;
  TIter next(fListOfEchos);
  while(TDSPSingleEcho *e = (TDSPSingleEcho*)next()) 
    if (e->GetMinTau()+e->GetTauRange()>maxtau) maxtau=e->GetMinTau()+e->GetTauRange();
  return maxtau;
}

Double_t TDSPMultiEcho::GetMaxTauRange() {
  Double_t maxtau=-99999;
  TIter next(fListOfEchos);
  while(TDSPSingleEcho *e = (TDSPSingleEcho*)next()) 
    if (e->GetMaxTau()>maxtau) maxtau=e->GetMaxTau();
  return maxtau;
}

Double_t TDSPMultiEcho::GetTauRange() {
  return GetMaxTau()-GetMinTau();
}


TGraph*  TDSPMultiEcho::Graph(TGraph *in, Double_t x1, Double_t x2, UInt_t num) {
  if (!in) in = new TGraph();
  in->SetName(GetName());
  in->SetTitle(GetTitle());
  in->Set(num);
  if (x1==-9999) x1=GetMinTau()-GetTauRange()*.1;
  if (x2==-9999) x2=GetMinTau()+GetTauRange()*1.1;
  Double_t dx=(x2-x1)/num;
  for(register UInt_t i=0;i<num;++i) {
    in->SetPoint(i,x1,GetProbability(x1));
    x1+=dx;
  }
  return in;
}


TMultiGraph*  TDSPMultiEcho::MultiGraph(TMultiGraph *in, Double_t x1, Double_t x2, UInt_t num) {
  if (!in) in = new TMultiGraph();
  in->SetName(GetName());
  in->SetTitle(GetTitle());
  if (x1==-9999) x1=GetMinTau()-GetTauRange()*.1;
  if (x2==-9999) x2=GetMinTau()+GetTauRange()*1.1;

  TIter next(fListOfEchos);

  while(TDSPSingleEcho *e = (TDSPSingleEcho*)next())
    in->Add(e->Graph(NULL,x1,x2,num));
  
  return in;
}


void TDSPMultiEcho::Draw(Option_t *o, Double_t x0, Double_t x1, UInt_t num) {
  TString opt = o;
  opt.ToLower();

  Ssiz_t pos;

  if ((pos = opt.Index("multi"))!= kNPOS) {
    opt.Remove(pos,5);
    TMultiGraph *m = MultiGraph(NULL,x0,x1,num);
    m->Draw(o);
    m->GetXaxis()->SetTitle("#tau / #mu s");
    gPad->Update();
    return;
  }
  TGraph *a = Graph(NULL,x0,x1,num);
  a->Draw(o);
  a->GetXaxis()->SetTitle("#tau / #mu s");
  gPad->Update();
}


TDSPMatrix* TDSPMultiEcho::GetTransferFunction() {
  fTransferFunction = CreateTransferFunction(fTransferFunction);
  return fTransferFunction;
}

TDSPMatrix* TDSPMultiEcho::GetScatteringFunction() {
  fScatteringFunction = CreateScatteringFunction(fScatteringFunction);
  return fScatteringFunction;
}
void TDSPMultiEcho::ChangeRepresentation() {
  
  
  TIter next(fListOfEchos);
  Int_t i=0;
  while(TDSPSingleEcho *e = (TDSPSingleEcho*)next()) {
    if (i>=fNumEchos) {
      Info("ChangeRepresentation","Echo %d lost ", i);
      continue;
    }
    Int_t N = TMath::Nint(fNumEchos*e->GetFraction());
    for(register Int_t j=0;j<N;++j) {
      e->ChangeRepresentation();
      fTaus->AddAt(e->GetTau(),i);
      fDoppler->AddAt(e->GetDoppler(),i);
      fPhase->AddAt(e->GetPhase(),i++);
    }
  }
  TDSPLTVChannel::ChangeRepresentation();
}

TDSPMatrix*        TDSPMultiEcho::CreateImpulseResponse(TDSPMatrix *out) {

  if (!out) {
    out=new TDSPMatrix();
    out->SetXTitle("#tau / #mu s");
    out->SetYTitle("t / #mu s");
    out->SetTitle("h(t,#tau)");
  }
  Int_t      Num          = fTaus->GetSize();
  Double_t   SymbolPeriod = 1./GetSymbolRateMHz(); // Symbol period in 탎ec
  Double_t   SamplePeriod = 1./GetSampleRateMHz(); // Sample period in 탎ec

  // Check normalisation of the bandlimit. filter

  if ((1.-fFilter->Norm2()/fOverSampling)>1.e-10) {
    Warning("GetImpulseResponse","Bandlimiting filter not correct normalized for oversampling.");
    Warning("                  ","oversampling*norm2=%.1f/%d=%.1f but should be =1.",fFilter->Norm2(),fOverSampling,
	    fFilter->Norm2()/fOverSampling);
  }
  // compute Echo-Arrivals in the symbol-takt
 
  fDiscreteTaus->Set(Num);  
  for(register Int_t i=0;i<Num;++i) 
    fDiscreteTaus->AddAt(TMath::Nint(fTaus->At(i)/SamplePeriod),i);
    
  // time of the smallest cluster in no. of taps

  Int_t mintaurange_sampled = Int_t(TMath::Ceil(GetMinTauRange()/SamplePeriod));
  
  if (mintaurange_sampled<4) {
    Warning("GetImpulseResponse","The approximation of the tau-continuous channel impulse response may be inexact,");
    Warning("                  ","since there is/are only %d<4 possible bins for the echo delay time within the", mintaurange_sampled);
    Warning("                  ","shortest echo cluster (duration=%.1f탎) - using a sample period %.1f탎.",GetMinTauRange(), SamplePeriod);
  }
  
  // sampled taurange

  Int_t taurange_sampled = Int_t(TMath::Nint(GetTauRange()/SamplePeriod));

  // Find the minimal discrete tau

  Int_t mindiscretetau   = 99999;
  for(Int_t jEcho=0;jEcho<Num;++jEcho) 
    if (mindiscretetau>fDiscreteTaus->At(jEcho)) mindiscretetau = fDiscreteTaus->At(jEcho);
  
  // Set the size of the matrix

  UInt_t filterlen=fFilter->GetLen();
  UInt_t NumTaps=taurange_sampled+filterlen;
  
  out->SetSize(NumTaps, fNumSlices);
  out->Zeros();

  // Normalisation

  //   Double_t norm=TMath::Sqrt(1./Num);
  Double_t norm=1./Num;

  // Loop over Echos

  for(Int_t iEcho=0;iEcho<Num;++iEcho) {
    Double_t omegaD = 2.*Pi*fDoppler->At(iEcho)*fMaxDoppler;
    Double_t phase  = fRandomPhases ? fPhase->At(iEcho) : 0.;
    UInt_t   pos    = fDiscreteTaus->At(iEcho)-mindiscretetau;
    UInt_t   end    = pos+filterlen;


    // Loop over the time
    //
    for(Int_t iTime=0;iTime<fNumSlices;++iTime) {
      UInt_t itau1 = 0;
      TComplex   p = ::Exp(I*(omegaD*iTime + phase))*norm;
      // Loop over the delay-axis
      //
      for(Int_t iTau=pos;iTau<end;++iTau) 
	out->Element(iTau, iTime) += fFilter->Element(itau1++)*p;
    }
  }
  return out;
}


TH1F*                 TDSPMultiEcho::Fill(UInt_t num, TH1F* h) {
  if (!h) {
    h = new TH1F(GetName(), GetTitle(), 50, GetMinTau(),GetMaxTau());
    h->SetAxisRange(GetMinTau()-GetTauRange()*.1,
		    GetMinTau()+GetTauRange()*1.1);
  }

  for(Int_t    i=0;i<num;++i) {
    ChangeRepresentation();
    Int_t N=fTaus->GetSize();
    for(Int_t  j=0;j<N;++j) 
      h->Fill(fTaus->At(j));
  }
  return h;
}



TDSPMatrix*     TDSPMultiEcho::CreateTransferFunction(TDSPMatrix *out) {

  TDSPMatrix *f = fft(GetImpulseResponse(),'C');
  f->FFTShiftMe('C');
  return f;
}

TDSPMatrix*     TDSPMultiEcho::CreateScatteringFunction(TDSPMatrix *out) {
  return NULL;
}
