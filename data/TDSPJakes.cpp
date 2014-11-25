/** TDSP *******************************************************************
                          TDSPJakes.cpp  -  description
                             -------------------
    begin                : Mon Jan 21 2002
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

#include <TError.h>
#include "TDSPJakes.h"

ClassImp(TDSPJakes)

const Double_t SpeedOfLight = 3.0e8;

#define _doppler fDopplerFrequency=fSpeed/SpeedOfLight*fCarrierFrequency

TDSPJakes::TDSPJakes()  {
  SetSpeedKMH(0);
}
TDSPJakes::~TDSPJakes(){
}

void TDSPJakes::SetCarrierFrequency(Double_t c) {
  fCarrierFrequency = c;
  _doppler;
}

void TDSPJakes::SetSpeed(Double_t ms) {
  fSpeed = ms;
  if (fCarrierFrequency) 
    _doppler;
}

void TDSPJakes::SetSpeedKMH(Double_t kmh){
  SetSpeed(kmh/3.6);
}

Double_t TDSPJakes::GetDopplerFrequency() {
  if (!fCarrierFrequency) Error("GetDopplerFrequency","Carrier Frequency not set");
  return fDopplerFrequency;
}

void  TDSPJakes::SetDopplerFrequency(Double_t d) {
  fDopplerFrequency   = d;
  if (fCarrierFrequency) 
    fSpeed            = fDopplerFrequency*SpeedOfLight/fCarrierFrequency;
  else
    fSpeed            = -1;
}


Double_t TDSPJakes::Correlation(Double_t delay) {
  return TDSPMath::BesselJ0(delay*2*TMath::Pi()*fDopplerFrequency);
}

