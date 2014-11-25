/** TDSP *******************************************************************
                          TGTabDialog.cpp  -  description
                             -------------------
    begin                : Thu Nov 8 2001
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

#include "TGTabDialog.h"


ClassImpQ(TGTabDialog)

TGTabDialog::TGTabDialog(const TGWindow *p, 
			 const TGWindow *main, UInt_t w,
			 UInt_t h, UInt_t options) 
  : TGTransientFrame(p, main, w, h, options) {

  fMain = main;
 
  fListOfTabs = new TList();
  
  fBottomFrame   = new TGHorizontalFrame(this, 60, 20, kFixedWidth);

  fOkButton = new TGTextButton(fBottomFrame, "&Ok", 1);
  fOkButton->Associate(this);
  fCancelButton = new TGTextButton(fBottomFrame, "&Cancel", 2);
  fCancelButton->Associate(this);

  fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,
			  2, 2, 2, 2);
  fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1);

  fBottomFrame->AddFrame(fOkButton, fL1);
  fBottomFrame->AddFrame(fCancelButton, fL1);
  
  fBottomFrame->Resize(150, fOkButton->GetDefaultHeight());
  AddFrame(fBottomFrame, fL2);
  
  fTab = new TGTab(this, 300, 300);

  TGLayoutHints *fL5 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
					 kLHintsExpandY, 2, 2, 5, 1);
  AddFrame(fTab, fL5);


}

void TGTabDialog::DrawDialog() {

  MapSubwindows();
  Resize(GetDefaultSize());

  // position relative to the parent's window
  Window_t wdum;
  int ax, ay;

  gVirtualX->TranslateCoordinates(fMain->GetId(), GetParent()->GetId(),
				  (((TGFrame *) fMain)->GetWidth() - fWidth) >> 1,
				  (((TGFrame *) fMain)->GetHeight() - fHeight) >> 1,
				  ax, ay, wdum);
  Move(ax, ay);
  
  MapWindow();

  fOkButton->Connect(    "Released()","TGTabDialog",this,"Ok()");
  fCancelButton->Connect("Released()","TGTabDialog",this,"Cancel()");  

}

TGTabDialog::~TGTabDialog(){

  //delete fListOfTabs;
  //delete fOkButton;
  //delete fCancelButton;
  //delete fBottomFrame;
  //delete fL1;
  //delete fL2;
  //delete fTab;

}

inline void TGTabDialog::HandlePopup(Int_t i) {
    printf("Im here\n");

  Emit("HandlePopup(Int_t)",i);
}

inline void TGTabDialog::Apply() {
  Emit("Apply()");
}

inline void TGTabDialog::Ok() {
  Apply();
  CloseWindow();
}
inline void TGTabDialog::Cancel() {
  CloseWindow();
}
void TGTabDialog::CloseWindow() {
  delete this;
}
