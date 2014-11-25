/** TDSP *******************************************************************
                          TDSPVector.cpp  -  description
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

#include <Riostream.h>
#include <TGraph.h>
#include <TError.h>

#include <TMatlab.h>
#include <TmxArray.h>

#include "TDSPVector.h"
#include <TDSPFifo.h>
#include "dl_atlas.h"
#include <TError.h>
#include "TEnv.h"

#ifdef DL_ATLAS
# include <TSystem.h>

dl_cblas_zgemm_function   dl_cblas_zgemm;
dl_cblas_cgemm_function   dl_cblas_cgemm;
dl_cblas_zgemv_function   dl_cblas_zgemv;
dl_cblas_cgemv_function   dl_cblas_cgemv;
dl_lapack_zgesdd_function dl_lapack_zgesdd;
dl_lapack_cgesdd_function dl_lapack_cgesdd;
dl_lapack_zgesvd_function dl_lapack_zgesvd;
dl_lapack_cgesvd_function dl_lapack_cgesvd;

#endif

//_____________________________________________________________
// TDSPVector
//
// This is base implementation of a complex signal
//_____________________________________________________________

ClassImp(TDSPVector)

TComplexBase _pi        = TMath::Pi();
TComplexBase _twopi     = 2*_pi;
TComplexBase _sqrt2     = TMath::Sqrt(2.0);
TComplexBase _inv_sqrt2 = 1/_sqrt2;

TDSPVector::TDSPVector(Int_t len){
  fZero = 0;
  Num   = fAllocedNum=0;
  fPos  = 0;

  fAllocHighScale=1;    // alloc exact the data size we really need ( these block normally are big!! )
  fAllocLowScale=4; // if the data reduced to 1/4 - realloc
  SetMinLen(100); 
  fVec=fAllocedVec=NULL;
  fSamplingRate = 1;
  fXTitle=NULL;
  fYTitle=NULL;
  if (len) SetLen(len);
}

TDSPVector::~TDSPVector(){
  Free();
}

Bool_t TDSPVector::LoadATLAS(char *name) {


  static Bool_t load_tried    = kFALSE;
  static Bool_t load_success  = kFALSE;

#ifdef DL_ATLAS
  
  if (!load_tried) {
    
    load_tried   = kTRUE;
    load_success = kTRUE;
    

#define DL_ERROR { load_success = kFALSE;}

    static char* g2c          = gEnv->GetValue("Unix.Tdsp.shlibs.g2c","/usr/lib/libf2c.so");
    static char* lapack       = gEnv->GetValue("Unix.Tdsp.shlibs.lapack","/usr/lib/liblapack.so");
    static char* blas         = gEnv->GetValue("Unix.Tdsp.shlibs.blas","/usr/lib/libblas.so");
    static char* lapack_atlas = gEnv->GetValue("Unix.Tdsp.shlibs.lapack_atlas","/usr/lib/liblapack_atlas.so");
    static char* f77blas      = gEnv->GetValue("Unix.Tdsp.shlibs.f77blas","/usr/lib/libf77blas.so");
    static char* cblas        = gEnv->GetValue("Unix.Tdsp.shlibs.cblas","/usr/lib/libcblas.so");

    
    if (gSystem->Load(g2c) == -1) {
      ::Error("LoadATLAS","Can not load %s",g2c);DL_ERROR;}
//     if (gSystem->Load(blas) == -1) {
//       ::Error("LoadATLAS","Can not load %s",blas);DL_ERROR;}
    if (gSystem->Load(lapack) == -1) {
      ::Error("LoadATLAS","Can not load %s",lapack);DL_ERROR;}
    if (gSystem->Load(lapack_atlas) == -1) {
      ::Error("LoadATLAS","Can not load %s",lapack_atlas);DL_ERROR;}
    if (gSystem->Load(f77blas) == -1) {
      ::Error("LoadATLAS","Can not load %s",f77blas);DL_ERROR;}
    if (gSystem->Load(cblas) == -1) {
      ::Error("LoadATLAS","Can not load %s",cblas);DL_ERROR;}

    
    if (!(dl_cblas_zgemm = 
	  (dl_cblas_zgemm_function)gSystem->DynFindSymbol(cblas,"cblas_zgemm"))) DL_ERROR;
    if (!(dl_cblas_cgemm = 
	  (dl_cblas_cgemm_function)gSystem->DynFindSymbol(cblas,"cblas_cgemm")))     DL_ERROR;
    if (!(dl_cblas_zgemv = 
	  (dl_cblas_zgemv_function)gSystem->DynFindSymbol(cblas,"cblas_zgemv")))     DL_ERROR;
    if (!(dl_cblas_cgemv = 
	  (dl_cblas_cgemv_function)gSystem->DynFindSymbol(cblas,"cblas_cgemv"))) DL_ERROR;
  }
  if (!load_success) {
    DisableATLAS();
    ::Error("LoadATLAS","atlas-library failed");
  } else {
    EnableATLAS();
    ::Info("LoadATLAS","atlas-library successfully loaded and enabled"); 
  }

#else

  load_tried   = kTRUE;
  ::Warning("LoadATLAS","Dynamic loading of ATLAS disabled during compile time!");
  
#endif
    
  return load_success;
  
}

void TDSPVector::Free() {
  if (fAllocedVec) delete [] fAllocedVec;
  fAllocedVec = NULL;
  fAllocedNum = 0;
}


void TDSPVector::SetMinLen(Int_t size) {
  fMinAlloc = size;
}

void TDSPVector::SetLen(Int_t size, Bool_t savedata) {
  // Do we working on extern signal memory or on
  // intern ?

  if (fVec==fAllocedVec) {

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
      fAllocedNum = (Int_t)(min_size*fAllocHighScale);
      
      
      // alloc memory
      //
      TComplex *Sig =  new TComplex[fAllocedNum];
      
      Int_t     min =  Num;
      if (fAllocedNum<min) min=fAllocedNum;

      if (savedata) {
	
	// copy the existing data
	//
	Int_t i;
	for(i=0;i<min;i++) 
	  Sig[i] = fVec[i];
	
	// Fill up the rest with zeros ?
	//
      
      }

      // Free alloced data blocks
      //
      if (fAllocedVec) delete fAllocedVec;

      fVec = fAllocedVec = Sig;
    
    }
  
  }
   
  // Simply set the num
  
  Num = size; 
}
istream &operator>>(istream &is,TDSPVector &z) {


  cout << "Enter a list of complex numbers :" << endl;
 
  z.SetLen(404);
  TComplex  *c = z.GetVec();
  Int_t i = 0;
  while(1) {
    is >> c[i];
    if (is.fail()) {is.clear();break;}
    i++;
  }
  cout << i << " values read." << endl;
  z.SetLen(i);
  return ( is );
}
ostream &operator<<(ostream &os,TDSPVector &z) {
  Int_t      i;
  Int_t      j = z.GetLen();
  TComplex  *c = z.GetVec();
    
  for(i=0;i<j;i++) {
    os << c[i];
    if ((!((i+1)%z.fPerLine))&&(i+1!=j)) os << endl;
  }
  return ( os );
}

istream &operator>>(istream &is,TDSPVector *z) {
  if (z)
    is >> *z;
  else 
    Error("operator>>","Can not input to (TDSPVector*)0x0");
  
  return ( is );
}
ostream &operator<<(ostream &os,TDSPVector *z) {
  if (z) 
    os << *z;
  else 
    os << "(TDSPVector*)0x0";
  return ( os );
}

void    TDSPVector::Print(Option_t* o) {
  cout << (*this) << endl;
}
void    TDSPVector::Input() {
  cin  >> (*this);
}

Double_t TDSPVector::Dist2(TDSPVector *s) {
  Double_t d1,d2,dist2=0;
  switch(fStrict) {
  case kStrictError:
    if (s->GetSamplingRate()!=GetSamplingRate()) 
      Warning("Dist2","Sampling rates differ (%f != %f) !",GetSamplingRate(),s->GetSamplingRate());
    if (s->Num!=Num) {
      Error("Dist2","Dimension differ (%d != %d)!",s->Num,Num);
      return 0;
    }
    break;
  case kStrictWarning:
    if (s->GetSamplingRate()!=GetSamplingRate()) 
      Warning("Dist2","Sampling rates differ (%f != %f) !",GetSamplingRate(),s->GetSamplingRate());
    if (s->Num!=Num) 
      Warning("Dist2","Dimension differ (%d != %d) - Using minimum!",s->Num,Num);
    break;
  }
  Int_t l=TMath::Min(s->Num,Num);
  for(register Int_t i=0;i<l;++i) {
    d1     = fVec[i].fRe-s->fVec[i].fRe;
    d2     = fVec[i].fIm-s->fVec[i].fIm;
    dist2 += d1*d1 + d2*d2;
  }
  
  return dist2;
}
//////////////////////////////////////////////////////////
//
// Simple Convolutions
//
// a[ 0..na(i)-1]   = channel response 
// b[ 0..nb     ]   = input signal
// c[ i = 0..nc-1 ] = Sum_(j=0..M-1) ( C(a[j])*b[i-j] )
//
// M=min[na,i]
//
// nc  = na+nb-1  
// nc  = na          ( opt==kTDSPVector_Conv_Cut )
// C() = Conjugate   ( opt==kTDSPVector_Conv_Conj ) 
// C() = 1        
// 
/////////////////////////////////////////////////////////


TDSPVector* TDSPVector::Conv(TDSPVector* si_b,  // input  signal
			     TDSPVector* si_c,  // output signal
			     Int_t       opt,
			     Int_t       blocklen) {
  
  
  Int_t     nb=si_b->GetLen();
  TComplex  *b=si_b->GetVec();

  if (!si_c) si_c = new TDSPVector();

  // Configure the Output Signal on the Base of the Input Signal
  //
  si_c->Configure(si_b);

  Int_t    nc=opt&kTDSPVector_Conv_MatchA ? Num : (opt&kTDSPVector_Conv_MatchB ? nb : Num+nb-1);
  
  si_c->SetLen(nc);
  TComplex *c = si_c->GetVec();
  
  Int_t    kk = 0;
  Int_t  num1 = Num-1;

  if (opt&kTDSPVector_Conv_Conj) {
    if (opt&kTDSPVector_Conv_UseBlockLen) {
      // Convolution with conjugated impulse response
      // with a use of blocklen
      //
      for(register Int_t k=0;k<nc;++k) {
	if (!(k%blocklen)) kk=0; // reset
	*c=0;
	register TComplex *b1 = b++;
	for(register Int_t j=0;j<=kk;++j) 
	  *c += fVec[j] % *b1--;
	++c;if (kk<num1) ++kk;
      }
    } else {
      // Convolution with conjugated impulse response
      //
      for(register Int_t k=0;k<nc;++k) {
	*c=0;
	register TComplex *b1 = b++;
	for(register Int_t j=0;j<=kk;++j) 
	  *c += fVec[j] %  *b1--;
	++c;if (kk<num1) ++kk;
      }
    }
  } else {
    if (opt&kTDSPVector_Conv_UseBlockLen) {
      // Convolution with impulse response
      // with a use of blocklen
      //
      for(register Int_t k=0;k<nc;++k) {
	if (!(k%blocklen)) kk=0; // reset
	*c=0;
	register TComplex *b1 = b++;
	for(register Int_t j=0;j<=kk;++j) 
	  *c += fVec[j] *  *b1--;
	++c;if (kk<num1) ++kk;
      }
    } else {
      // Convolution with impulse response
      //
      for(register Int_t k=0;k<nc;++k) {
	*c=0;
	register TComplex *b1 = b++;
	for(register Int_t j=0;j<=kk;++j) 
	  *c += fVec[j] * *b1--;
	++c;if (kk<num1) ++kk;
      }
    }
  }
  

  return si_c;

}

//////////////////////////////////////////////////////////
//
// More Convolutions
//
// c[ i = 0..nc-1 ] = Sum_(j=0..M-1) ( C(a[i])*b[i-(j+offset)*step] )
//
// M=min[na,i]
//
// nc  = na+nb-1    
// nc  = na          ( opt==kTDSPVector_Conv_Cut )
// C() = Conjugate   ( opt==kTDSPVector_Conv_Conj ) 
// C() = 1        
//
// (makes checks for legal subscripts...)
/////////////////////////////////////////////////////////
void econv(TDSPVector* si_a,
	   TDSPVector* si_b,
	   TDSPVector* si_c,
	   Int_t offset, Int_t step,
	   Int_t       opt) {
  
  register Int_t k,j;
  
  Int_t     na=si_a->GetLen();
  TComplex  *a=si_a->GetVec();
  Int_t     nb=si_b->GetLen();
  TComplex  *b=si_b->GetVec();

  
  // Configure the Output Signal on the Base of the Input Signal
  //
  si_c->Configure(si_a);

  Int_t    nc=opt&kTDSPVector_Conv_MatchA ? na : (opt&kTDSPVector_Conv_MatchB ? nb : na+nb-1);

  si_c->SetLen(nc);
  TComplex  *c=si_c->GetVec();

  Int_t    kk,o;

  TComplex tmp,tmp1;

  for(k=0;k<nc;k++) {
    if (k<na) kk=k; else kk=na-1;
    tmp=0;
    for(j=0;j<=kk;j++) {
      o = k-(j+offset)*step;
      if ((o<0)||(o>=nb)) continue; // simple index check ...
      tmp1=a[j];
      if (opt&kTDSPVector_Conv_Conj) tmp1.fIm*=-1;
      tmp+=tmp1*b[o];
    }
    c[k]=tmp;
  }
  
}


TDSPVector *filter(TDSPVector *in,
		   TDSPVector *out,
		   TDSPVector *fir,
		   TDSPVector *iir) {
  
  TComplex     tmp,norm;
  Int_t        len;

  // Check the Ratings of the Coefficients - they must be equal
  //
  
  Double_t     rate = fir->GetSamplingRate();
  if (iir) if (rate!=iir->GetSamplingRate()) {
    Error("filter","Sampling Rates of FIR- and IIR-Filters differ !\n");
    return NULL;
  }

  // Check the Rates of the filter and the input signal - they must be equal
  //
  if (rate!=in->GetSamplingRate()) {
    Error("filter","Sampling Rates of the Filter and the input Signals differ !\n");
    return NULL;
  }

  // Input Signal
  //
  Int_t                 iLen = in->GetLen();
  register TComplex    *ivec = in->GetVec();

  // Output Signal
  //
  if (!out) out = new TDSPVector();
  out->SetLen(iLen);
  register TComplex    *ovec = out->GetVec();
  
  // Configure the Output Signal on the Base of the Input Signal
  //
  out->Configure(in);

  // FIR part
  //
  Int_t              fir_len = fir->GetLen();
  register TComplex *fir_vec = fir->GetVec();

  // IIR part
  //
  Int_t     iir_len;
  register TComplex *iir_vec;
  
  if (iir) {
    iir_len = iir->GetLen();
    iir_vec = iir->GetVec();
    norm    = iir_vec[0];
  } else {
    iir_len = 0;
    iir_vec = NULL;
    norm    = 1; 
  } 

  register Int_t j,i;
 
  if ((norm.fRe==1.0)&&(norm.fIm==0.0)) {
    for(i=0;i<iLen;i++) {
      len=fir_len<i ? fir_len : i+1; 
      tmp=ivec[i]*fir_vec[0];
      for(j=1;j<len;j++) tmp += ivec[i-j]*fir_vec[j];
      len=iir_len<i ? iir_len : i+1; 
      for(j=1;j<len;j++) tmp -= ovec[i-j]*iir_vec[j];
      ovec[i]=tmp;
    }
  } else {
    for(i=0;i<iLen;i++) {
      len=fir_len<i ? fir_len : i+1; 
      tmp=ivec[i]*fir_vec[0];
      for(j=1;j<len;j++) tmp += ivec[i-j]*fir_vec[j];
      len=iir_len<i ? iir_len : i+1; 
      for(j=1;j<len;j++) tmp -= ovec[i-j]*iir_vec[j];
      ovec[i]=tmp/norm;
    }
  }

  return out;
}


Double_t      filternorm(TDSPVector *fir,
			 TDSPVector *iir,
			 UInt_t      p) {
  
#ifdef HAVE_MATLAB
  
  static TMatlab *_Matlab = NULL;
  static TmxArray *amx    = NULL;
  static TmxArray *bmx    = NULL;
  static TmxArray *cmx    = NULL;
  
  Info("filternorm","Not yet implemented - using matlab");
  if (!amx)         amx = new TmxArray("a");
  if (!bmx)         bmx = new TmxArray("b");
  if (!_Matlab) {
    _Matlab = new TMatlab();
  }  
  amx->Set(fir);
  _Matlab->Put(amx);
  bmx->Set(iir);
  _Matlab->Put(bmx);
  if (!_Matlab->EvalString("c = filternorm(a,b,%d);",p)) {
    Error("filternorm","maybe filter is not stable or you do not have the signal processing toolbox!");
    return 0;
  }
  if ((cmx = _Matlab->Get("c"))) 
    return (*cmx)(0).fRe;
  else {
    Error("filternorm","Error in matlabs filternorm(can not get result...)");
    return 0;
  }
#else
  Error("filternorm","Not yet implemented - need matlab !");
  return 0;
#endif
}


void     mult(TDSPVector *a,TDSPVector *b,
	      TDSPVector *c) {
  Int_t     na = a->GetLen();
  TComplex *va = a->GetVec();
  Int_t     nb = b->GetLen();
  TComplex *vb = b->GetVec();
  TComplex  *v;
  
  register Int_t n;
  
  if (na>nb) {
    v = va;
    n = na;
  } else {
    v = vb;
    n = nb;
  }
  
  c->SetLen(n);
  TComplex *vc = c->GetVec();
  register Int_t i;
  for(i=0;i<n;i++) vc[i] = v[i];
  
  if (na<nb) {
    v = va;
    n = na;
  } else {
    v = vb;
    n = nb;
  }

  for(i=0;i<n;i++) vc[i] = vc[i]*v[i];
  
}

extern TDSPVector* fft(TDSPVector*,Int_t,TDSPVector*);

TGraph* TDSPVector::Graph(Option_t *option,
			  TGraph *in,
			  Double_t dx, Double_t xoff) {
  
  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  if (!in) in = new TGraph();
  in->SetNameTitle(GetName(), GetTitle());
  in->Set(Num);
  
  Double_t *x    = in->GetX();
  Double_t *y    = in->GetY();

  if ((pos = opt.Index("complex"))!= kNPOS) {
    opt.Remove(pos,7);
    // Real part vs. Imag part
    for(register Int_t i=0;i<Num;++i) {
      x[i]  = fVec[i].fRe;
      y[i]  = fVec[i].fIm;
    }
    return in;
  } 

  if ((pos = opt.Index("real"))!= kNPOS) {
    opt.Remove(pos,4);
     for(Int_t i=0;i<Num;i++) {
      x[i]  = i*dx+xoff;
      y[i]  = fVec[i].fRe;
    }
    return in;
  }
  if ((pos = opt.Index("imag"))!= kNPOS) {
    opt.Remove(pos,4);
    
    for(Int_t i=0;i<Num;i++) {
      x[i]  = i*dx+xoff;
      y[i]  = fVec[i].fIm;
    }
    return in;
  }
  if ((pos = opt.Index("abs2"))!= kNPOS) {
    opt.Remove(pos,4);
    
    for(Int_t i=0;i<Num;i++) {
      x[i]  = i*dx+xoff;
      y[i]  = ::Abs2(fVec[i]);
    }
    return in;
  }
  if ((pos = opt.Index("abs"))!= kNPOS) {
    opt.Remove(pos,3);
    
    for(Int_t i=0;i<Num;i++) {
      x[i]  = i*dx+xoff;
      y[i]  = ::Abs(fVec[i]);
    }
    return in;
  }

  if ((pos = opt.Index("phase"))!= kNPOS) {
    opt.Remove(pos,5);
    
    for(Int_t i=0;i<Num;i++) {
      x[i]  = i*dx+xoff;
      y[i]  = ::Phase(fVec[i]);
    }
    return in;
  }
  if ((pos = opt.Index("fft2"))!= kNPOS) {
    
    opt.Remove(pos,4);
    
    TDSPVector power(Num);
    fft(this,0,&power);
    power.FFTShiftMe();
    TComplex   *f = power.GetVec();
    Double_t off1 = 0.5*dx;
    
    for(Int_t i=0;i<Num;i++) {
      x[i]  = Double_t(i)/Num*dx+xoff-off1;
      y[i]  = ::Abs2(f[i]);
    }
    return in;
  }
  
  if ((pos = opt.Index("fft"))!= kNPOS) {
    Info("Graph","Not yet implemented!");
    return NULL;
  }
  
  if (opt.Length())
    Error("Graph","Unkown option (%s)!",opt.Data());
  
  return NULL;

}


void TDSPVector::SetXTitle(char *tit) {
  if (fXTitle) delete fXTitle;
  fXTitle = new char[strlen(tit)+1];
  strcpy(fXTitle,tit);
}
void TDSPVector::SetYTitle(char *tit) {
  if (fYTitle) delete fYTitle;
  fYTitle = new char[strlen(tit)+1];
  strcpy(fYTitle,tit);
}
void TDSPVector::Draw(Option_t *option, Double_t dx, Double_t xoff) {

  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  Bool_t spec=kFALSE;

  if ((pos = opt.Index("complex"))!= kNPOS) {
    
    opt.Remove(pos,7);
    
    // Real part vs. Imag part
    TGraph *Greal = Graph("complex");
    Greal->SetTitle("");
    Greal->Draw(opt);
    Greal->GetXaxis()->SetTitle("Re");
    Greal->GetYaxis()->SetTitle("Im");
  
  } else {

    Ssiz_t posr = opt.Index("real");
    if (posr!=kNPOS) {opt.Remove(posr,4);spec=kTRUE;};

    Ssiz_t posl = opt.Index("imag");
    if (posl!=kNPOS) {opt.Remove(posl,4);spec=kTRUE;};

    Ssiz_t posa2 = opt.Index("abs2");
    if (posa2!=kNPOS) {opt.Remove(posa2,4);spec=kTRUE;};
    
    Ssiz_t posa = opt.Index("abs");
    if (posa!=kNPOS) {opt.Remove(posa,3);spec=kTRUE;};
 
    Ssiz_t posp = opt.Index("phase");
    if (posp!=kNPOS) {opt.Remove(posp,3);spec=kTRUE;};

    Ssiz_t posfft2 = opt.Index("fft2");
    if (posfft2!=kNPOS) {opt.Remove(posfft2,4);spec=kTRUE;};

    Ssiz_t posfft = opt.Index("fft");
    if (posfft!=kNPOS) {opt.Remove(posfft,3);spec=kTRUE;};

    // Defaults - if nothing was specified 
    
    if (!spec) {
      posr=1;
      posl=1;
    };

    if (posr!=kNPOS) {
      // Draw Real Part
      TGraph  *Greal = Graph("real");
      Greal->SetLineColor(2);
      Greal->Draw(opt);
      if (fXTitle) Greal->GetXaxis()->SetTitle(fXTitle);
      if (fYTitle) Greal->GetYaxis()->SetTitle(fYTitle);
      else Greal->GetYaxis()->SetTitle("Re");
      if ((pos=opt.Index("a"))!=kNPOS) opt.Replace(pos,1," ");
    }

    if (posl!=kNPOS) {
      
      // Draw Imag Part
      //
      
      TGraph  *Gimag = Graph("imag");
      Gimag->SetLineColor(4);      
      Gimag->Draw(opt);
      if (fXTitle) Gimag->GetXaxis()->SetTitle(fXTitle);
      if (fYTitle) Gimag->GetYaxis()->SetTitle(fYTitle);
      else Gimag->GetYaxis()->SetTitle("Im");
      if ((pos=opt.Index("a"))!=kNPOS) opt.Replace(pos,1," ");
    }

    if (posa!=kNPOS) {
      
      // Draw Abs
      TGraph  *GAbs = Graph("abs");
      GAbs->SetLineColor(4);
      
      GAbs->Draw(opt);
      if (fXTitle) GAbs->GetXaxis()->SetTitle(fXTitle);
      if (fYTitle) GAbs->GetYaxis()->SetTitle(fYTitle);
      else GAbs->GetYaxis()->SetTitle("Abs");
      if ((pos=opt.Index("a"))!=kNPOS) opt.Replace(pos,1," ");
    }

    if (posa2!=kNPOS) {
      
      // Draw Abs
      TGraph  *GAbs2 = Graph("abs2");
      GAbs2->SetLineColor(4);
      GAbs2->Draw(opt);
      if (fXTitle) GAbs2->GetXaxis()->SetTitle(fXTitle);
      if (fYTitle) GAbs2->GetYaxis()->SetTitle(fYTitle);
      else GAbs2->GetYaxis()->SetTitle("Abs2");
      if ((pos=opt.Index("a"))!=kNPOS) opt.Replace(pos,1," ");
    }

    if (posp!=kNPOS) {
      
      // Draw Phase
      TGraph  *GPhase = Graph("phase");
      GPhase->SetLineColor(3);
      GPhase->Draw(opt);
      if (fXTitle) GPhase->GetXaxis()->SetTitle(fXTitle);
      if (fYTitle) GPhase->GetYaxis()->SetTitle(fYTitle);
      else GPhase->GetYaxis()->SetTitle("#phi");
      if ((pos=opt.Index("a"))!=kNPOS) opt.Replace(pos,1," ");
    }

    if (posfft2!=kNPOS) {
      
      // Draw the fft2
      TGraph  *Gfft2 = Graph("fft2");
      Gfft2->SetLineColor(3);
      Gfft2->Draw(opt);
      if (fXTitle) Gfft2->GetXaxis()->SetTitle(fXTitle);
      if (fYTitle) Gfft2->GetYaxis()->SetTitle(fYTitle);
      else Gfft2->GetYaxis()->SetTitle("fft2()");
      if ((pos=opt.Index("a"))!=kNPOS) opt.Replace(pos,1," ");
    }

    if (posfft!=kNPOS) {
      
      // Draw the fft
      TGraph  *Gfft = Graph("fft");
      Gfft->SetLineColor(3);
      Gfft->Draw(opt);
      if (fXTitle) Gfft->GetXaxis()->SetTitle(fXTitle);
      if (fYTitle) Gfft->GetYaxis()->SetTitle(fYTitle);
      else Gfft->GetYaxis()->SetTitle("fft()");
      if ((pos=opt.Index("a"))!=kNPOS) opt.Replace(pos,1," ");
    }
  }
}

TDSPVector*  TDSPVector::XCorr(TDSPVector *i2,                      // Crosscorrelation
			       TDSPVector *result, 
			       Int_t max_lags,
			       eXCorr_Options o) {
  
  TComplex tmp,tmp1;
  Int_t Num1 = i2->GetLen();

  Int_t Nmax    = Num1;

  if (Num>Num1) {Nmax = Num;};

  Int_t max_support  = Nmax-1;
  if (!max_lags) max_lags=max_support;
  else 
    if (max_support>TMath::Abs(max_lags)) max_support=TMath::Abs(max_lags);

  if (!result) result = new TDSPVector();
  
  if (max_lags>0) result->SetLen(max_lags*2+1);
  else result->SetLen(1-max_lags);

  TComplex *ivec   = i2->GetVec(); 
  TComplex *ovec   = result->GetVec();

  if (max_lags>0) {

    register Int_t m = max_lags;
    
    // Fill the the first part with zeros
    //
    for(;m>max_support;--m) 
      *(ovec++)=0;
    // Calculate only the first half
    //
    for(;m;--m) {
      
      
      Int_t N1 = Num1-m;
      if (N1>Num) N1=Num;
      
      tmp   = fVec[0];
      tmp.C();
      tmp   *= ivec[m];
      for(register Int_t n=1; n<N1;++n) { 
	tmp1 = fVec[n];
	tmp1.C();
	tmp += ivec[n+m]*tmp1;
      }
      
      *(ovec++) = tmp;
      
    }
  } else max_lags=-max_lags;

  // Calculate only the second half
  //
  register Int_t m=0;
  for(;m<=max_support;++m) {

    Int_t N1 = Num-m;
    if (N1>Num1) N1=Num1;

    tmp   = ivec[0];
    tmp.C();
    tmp   *= fVec[m];
    for(register Int_t n=1; n<N1;++n) { 
      tmp1 = ivec[n];
      tmp1.C();
      tmp += fVec[n+m]*tmp1;
    }
    
    *(ovec++) = tmp;

  }

  // Fill the the first part with zeros
  //
  for(;m<=max_lags;++m)
    *(ovec++)=0;

  // Normalisation
  //
  switch(o) {
  case kXCorrNone:
    break;
  case kXCorrCoeff:
    (*result)/=Nmax;
    break;
  case kXCorrBiased:
    Error("XCorr","Not yet implemented!");
    break;
  case kXCorrUnBiased:
    Error("XCorr","Not yet implemented!");
    break;
  }
  return result;

}


TH1* TDSPVector::FillAbs(TH1*h) {

  if (!h) {
    if (fHistogram) delete fHistogram;
    fHistogram = h = new TH1F("htemp","",100,MinAbs(),MaxAbs());
    //    h->SetBit(TH1::kCanRebin);
  }

  for(register Int_t i=0;i<Num;i++) 
    h->Fill(::Abs(fVec[i]));
  
  return h;
}

TH1* TDSPVector::FillAbs2(TH1*h) {

  if (!h) {
    if (fHistogram) delete fHistogram;
    fHistogram = h = new TH1F("htemp","",100,MinAbs2(),MaxAbs2());
    //    h->SetBit(TH1::kCanRebin);
  }

  for(register Int_t i=0;i<Num;i++) 
    h->Fill(::Abs2(fVec[i]));
  
  return h;
}


TH1* TDSPVector::FillPhase(TH1*h) {

  if (!h) {
    if (fHistogram) delete fHistogram;
    fHistogram = h = new TH1F("htemp","",100,-1,1);
    h->SetBit(TH1::kCanRebin);
  }

  for(register Int_t i=0;i<Num;i++) 
    h->Fill(fVec[i].Phase());
  
  return h;
}


TH1* TDSPVector::FillReal(TH1*h) {

  if (!h) {
    if (fHistogram) delete fHistogram;
    fHistogram = h = new TH1F("htemp","",100,Min(),Max());
    h->SetBit(TH1::kCanRebin);
  }

  for(register Int_t i=0;i<Num;i++) 
    h->Fill(fVec[i].fRe);
  
  return h;
}

TH1* TDSPVector::FillImag(TH1*h) {

  if (!h) {
    if (fHistogram) delete fHistogram;
    fHistogram = h = new TH1F("htemp","",100,MinImag(),MaxImag());
    h->SetBit(TH1::kCanRebin);
  }

  for(register Int_t i=0;i<Num;i++) 
    h->Fill(fVec[i].fIm);
  
  return h;
}

TH2* TDSPVector::Fill(TH2*h) {

  if (!h) {
    if (fHistogram) delete fHistogram;
    fHistogram = h = new TH2F("htemp","",100,Min(),Max(),100,MinImag(),MaxImag());
    h->SetBit(TH1::kCanRebin);
  }

  for(register Int_t i=0;i<Num;++i) 
    h->Fill(fVec[i].fRe,fVec[i].fIm);
  
  return h;
}


#ifdef HAVE_MATLAB
# include "../Matlab/TmxArray.h"
TDSPVector& TDSPVector::operator=(TmxArray& m) { 
  m.Get(this); 
  return (*this);
}

#endif

void TDSPVector::DumpParams(ostream *os) {
  
  if (!os) os = &cout;

  *os << "class(TDSPVector*)" << (void*)this << endl;
  *os << " Signal length = " << Num << " Sampling rate = " << GetSamplingRate() << endl;
  *os << " Signal values = ";
  for(UInt_t i=0;i<fPerLine;i++) if (i<UInt_t(Num)) *os << fVec[i]; else break;
  if (fPerLine<Num) *os << " ...";
  *os << endl;
}


/*****************************************************************/


