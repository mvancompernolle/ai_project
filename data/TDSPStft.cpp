/** TDSP *******************************************************************
                          TDSPStft.cpp  -  description
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

#include "TDSPStft.h"

ClassImpQ(TDSPStft)

TDSPStft::TDSPStft(char *name) : TDSPfft(name) {
  fBlockLen = 0; // Auto Block Len
}
TDSPStft::~TDSPStft(){
}


#ifdef HUHU

////////////////////////////////////////////////////////////////////////
//
// Calculate the fft of an incoming signal using the Dual Window
// Function
//  
////////////////////////////////////////////////////////////////////////
  
TDSPSignal *TDSPStft::FFT(TDSPSignal *input) {
  
  Int_t len           = input->GetLen(); 
  TComplex     *ivec  = input->GetVec();
  
  // The Dual Window
  
  Int_t Ng            = fDualWindow->GetLen();
  TComplex *g         = fDualWindow->GetVec();
  
  // How many blocks to process
  
  Int_t times = len/fBlockLen;
  
  // How many memory we need
  // Alloc the Output Signal
  //
  Int_t olen          = times*Ng;
  fFFT_Signal->SetLen(olen);
  
  // Multiply with the Dual Window
  //
  mult(input,fDualWindow,fFFT_Signal);
  
  // Continue with the normal block fft
  //
  
  Int_t BlockLen =  fBlockLen ?  fBlockLen :  len;  // Auto-Blocklen
  
  
  
  // The output could be less than the input
  // due to oversampling in the time-domain
  // (fOverSampling<1 not implemented = OverSampling in frequ. domain)
  
  Int_t BlockLen1;
  if (fOverSampling>1) 
    BlockLen1 = (Int_t)(BlockLen*fOverSampling);
  else
    BlockLen1 = BlockLen;
  
  // How many blocks to process
  
  Int_t times = len/BlockLen1;
  
  // Alloc the Output Signal
  //
  Int_t olen          = times*BlockLen;
  
  // But : Reserve len NOT olen because fft-routines
  // have the lost spectrum of the last block there
  //
  fFFT_Signal->SetLen((times-1)*BlockLen+BlockLen1);
  
  TComplex     *ovec  = fFFT_Signal->GetVec();
  
  for (Int_t i=0;i<times;i++) {
    
    fft( ivec+i*BlockLen1,
	 ovec+i*BlockLen,
	 BlockLen1);
    
  }
  
  
  if (len%BlockLen1) {
    fprintf(stderr,"%s::FFT\n",ClassName());
    fprintf(stderr,"Lost %d data points due to non matching signallength to blocklength\n", len%BlockLen1);
    fprintf(stderr,"signallength = %d, Blocklength = %d\n",len,BlockLen1);
  }
  
  // Ignore part of spectrum which comes from the Oversampling of the last Block
  //
  if (fOverSampling>1) 
    fFFT_Signal->SetLen(olen);
  
  // Set the sampling-scale
  
  fFFT_Signal->SetSamplingRate(input->GetSamplingRate()/fOverSampling);
  
  // Send out that we are ready
  
  Emit(Signal_FFT,fFFT_Signal);
  
  
}
////////////////////////////////////////////////////////////////////////
  //
  // Calculate the inverse short time fourier trafo  of an incoming signal
  //
  ////////////////////////////////////////////////////////////////////////

TDSPSignal* IFFT(TDSPSignal *input) { 
  
  // The Total size of the input signal 
  //
  Int_t len           = input->GetLen(); 
  
  TComplex     *ivec  = input->GetVec();
  
  // In NOFDM that is the number of Carriers 
  //
  Int_t BlockLen =  fBlockLen ?  fBlockLen :  len;  // Auto-Blocklen
  
  ///////////////////////////////////////////
  //
  // Now perform the simple Blockfft
  //
  
  // How many blocks to process
  
  Int_t times     = len/BlockLen;
  
  // The output could be larger than the input
  // due to oversampling (fOverSampling<1 not implemented)
  //
  
  Int_t BlockLen1;
  if (fOverSampling>1) 
    BlockLen1 = (Int_t)(BlockLen*fOverSampling);
  else
    BlockLen1 = BlockLen;
  
  // Alloc the Output Signal
  //
  Int_t olen      = times*BlockLen1;
  fIFFT_Signal->SetLen(olen);
  TComplex     *ovec  = fIFFT_Signal->GetVec();
  
  
  for (Int_t i=0;i<times;i++) {
    ifft( ivec+i*BlockLen,
	  ovec+i*BlockLen1,
	  BlockLen);
    
  }
  if (len%BlockLen) {
    fprintf(stderr,"%s::IFFT\n",ClassName());
    fprintf(stderr,"Lost %d data points due to non matching signallength to blocklength\n", len%BlockLen);
    fprintf(stderr,"signallength = %d, Blocklength = %d\n",len,BlockLen);
  }
  
  // like matlab the ifft is normalizied
  normalize(ovec,olen,BlockLen1);
  
  // Set the sampling-scale
  
  fIFFT_Signal->SetSamplingRate(input->GetSamplingRate()*fOverSampling);
  
  
  //////////////////////////////////////////////////
  //
  // simple Blockfft is ready. Now convolute with the
  // the window function
  //
  
  econv(fWindow, fIFFT_Signal, fISTFT_Signal,
	fWindow->GetZero(),BlockLen/fMulti,
	kTDSPSignal_Conv_MatchB | (!kTDSPSignal_Conv_Conj));
  
  // Send out that we are ready
  
  Emit(Signal_IFFT,fISTFT_Signal);
}

#endif
