/** TDSP *******************************************************************
                          TDSPFilter.cpp  -  description
                             -------------------
    begin                : Wed Jan 9 2002
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

#include "TDSPFilter.h"

ClassImp(TDSPFilter)

TDSPFilter::TDSPFilter()  {
  fFIR   = NULL;
  fIIR   = NULL;  
  fNorm  = 1;
  fNormalized = kFALSE;
}
TDSPFilter::~TDSPFilter(){
}

void TDSPFilter::Normalize() { 
  if (!fNormalized) {
    if (!fFIR) 
      Error("Normalize","FIR Filter not set ! ");
    else {
      if (fNorm) {
	(*fFIR)/=fNorm;
	fNormalized=kTRUE;
      } else 
	Error("Normalize","Filter norm not yet calculated and/or is zero!");
    }
  }
}