void TDSPVector::RandNC(Double_t sigma) {
  sigma*=_inv_sqrt2;
  for(register Int_t i=0;i<Num;++i) {
    fVec[i].fRe = gRandom->Gaus(0,sigma);
    fVec[i].fIm = gRandom->Gaus(0,sigma);
  }
}

void TDSPVector::RandNC(TComplex& mean, Double_t sigma) {
  sigma*=_inv_sqrt2;
  for(register Int_t i=0;i<Num;++i) {
    fVec[i].fRe = gRandom->Gaus(mean.fRe,sigma);
    fVec[i].fIm = gRandom->Gaus(mean.fIm,sigma);
  }
}

void TDSPVector::RandNC(TComplex *means, Double_t sigma) {
  sigma*=_inv_sqrt2;
  for(register Int_t i=0;i<Num;++i) {
    fVec[i].fRe = gRandom->Gaus(means[i].fRe,sigma);
    fVec[i].fIm = gRandom->Gaus(means[i].fIm,sigma);
  }
}


void TDSPVector::RandNC(TComplex *means, TComplex *sigmas) {
  if (!sigmas) RandNC(means);
  if (means) {
    for(register Int_t i=0;i<Num;++i) {
      fVec[i].fRe = gRandom->Gaus(means[i].fRe,sigmas[i].fRe*_inv_sqrt2);
      fVec[i].fIm = gRandom->Gaus(means[i].fIm,sigmas[i].fRe*_inv_sqrt2);
    }
  } else {
    for(register Int_t i=0;i<Num;++i) {
      fVec[i].fRe = gRandom->Gaus(0,sigmas[i].fRe*_inv_sqrt2);
      fVec[i].fIm = gRandom->Gaus(0,sigmas[i].fRe*_inv_sqrt2);
    }
  }

}

