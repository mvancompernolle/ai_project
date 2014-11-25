/** TDSP *******************************************************************
                          TDSPfft.cpp  -  description
                             -------------------
    begin                : Mon Sep 10 2001
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

#include <TMath.h>
#include <Riostream.h>
#include <TSystem.h>
#include <TError.h>
#include "TDSPfft.h"
ClassImpQ(TDSPfft)

#ifdef WIN32
# include "../../config.h"
#else
# include "config.h"
#endif




#ifdef HAVE_FFTW

# ifdef WIN32
#  undef DL_FFTW
#  include <fftw.h>
# else
#  include <dfftw.h>
# endif

# ifdef DL_FFTW


typedef void          (*dl_fftw_one_function)         (fftw_plan, fftw_complex*, fftw_complex*);
typedef fftw_plan     (*dl_fftw_create_plan_function) (int,fftw_direction,int);
typedef void          (*dl_fftw_destroy_plan_function)(fftw_plan);
typedef fftw_complex* (*dl_fftw_malloc_function)      (int);
dl_fftw_one_function               dl_fftw_one;
dl_fftw_create_plan_function       dl_fftw_create_plan;
dl_fftw_destroy_plan_function      dl_fftw_destroy_plan;
dl_fftw_malloc_function            dl_fftw_malloc;
fftw_die_type_function             dl_fftw_die;

// FFTW-Hooks

fftw_die_type_function             *dl_fftw_die_hook;
fftw_malloc_type_function          *dl_fftw_malloc_hook;
fftw_free_type_function            *dl_fftw_free_hook;

# endif
#endif  


Bool_t TDSPfft::LoadFFTW() {
  
#ifdef HAVE_FFTW
  
  static Bool_t load_tried    = kFALSE;
  static Bool_t load_success  = kFALSE;

  if (!load_tried) {

    load_tried=kTRUE;
    load_success=kTRUE;
    
# ifdef DL_FFTW

    static char* fftw       = "/usr/lib/libdfftw.so";
    
   
#  define DL_ERROR { load_success = kFALSE;}
#  define DL_INFO(A) { ::Info("LoadFFTW",A);}
    
    if (gSystem->Load(fftw)==-1) {
      ::Error("LoadFFTW","fftw( %s ) not found !",fftw); 
      DL_ERROR;
    } else {
      if (!(dl_fftw_one          = 
	    (dl_fftw_one_function)gSystem->DynFindSymbol(fftw,"fftw_one"))) DL_ERROR;
      if (!(dl_fftw_create_plan  = 
	    (dl_fftw_create_plan_function)gSystem->DynFindSymbol(fftw,"fftw_create_plan"))) DL_ERROR;
      if (!(dl_fftw_destroy_plan = 
	    (dl_fftw_destroy_plan_function)gSystem->DynFindSymbol(fftw,"fftw_destroy_plan"))) DL_ERROR;
      if (!(dl_fftw_malloc       = 
	    (dl_fftw_malloc_function)gSystem->DynFindSymbol(fftw,"fftw_malloc"))) DL_ERROR;  
      if (!(dl_fftw_die          = 
	    (fftw_die_type_function)gSystem->DynFindSymbol(fftw,"fftw_die"))) DL_ERROR;

      // Loading the pointers to the hooks

      if (!(dl_fftw_malloc_hook  = 
	    (fftw_malloc_type_function*)gSystem->DynFindSymbol(fftw,"fftw_malloc_hook"))) DL_ERROR;
      if (!(dl_fftw_die_hook     = 
	    (fftw_die_type_function*)gSystem->DynFindSymbol(fftw,"fftw_die_hook"))) DL_ERROR;
      if (!(dl_fftw_free_hook    = 
	    (fftw_free_type_function*)gSystem->DynFindSymbol(fftw,"fftw_free_hook"))) DL_ERROR;
    }
   

    

    if (!load_success) {
      
      DisableFFTW();
      ::Error("LoadFFTW","fftw-library failed");
    } else {
      // Checking the hooks !
      
      if (*dl_fftw_die_hook)    DL_INFO("fftw_die_hook is used by somebody ...");
      if (*dl_fftw_malloc_hook) DL_INFO("fftw_malloc_hook is used by somebody ...");
      if (*dl_fftw_free_hook)   DL_INFO("fftw_free_hook is used by somebody ...");
      EnableFFTW();
      ::Info("LoadFFTW","fftw-library successfully loaded and enabled"); 
    }
    

# else
    // static linked ... (WIN32)

    EnableFFTW();
    
# endif

#endif  
}
  

  return load_success;
}

TDSPfft::TDSPfft(char *name) : TDSPOperator(name){

  LoadFFTW();
#ifndef HAVE_FFTW
  Error("TDSPfft","Compiled without fftw (www.fftw.org)!");
  return NULL;
#endif

  fftw_plan_forward  = NULL;
  fftw_plan_backward = NULL;
  fftw_vec = NULL;
  
  // Default Cyclic Prefix
  //
  
  fCyclicPrefix = 0;

  // Default OverSampling
  //

  fOverSampling = 1;

  // register my slots and signals
  //

  RegisterSignalsAndSlots();

  // Create the output vectors for fft
  // and ifft

  fFFT_Signal  = new TDSPSignal("output2");
  fFFT_Signal->SetTitle("the output signal of the FFT");
  fIFFT_Signal = new TDSPSignal("output1");
  fIFFT_Signal->SetTitle("the output signal of the IFFT");

}

TDSPfft::~TDSPfft(){
  
#ifdef HAVE_FFTW
  
  if (fftw_plan_forward&&_fftw_destroy_plans) {
# ifdef DL_FFTW
    if (fFFTW) dl_fftw_destroy_plan((fftw_plan)fftw_plan_forward);
# else
    fftw_destroy_plan((fftw_plan)fftw_plan_forward);
# endif
    
  }
  if (fftw_plan_backward&&_fftw_destroy_plans) {
# ifdef DL_FFTW
    if (fFFTW) dl_fftw_destroy_plan((fftw_plan)fftw_plan_forward);
# else
    fftw_destroy_plan((fftw_plan)fftw_plan_backward);
# endif
  } 

  if (fftw_vec) delete [] (fftw_complex**)(fftw_vec);

#endif
  
  if (fFFT_Signal)  delete fFFT_Signal;
  if (fIFFT_Signal) delete fIFFT_Signal;
 
}




istream &operator>>(istream &is,TDSPfft &z) {
  
  is >> z.fFFT_Signal;
  is >> z.fIFFT_Signal;
  
  return ( is );
}

ostream &operator<<(ostream &os,TDSPfft &z) {

  os << "fFFT_Signal = " << endl;
  os << z.fFFT_Signal;
  os << "fIFFT_Signal = " << endl;
  os << z.fIFFT_Signal;
  return ( os );
}

istream &operator>>(istream &is,TDSPfft *z) {
  is >> *z;
  return ( is );
}
ostream &operator<<(ostream &os,TDSPfft *z) {
  os << *z;
  return ( os );
}





/*******************************************************************/
/*  the following routines wrap fftw-routines                      */
/*******************************************************************/


