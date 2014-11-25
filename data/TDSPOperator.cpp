/** TDSP *******************************************************************
                          tdspoperator.cpp  -  description
                             -------------------
    begin                : Mon Sep 10 2001
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

#include "TDSPOperator.h"
#include <TGMsgBox.h>
#include <TDSPOperatorGControl.h>


ClassImpQ(TDSPOperator)

TDSPOperator::TDSPOperator(char *name) {
  SetName(name);
  fInitialized = kFALSE;
  New(this);             // Emit the Creation of a new TDSPOperator  

  fDebugWindow=NULL;
  
}
TDSPOperator::~TDSPOperator(){
  Delete(this);
}

TQQGControl* TDSPOperator::NewGControl() {
  TDSPOperatorGControl *o = new TDSPOperatorGControl();
  o->SetObject(this);
  return  o;
}

TCanvas* TDSPOperator::DebugGetWindow(char *title) {
  if (!fDebugWindow) fDebugWindow = new TCanvas(GetName(),title,640,480);
  else fDebugWindow->SetTitle(title);
  return fDebugWindow;
}

Bool_t     TDSPOperator::DebugWaitToContinue() {
  Int_t ret;
  TGMsgBox* m= new  TGMsgBox(0,0,"DebugWaitToContinue","continue Debug mode?",kMBIconQuestion,kMBYes|kMBNo,&ret);
  if (ret==kMBNo) return kFALSE;
  return kTRUE;
}
