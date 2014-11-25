/** TDSP *******************************************************************
                          TDSPGlobals.cpp  -  description
                             -------------------
    begin                : Wed Jan 2 2002
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


// The Global Group - containing all Operators
//

#include <TDSPGroup.h>
#include <TDSPVector.h>
#include <TDSPfft.h>
#include <TDSPBlockfft.h>

TDSPGroup *gGroup;
void Init_Globals() {
  
  gGroup = new TDSPGroup("gGroup");

  gGroup->Connect("TDSPOperator","New(TDSPOperator*)",
		  "TDSPGroup",gGroup,"Add(TDSPOperator*)");
  
  gGroup->Connect("TDSPOperator","Delete(TDSPOperator*)",
		  "TDSPGroup",gGroup,"Remove(TDSPOperator*)");

  gFFT       = new TDSPfft("gFFT");
  gFFT->SetTitle("globally avaible fft/ifft object");
  gBlockFFT  = new TDSPBlockfft("gBlockFFT");
  gBlockFFT->SetTitle("globally avaible blockwise fft/ifft object");

  // Enable ATLAS by default

  TDSPVector::LoadATLAS();

}

