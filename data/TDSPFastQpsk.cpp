/** TDSP *******************************************************************
                          TDSPFastQpsk.cpp  -  description
                             -------------------
    begin                : Mon Mai 6 2002
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

#include "TDSPFastQpsk.h"

ClassImpQ(TDSPFastQpsk)
  
TDSPFastQpsk::TDSPFastQpsk(char *name) : TDSPPsk(name,4) {
}
TDSPFastQpsk::~TDSPFastQpsk(){
}
