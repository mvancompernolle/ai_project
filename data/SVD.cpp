/** TDSP *******************************************************************
                           SVD.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 2002
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



#include <TMath.h>
#include <TVectorD.h>
#include <TDSPVector.h>
#include <TDSPMatrix.h>

#include <TMatlab.h>
#include <TmxArray.h>
#include <TSystem.h>
#include "dl_atlas.h"




TDSPMatrix* TDSPMatrix::PseudoInverse(TDSPMatrix *result) {

#ifdef HAVE_MATLAB
  
  static TmxArray *amx    = NULL;
  static TmxArray *bmx    = NULL;
  Info("PseudoInverse","Not yet implemented - using matlab");
  TMatlab *m = TMatlab::GlobalEngine();
  if (!amx) amx = new TmxArray("PseudoInverseA");
  *amx=*this;
  gMatlab->Put(amx);
  if (!m->EvalString("PseudoInverseB=pinv(PseudoInverseA);")) {
    Error("PseudoInverse","- during calculation of \"pinv\" in matlab!");
    return NULL;
  }
  if (!(bmx = m->Get("PseudoInverseB"))) {
    Error("PseudoInverse","- can get the result of \"pinv\" from matlab!");
    return NULL;
  }

  if (!result) result=new TDSPMatrix();
  *result = *bmx;
    
#else
# ifndef WIN32
#  warning "PseudoInverse disabled - need matlab"
# endif
  Error("PseudoInverse","Not yet implemented - need matlab !");
  return NULL;
#endif

  return result;

}



#undef USE_GESVD

TDSPVector* TDSPMatrix::SVD(TDSPVector *S, 
			    TDSPMatrix *U, 
			    TDSPMatrix *VStar) {

#ifdef DL_ATLAS
  
# ifdef  TCOMPLEX_USE_DOUBLE
  //////////////////////////////////////////////////////////////////////////
#  ifdef USE_GESVD
  if (!dl_lapack_zgesvd) {
    if (!(dl_lapack_zgesvd = (dl_lapack_zgesvd_function)gSystem->DynFindSymbol("/usr/lib/liblapack.so","zgesvd_"))) {
      Error("SVD","Error loading lapack's zgesvd");
      return NULL;
    }
  }
#  else
  if (!dl_lapack_zgesdd) {
    if (!(dl_lapack_zgesdd = (dl_lapack_zgesdd_function)gSystem->DynFindSymbol("/usr/lib/liblapack.so","zgesdd_"))) {
      Error("SVD","Error loading lapack's zgesdd");
      return NULL;
    }
  }
#  endif
  //////////////////////////////////////////////////////////////////////////
# else
#  ifdef USE_GESVD
  if (!dl_lapack_cgesvd) {
    if (!(dl_lapack_cgesvd = (dl_lapack_cgesvd_function)gSystem->DynFindSymbol("/usr/lib/liblapack.so","cgesvd_"))) {
      Error("SVD","Error loading lapack's cgesvd");
      return NULL;
    }
  }
#  else
  if (!dl_lapack_cgesdd) {
    if (!(dl_lapack_cgesdd = (dl_lapack_cgesdd_function)gSystem->DynFindSymbol("/usr/lib/liblapack.so","cgesdd_"))) {
      Error("SVD","Error loading lapack's cgesdd");
      return NULL;
    }
  }
#  endif
  //////////////////////////////////////////////////////////////////////////
# endif

  Int_t       M = fRows;
  Int_t       N = fCols;
  Int_t   maxMN = TMath::Max(M,N);
  Int_t   minMN = TMath::Min(M,N);
  
  TComplexBase *vU,*vV;

  if (!U)     vU=(TComplexBase*)calloc(M*M*2,sizeof(TComplexBase)); else { U->SetSize(M,M);    vU=(TComplexBase*)U->GetVec();    };
  if (!VStar) vV=(TComplexBase*)calloc(N*N*2,sizeof(TComplexBase)); else { VStar->SetSize(N,N);vV=(TComplexBase*)VStar->GetVec();};

  if (!S)     S       = new TDSPVector(maxMN);

  Char_t        CH_A  = 'A';
  Int_t         INFO;
  Int_t         LWORK = -1;
  TComplexBase  REQUEST_WORK[2];
  

# ifdef  TCOMPLEX_USE_DOUBLE
  ////////////////////////////// requesting the optimal LWORK value !
#  ifdef USE_GESVD
  dl_lapack_zgesvd(&CH_A,&CH_A, &M, &N,NULL,&M,NULL,NULL,&M,NULL,&N,
		   REQUEST_WORK, &LWORK,NULL,&INFO);
#  else
  dl_lapack_zgesdd(&CH_A,&M, &N,NULL,&M,NULL,NULL,&M,NULL,&N,
		   REQUEST_WORK, &LWORK,NULL,NULL, &INFO);
#  endif
# else
#  ifdef USE_GESVD
  dl_lapack_cgesvd(&CH_A,&CH_A, &M, &N,NULL,&M,NULL,NULL,&M,NULL,&N,
		   REQUEST_WORK, &LWORK,NULL,&INFO);
#  else
  dl_lapack_cgesdd(&CH_A,&M, &N,NULL,&M,NULL,NULL,&M,NULL,&N,
		   REQUEST_WORK, &LWORK,NULL,NULL, &INFO);
#  endif
# endif
  
  LWORK                 = Int_t(REQUEST_WORK[0]); // the optimal LWORK value
  TComplexBase   *WORK  = (TComplexBase*)calloc(LWORK, sizeof(        TComplexBase)*2); // alloc the optimal workplace
  TComplexBase   *RWORK = (TComplexBase*)calloc(7*maxMN*maxMN, sizeof(TComplexBase));
# ifndef USE_GESVD
  Int_t        *IWORK   = (Int_t*) calloc(8*maxMN,   sizeof(Int_t));
# endif
  TComplexBase *SingVal = (TComplexBase*)calloc(minMN,sizeof(TComplexBase)); // the real singular values !

  // Because the Matrix will be destroyed AND we need the transpose(It's a fortran routine)
  // we use Transposed()...

# ifdef LINALG_MAT_ROWWISE
  TDSPMatrix         *A = Transposed();
# else
  TDSPMatrix         *A = Dup();
# endif

  TComplexBase      *vA = (TComplexBase*)A->GetVec();

# ifdef  TCOMPLEX_USE_DOUBLE
#  ifdef USE_GESVD
  dl_lapack_zgesvd(&CH_A,&CH_A,&M, &N, vA,  &M, SingVal, vU,&M,vV,&N,
		   WORK, &LWORK,RWORK,&INFO);		   
#  else
  dl_lapack_zgesdd(&CH_A,&M, &N, vA,  &M, SingVal, vU,&M,vV,&N,
		   WORK, &LWORK,RWORK,IWORK, &INFO);
#  endif
# else
#  ifdef USE_GESVD
  dl_lapack_cgesvd(&CH_A,&CH_A,&M, &N, vA,  &M, SingVal, vU,&M,vV,&N,
		   WORK, &LWORK,RWORK,&INFO);
#  else
  dl_lapack_cgesdd(&CH_A,&M, &N, vA,  &M, SingVal, vU,&M,vV,&N,
		   WORK, &LWORK,RWORK,IWORK, &INFO);		     
#  endif
# endif

  delete A;
  
  free(WORK);
  free(RWORK);

# ifdef USE_GESVD
  free(IWORK);
# endif

  if (!U) free(vU);
# ifdef LINALG_MAT_ROWWISE
  else 
    U->TransposeMe();
# endif
  
  if (!VStar) free(vV);
# ifdef LINALG_MAT_ROWWISE
  else 
    VStar->TransposeMe();
# endif
  

  S->SetLen(maxMN);
  TComplex* Sv = S->GetVec();
  for(register Int_t i=0;    i<minMN;++i) Sv[i] = SingVal[i];
  for(register Int_t i=minMN;i<maxMN;++i) Sv[i] = 0;
  free(SingVal);

  
#else
  Error("SVD","Atlas-support not avaible !"); 
#endif  
		   
  return S;

}
