/** TDSP *******************************************************************
                          TDSPReadSignal.cpp  -  description
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
#include "TDSPReadSignal.h"


ClassImpQ(TDSPReadSignal)

TDSPReadSignal::~TDSPReadSignal(){
}

void TDSPReadSignal::Generate() {

  cout << GetName() << " (" << ClassName() << ") " << endl;
  
  *is >> this;
  
  Emit(Signal_NewData,GetSignal());
}
