/** TDSP *******************************************************************
                          TDSPFifoFilter.cpp  -  description
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

#include "TDSPFifoFilter.h"

ClassImp(TDSPFifoFilter)

TDSPFifoFilter::TDSPFifoFilter() {
  fInput  = new TDSPFifo();
  fOutput = new TDSPFifo();
  fFilter = NULL;
}
TDSPFifoFilter::~TDSPFifoFilter(){
}


void TDSPFifoFilter::SetFilter(TDSPFilter *f) {
  fFilter = f;
  fInput->SetDepth( fFilter->GetFIR()->GetLen());
  fOutput->SetDepth(fFilter->GetIIR()->GetLen());

}

void TDSPFifoFilter::Reset() {
  if (fInput)  fInput->Reset();
  if (fOutput) fOutput->Reset();
}
