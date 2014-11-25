/** TDSP *******************************************************************
                          TDSPLmmse.cpp  -  description
                             -------------------
    begin                : Mon Apr 29 2002
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

#include "TDSPLmmse.h"

ClassImpQ(TDSPLmmse)

void TDSPLmmse::_init() {

  fNoiseScaler     = 1; // noise variance = 1
  fEstimatorMatrix = NULL; 
  // Variables for a Linear Equation y=Hx+n
  fXCovariance     = NULL;
  fNoiseCovariance = NULL;
  fPost            = NULL;
  fH               = NULL;
  fHCxHStar        = NULL;
  recalc1();
  recalc2();
  _my_x_matrix     = kFALSE;
  _my_noise_matrix = kFALSE;
}

TDSPLmmse::TDSPLmmse(){
  _init();
}

TDSPLmmse::TDSPLmmse(TDSPMatrix* H) {
  _init();
  SetMatrix(H);
  default_x_covariance(H->GetCols(),H->GetCols());
  default_noise_covariance(H->GetRows(),H->GetRows());
}

TDSPLmmse::TDSPLmmse(TDSPMatrix* H, TDSPMatrix* Cx, TDSPMatrix* Cn) {  
  _init();
  SetMatrix(H);
  SetXCovariance(Cx);
  SetNoiseCovariance(Cn);
   
}

void TDSPLmmse::SetNoiseCovariance(TDSPMatrix *c){
  if (_my_noise_matrix&&fNoiseCovariance) delete fNoiseCovariance; 
  fNoiseCovariance = c;
  _my_noise_matrix = kFALSE;
  recalc2(); // redo the inversion next time 
}

void TDSPLmmse::SetXCovariance(TDSPMatrix *c){
  if (_my_x_matrix&&fXCovariance) delete fXCovariance; 
  fXCovariance = c;
  _my_x_matrix = kFALSE; 
  recalc2(); // redo the inversion next time 
  recalc1(); // redo the transformation next time 
}

void TDSPLmmse::default_noise_covariance(Int_t rows, Int_t cols) {
  SetNoiseCovariance(Zeros(rows,cols));
  _my_x_matrix=kTRUE;
}

void TDSPLmmse::default_x_covariance(Int_t rows, Int_t cols) {
  SetXCovariance(UnitMatrix(rows,cols));
  _my_noise_matrix=kTRUE;
}


// Calculate H Cx H^*
void TDSPLmmse::updatefHCxHStar() {
  // Transformed Covariance-Matrix of X
  fHCxHStar = fXCovariance->Transform(fH,fHCxHStar);
  _recalc1=kFALSE;
}

void TDSPLmmse::Calculate() {
  if (!_recalc2) return; 
  if ( _recalc1) updatefHCxHStar();
  
  //TDSPMatrix *tmp  = fHCxHStar->Dup();
  //(*tmp)+=*fNoiseCovariance;

  TDSPMatrix *tmp = fNoiseCovariance->Dup();
  (*tmp)*=fNoiseScaler; // Scale the Noise
  (*tmp)+=*fHCxHStar; // add H Cx H^*

  fEstimatorMatrix=tmp->Inverse(fEstimatorMatrix);
  fH->AdjointRightApply(fEstimatorMatrix,tmp);

  if (fPost) {
    TDSPMatrix *tmp1;
    tmp1=tmp->LeftApply(fXCovariance);
    tmp1->RightApply(fPost,fEstimatorMatrix);
    delete tmp1;
  } else {
    tmp->LeftApply(fXCovariance,fEstimatorMatrix);
  }

  delete tmp;

  _recalc2 = kFALSE;
}

TDSPLmmse::~TDSPLmmse(){
  if (fEstimatorMatrix) delete fEstimatorMatrix;
  if (fHCxHStar)        delete fHCxHStar;
			 
}
