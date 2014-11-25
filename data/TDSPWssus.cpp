/** TDSP *******************************************************************
                          TDSPWssus.cpp  -  description
                             -------------------
    begin                : Don Jun 27 2002
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

#include <TDSPMath.h>
#include <TError.h>
#include "TDSPfft.h"
#include "TDSPWssus.h"

ClassImpQ(TDSPWssus)


#define SCAT_TYPE_JAKES 1
#define SCAT_TYPE_FLAT  2
  
TDSPWssus::TDSPWssus(char *name) : TDSPLTVChannel(name){
  SetOverSampling(1.);
  SetSymbolRateMHz(1.);
  SetCarrierFrequencyGHz(5.2); // 5.2GHz

  fHaveScatteringFunction=kFALSE;
  fHaveSpreadingFunction=kFALSE;

  fType = SCAT_TYPE_JAKES;

  fTransferFunction   = NULL;
  fScatteringFunction = NULL;
  fSpreadingFunction  = NULL;

  /* per default make no assumptions on the blocklen -
     "CreateScatteringFunction" then selects a useful blocklen
     depending of the required oversampling ... */

  SetBlockLen(0);

}
TDSPWssus::~TDSPWssus(){
}


TDSPMatrix* TDSPWssus::CreateSpreadingFunction(TDSPMatrix *out) {
  if (!out) out = new TDSPMatrix();
  TDSPMatrix *r = GetScatteringFunction();
  out->SetSize(r->GetRows(), r->GetCols());
  out->RandNC(NULL,r);
  //r->Copy(out);
  return out;
}

void TDSPWssus::ChangeRepresentation() {
  fHaveSpreadingFunction=kFALSE;
  TDSPLTVChannel::ChangeRepresentation();
}

// This routine creates the Scattering Function S(tau,nu)
//
// the frequency is the column-index
// the delay is the row-index
//

TDSPMatrix* TDSPWssus::ScatFunction(Int_t     type,       // Type 
				    Double_t  alpha,      // exponential decay
				    Double_t  beta,       // weighting 
				    UInt_t    t0,         // the time support (maximum delay)
				    UInt_t    f0,         // the maximum Doppler shift
				    UInt_t    clen,       // Matrix size
				    TDSPMatrix *ScatFun) {
  
  if (!ScatFun)
    ScatFun = new TDSPMatrix();
  
  ScatFun->SetSize(clen,clen);
  ScatFun->Zeros();

  //Double_t a = (t0>1) ? a=(1./(t0-1.))*TMath::Log(alpha) : a=1.;
  for(register Int_t freq=0;freq<f0;++freq) {
    Double_t help = Double_t(freq)/Double_t(f0);
    switch(type) {
    case SCAT_TYPE_JAKES:
      help = TMath::Sqrt(1.-beta*help*help);
      break;
    case SCAT_TYPE_FLAT:
      help = 1;
      break;
    }
    for(register Int_t delay=0;delay<t0;++delay) {
      Double_t x = TMath::Exp(alpha*delay) / help;
      ScatFun->operator()(delay, freq)        = x;
      ScatFun->operator()(delay, clen-1-freq) = x;
    }
  }

  // Normalize the maximum to 1
  //*ScatFun /= ScatFun->MaxAbs();

  // new : (only true if the scattering function is square !!)

  ScatFun->Normalize(TMath::Sqrt(Double_t(clen)));

  return ScatFun;

}

// This routine creates from S(tau,nu) -> H(tau,t) for an intervall 0..t-1 (=TapFunLen)
//
// the time axis are the columns and the delay axis are the rows
//

