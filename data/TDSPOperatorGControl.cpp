/** TDSP *******************************************************************
                          TDSPOperatorGControl.cpp  -  description
                             -------------------
    begin                : Fri Nov 9 2001
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

ClassImpQ(TDSPOperatorGControl)

TDSPOperatorGControl::TDSPOperatorGControl(const TGWindow *parent,
					   UInt_t w, UInt_t h,
					   UInt_t options):
  TQQObjectGControl(parent,w,h,options)
{

  // Show only Connections to TDSPOperators
  //
  AddVisibleBaseClass("TDSPOperator");
 
}
TDSPOperatorGControl::~TDSPOperatorGControl(){
}