void TDSPVector::RandNC(TDSPVector *means, Double_t sigma) {
  if (means)
    SetLen(means->GetLen(),kFALSE);
  RandNC(means->GetVec(),sigma);
}
void TDSPVector::RandNC(TDSPVector* means, TDSPVector* sigmas) {
  TComplex* Means = NULL;
  TComplex* Sigmas = NULL;

  if (means)  Means  = means->GetVec();
  if (sigmas) Sigmas = sigmas->GetVec();

  if (means) {
    SetLen(means->GetLen(),kFALSE);
    if (sigmas)
      if (sigmas->GetLen()!=means->GetLen()) {
	Error("RandNC","Length of means(%d) and sigmas(%d) differ !",
	      means->GetLen(), sigmas->GetLen());
	return;
      }
  } else {
    if (sigmas) 
      SetLen(sigmas->GetLen(),kFALSE);
    else {
      Error("RandNC","Dont know means either sigmas !");
      return;
    }
  }
  RandNC(Means,Sigmas);
}

// Create a vector of uniform distributed phasors 
//
//
void  TDSPVector::RandPhase() {

  Double_t phase;
  for(register Int_t i=0;i<Num;++i) {    
    phase  = gRandom->Rndm()*_twopi;
    fVec[i]  = TComplex(TMath::Cos(phase),TMath::Sin(phase));   
  }
}


