/** TDSP *******************************************************************
                          MultiRate.cpp  -  description
                             -------------------
    begin                : Don Okt 24 2002
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


#include "MultiRate.h"
#include <TError.h>

#undef MULTIRATE_USE_OPTIMIZED

#ifndef MULTIRATE_USE_OPTIMIZED
# ifndef WIN32
#  warning "MultiRate.cpp (Up+DownSampling) not compiled in optimized version !!"
# endif
#endif

// Apply a M-fold decimation filter (downsampler) onto this signal
// and return the result as output. For a given output "zero_output"
// determines wether to zero it or not. If the pulse response is neglected
// a delta() is assumed
//
//      y(n) = sum_k x(k) h(nM-k+zero)
// =>   y(n) = sum_l=0^L-1 x(nM-l+zero) h(l)   L=size of h()
//

TDSPVector* DownSampling(TDSPVector *input,      // the input-signal
			 UInt_t M,               // M-fold
			 TComplex   *pulse,      // adress to the pulse vector
			 UInt_t      hlen,       // length of pulse-vector
			 Int_t      iZero,       // position of the zero
			 TDSPVector *output,     // output vector 
			 UInt_t      NumStreams,
			 UInt_t      iStream, 
			 UInt_t      opt         // options
			 ) {

  if (!input) {
    Error("DownSampling","input vector is NULL");
    return NULL;
  }

  UInt_t Num     = input->GetLen();
  UInt_t N       = (Num-hlen)/M+1;
  UInt_t olen    = N*NumStreams;
  
  if (opt&MULTIRATE_SAMPLING_RAWMODE) {  
    
    // This is a rawmode and can be used to call in other routines
    // there is no set of the sampling rate etc.

    if (!output) {
      Error("DownSampling","output vector is NULL (in rawmode !)");
      return NULL;
    }

    // The caller have to make sure that output has enough space
    // for the desired length of the vector


  } else {

    // This is the user-callable mode
    //

    if (!output) 
      output      = new TDSPVector(); 

    output->SetLen(olen);
    output->Zeros(); 
    output->SetSamplingRate(input->GetSamplingRate()/M*NumStreams); // Decrease SamplingRate
  
  }
  
  TComplex *fVec = input->GetVec();
  TComplex* ovec = output->GetVec();
  
  if ((Num%M)&&(NumStreams==1)) 
    Warning("DownSampling","input signal length (%d) is not a multiple of the downsampling rate (%d)!",
	    Num,M);
  
  
  if (pulse) {
    
#ifdef MULTIRATE_USE_OPTIMIZED
    
    UInt_t     nM = iZero;
    
    for(register   UInt_t n=iStream;n<olen;n+=NumStreams) {

      TComplex   tmp = 0;
      TComplex* ivec = fVec+nM;
      TComplex* hvec = pulse;
      UInt_t   nM1   = nM+1;
      UInt_t   hlen1 = nM1<hlen ? nM1 : hlen;

      if (nM1>Num) {
	UInt_t missing = nM1-Num;
	Warning("DownSampling","missing %d(required=%d,inputlen=%d) samples for complete decimation- and filter-operation!",
		missing,nM1,Num);
	ivec-=missing;
	hlen1-=missing;
      }

      for(register UInt_t k=0;k<hlen1;++k) 
	tmp +=ivec[-k] * hvec[k];
      ovec[n] += tmp;
      nM+=M;
    }
#else 
    
    // Direct version - yet checked !!

    
    for(register UInt_t n=0;n<N;++n) {   // loop over output
      Int_t nMiZero = n*M+iZero;
      Int_t k1 = TMath::Max(nMiZero-Int_t(hlen)+1,0);
      Int_t k2 = TMath::Min(nMiZero,       Int_t(Num)-1);      
      TComplex   tmp     =0;
      for(register Int_t k=k1;k<=k2;++k) // loop over input
	tmp += fVec[k] * pulse[nMiZero-k];
      
      ovec[iStream+NumStreams*n] += tmp;
    }
    
#endif


    
  } else 
    Error("DownSampling","Downsampling without impulse response not yet implemented - give a delta !");
  
  
  return output;
}


// Apply a K-fold interpolation filter (upsampler) onto this signal
// and return the result as output. For a given output "zero_output"
// determines wether to zero it or not. If the pulse response is neglected
// a delta() is assumed
//
//      y(n) = sum_k x(k) h(n-kK+zero)
//
// (NumStreams,iStream) can be used for automaticly serial/parallel-conversion 
// example (5,2) select the third stream of 5 parallel streams 
//

TDSPVector* UpSampling(TDSPVector *input,           // the input-signal
		       UInt_t      K, 
		       TComplex   *pulse,
		       UInt_t      hlen,
		       Int_t       iZero,
		       TDSPVector *output,
		       UInt_t      NumStreams,      // number of parallel streams
		       UInt_t      iStream,         // index of the stream to process 				   
		       UInt_t      opt              // Options
		       ) {
  if (!input) {
    Error("UpSampling","input vector is NULL");
    return NULL;
  }
  UInt_t Num     = input->GetLen(); // Total length of the input
  Int_t  N       = Num/NumStreams;  // Total length of the sub streams
  if (Num%NumStreams) 
    Warning("UpSampling","input signal length(%d) is not a multiple of the number of parallel streams (%d)!",
	    Num,NumStreams);

  UInt_t olen    = (N-1)*K+hlen;
  
  if (opt&MULTIRATE_SAMPLING_RAWMODE) {  
    
    // This is a rawmode and can be used to call in other routines
    // there is no set of the sampling rate etc.
    

  } else {

    if (!output) 
      output      = new TDSPVector();

    output->SetLen(olen);
    output->Zeros(); 
    output->SetSamplingRate(K*input->GetSamplingRate()/NumStreams); // Increase SamplingRate

  }

  TComplex* ovec = output->GetVec();

  if (pulse) {

    TComplex* fVec = input->GetVec()+iStream;

#ifdef MULTIRATE_USE_OPTIMIZED

    
    // Optimized version - not yet checked !!
    
    for(register UInt_t n=0;n<olen;++n) { // loop over output
      TComplex   tmp     =0;
      register Int_t n1=n+iZero;
      register Int_t  k=n1/K;
      while(k>=0) {
	register Int_t p  = n1-k*K; 
	if (p<0) 
	  Error("UpSampling","Error in formula!");
	if (p>=hlen) break;
	tmp+=fVec[k*NumStreams]*pulse[p];
	--k;
      }
      ovec[n] += tmp;
    }
    
#else

    // Direct version - yet checked !!

    for(register UInt_t n=0;n<olen;++n) { // loop over output
      Int_t niZero = n+iZero;
      Int_t k1 = TMath::Max(Int_t(TMath::Ceil( Double_t(niZero-Int_t(hlen)+1)/Double_t(K))),0);
      Int_t k2 = TMath::Min(Int_t(TMath::Floor(Double_t(niZero)       /Double_t(K))),N-1);      
      TComplex   tmp     =0;
      for(register Int_t k=k1;k<=k2;++k)  // loop over input
	tmp += fVec[NumStreams*k /* +iStream */] * pulse[niZero-k*K];
      
      ovec[n] += tmp;
    }

#endif
    
  } else 
    Error("UpSampling","Upsampling without impulse response not yet implemented - give a delta !");
  
  
  return output;
  
}