TDSPMatrix* TDSPWssus::TapFunction(TDSPMatrix *ScatFun,   // The Scattering Matrix (delay=rows,freq=cols)
				   UInt_t      tlen,      // Maximum Time Delay
				   UInt_t      flen,      // Maximum Doppler Shift
				   UInt_t      TapFunLen, // The length of the tapfunction
				   TDSPMatrix *TapFun) {  // The tapfun itself
  
  Int_t scatcols = ScatFun->GetCols();
  Int_t scatrows = ScatFun->GetRows();

  // (expecting that scatrows/scatcols are the same ... )*/
  
  if (scatrows!=scatcols) {
    ::Error("TapFunction","Not yet tested for scatrows!=scatcols = %d != %d",
	    scatrows,scatcols);
    return NULL;
  }

  if (!TapFun) TapFun = new TDSPMatrix();
  
  TapFun->SetSize(tlen, TapFunLen);
    
# ifdef LINALG_MAT_ROWWISE
  
  TDSPVector *Alias = TapFun->Alias(NULL,TapFunLen);
  
  for(register Int_t l=0;l<tlen;++l) { // loop over the delays
    
    // Build the spectrum
    
    Alias->Zeros();
    Alias->Set(0,              ScatFun,  l   *scatcols,     flen);
    Alias->Set(TapFunLen-flen, ScatFun, (l+1)*scatcols-flen,flen); 
    

    // Processing row l of the matrix TapFun (with oversampling ...)

    gFFT->ifft(Alias->GetVec(),Alias->GetVec(), TapFunLen);

    // Move the Alias to the next row
    
    Alias->MoveVec(TapFunLen);

  }

  delete Alias;

  // renormalization (expecting that scatrows==scatcols... -but tested)
  
  // (*TapFun) *= TMath::Sqrt(Double_t(TapFunLen)/Double_t(scatcols));

  // Normalize direct - does this work ??
  
  TapFun->Normalize(TMath::Sqrt(Double_t(TapFunLen)));

  return TapFun;
  
# else
#  warning "Columnwise TDSPWSSUS::TapFunfunction not yet implemented"
  
  Error("TapFunction","Columnwise not yet implemented !");
  return NULL;
# endif
  
}

Int_t       TDSPWssus::GetDiscreteScatteringMatrixSize() {
  Int_t b = Int_t(TMath::Max(GetDiscreteMaxDelay(),2*(GetDiscreteMaxDoppler()-1))*1.2);
  /* make it odd - so that we have the zero-frequency in ... */
  return b%2 ? b : b+1;
}

TDSPMatrix* TDSPWssus::CreateTransferFunction(TDSPMatrix *out) {
  
  TDSPMatrix *f = fft(GetImpulseResponse(),'C',out);
  f->FFTShiftMe('C');
  return f;
}

Int_t       TDSPWssus::GuessChannelBlockLen() {
  // Set the BlockLen ------------------------------------------------
  // Because - we expect the MaxDoppler in Hz - the matrix indizies
  // are in Hz - To fit to the right sampling rate (GetSamplingRate()) we
  // will have to do an (huge) Oversampling in "CreateImpulseResponse" via "TapFunction"
  // 
  return   Int_t(
		 GetSamplingRate() /* [Hz] */
		 );
}

TDSPMatrix* TDSPWssus::CreateScatteringFunction(TDSPMatrix *out) {

  Int_t     b = GetBlockLen();
  // Set the BlockLen ------------------------------------------------
  // Because - we expect the MaxDoppler in Hz - the matrix indizies
  // are in Hz - To fit to the right sampling rate (GetSamplingRate()) we
  // will have to do an (huge) Oversampling in "CreateImpulseResponse" via "TapFunction"
  // 
  Int_t    b1 = GuessChannelBlockLen();

  if (!b) {
    Info("CreateScatteringFunction","Fixing the size of H(tau,t) as %d x %d",
	 GetDiscreteMaxDelay(),
	 b1);
  }

  /* make it a multiple of the channel matrix size if we need channel matrices */
    
  if (fCalculateChannelMatrix) {
    if (GetChannelMatrixSize()) {
      Int_t b2=b1%GetChannelMatrixSize();
      if (b2) 
	Warning("CreateScatteringFunction","Fixing the size of H(tau,t) to %d x %d (next multiple of %d)!",
		GetDiscreteMaxDelay(),
		b1+=GetChannelMatrixSize()-b2,
		GetChannelMatrixSize()
		);
    } else 
      Error("CreateScatteringFunction","matrix size of the channel matrices not specified (SetChannelMatrixSize(..))!");
  }
  
  SetBlockLen(b1);

  return ScatFunction(fType,
		      GetDiscreteExpDecay(),
		      1., 
		      GetDiscreteMaxDelay(), 
		      GetDiscreteMaxDoppler(), 
		      GetDiscreteScatteringMatrixSize(), 
		      out);
}

