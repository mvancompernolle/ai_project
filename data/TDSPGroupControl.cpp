/** TDSP *******************************************************************
                          TDSPGroupControl.cpp  -  description
                             -------------------
    begin                : Mon Nov 5 2001
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

#include "TDSPGroupControl.h"

ClassImpQ(TDSPGroupControl)

TDSPGroupControl::TDSPGroupControl(){
  fOperatorControls = NULL;
}
TDSPGroupControl::~TDSPGroupControl(){
}

void TDSPGroupControl::ExploreGroup() {

  TDSPGroup *g = (TDSPGroup*)GetObject();
  TIter next(g->GetListOfOperators());
  while(TDSPOperator *o = (TDSPOperator*)next()) {
    if (!fOperatorControls) fOperatorControls = new TList();
    TIter next1(GetListOfControls());
    ControlOperatorConnection *c = NULL;
    while(( c = (ControlOperatorConnection*)next1())) {
      if (c->Operator == o) break;
    }

    // if for this operator is no entry in the ControlList
    // add it

    if (!c) {      
      c = new ControlOperatorConnection(o);
      GetListOfControls()->Add(c);
    }
  }
}

void TDSPGroupControl::PrintComponents() {

  TDSPGroup *g = (TDSPGroup*)GetObject();
  TIter next(g->GetListOfOperators());
  while(TDSPOperator *o = (TDSPOperator*)next()) {
    if (!fOperatorControls) fOperatorControls = new TList();
    TIter next1(GetListOfControls());
    ControlOperatorConnection *c = NULL;
    while(( c = (ControlOperatorConnection*)next1())) {
      if (c->Operator == o) break;
    }

    cout << o->GetName() << "[" << o->ClassName() << "]";
    if (c) {      
      c = new ControlOperatorConnection(o);
      GetListOfControls()->Add(c);
      cout << " {controlled}";
    }
    cout << endl;

  }

}

void TDSPGroupControl::Print() {
  PrintComponents();
}
