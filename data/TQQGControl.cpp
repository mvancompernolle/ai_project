/** TDSP *******************************************************************
                          TQQGControl.cpp  -  description
                             -------------------
    begin                : Wed Oct 31 2001
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

#include "TQQGControl.h"

ClassImpQ(TQQGControl)


TQQGControl::TQQGControl(const TGWindow *parent, UInt_t w,
			   UInt_t h, UInt_t options) 
{  
  if (parent) 
    fCanvas = new TGCanvas(parent,w,h,options);
}

TQQGControl::~TQQGControl(){
}




void TQQGControl::ListTree(TGControlList  *list,
			    TGListTreeItem *parent) {
  
  if ((!list)&&(fCanvas)) {
    fControlList = new TGControlList(fCanvas->GetViewPort(),
				     500,500,kHorizontalFrame);
    fControlList->SetCanvas(fCanvas);

    fCanvas->SetContainer(fControlList);

  }  else
    fControlList = list;
  
  if (!fControlList) return;

  if (!parent)
    fTopItem     = fControlList->GetFirstItem();
  else
    fTopItem     = parent;


}



