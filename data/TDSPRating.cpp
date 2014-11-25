/** TDSP *******************************************************************
                          TDSPRating.cpp  -  description
                             -------------------
    begin                : Fri Mar 8 2002
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

#include "TDSPRating.h"

ClassImp(TDSPRating);

TDSPRating::TDSPRating(){
  
  fRelativeRate   = 0;
  fRelativeRateMean = 0;
  fRateIndex      = 0;
  fRate           = 1;
  fRelativeRateSigma = 1.0;
}
TDSPRating::~TDSPRating(){
}


void TDSPRating::SetRate(Double_t rate) {
  
  // Set the Rate of the Stochastic Process
  //
  
  if (rate!=fRate) {
    ResetRating();
    fRate=rate;
  }
}