// Create a vector of uniform distributed phasors 
//
//
void  TDSPVector::MultRandPhase(TDSPVector *in) {
  if (!in) in=this;
  if (Num!=in->GetLen()) SetLen(in->GetLen(),kFALSE);

  Double_t phase;
  for(register Int_t i=0;i<Num;++i) {    
    phase  = gRandom->Rndm()*_twopi;
    fVec[i]  = in->fVec[i];
    fVec[i] *= TComplex(TMath::Cos(phase),TMath::Sin(phase));   
  }
}




// Create a vector of normal distributed phasors with 
// Phase variance 2*Pi*variance
//
//
void  TDSPVector::RandNPhase(Double_t variance, Double_t rate) {

  Double_t  phase    = 0;// initial phase
  Double_t      d    = 1;
  Double_t   freq    = 0;

  Double_t sigma=TMath::Sqrt(_twopi*variance/GetSamplingRate()); // randn(a*s)=a*randn(s)

  for(register Int_t i=0;i<Num;++i) {    
    if (d>=1) {freq  = gRandom->Gaus(0,sigma);d-=1;} 
    d+=rate;
    phase    = freq*i;
    fVec[i]  = TComplex(TMath::Cos(phase),TMath::Sin(phase));   
  }
}
// Multiply the vector by normal distributed phasors with 
// Phase variance 2*Pi*variance
//
//
void  TDSPVector::MultRandNPhase(Double_t variance, Double_t rate) {
  Double_t  phase    = 0;// initial phase
  Double_t      d    = 1;
  Double_t   freq    = 0;
  Double_t     sigma = TMath::Sqrt(variance*_twopi/GetSamplingRate()); // randn(a*s)=a*randn(s)
  for(register Int_t i=0;i<Num;++i) { 
    if (d>=1) {freq  = gRandom->Gaus(0,sigma);d-=1;}
    d+=rate;
    phase    = freq*i;
    fVec[i] *= TComplex(TMath::Cos(phase),TMath::Sin(phase));   
  }
}
// Multiply a vector "in" by normal distributed phasors with 
// Phase variance 2*Pi*variance
//
//
void  TDSPVector::MultRandNPhase(Double_t variance, Double_t rate, TDSPVector *in) {

  if (!in) {
    Error("RandPhase","no input vector given !");
    return;
  } 
  
  TComplex *fin = in->GetVec();
  
  if (!fin) {
    Error("RandPhase","input vector not allocated !");
    return;
  }

  SetLen(in->GetLen());
  SetSamplingRate(in->GetSamplingRate());

  Double_t  phase    = 0;// initial phase
  Double_t      d    = 1;
  Double_t   freq    = 0;
  Double_t     sigma = TMath::Sqrt(variance*_twopi/GetSamplingRate()); // randn(a*s)=a*randn(s)
  TComplex  cphase;
  for(register Int_t i=0;i<Num;++i) {
    if (d>=1) {freq  = gRandom->Gaus(0,sigma);d-=1;}
    d+=rate;
    phase   = freq*i;
    cphase  = TComplex(TMath::Cos(phase),TMath::Sin(phase));
    fVec[i] = fin[i] * cphase;
  }
}

