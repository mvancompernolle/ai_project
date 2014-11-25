/** TDSP *******************************************************************
                          TDSPSolver.cpp  -  description
                             -------------------
    begin                : Thu Mar 14 2002
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

#include "TDSPSolver.h"

ClassImp(TDSPSolver)

TDSPSolver::TDSPSolver(){
  fHaveSolution = kFALSE;
}
TDSPSolver::~TDSPSolver(){
}

void TDSPSolver::SetXAlphabet(TComplex *a, Int_t l) {
  fNumXAlphabet = l;
  fXAlphabet    = a;
  fMinDist      = 0;
}

void TDSPSolver::SetXMinDist(Double_t x) {
  fMinDist = x;
}