#ifdef HAVE_FFTW
void fftw_make_plan(fftw_plan *p,   
		    Int_t n,
		    fftw_complex *vec[],
		    fftw_direction dir, 
		    Int_t est) {
  
  if ((!(*p))||((*p)->n!=n)) {
    // Destroy plan
    // 
    if (*p) {
# ifdef DL_FFTW
      if (TDSPfft::UsingFFTW()) dl_fftw_destroy_plan(*p);
# else
      fftw_destroy_plan(*p);
# endif
    }
# ifdef DL_FFTW
    *p = (TDSPfft::UsingFFTW()) ? dl_fftw_create_plan(n,dir,est) : NULL;
# else    
    *p = fftw_create_plan(n, dir, est);
# endif
    
    if (!*p) 
      Error("fftw_make_plan","fftw_create_plan failed (plan is NULL)");

    // alloc temps
    //
    if (*vec) delete [] (*vec);
    *vec  = new fftw_complex[n];
  }
}
#endif

// This calculates the FFT of a Signal at "v" with a length of ndat.
// It respects an given Oversampling
//
// The Routine removes an oversampled Cyclic Prefix if given (by fCyclicPrefix)

void TDSPfft::do_fft(TComplex *v,
		     TComplex *result,
		     Int_t     ndat,
		     Int_t     stepin,
		     Int_t     stepout,
		     Int_t     cp,
		     Double_t  oversampling
		     )
{

#ifdef HAVE_FFTW
  register Int_t i,j;

  // How increase the Cyclic Prefix due to oversampling
  //
  Int_t   OverSampledCyclicPrefix = oversampling>1 ? (Int_t)(cp*oversampling) : cp;

  // Remove the Cyclic Prefix
  //
  ndat -= OverSampledCyclicPrefix;
  
  // How many Samples we will left after the FFT
  //
  Int_t    ndat2 = oversampling>1 ? (Int_t)(ndat/oversampling) : ndat;

  // Planning
  // (because we make already copies - we do not need OUT_PLACE. Its
  //  faster to do this IN_PLACE)
  fftw_make_plan((fftw_plan*)(&fftw_plan_forward),
		 ndat,
		 (fftw_complex**)(&fftw_vec),
		 FFTW_FORWARD, FFTW_ESTIMATE | FFTW_IN_PLACE | FFTW_THREADSAFE);

 
  // copy the data (jump over the cyclic prefix)
  //
  if (stepin==1) {
    j=OverSampledCyclicPrefix;
    for(i=0;i<ndat;i++) { 
      ((fftw_complex*)fftw_vec)[i].re = v[j].fRe;
      ((fftw_complex*)fftw_vec)[i].im = v[j].fIm;
      j++;
    }
  } else {
    j=OverSampledCyclicPrefix*stepin;
    for(i=0;i<ndat;i++) { 
      ((fftw_complex*)fftw_vec)[i].re = v[j].fRe;
      ((fftw_complex*)fftw_vec)[i].im = v[j].fIm;
      j+=stepin;
    }
  }

#ifdef DL_FFTW
  if (fFFTW) dl_fftw_one((fftw_plan)fftw_plan_forward, 
			 (fftw_complex*)fftw_vec, NULL);
#else
  // run the fftw
  //
  fftw_one((fftw_plan)fftw_plan_forward, 
	   (fftw_complex*)fftw_vec, NULL);
#endif
  // NOT like in matlab : also the fft  gets normalizied
  //
  Double_t fndat=TMath::Sqrt((Double_t)ndat);
  for(i=0;i<ndat;++i) { 
    ((fftw_complex*)fftw_vec)[i].re /= fndat;
    ((fftw_complex*)fftw_vec)[i].im /= fndat;
  }

  // copy the data (forget the frequencies coming from the oversampling)
  //

  if (stepout==1) {
    for(i=0;i<ndat2;i++) { 
      result[i].fRe  =  ((fftw_complex*)fftw_vec)[i].re;
      result[i].fIm  =  ((fftw_complex*)fftw_vec)[i].im;
    }
  } else {
    j=0;
    for(i=0;i<ndat2;i++) { 
      result[j].fRe  =  ((fftw_complex*)fftw_vec)[i].re;
      result[j].fIm  =  ((fftw_complex*)fftw_vec)[i].im;
      j+=stepout;
    }

  }
#endif
}