void TDSPVector::FFTShiftMe() {
  
#ifdef _0
  Int_t l  = Num/2;

  
  if (Num%2) {
 
#ifndef WIN32
# warning "TDSPVector::FFTShift for odd vector len not optimized!"
#endif

    // Odd vector len - not optimized !!
    //
    TDSPVector *_tmp = Dup();
    TComplex   *_vec = _tmp->GetVec();
    
    for(register Int_t i=0;i<=l;++i) 
      fVec[i+l] = _vec[i];
    
    for(register Int_t j=0;j<l;++j)
      fVec[j]   = _vec[j+l+1];
    
    delete _tmp;

  }   else  
    // Even vector len
    for(register Int_t i=0;i<l;++i) 
      Swap(i,i+l);
#else 
  Int_t l  = (Num/2)-Num;
  if (Num%2) l++;
  
  TDSPVector *_tmp = Dup();
  TComplex   *_vec = _tmp->GetVec();
  for(register Int_t i=0;i<Num;++i) 
    fVec[(i-l)%Num] = _vec[i];
  delete _tmp;
  
#endif
}

void TDSPVector::IFFTShiftMe() {
  
  
#ifdef _0
  Int_t l  = Num/2;

  if (Num%2) {
 
#ifndef WIN32
# warning "TDSPVector::IFFTShift for odd vector len not optimized!"
#endif
    
    // Odd vector len - not optimized !!
    //
    TDSPVector *_tmp = Dup();
    TComplex   *_vec = _tmp->GetVec();
    
    for(register Int_t i=0;i<=l;++i) 
      fVec[i+l] = _vec[i];
    
    for(register Int_t j=0;j<l;++j)
      fVec[j]   = _vec[j+l+1];
    
    delete _tmp;

  }   else  
    // Even vector len
    for(register Int_t i=0;i<l;++i) 
      Swap(i,i+l);
#else

  Int_t l  = Num/2;
  if (Num%2) l++;
  TDSPVector *_tmp = Dup();
  TComplex   *_vec = _tmp->GetVec();
  for(register Int_t i=0;i<Num;++i) 
    fVec[(i+l)%Num] = _vec[i];
  delete _tmp;
#endif
}

