/** TDSP *******************************************************************
                          TDSPEstimator.cpp  -  description
                             -------------------
    begin                : Mon Apr 29 2002
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

#include "TDSPEstimator.h"

ClassImpQ(TDSPEstimator);

TDSPEstimator::TDSPEstimator(){
  fEstimate = new TDSPVector();
}
TDSPEstimator::~TDSPEstimator(){
  if (fEstimate) delete fEstimate;
}

