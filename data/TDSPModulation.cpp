/** TDSP *******************************************************************
                          TDSPModulation.cpp  -  description
                             -------------------
    begin                : Mon Mai 13 2002
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

#include "TDSPModulation.h"

ClassImpQ(TDSPModulation)

TDSPModulation::TDSPModulation(char *name) : TDSPOutput1(name) {
  fPhase=0;
  fBurstOff=0;
}
TDSPModulation::~TDSPModulation(){
}

TDSPSignal *TDSPModulation::Update(TDSPSignal *in) {
  Modulate(in);
  Emit(Slot_Update,fSignal);
  return fSignal;
}

void        TDSPModulation::Reset() {

}