void TDSPVector::Append(TDSPVector *a) {
  Int_t al  = a->GetLen();
  Int_t num = Num;

  SetLen(Num+al,kTRUE);
  
  TComplex *f  = fVec+num;
  TComplex *fa = a->GetVec();
  
  for(register Int_t i=0;i<al;++i) 
    *f++ = *fa++;
}

void TDSPVector::ModulateMe(TComplex  freq, Double_t burstlen,Double_t burstoff,
			    Double_t phase) {
  TComplex *fV   = fVec;

  Double_t dd;
  Int_t    iBurstlen,iBurstoff;

  // Adjust the burstlen for the Samplingrate
  //

  if (!burstlen) {
    burstlen  = Double_t(Num)/fSamplingRate;
    iBurstlen = Num;
  } else {
    dd        = Double_t(burstlen)*fSamplingRate;
    iBurstlen = TMath::Nint(dd);
    if (TMath::Abs(iBurstlen-dd)>1.0e-4) 
      Warning("ModulateMe","Burstlen %d * Samplingrate %f = %f is not an integer (using %d)!", 
	      burstlen,fSamplingRate,dd,iBurstlen);
  }

  // Adjust the burst offset for the Samplingrate
  //

  dd          = Double_t(burstoff)*fSamplingRate;
  iBurstoff   = TMath::Nint(dd);
  if (TMath::Abs(iBurstoff-dd)>1.0e-4) 
    Warning("ModulateMe","Burstoffset %d * Samplingrate %f = %f is not an integer (using %d)!", 
	    burstoff,fSamplingRate,dd,iBurstoff);
  
  
  // Adjust the frequency for the Samplingrate
  //
  
  TComplex   f = freq*(burstlen-burstoff) + phase;
  f=::Exp(I*f);
  freq         = ::Exp(I*freq/fSamplingRate);
  
  

  Int_t j = -iBurstoff;
  for(register Int_t i=0;i<Num;++i) {
    if (!(j++%iBurstlen)) f=::Exp(I*phase); // Reset f to the exp(i*phase) 
    *fV++ *= f;
    f*=freq;
  }
}

TDSPVector* TDSPVector::Modulate(TComplex  freq, 
				 Double_t burstlen,
				 Double_t burstoff,
				 Double_t phase,
				 TDSPVector *out) {
  
  if (!out) out = new TDSPVector();
  else if (out==this) {
    ModulateMe(freq, burstlen,burstoff,phase);
    return this;
  }
  
  Double_t dd;
  Int_t    iBurstlen,iBurstoff;
  
    

  // Adjust the burstlen for the Samplingrate
  //

  if (!burstlen) {
    burstlen  = Double_t(Num)/fSamplingRate;
    iBurstlen = Num;
  } else {
    dd        = Double_t(burstlen)*fSamplingRate;
    iBurstlen = TMath::Nint(dd);
    if (TMath::Abs(iBurstlen-dd)>1.0e-4)
      Warning("Modulate","Burstlen %d * Samplingrate %f = %f is not an integer (using %d)!", 
	      burstlen,fSamplingRate,dd,iBurstlen);
  }

  // Adjust the burst offset for the Samplingrate
  //
  
  dd         = Double_t(burstoff)*fSamplingRate;
  iBurstoff  = TMath::Nint(dd);
  if (TMath::Abs(iBurstoff-dd)>1.0e-4) 
    Warning("Modulate","Burstoffset %d * Samplingrate %f = %f is not an integer (using %d)!", 
	    burstoff,fSamplingRate,dd,iBurstoff);

  // Adjust the frequency for the Samplingrate
  //

  TComplex   f   = freq*(burstlen-burstoff)+phase;   // the start value
  f=::Exp(I*f);
  freq           = ::Exp(I*freq/fSamplingRate);

  out->SetLen(Num);
  TComplex *oV = out->GetVec();
  TComplex *fV = fVec;

  Int_t j = -iBurstoff;
  for(register Int_t i=0;i<Num;++i) {
    if (!(j++%iBurstlen)) f = ::Exp(I*phase);
    *oV++ = *fV++ * f;
    f*=freq;
  }

  out->Configure(this);

  return out;
  
}


//______________________________________________________________________________
void TDSPVector::Streamer(TBuffer &R__b)
{
  // Stream an object of class TDSPVector.
  
  UInt_t R__s, R__c;
  if (R__b.IsReading()) {
    Int_t num;
    Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
    TObject::Streamer(R__b);
    R__b >> fMinAlloc;
    R__b >> fAllocHighScale;
    R__b >> fAllocLowScale;
    R__b >> num;
    SetLen(num);
    R__b >> fPos;
    //R__b >> fAllocedNum;
    R__b >> fSamplingRate;
    R__b >> fZero;
    for(Int_t i=0;i<Num;++i) fVec[i].Streamer(R__b);
    
    R__b.CheckByteCount(R__s, R__c, TDSPVector::IsA());
  } else {
    if (fVec) {
      R__c = R__b.WriteVersion(TDSPVector::IsA(), kTRUE);
      TObject::Streamer(R__b);
      R__b << fMinAlloc;
      R__b << fAllocHighScale;
      R__b << fAllocLowScale;
      R__b << Num;
      R__b << fPos;
      //R__b << fAllocedNum;
      R__b << fSamplingRate;
      R__b << fZero;
      for(Int_t i=0;i<Num;++i) fVec[i].Streamer(R__b);
      R__b.SetByteCount(R__c, kTRUE);
    } else 
      Error("Streamer","Streaming of unalloced vectors not yet implemented");
  }
}


TDSPVector* TDSPVector::DivBy(TDSPVector *a, TDSPVector *result) {
  if (!result) result = new TDSPVector();
  Int_t l = TMath::Min(Num,a->GetLen());

  if (a->GetLen()!=Num)
    Warning("DivBy","Vector length differ (%d!=%d) using %d",Num,a->GetLen(),l);

  result->SetLen(l);
  TComplex* f1 = result->GetVec();
  TComplex* f2 = a->GetVec();
  
  for(register Int_t i=0;i<l;++i) 
    f1[i] = fVec[i] / f2[i];
  
  return result;
}

TDSPVector* TDSPVector::MultBy(TDSPVector *a, TDSPVector *result) {
  if (!result) result = new TDSPVector();
  Int_t l = TMath::Min(Num,a->GetLen());

  if (a->GetLen()!=Num)
    Warning("MultBy","Vector length differ (%d!=%d) using %d",Num,a->GetLen(),l);

  result->SetLen(l);
  TComplex *f1 = result->GetVec();
  TComplex *f2 = a->GetVec();
  
  for(register Int_t i=0;i<l;++i) 
    f1[i] = fVec[i] * f2[i];

  return result;

}


