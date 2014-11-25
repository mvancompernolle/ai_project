/** TDSP *******************************************************************
                          TGControlList.cpp  -  description
                             -------------------
    begin                : Wed Nov 7 2001
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

#include "TGControlList.h"


ClassImpQ(TGControlList)

TGControlList::TGControlList(TGWindow *parent,UInt_t w,UInt_t h,UInt_t o)  :
  TGListTree(parent,w,h,o) {
  Connect("DoubleClicked(TGListTreeItem*,Int_t)","TGControlList",
	  this,"ToggleItem(TGListTreeItem*, Int_t)");
  Connect("Clicked(TGListTreeItem*,Int_t)","TGControlList",
	  this,"ClickItem(TGListTreeItem*, Int_t)");
}

TGControlList::~TGControlList(){
}


TGControlListItemUserData * TGControlList::UserData(TGListTreeItem* item) {
  
  TGControlListItemUserData *userdata;
  
  if (item) {
    if (!(userdata = (TGControlListItemUserData*)item->GetUserData())) {
      userdata = new TGControlListItemUserData();
      item->SetUserData(userdata);
    }  
  }
  return userdata;
}
void TGControlList::ContextMenu(TGListTreeItem* item,TGPopupMenu* popup) {
  if (item) {
    TGControlListItemUserData *userdata = UserData(item);
    userdata->SetContextMenu(popup);
  }
}

void TGControlList::SetObject(TGListTreeItem* item,void* p) {
  
  if (item) {
    TGControlListItemUserData *userdata = UserData(item);
    userdata->SetObject(p);
  }
}

Bool_t TGControlList::HandleButton(Event_t *e) {

  fEvent = e;
  return TGListTree::HandleButton(e);
  
}

// The Slots!!

void TGControlList::ToggleItem(TGListTreeItem* item ,Int_t) {
  if (!item->IsOpen()) 
    CloseItem(item);
  else
    OpenItem(item);
  
  fClient->NeedRedraw(this);
}

void TGControlList::ClickItem(TGListTreeItem* item ,Int_t button) {
  if (button == kButton3) OpenContextMenu(item);
}

void TGControlList::OpenContextMenu(TGListTreeItem* item) {
  
  TGControlListItemUserData *userdata = (TGControlListItemUserData*)item->GetUserData();
  if (userdata) {
    userdata->GetContextMenu()->PlaceMenu(fEvent->fXRoot,fEvent->fYRoot,1,1);
    fClient->NeedRedraw(this);
  }

}
