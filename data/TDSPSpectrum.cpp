/** TDSP *******************************************************************
                          TDSPSpectrum.cpp  -  description
                             -------------------
    begin                : Thu Nov 1 2001
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

#include "TDSPSpectrum.h"

ClassImp(TDSPSpectrum)

TDSPSpectrum::TDSPSpectrum(){
  Num = fAllocedNum=0;
  fAllocHighScale=2; // alloc twice the data size we really need
  fAllocLowScale=fAllocHighScale*fAllocHighScale; // at which ratio we realloc the memory
  fMinAlloc=100; 
  Frequencies = fAllocedFrequencies = NULL;
  Amplitudes  = fAllocedAmplitudes  = NULL;
}

TDSPSpectrum::~TDSPSpectrum(){
}

void TDSPSpectrum::SetNum(Int_t size) {
  
  // If the required size is more than the alloced
  // size OR
  // If the required size is significant less than the alloced
  // size 

  Int_t min_size = (size<fMinAlloc) ? fMinAlloc : size;
  
  // Do we are below or greater the cache
  //
  if (( min_size > fAllocedNum )||
      ( min_size < (Int_t)(fAllocedNum/fAllocLowScale) )) {
    
    
    // Set the size of data block to alloc
    //
    fAllocedNum =  (Int_t)(min_size*fAllocHighScale);
    
    // alloc memory
    //
    TComplex *Freq =  new TComplex[fAllocedNum];
    TComplex *Amp  =  new TComplex[fAllocedNum];
    
    Int_t     min =  Num;
    if (fAllocedNum<min) min=fAllocedNum;

    // copy the existing data
    //
    Int_t i;
    for(i=0;i<min;i++) {
      Freq[i] = Frequencies[i];
      Amp[i]  = Amplitudes[i];
    }

    // Fill up the rest with zeros ?
    //

    
    // Free alloced data blocks
    //
    if (fAllocedFrequencies) delete fAllocedFrequencies;
    if (fAllocedAmplitudes)  delete fAllocedAmplitudes;

    Frequencies = fAllocedFrequencies = Freq;
    Amplitudes  = fAllocedAmplitudes  = Amp;
  }
  
  // Simply set the num
  
  Num = size;

}
     

void TDSPSpectrum::SetFreqAmp(Int_t num, 
			      TComplex *f,
			      TComplex *a) {
  
  TComplex c;
  Int_t i,j;

  SetNum(num);

  j=0;
  for(i=0;i<num;i++) {
    c = a[i];
    if ((c.Re()!=0)||(c.Im()!=0)) {
      Amplitudes[j]  = c;
      Frequencies[j] = f[i];
      j++;
    }
  }

  Num = j;

}

// Add one Frequ.-Point

void TDSPSpectrum::AddFreqAmp(TComplex f,
			      TComplex a) {

  Int_t i;
  
  // Check that we do not have this point already 
  //
  for(i=0;i<Num;i++)
    if (Frequencies[i]==f) break;

  
  if (i==Num) {
    // Add this point
    //
    SetNum(Num+1);
    i=Num-1;
    Frequencies[i] = f;

  }
   
  Amplitudes[i] = a;

}