TDSPVector* TDSPVector::AddBy(TDSPVector *a, TDSPVector *result) {
  if (!result) result = new TDSPVector();
  Int_t l = TMath::Min(Num,a->GetLen());

  if (a->GetLen()!=Num)
    Warning("AddBy","Vector length differ (%d!=%d) using %d",Num,a->GetLen(),l);

  result->SetLen(l);
  TComplex *f1 = result->GetVec();
  TComplex *f2 = a->GetVec();
  
  for(register Int_t i=0;i<l;++i) 
    f1[i] = fVec[i] + f2[i];

  return result;

}

TDSPVector* TDSPVector::SubBy(TDSPVector *a, TDSPVector *result) {
  if (!result) result = new TDSPVector();
  Int_t l = TMath::Min(Num,a->GetLen());

  if (a->GetLen()!=Num)
    Warning("SubBy","Vector length differ (%d!=%d) using %d",Num,a->GetLen(),l);

  result->SetLen(l);
  TComplex *f1 = result->GetVec();
  TComplex *f2 = a->GetVec();
  
  for(register Int_t i=0;i<l;++i) 
    f1[i] = fVec[i] - f2[i];

  return result;

}

TDSPVector* TDSPVector::MultBy(TComplex a, TDSPVector *result) {
  if (!result) result = new TDSPVector();
  result->SetLen(Num);
  TComplex *f1 = result->GetVec();
  
  for(register Int_t i=0;i<Num;++i) 
    f1[i] = fVec[i] * a;
  return result;

}
TDSPVector* TDSPVector::DivBy(TComplex a, TDSPVector *result) {
  if (!result) result = new TDSPVector();
  result->SetLen(Num);
  TComplex *f1 = result->GetVec();
  
  for(register Int_t i=0;i<Num;++i) 
    f1[i] = fVec[i] / a;
  return result;

}
TDSPVector* TDSPVector::AddBy(TComplex a, TDSPVector *result) {
  if (!result) result = new TDSPVector();
  result->SetLen(Num);
  TComplex *f1 = result->GetVec();
  
  for(register Int_t i=0;i<Num;++i) 
    f1[i] = fVec[i] + a;
  return result;

}
TDSPVector* TDSPVector::SubBy(TComplex a, TDSPVector *result) {
  if (!result) result = new TDSPVector();
  result->SetLen(Num);
  TComplex *f1 = result->GetVec();
  
  for(register Int_t i=0;i<Num;++i) 
    f1[i] = fVec[i] - a;
  return result;

}


TDSPVector* TDSPVector::GetPhase(TDSPVector *result) {
  if (!result) result = new TDSPVector();
  result->SetLen(Num);
  TComplex *f1 = result->GetVec();
  
  for(register Int_t i=0;i<Num;++i) 
    *f1++ = fVec[i].Phase();

  return result;

}



#include <MultiRate.h>

TDSPVector* TDSPVector::DownSampling(UInt_t M, 
				     TDSPVector *pulse,
				     TDSPVector *output) {
  if (pulse) {
    if (pulse->GetSamplingRate()!=1.)
      Warning("DownSampling","impulse response sampling rate(%f) ignored !",pulse->GetSamplingRate());
    return ::DownSampling(this,M, pulse->GetVec(), pulse->GetLen(),pulse->GetZero(),
			output);
  } else
    return ::DownSampling(this,M,NULL,0,0,output);
}

TDSPVector* TDSPVector::UpSampling(UInt_t M, 
				   TDSPVector *pulse,
				   TDSPVector *output,
				   UInt_t iStep,
				   UInt_t iOff) {
  if (pulse) {
    if (pulse->GetSamplingRate()!=1.)
      Warning("UpSampling","impulse response sampling rate(%f) ignored !",pulse->GetSamplingRate());
    return ::UpSampling(this,M, pulse->GetVec(), pulse->GetLen(),pulse->GetZero(),
			output, iStep, iOff);
  } else
    return ::UpSampling(this,M,NULL,0,0,output,iStep, iOff);
}

void        TDSPVector::Save(TFile *f, char *key) {
  TDirectory *d = gDirectory;
  f->cd();
  Write(key);
  if ((gDirectory)&&(gDirectory!=f)) gDirectory->cd();
}

void        TDSPVector::Save(char *fname, char *key, char *opt) {
  TFile f(fname,opt);
  Save(&f, key);
  f.Close();  
}

TDSPVector* TDSPVector::Load(TFile *f, char *key) {
  return static_cast<TDSPVector*>(f->Get(key));
}

TDSPVector* TDSPVector::Load(char *fname, char *key) {
  TFile f(fname);
  TDSPVector *v = Load(&f,key);
  f.Close();
  return v;
}

TComplex TDSPVector::Prod() {
  TComplex p=1.;
  for(register Int_t i=0;i<Num;++i) {
    p*=fVec[i];
  }
  return p;
}

Double_t TDSPVector::ProdReal() {
  Double_t p=1.;
  for(register Int_t i=0;i<Num;++i) {
    p*=fVec[i].fRe;
  }
  return p;
}
Double_t TDSPVector::ProdImag() {
  Double_t p=1.;
  for(register Int_t i=0;i<Num;++i) {
    p*=fVec[i].fIm;
  }
  return p;
}

TComplex TDSPVector::Sum(TDSPVector *weights) {
  TComplex mean=0;
  
  if (weights) {
    // weighted Sum
    if (weights->GetLen()!=Num) {
      Error("Sum","Length of the weights vector (%d) differs from vector length (%d)",
	    weights->GetLen(),Num);
      return 0;
    }
    // additional checks here ignored: weights should be real and positiv
    //
    for(register Int_t i=0;i<Num;++i) {
      mean.fRe += fVec[i].fRe*weights->fVec[i].fRe;
      mean.fIm += fVec[i].fIm*weights->fVec[i].fRe;
    }
  } else {
    
    // unweighted Sum
    for(register Int_t i=0;i<Num;++i) mean += fVec[i];
  }
  return mean;
}



Double_t TDSPVector::Sum2(TDSPVector *weights) {
  
  Double_t norm = 0;
  
  if (weights) {
    // weighted Norm2
    if (weights->GetLen()!=Num) {
      Error("Norm2","Length of the weights vector (%d) differs from vector length (%d)",
	    weights->GetLen(),Num);
      return 0;
    }
    // additional checks here ignored: weights should be real and positiv
    //
    for(register Int_t i=0;i<Num;++i) 
      norm += (fVec[i].fRe*fVec[i].fRe+fVec[i].fIm*fVec[i].fIm)*weights->fVec[i].fRe;
    
  } else {
    // Unweighted Norm2
    for(register Int_t i=0;i<Num;++i) 
      norm += fVec[i].fRe*fVec[i].fRe+fVec[i].fIm*fVec[i].fIm;
  }

  return norm;
}



Double_t    TDSPVector::Variance(TDSPVector *weights) {

  if (weights) {
    // weighted Variance
    //return Mean2(weights)-::Abs2(Mean(weights));  
    Double_t n=weights->Norm2();  
    return Sum2(weights)/n - ::Abs2(Sum(weights)/n);    
  } else
    // Unweighted Variance
    return Mean2()-::Abs2(Mean());  
}

// Several Moments
//

Double_t    TDSPVector::Moment1(TDSPVector *x) {

  Double_t n2 = Norm2();  
  
  Double_t mean1=0; // Mean1 value

  if (!x) {
    for(register Int_t i=0;i<Num;++i) 
      mean1 += i * ::Abs2(fVec[i]);
    
  } else {
    
    if (x->GetLen()!=Num) {
      Error("Moment1","Length of the x vector (%d) differs from vector length (%d)",
	    x->GetLen(),Num);
      return 0;
    }
    for(register Int_t i=0;i<Num;++i) 
      mean1 += ::Abs(x->fVec[i]) * ::Abs2(fVec[i]);
  }
  mean1/=n2;
  return mean1;
} 

