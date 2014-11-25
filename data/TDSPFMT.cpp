/** TDSP *******************************************************************
                          TDSPFMT.cpp  -  description
                             -------------------
    begin                : Fri Nov 23 2001
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

#include <TFile.h>
#include "TDSPFMT.h"

ClassImpQ(TDSPFMT)

TDSPFMT::TDSPFMT(char *name) : TDSPFilterMap(name) {

  // Add Delegations

  Delegation_Transmitted = AddDelegation("Transmitted(TDSPSignal*)");
  Delegation_Received    = AddDelegation("Received(TDSPSignal*)");

  Delegation_Transmit    = AddDelegation("Transmit(TDSPSignal*)");
  Delegation_Receive     = AddDelegation("Receive(TDSPSignal*)");

  
  // Give them a meaning as a slot
  //
  Connect(Delegation_Transmit,
	  ClassName(),
	  this,
	  Slot_Update);


  Connect(Delegation_Receive,
	  ClassName(),
	  this,
	  Slot_InverseUpdate);

  // Give them a meaning as a signal
  //
  Connect(Signal_Update,
	  ClassName(),
	  this,
	  Delegation_Transmitted);

  Connect(Signal_InverseUpdate,
	  ClassName(),
	  this,
	  Delegation_Received);

  fSignal->SetTitle("the transmit  signal");
  fInverseSignal->SetTitle("the received signal");

}
TDSPFMT::~TDSPFMT(){
}



void TDSPFMT::SetOption(Option_t *option) {
  
  TString opt = option;
  opt.ToLower();
  
  Ssiz_t pos;
  
  if ((pos = opt.Index("bfdm"))!= kNPOS) {
    opt.Remove(pos,4);
  }
  if (opt.Strip().Length()) 
    Error("SetOption","Unknown Option(s) \"%s\" !!",opt.Strip().Data());
  
}


void TDSPFMT::LoadPulses(char *fname, char *transmitname, char *receivername) {
  TFile f(fname);
  SetTransmitMatrix((TDSPMatrix*)f.Get(transmitname));
  SetReceiverMatrix((TDSPMatrix*)f.Get(receivername));
  f.Close();
}

void TDSPFMT::SavePulses(char *fname, char *transmitname, char *receivername) {
  TFile f(fname,"RECREATE");
  GetTransmitMatrix()->Write(transmitname);
  GetReceiverMatrix()->Write(receivername);
  f.Close();
}

TDSPVector* TDSPFMT::GetTransmitPulse(TDSPVector *p) {
  p = GetMatrix()->GetRow(0,p);
  return p;
}

TDSPVector* TDSPFMT::GetReceiverPulse(TDSPVector *p) {
  p = GetInverseMatrix()->GetRow(0,p);
  p->Reverse();
  p->AdjointMe();
  return p;
}


TDSPSignal* TDSPFMT::Update(TDSPSignal *input) {
  CheckInitialization();
  Synthesize(input,fSignal);
  Emit(Signal_Update,fSignal);
  return fSignal;
}

TDSPSignal* TDSPFMT::InverseUpdate(TDSPSignal *input) {
  CheckInitialization();
  Analyze(input,fInverseSignal);
  Emit(Signal_InverseUpdate,fInverseSignal);
  return fInverseSignal;
}
