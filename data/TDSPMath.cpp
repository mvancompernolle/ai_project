/** TDSP *******************************************************************
                          TDSPMath.cpp  -  description
                             -------------------
    begin                : Thu Jan 31 2002
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
#include <TFormula.h>
#include "TDSPMath.h"

ClassImp(TDSPMath)

const Double_t Pi=TMath::Pi();

TFormula Formula_dB("dB","10*log10(x)");
TFormula Formula_FromdB("FromdB","pow(10,x/10)");
