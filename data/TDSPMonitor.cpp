/** TDSP *******************************************************************
                          TDSPMonitor.cpp  -  description
                             -------------------
    begin                : Sam Aug 3 2002
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

#include <TDSPSource.h>
#include "TDSPMonitor.h"

ClassImpQ(TDSPMonitor)

TDSPMonitor::TDSPMonitor(char *name) : TDSPOperator(name) {
  
  Slot_Update = AddSlot("Update(TDSPSignal*)");
 
}
TDSPMonitor::~TDSPMonitor(){
}



void TDSPMonitor::Update(TDSPSignal *s) {
  
  // this source
  //
  TDSPSource *SO = new TDSPSource();
  SO->SetSignal(s);
  SO->SetSender(static_cast<TQQObject*>(GetSender()));
  char n[100];
  sprintf(n,"%s::%s",SO->GetSender()->GetName(),s->GetName());  
  TString nam = n;
  nam.ToLower();
  SO->SetName(nam);
  
  // Loop over all sources to find out 
  // which source is coming here 

  TIter next(&fSignalList);
  while(TDSPSource *so = static_cast<TDSPSource*>(next())) {
    if (*so == *SO) {
      so->SetSignal(s); // the signal itself (not its name) could differ !!
      delete SO;
      SO=so;
      goto haveit;
    }
  }

  fSignalList.Add(SO); // new source - add it 

 haveit:
  
  Monitor((char*)SO->GetName());

}

TDSPSignal* TDSPMonitor::GetNamedSignal(char *name) {  
  TString nam = name;
  nam.ToLower();
  TDSPSource * s = static_cast<TDSPSource*>(fSignalList.FindObject(nam));
  if (!s) {
    Error("GetNamedSignal","Can not find the signal \"%s\" !",nam.Data());
    return NULL;
  }
  return s->GetSignal();
}

void TDSPMonitor::Monitor(char *action) {

  if (!strcmp(action,"::output2")) {
    TDSPSignal *s1 = GetNamedSignal("::GeneratedSignal"); 
    if (!s1) {
      Error("Monitor","Have no \"::GeneratedSignal\"!");
      return;
    }
    TDSPSignal *s2 = GetNamedSignal("::output2");
    if (!s2) {
      Error("Monitor","Have no \"::output2\"!");
      return;
    }
    
    //MSE(s1,s2,&fMSE);
    MeanPhaseDiff(s1,s2,&fMSE);

  }

}
