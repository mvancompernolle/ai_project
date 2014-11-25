/** TDSP *******************************************************************
                          TDSPGroupGControl.cpp  -  description
                             -------------------
    begin                : Mon Nov 5 2001
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

#include "TDSPGroupGControl.h"

ClassImpQ(TDSPGroupGControl)

TDSPGroupGControl::TDSPGroupGControl(const TGWindow *parent,
				     UInt_t w, UInt_t h,
				     UInt_t options) : TQQGControl(parent,w,h,options) {
}
TDSPGroupGControl::~TDSPGroupGControl(){
}

void TDSPGroupGControl::Print() {
  TQQGControl::Print();
  TDSPGroupControl::Print();
}

void TDSPGroupGControl::SetObject(TDSPOperator *o) {
  
  TDSPGroupControl::SetObject(o);
  // if we not have already a name give
  // me a default name
  
  const char *name = GetName();
  if (!((name)&&(*name))) {
    TString s = fObject->GetName(); 
    s.Append("_Control");
    SetName(s);
  }
  
  // Connect to the Update slot
  fObject->Connect("Update()",ClassName(),this,"Update()");
  
}
#define kBrowserComponent1 999

void TDSPGroupGControl::ListTree(TGControlList   *list,
				 TGListTreeItem  *parent){



  TDSPOperator      *o   = (TDSPOperator*)GetObject();
  TQQGControl::ListTree(list,parent);

  TGControlList     *cl  = GetControlList();
  TGListTreeItem    *top = GetFirstItem();
 

  if (o&&cl) {

    const TGWindow *win  = cl->GetClient()->GetRoot();

    // Create an Control for the group (as an operator)
    
    if (!fGroupControl) fGroupControl=new TDSPOperatorGControl();
    fGroupControl->SetObject(o);
    fGroupControl->ListTree(cl,top);

    /////////////////////////////////////////////////
    //
    // add an item for the components
    //
    /////////////////////////////////////////////////

    fComponentsListTreeItem = cl->AddItem(GetFirstItem(),"Components");
    TString name = o->GetName();
    if (name.IsNull()||(name.Length()==0)) name = "(no name)";
    cl->SetToolTipItem(fComponentsListTreeItem,"the group \""+name+"\" can contain a list of operators");

    // Add a popup menu for the components

    TGPopupMenu       *p = new TGPopupMenu(win);
    p->AddLabel("components");
    p->AddSeparator();
    p->AddEntry("Browser",kBrowserComponent1);
    p->Connect("Activated(Int_t)",ClassName(),
	       this,"HandlePopup(Int_t)");
    cl->ContextMenu(fComponentsListTreeItem,p);
    cl->SetObject(  fComponentsListTreeItem,GetObject());

    /////////////////////////////////////////////////
    //
    // make the list of the components
    //
    /////////////////////////////////////////////////
    
    TIter next(GetListOfControls());
    while(ControlOperatorConnection *op = (ControlOperatorConnection*)next()){
      TString name      = op->Operator->GetName();
      if ((name.IsNull())||(name.Length()==0)) name = "(no name)";
      TString classname = op->Operator->ClassName();
      TString itemName  = name+" ["+classname+"]";
      TGListTreeItem *c = cl->AddItem(fComponentsListTreeItem,itemName);
      cl->SetToolTipItem(c,"\""+name+"\" is of type "+classname);

      if (!op->Control) {
	// Connecting a default control
	TDSPOperatorGControl *c1=new TDSPOperatorGControl();

	// If one Control needs a Paint all Controls get a Paint 
	//
	c1->Connect("NeedPaint()",this,"Paint()");
	Connect("Paint()",c1,"Paint()");
	//Connect("Paint()",op->Operator,"Paint()");
	
	c1->SetObject(op->Operator);
	op->Control = c1;
	c1->ListTree(cl,c);
      }

      // Popup menues

      TGPopupMenu       *p = new TGPopupMenu(win);
      p->AddLabel(itemName);
      p->AddSeparator();
      p->AddEntry("Browser",kBrowserComponent);
      p->Connect("Activated(Int_t)",ClassName(),
		 this,"HandlePopup(Int_t)");
      cl->ContextMenu(c,p);
      cl->SetObject(c,op->Operator);

    }
  }
}

// Slots


// Handle actions from the PopUp Menus
//

void TDSPGroupGControl::HandlePopup(Int_t id) {

  TGControlList   *cl  = GetControlList();
  TGListTreeItem  *sel = cl->GetSelected();
  const char *name     = sel->GetText();
  
  TDSPOperator    *op = static_cast<TDSPOperator*>(cl->GetObject(sel));
    
  cl->OpenItem(sel);

  switch(id) {
    
  case kBrowserComponent:
    new TBrowser("browse",op,op->GetName());
    break;
  case kBrowserComponent1:
    
    TDSPGroup    *op1 = static_cast<TDSPGroup*>(cl->GetObject(sel));
    
    new TBrowser("browse",op1,op1->GetName());
    break;

  }
}

