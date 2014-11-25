/** TDSP *******************************************************************
                          TDSPClip.cpp  -  description
                             -------------------
    begin                : Tue Jan 22 2002
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

#include "TDSPClip.h"

ClassImpQ(TDSPClip)

TDSPClip::TDSPClip(char *name) : TDSPFunction(name) {
  fClipAmplitude = 0;
}
TDSPClip::~TDSPClip(){
}