void TDSPfft::do_ifft(TComplex *v,
		      TComplex *result,
		      Int_t     ndat,
		      Int_t     stepin,
		      Int_t     stepout,
		      Int_t     cp,
		      Double_t  oversampling
		      )
{

#ifdef HAVE_FFTW
  register Int_t i,j;

  // How many Samples we will need for the IFFT
  //
  Int_t    ndat2 = oversampling>1 ? (Int_t)(ndat*oversampling) : ndat;

  // How increase the Cyclic Prefix due to oversampling
  //
  Int_t    OverSampledCyclicPrefix = oversampling>1 ? (Int_t)(cp*oversampling) : cp;

  // Planning
  // (because we make already copies - we do not need OUT_PLACE. Its
  //  faster to do this IN_PLACE)
  //
  fftw_make_plan((fftw_plan*)(&fftw_plan_backward),
		 ndat2,
		 (fftw_complex**)(&fftw_vec),
		 FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_IN_PLACE | FFTW_THREADSAFE);
  
  // copy the data 
  //
  if (stepin==1) {
    for(i=0;i<ndat;++i) { 
      ((fftw_complex*)fftw_vec)[i].re = v[i].fRe;
      ((fftw_complex*)fftw_vec)[i].im = v[i].fIm;
    }
  } else {
    j=0;
    for(i=0;i<ndat;++i) { 
      ((fftw_complex*)fftw_vec)[i].re = v[j].fRe;
      ((fftw_complex*)fftw_vec)[i].im = v[j].fIm;
      j+=stepin;
    }
  }

  // Zero Padding ?
  //
  if (oversampling>1) {

    for(i=ndat;i<ndat2;++i) {
      ((fftw_complex*)fftw_vec)[i].re = 0;
      ((fftw_complex*)fftw_vec)[i].im = 0;
    }
    ndat=ndat2;
  }   

  
#ifdef DL_FFTW
  if (fFFTW)  dl_fftw_one((fftw_plan)fftw_plan_backward, 
			  (fftw_complex*)fftw_vec, NULL);
#else
  // run the fftw
  //
  fftw_one((fftw_plan)fftw_plan_backward, 
	   (fftw_complex*)fftw_vec, NULL);
#endif

  // NOT like in matlab : the ifft AND the fft gets normalizied
  //
  Double_t fndat=TMath::Sqrt((Double_t)ndat);
  for(i=0;i<ndat2;++i) { 
    ((fftw_complex*)fftw_vec)[i].re /= fndat;
    ((fftw_complex*)fftw_vec)[i].im /= fndat;
  }

  if (stepout==1) {
    // Cyclic Prefix ?
    //
    j=0;
    for(i=ndat-OverSampledCyclicPrefix;i<ndat;++i) { 
      result[j].fRe  =  ((fftw_complex*)fftw_vec)[i].re;
      result[j].fIm  =  ((fftw_complex*)fftw_vec)[i].im;
      ++j;
    }
    // DO NOT USE "j" between !!
    //
    // copy the data 
    // ( this continues on j from the for-loop before and
    //   has the value = OverSampledCyclicPrefix !!)
    // j=OverSampledCyclicPrefix;
    
    for(i=0;i<ndat;++i) { 
      result[j].fRe  =  ((fftw_complex*)fftw_vec)[i].re;
      result[j].fIm  =  ((fftw_complex*)fftw_vec)[i].im;
      ++j;
    }
  } else {
     // Cyclic Prefix ?
    //
    j=0;
    for(i=ndat-OverSampledCyclicPrefix;i<ndat;++i) { 
      result[j].fRe  =  ((fftw_complex*)fftw_vec)[i].re;
      result[j].fIm  =  ((fftw_complex*)fftw_vec)[i].im;
      j+=stepout;
    }
    // DO NOT USE "j" between !!
    //
    // copy the data 
    // ( this continues on j from the for-loop before and
    //   has the value = OverSampledCyclicPrefix !!)
    // j=OverSampledCyclicPrefix;
    
    for(i=0;i<ndat;++i) { 
      result[j].fRe  =  ((fftw_complex*)fftw_vec)[i].re;
      result[j].fIm  =  ((fftw_complex*)fftw_vec)[i].im;
      j+=stepout;
    }
  }
#endif
}


