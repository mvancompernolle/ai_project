/** TDSP *******************************************************************
                          TDSPOutput1.cpp  -  description
                             -------------------
    begin                : Wed Nov 14 2001
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

#include <Riostream.h>
#include "TDSPOutput1.h"

ClassImpQ(TDSPOutput1)
  
TDSPOutput1::TDSPOutput1(char *name) : TDSPOperator(name) {  
  
  Signal_NewData = AddSignal("Update(TDSPSignal*)");
  Slot_Update    = AddSlot("Update(TDSPSignal*)");
  
  fSignal   = new TDSPSignal("output1"); 
  fSignal->SetTitle("the output signal of this operator");

  Activate();

}

TDSPOutput1::~TDSPOutput1(){
  if (fSignal) delete fSignal;
}

void TDSPOutput1::Print() {
  TDSPOperator::Print();
  if (!fIsActive)
    cout << " is disabled" << endl;
  
}

istream &operator>>(istream &is,TDSPOutput1 &z) {
  TDSPSignal  *output = z.GetSignal();
  is >> output;
  return ( is );
}
ostream &operator<<(ostream &os,TDSPOutput1 &z) {
    
  TDSPSignal  *output = z.GetSignal();
  os << "fSignal = " << endl;
  os << output;
  return ( os );
}

istream &operator>>(istream &is,TDSPOutput1 *z) {
  is >> *z;
  return ( is );
}
ostream &operator<<(ostream &os,TDSPOutput1 *z) {
  os << *z;
  return ( os );
}


void TDSPOutput1::Browse(TBrowser *b) {
  if (b) {
    b->Add(fSignal,    "fSignal");
  } else 
    TDSPOperator::Browse(b);
}
