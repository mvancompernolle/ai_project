/** TDSP *******************************************************************
                          TDSPGeneratorGControl.cpp  -  description
                             -------------------
    begin                : Wed Oct 31 2001
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

#include "TDSPOperatorGControl.h"
#include "TDSPGeneratorGControl.h"

ClassImpQ(TDSPGeneratorGControl)

TDSPGeneratorGControl::TDSPGeneratorGControl(const TGWindow *parent,
					     UInt_t w, UInt_t h,
					     UInt_t options) 
  : TDSPOperatorGControl(parent,w,h,options) {
}
TDSPGeneratorGControl::~TDSPGeneratorGControl(){
}
