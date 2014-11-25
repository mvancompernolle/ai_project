/** TDSP *******************************************************************
                          TDSPSignalStudy.cpp  -  description
                             -------------------
    begin                : Die Okt 8 2002
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

#include "TDSPSignalStudy.h"

ClassImpQ(TDSPSignalStudy)



TDSPSignalStudy::TDSPSignalStudy(char *name) : TDSPOutput1(name) {

  // Set Some Defaults

  fMean = NULL;

}
TDSPSignalStudy::~TDSPSignalStudy(){
}

void TDSPSignalStudy::Analyse(TDSPSignal *input) {
  
  if ( IsCrestFactor() ) {  // Calculate the crestfactor
    
    // Calculate the Mean Value

    fMean    = I_MeanAbs(input,fMean);
    
    // Calculate the Maximum Value

    fMaxAbs  = I_MaxAbs(input, fMaxAbs);

    // Calculate the 

    fCrest   = fMaxAbs->DivBy(fMean,fCrest);
    

  }

}
