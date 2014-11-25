/** TDSP *******************************************************************
                          TDSPBlockfft.cpp  -  description
                             -------------------
    begin                : Wed Nov 14 2001
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

#include "TDSPBlockfft.h"

ClassImpQ(TDSPBlockfft)

TDSPBlockfft::TDSPBlockfft(char *name) : TDSPfft(name) {
  fBlockLen = 0; // Auto Block Len
}
TDSPBlockfft::~TDSPBlockfft(){
  cout << "hello" << flush << endl;
}

TDSPVector* TDSPBlockfft::fft(TDSPVector *input, TDSPVector *out) {

  Int_t len           = input->GetLen(); 
  
  // What should be the output blocklen ?
  //
  Int_t BlockLen      =  fBlockLen ?  fBlockLen :  len;  // Auto-Blocklen
  
  TComplex     *ivec  = input->GetVec();
  
  
  // How Large is a Block in the Input Domain ?
  // Because BlockLen/Oversampling etc. are defined in IFFT-Region
  // (final Application is here ofdm ...) we transform them
  
  Int_t BlockLen1 = GetIFFTLen(BlockLen);
  
  // How many blocks of the input signal to process
  
  Int_t times = len/BlockLen1;
  
  // Alloc the Output Signal
  //

  if (!out) out = new TDSPVector();

  Int_t olen          = times*BlockLen;
  out->SetLen(olen);
  TComplex     *ovec  = out->GetVec();
  
  for (Int_t i=0;i<times;i++)
    do_fft( ivec+i*BlockLen1,
	    ovec+i*BlockLen,
	    BlockLen1,1,1,fCyclicPrefix,fOverSampling);
  
  
  if (len%BlockLen1) {
    Error("fft","Lost %d data points due to non matching signallength to blocklength\nsignallength = %d, Blocklength = %d", 
	  len%BlockLen1,len,BlockLen1);
  }
  
  // Set the sampling-scale
  out->Configure(input);
  out->SetSamplingRate(input->GetSamplingRate()*Double_t(olen)/Double_t(len));
    
  return out;
  
}

// a slot version 
//

TDSPSignal* TDSPBlockfft::FFT(TDSPSignal *input) {

  // Do the fft
  
  fft(input,fFFT_Signal);
  
  // Send out that we are ready
  
  Emit(Signal_FFT,fFFT_Signal);
  
  return fFFT_Signal;
  
}

TDSPVector* TDSPBlockfft::ifft(TDSPVector *input, TDSPVector *out) { 
  
  Int_t len           = input->GetLen(); 
  
  // Split the input stream into Blocks of size BlockLen
  //
  
  Int_t BlockLen =  fBlockLen ?  fBlockLen :  len;  // Auto-Blocklen
  
  TComplex     *ivec  = input->GetVec();
  
  // How many blocks to process
  
  Int_t times     = len/BlockLen;
  
  // The output could be larger than the input
  // due to oversampling (fOverSampling<1 not implemented)
  // and cyclic prefix - calculate the size of the output block
  
  Int_t BlockLen1 = GetIFFTLen(BlockLen);
  
  // Alloc the Output Signal
  //
  Int_t olen      = times*BlockLen1;
  
  if (!out) out = new TDSPVector();
  out->SetLen(olen);
  TComplex     *ovec  = out->GetVec();
  
  
  for (Int_t i=0;i<times;i++) 
    do_ifft( ivec+i*BlockLen,
	     ovec+i*BlockLen1,
	     BlockLen,1,1,fCyclicPrefix,fOverSampling);
  
  if (len%BlockLen) {
    Error("ifft","Lost %d data points due to non matching signallength to blocklength\nsignallength = %d, Blocklength = %d", 
	  len%BlockLen,len,BlockLen);
  }
  
  // Set the sampling-scale
  out->Configure(input);
  out->SetSamplingRate(input->GetSamplingRate()*Double_t(olen)/Double_t(len));
    
  return out;
  
}
// a slot version 
//

TDSPSignal* TDSPBlockfft::IFFT(TDSPSignal *input) { 
  
  // Do the fft
  
  ifft(input,fIFFT_Signal);
  
  // Send out that we are ready
  
  Emit(Signal_IFFT,fIFFT_Signal);
  
  return fIFFT_Signal;
  
}

TDSPBlockfft *gBlockFFT = NULL;

void _check_blockfft() {
  if (!gBlockFFT) gBlockFFT = new TDSPBlockfft();
}

TDSPVector *blockfft(TDSPVector *in, Int_t BlockLen, TDSPVector *out) {  
  _check_blockfft();
  gBlockFFT->SetBlockLen(BlockLen);
  return gBlockFFT->fft(in, out);
}


TDSPVector *blockifft(TDSPVector *in, Int_t BlockLen, TDSPVector *out) { 
  _check_blockfft();
  gBlockFFT->SetBlockLen(BlockLen);
  return gBlockFFT->ifft(in, out);
}
