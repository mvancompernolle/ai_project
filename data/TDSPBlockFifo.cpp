/** TDSP *******************************************************************
                          TDSPBlockFifo.cpp  -  description
                             -------------------
    begin                : Thu Jan 10 2002
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

#include "TDSPBlockFifo.h"

ClassImpQ(TDSPBlockFifo)

TDSPBlockFifo::TDSPBlockFifo(char *name) : TDSPOutput1(name) {
  fWidth = 0;
}
TDSPBlockFifo::~TDSPBlockFifo(){
}

void TDSPBlockFifo::SetDepth(Int_t depth) { 
  Error("SetDepth","Use SetSize(Int_t width, Int_t depth) on BlockFifo's");
}

void TDSPBlockFifo::SetSize(Int_t width, Int_t depth) { 
  if ((width!=fWidth)||(depth!=fDepth)) {
    fWidth = width;fDepth=depth;
    if (fBlock) delete [] fBlock;
    fBlock = new TComplex[fWidth*(fDepth+1)];
  }
}