// Perform the FFT on a vector
//

void        TDSPfft::fft(TComplex   *in, TComplex   *out, Int_t n, Int_t stepin, Int_t stepout) {
  do_fft(in,out,n,stepin,stepout, fCyclicPrefix, fOverSampling);
}

void        TDSPfft::ifft(TComplex   *in, TComplex   *out, Int_t n, Int_t stepin, Int_t stepout) {
  do_ifft(in,out,n,stepin,stepout, fCyclicPrefix, fOverSampling);
}

TDSPVector* TDSPfft::fft(TDSPVector *in, TDSPVector *out) {
  
  Int_t len           = in->GetLen(); 
  Int_t olen          = GetFFTLen(len);
  
  if (!out) out = new TDSPVector();
  out->SetLen(olen);
  
  fft( in->GetVec(), out->GetVec(), len,1,1);
  
  // Set the sampling-scale
  
  out->SetSamplingRate(in->GetSamplingRate()/((Double_t)olen)*((Double_t)len));
  
  return out;
  
}

TDSPMatrix* TDSPfft::fft(TDSPMatrix *in, Int_t dim,  TDSPMatrix *out) {
  
  if (!out) out = new TDSPMatrix();

  Int_t cols           = in->GetCols(); 
  Int_t rows           = in->GetRows(); 
  
  switch(dim) {

  case 1: // Row-Wise
  case 'R':
  case 'r':
    {
      Int_t olen         = GetFFTLen(cols);
      out->SetSize(rows,olen);
#ifdef LINALG_MAT_ROWWISE
      for(Int_t row=0;row<rows;++row) 
	fft( &in->Element(row,0), &out->Element(row,0), cols,1,1);
#else
      for(Int_t row=0;row<rows;++row) 
	fft( &in->Element(row,0), &out->Element(row,0), cols,rows,rows);
#endif
      // Set the sampling-scale
      out->SetSamplingRate(in->GetSamplingRate()/((Double_t)olen)*((Double_t)cols));
    }
    break;
  case 2: // column-Wise
  case 'C':
  case 'c':
    {
      Int_t olen         = GetFFTLen(rows);
      out->SetSize(olen,cols);
#ifdef LINALG_MAT_ROWWISE
      for(Int_t col=0;col<cols;++col) 
	fft( &in->Element(0,col), &out->Element(0,col), rows,cols,cols);
#else
      for(Int_t col=0;col<cols;++col) 
	fft( &in->Element(0,col), &out->Element(0,col), rows,1,1);
#endif
      // Set the sampling-scale
      out->SetSamplingRate(in->GetSamplingRate()/((Double_t)olen)*((Double_t)cols));
    }
    break;
  default:
    Error("fft","Unknown Option !");
    break;
  }
  
  return out;
  
}



