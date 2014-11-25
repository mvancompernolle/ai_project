/** TDSP *******************************************************************
                          TDSPSignalDump.cpp  -  description
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

#include <TSystem.h>
#include "TDSPSignalDump.h"

ClassImpQ(TDSPSignalDump)

TDSPSignalDump::TDSPSignalDump(char *name) : TDSPOperator(name) {
  // My Slots
  Slot_Update=AddSlot("Update(TDSPSignal*)");
  os = &cout;
  fMode = kSignalDumpParams;
  fPause = -1;
};

TDSPSignalDump::~TDSPSignalDump(){
}

void TDSPSignalDump::Update(TDSPSignal *input) {

  *os << "received from (class " << GetSenderClassName() << "*)" << GetSender() << endl;
  
  switch(fMode) {
    
  case kSignalDumpAll:
    input->DumpParams(os);
    *os << input << endl;
    break;
  case kSignalDumpParams:
    input->DumpParams();
    break;

  }

  if (fPause) {
    if (fPause<0) {
      cout << "Press Enter to continue" << flush;
      getchar();
    } else {
      cout << "Waiting " << fPause << " seconds ...";
      gSystem->Sleep(1000*fPause);
    }
  }

}
