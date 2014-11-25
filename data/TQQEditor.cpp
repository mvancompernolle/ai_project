/** TDSP *******************************************************************
                          TQQEditor.cpp  -  description
                             -------------------
    begin                : Fri Nov 30 2001
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

#include "TQQEditor.h"

ClassImpQ(TQQEditor)

  TQQEditor::TQQEditor(TQQGControl *control,
		     const TGWindow *win ,
		     UInt_t w,
		     UInt_t h) :
  TGMainFrame(win,w,h) 
{
  
  fLayout = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY);
  fCanvas = new TGCanvas(this,w,h);
  control->SetCanvas(fCanvas);
  control->ListTree();
  AddFrame(fCanvas,fLayout);
  MapSubwindows();
  Layout();
  Resize(GetDefaultSize());
  SetWindowName(control->GetName());  
  MapWindow();
}

TQQEditor::~TQQEditor(){
}