TDSPMatrix*  TDSPWssus::CreateImpulseResponse(TDSPMatrix *out) { 
  TDSPMatrix *spreadfunction=GetSpreadingFunction();
  return TapFunction(spreadfunction, 
		     GetDiscreteMaxDelay(),
		     GetDiscreteMaxDoppler(), 
		     GetBlockLen(), out);
}

TDSPMatrix* TDSPWssus::GetTransferFunction() {
  fTransferFunction = CreateTransferFunction(fTransferFunction);
  return fTransferFunction;
}

TDSPMatrix* TDSPWssus::GetSpreadingFunction() {
  if (!fHaveSpreadingFunction) {
    fSpreadingFunction = CreateSpreadingFunction(fSpreadingFunction);
    fHaveSpreadingFunction=kTRUE;
  }
  return fSpreadingFunction;
}

TDSPMatrix* TDSPWssus::GetScatteringFunction() {
  if (!fHaveScatteringFunction) {
    fScatteringFunction = CreateScatteringFunction(fScatteringFunction);
    fHaveScatteringFunction=kTRUE;
  }
  return fScatteringFunction;
}


void  TDSPWssus::Draw(Option_t *o) {
  TDSPMatrix *s = GetScatteringFunction()->Dup();
  s->FFTShiftMe('R');
  s->SetName("Scatteringfunction");
  s->SetTitle("The Scattering function S(#nu,#tau)");
  s->SetXTitle("#tau [#mu s]");
  s->SetYTitle("#nu [Hz]");
  s->Draw("hist surf4", GetSamplePeriodMus(), 0,
	  1, s->GetCols()*-0.5);
  delete s;
}


void TDSPWssus::Print() {
  
  
  TDSPOutput1::Print();
  cout << "Type                    : ";
  switch(fType) {
  case SCAT_TYPE_JAKES:
    cout << "Jakes" << endl;
    break;
  case SCAT_TYPE_FLAT:
    cout << "Flat" << endl;
    break;
  }
  cout << "Carrier frequency       : " << GetCarrierFrequencyGHz() << " GHz" << endl;
  cout << "Relative speed          : " << GetSpeedKMH() << " kmh" << endl;
  cout << "Sampling rate           : " << GetSamplingRateMHz() << " MHz" << endl;
  cout << "symbolrate * oversam.   : " << GetSymbolRateMHz() << " MHz * " << GetOverSampling() << endl;
  cout << "symbol-, sampleperiod   : " << GetSymbolPeriodMus() << " µs , " << GetSamplePeriodMus() << " µs" << endl;
  cout << "Maximum Doppler         : " << GetMaxDoppler() << " Hz" << endl;
  cout << "Maximum Delay Spread    : " << GetMaxDelayMus() << " µs" << endl;
  cout << "Max.Delay/Max.Dopp      : " << GetMaxDelay()/GetMaxDoppler() << " s^2" << endl;
  cout << "Exponentiell power decay: " << GetExpDecayMHz() << " (MHz)" << endl;
  cout << "discr. support Tx(2*F-1): " << GetDiscreteMaxDelay() << " x (2*" << GetDiscreteMaxDoppler()<< "-1)" << endl;
  cout << "discr. matrix size of S : " << GetDiscreteScatteringMatrixSize() << endl;
  cout << "discr. exp. decay       : " << GetDiscreteExpDecay() << endl;
  cout << "channel matrix calcul.  : ";
  if (fCalculateChannelMatrix) 
    cout << (fCalculateChannelMatrixCyclic ? "cyclic" : "non-cyclic") << endl;
  else
    cout << "none" << endl;
  Int_t b=(GetBlockLen() ? GetBlockLen() : GuessChannelBlockLen());
  cout << "channel blocklen CBL    : "  << b << endl;
  cout << "delta_t^2 * CBL         : "  << GetSamplePeriod()*GetSamplePeriod()*b << " s^2" << endl;
}


void TDSPWssus::SetOption(Option_t *option) {
  
  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  if ((pos = opt.Index("jakes"))!= kNPOS) {
    fType = SCAT_TYPE_JAKES;
    opt.Remove(pos,5);
  }
  if ((pos = opt.Index("flat"))!= kNPOS) {
    fType = SCAT_TYPE_FLAT;
    opt.Remove(pos,4);
  }
  TDSPLTVChannel::SetOption(opt);
}