// Perform the ifft on a vector
//

TDSPVector* TDSPfft::ifft(TDSPVector *in, TDSPVector *out) { 
  

  
  Int_t len           = in->GetLen(); 
  
  Int_t olen          = GetIFFTLen(len);
  
  if (!out) out = new TDSPVector();

  out->SetLen(olen);
  
  ifft(in->GetVec(), out->GetVec(), len,1,1);
  
  // Set the sampling-scale
  
  out->SetSamplingRate(in->GetSamplingRate()*((Double_t)olen)/((Double_t)len));
    
  return out;
  
}
// Perform the IFFT on a Matrix
//

TDSPMatrix* TDSPfft::ifft(TDSPMatrix *in, Int_t dim, TDSPMatrix *out) { 
  
  
  if (!out) out = new TDSPMatrix();
  
  Int_t cols           = in->GetCols(); 
  Int_t rows           = in->GetRows(); 
  
  

  switch(dim) {

  case 1: // Row-Wise 
  case 'R':
  case 'r':
    {
      
      Int_t olen      = GetIFFTLen(cols);
      out->SetSize(rows,olen);
#ifdef LINALG_MAT_ROWWISE
      for(Int_t row=0;row<rows;++row) 
	ifft( &in->Element(row,0), &out->Element(row,0), cols,1,1);
#else
      for(Int_t row=0;row<rows;++row) 
	ifft( &in->Element(row,0), &out->Element(row,0), cols,rows,rows);
#endif
      // Set the sampling-scale
      out->SetSamplingRate(in->GetSamplingRate()/((Double_t)olen)*((Double_t)cols));
    }
    break;
  case 2: // column-Wise
  case 'C':
  case 'c':
    {
      Int_t olen      = GetIFFTLen(rows);
      out->SetSize(olen,cols);
#ifdef LINALG_MAT_ROWWISE
      for(Int_t col=0;col<cols;++col) 
	ifft( &in->Element(0,col), &out->Element(0,col), rows,cols,cols);
#else
      for(Int_t col=0;col<cols;++col) 
	ifft( &in->Element(0,col), &out->Element(0,col), rows,1,1);
#endif
      // Set the sampling-scale
      out->SetSamplingRate(in->GetSamplingRate()/((Double_t)olen)*((Double_t)cols));
    }
    break;
  default:
    Error("ifft","Unknown Option !");
    break;

  }
  
  return out;
    
}

// a slot version of the 'FFT' for use on signals
//

TDSPSignal* TDSPfft::FFT(TDSPSignal *input) {

  if (input->GetRows()==1) {

    // Do the fft on the single row (the old mode ...)    
    fft(input,fFFT_Signal);

  } else {
    // Do the fft (the new mode...)
    fft(input,fDim,fFFT_Signal);
  }

  // Send out that we are ready
  
  Emit(Signal_FFT,fFFT_Signal);
  
  return fFFT_Signal;
  
}

// a slot version of the 'IFFT' for use on signals
//

TDSPSignal* TDSPfft::IFFT(TDSPSignal *input) { 
  
  if (input->GetRows()==1) {

    // Do the ifft on the single row (the old mode ...)    
    ifft(input,fIFFT_Signal);

  } else {

    // Do the ifft (the new mode ...)    
    ifft(input,fDim,fIFFT_Signal);

  }

  // Send out that we are ready
  
  Emit(Signal_IFFT,fIFFT_Signal);
  
  return fIFFT_Signal;
  
}

TDSPfft *gFFT = NULL;

void _check_fft() {
  if (!gFFT) gFFT = new TDSPfft();
}


TDSPVector *fft(TDSPVector *in, Int_t N, TDSPVector *out) { 
  _check_fft();
  return gFFT->fft(in, out);
}


TDSPVector *ifft(TDSPVector *in, Int_t N, TDSPVector *out) { 
  _check_fft();
  return gFFT->ifft(in, out);
}


TDSPMatrix *fft(TDSPMatrix *in, Int_t dim, TDSPMatrix *out) { 
  _check_fft();
  return gFFT->fft(in, dim, out);
}


TDSPMatrix *ifft(TDSPMatrix *in, Int_t dim, TDSPMatrix *out) { 
  _check_fft();
  return gFFT->ifft(in, dim, out);
}



Bool_t TDSPfft::_fftw_destroy_plans = kTRUE;
Bool_t TDSPfft::fFFTW               = kFALSE;