Double_t    TDSPVector::Moment2(TDSPVector *x) {

  Double_t n2 = Norm2();  
  
  Double_t mean2=0; // Mean2 value

  if (!x) {
    for(register Int_t i=0;i<Num;++i) 
      mean2 += i*i * ::Abs2(fVec[i]);
    
  } else {
    
    if (x->GetLen()!=Num) {
      Error("Moment2","Length of the x vector (%d) differs from vector length (%d)",
	    x->GetLen(),Num);
      return 0;
    }
    for(register Int_t i=0;i<Num;++i) 
      mean2 += ::Abs2(x->fVec[i]) * ::Abs2(fVec[i]);
  }
  mean2/=n2;
  return mean2;
} 

Double_t    TDSPVector::CMoment2(TDSPVector *x) {

  Double_t n2 = Norm2();  
  
  Double_t mean1=0; // Mean1 value
  Double_t mean2=0; // Mean2 value

  if (!x) {
    for(register Int_t i=0;i<Num;++i) {
      Double_t weight = ::Abs2(fVec[i]);
      mean1 += i   * weight;
      mean2 += i*i * weight;
    }
  } else {
    
    if (x->GetLen()!=Num) {
      Error("CMoment2","Length of the x vector (%d) differs from vector length (%d)",
	    x->GetLen(),Num);
      return 0;
    }
    for(register Int_t i=0;i<Num;++i) {
      Double_t weight = ::Abs2(fVec[i]);
      Double_t      t = ::Abs(x->fVec[i]);
      mean1 += t  * weight;
      mean2 += t*t * weight;
    }
  }

  mean1/=n2;
  mean2/=n2;
  
  return mean2-mean1*mean1;
  
} 

void    TDSPVector::Set(Int_t j,
			TDSPVector* v,Int_t p,Int_t len) {
  
  Int_t vlen = v->GetLen();
  if (len==-1) len=vlen-p; 
  else
    if (p+len>vlen) {
      Error("Set","length %d starting at %d exceeds vector dimensions (%d)", len,p,vlen);
      return;
    }
  if (j+len>Num) {
    Error("Set","length %d starting at %d exceeds dimension of (this) vector which has length %d",
	  len,p,Num);
    return;
  }
  TComplex    *inp = v->fVec+p;
  TComplex    *out = fVec+j;

  for(register Int_t i=0;i<len;++i)    *out++ = *inp++;
}


TDSPVector& TDSPVector::operator+=(const TDSPVector& s) {
  register Int_t i;
  switch(fStrict) {
  case kStrictError:
    if (s.Num!=Num) {
      Error("operator+=","Dimension differ (%d != %d)!",s.Num,Num);
      return (*this);
    }
    break;
  case kStrictWarning:
    if (s.Num!=Num) 
      Warning("operator+=","Dimension differ (%d != %d)!",s.Num,Num);
    break;
  }
   
  if (s.Num<=Num) {
    
    for(i=0;i<s.Num;++i) 
      fVec[i]+=s.fVec[i];
    return *this;
    
  }

  SetLen(s.Num,kFALSE);

  for(i=0;i<Num;++i) 
    fVec[i]+= s.fVec[i];
  for(   ;i<s.Num;  ++i)
    fVec[i] = s.fVec[i];
  
  return *this;
}

TDSPVector& TDSPVector::operator*=(const TDSPVector& s) {
  register Int_t i;
  if (s.Num<=Num) {

    for(i=0;i<s.Num;++i) 
      fVec[i]*=s.fVec[i];
    for(   ;i<Num;++i)
      fVec[i] =0;
    
    return *this;
  
  } 

  for(i=0;i<Num;++i) 
    fVec[i]*= s.fVec[i];
  
  return *this;
}

TDSPVector& TDSPVector::operator-=(const TDSPVector& s) {
  register Int_t i;
  if (s.Num<=Num) {

    for(i=0;i<s.Num;++i) 
      fVec[i]-=s.fVec[i];
    return *this;
  
  }

  SetLen(s.Num,kFALSE);

  for(i=0;i<Num;++i) 
    fVec[i]-= s.fVec[i];
  for(   ;i<s.Num;  ++i)
    fVec[i] = -s.fVec[i];
  
  return *this;
}

void TDSPVector::ConjugateMe() {
  if (!fVec) 
    Error("ConjugateMe","Vector not alloced !!");
  else 
    for(register Int_t i=0;i<Num;++i) fVec[i].fIm *= -1.;
  
}



/* increments this vector (seen as a sequence of complex number
   taken from a finite alphabet) 
   return =  1 - no error in incrementation
   return =  0 - maximum number reached
   return = -1 - error in codeword (a digit was not from alphabet) */


Int_t TDSPVector::OnCodeInc(TDSPVector*   al) {
  return OnCodeInc(al->GetVec(),al->GetLen());
}
Int_t TDSPVector::OnCodeInc(TComplex*   al,
			  Int_t       num_al) {
  
  register Int_t i = Num-1;
  for(  ;i>=0    ;--i) {
    register Int_t j = 0;
    for(;  j<num_al;++j)
      if (fVec[i] == al[j]) break;
    if (j==num_al)      /* this is not from alphabet - error !! */
      return -1; 
    if (j==num_al-1) {  /* this was the last symbol  - reset and continue */
      fVec[i] = al[0]; 
      continue;
    }
    fVec[i]   = al[j+1];   /* this was a regular symbol - increment and exit */
    return 1;
  }

  /* maximum number reached of combinations reached ! */

  return 0;

}



Int_t TDSPVector::OnCodeSet(Int_t       value,
			  TDSPVector* al) {
  return OnCodeSet(value, al->GetVec(), al->GetLen());
}
Int_t TDSPVector::OnCodeSet(Int_t     value,
			  TComplex* al,
			  Int_t     num_al) {

  Int_t pos = Num-1;
  Int_t iCode;
  while(value) {
    iCode     = value%num_al;
    value    /= num_al;
    if (pos<0) return 0;
    fVec[pos--] = al[iCode];
  }
  
  return 1;

}

Int_t TDSPVector::OnCodeGet(Int_t*      value,
			  TDSPVector* al) {
  return OnCodeGet(value, al->GetVec(), al->GetLen());
}
Int_t TDSPVector::OnCodeGet(Int_t*    value,
			  TComplex* al,
			  Int_t     num_al) {

  Int_t base = 1;
  *value     = 0;
  for(register Int_t pos=Num-1;pos>=0;--pos) {
    register Int_t j = 0;
    for(;  j<num_al;++j)
      if (fVec[pos] == al[j]) break;
    if (j==num_al)      /* this is not from alphabet - error !! */
      return -1; 
    *value += base*j;
    base   *= num_al;
  }

  
  return 1;

}


/* Calculate the p-norm */

Double_t TDSPVector::Norm(TDSPVector *weights, Int_t p) {
  
  /* special for weighted 2-norm */
  if (p==2) return TMath::Sqrt(Norm2(weights));
  
  if (!Num) return 0;

  Double_t n = 0;
  if (weights) {
    if (weights->GetLen()!=Num) {
      Error("Norm","Length of weights-vector(%d) differs from vector length(%d)!",
	    weights->GetLen(),Num);
      return 0;
    }
    TComplex *w = weights->GetVec();
    if (p==1) {
      for(register Int_t i=0;i<Num;++i) n+=::Abs(fVec[i])*w[i].fRe;
      return n;
    } else
      for(register Int_t i=0;i<Num;++i) n+=TMath::Power(::Abs(fVec[i]),p)*w[i].fRe;
    
  } else {
    if (p==1) {
      for(register Int_t i=0;i<Num;++i) n+=TMath::Power(::Abs(fVec[i]),p);    
      return n;
    } else
      for(register Int_t i=0;i<Num;++i) n+=::Abs(fVec[i]);    
  }
  
  return TMath::Power(n,1./p);
}

// Number of Values to Print per Line

UInt_t  TDSPVector::fStrict    = 0;
UInt_t  TDSPVector::fPerLine   = 4;
TH1    *TDSPVector::fHistogram = NULL;
Bool_t  TDSPVector::fATLAS = kFALSE;
