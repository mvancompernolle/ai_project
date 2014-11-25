/** TDSP *******************************************************************
                          TDSPSignalView.cpp  -  description
                             -------------------
    begin                : Mon Oct 29 2001
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

#include "TDSPSignalView.h"
#include <TCanvas.h>
#include <TGraph.h>
#include <stdlib.h>

ClassImpQ(TDSPSignalView)


  TDSPSignalView::TDSPSignalView(char *name,char *title, Option_t* op) : TDSPOperator(name) {
  Canvas=NULL;
  CanvasTitle=NULL;
  CanvasName=NULL;
  SetName(name);
  if (!title) SetTitle(name); else SetTitle(title);
  SetOption(op);
  // Add my Slots
  Slot_Update =  AddSlot("Update()");
}


TDSPSignalView::~TDSPSignalView(){
}

void TDSPSignalView::SetOption(Option_t *op) {
  fOption=op;
}

void TDSPSignalView::Update(TDSPSignal *input){

  if (!Canvas) {
    if (!CanvasName)  CanvasName  = new TString(fName);
    if (!CanvasTitle) CanvasTitle = new TString(fTitle);
    Canvas = new TCanvas(*CanvasName,*CanvasTitle,800,600);
  }
  
  Canvas->Clear();
  input->Draw(fOption);
  Canvas->Update();
    
}



