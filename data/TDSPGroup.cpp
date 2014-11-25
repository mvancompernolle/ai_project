/** TDSP *******************************************************************
                          TDSPGroup.cpp  -  description
                             -------------------
    begin                : Fri Nov 2 2001
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

#include "TDSPGroup.h"
#include <TDSPGroupGControl.h>

ClassImpQ(TDSPGroup)

TDSPGroup::TDSPGroup(char *name) : TDSPOperator(name) {
  fListOfOperators= new TList();
}
TDSPGroup::~TDSPGroup(){
}

TQQGControl* TDSPGroup::NewGControl() {
  TDSPGroupGControl *o = new TDSPGroupGControl();
  o->SetObject(this);
  return  o;
}

void TDSPGroup::Print() {
  fListOfOperators->Print();
}
