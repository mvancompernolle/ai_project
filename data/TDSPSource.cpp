/** TDSP *******************************************************************
                          TDSPSource.cpp  -  description
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

#include "TDSPSource.h"

ClassImp(TDSPSource)

TDSPSource::TDSPSource(){
  fSender = NULL;
  fSignal = NULL;
}
TDSPSource::~TDSPSource(){
}

void TDSPSource::SetSender(TQQObject*s) {
  fSender=s;
}

void TDSPSource::SetSignal(TDSPSignal*s) {
  fSignal=s;
}



Bool_t operator==(TDSPSource& s1, TDSPSource& s2) {
  if (s1.GetSender()!=s2.GetSender()) return kFALSE;
  if (!s1.GetSignal()) {
    Error("operator==","Comparing a TDSPSource without a Signal inside !");
    return kFALSE;
  }
  if (!s2.GetSignal()) {
    Error("operator==","Comparing a TDSPSource without a Signal inside !");
    return kFALSE;
  }
  if (!strcmp(s1.GetName(),s2.GetName())) return kTRUE;
  return kFALSE;
}
