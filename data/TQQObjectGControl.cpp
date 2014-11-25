/** TDSP *******************************************************************
                          TDSPOperatorGControl.cpp  -  description
                             -------------------
    begin                : Tue Nov 6 2001
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

#include <Riostream.h>
#include <TList.h>
#include <TGLabel.h>
#include <TQConnection.h>
#include <TVirtualPad.h>
#include "../TGObjects/TGControlList.h"
#include "TQQObjectGControl.h"

ClassImpQ(TQQObjectGControl)


void TQQObjectGControl::SetObject(TQQObject *o) {
  TQQObjectControl::SetObject(o);
  
  // if we not have already a name give
  // me a default name
  
  const char *name = GetName();
  if (!((name)&&(*name))) {
    TString s = fObject->GetName(); 
    s.Append("_Control");
    SetName(s);
  }
  
  // Connect to the Update slot
  // (this is needed if something has been changed in the object 
  //  and this is somehow visible in the control - so we have to update
  //  the control)
  fObject->Connect("Update()",ClassName(),this,"Update()");
  
  // Connect the Paint Signal
  // (this is useful if the control is something changing or starting in the objects
  //  and the objects have other displays like open histograms. Then this will call
  //  a paint on the object (if connected) - and the objects know its other components
  //  (connected histograms etc.) and they get updated.)
  Connect("Paint()",fObject,"Paint()");
}
  
// Put the Connection list "connections"
// into a ControlList "cl" starting at item "item"
//
Bool_t  TQQObjectGControl::UpdateConnections(TGControlList  *cl,
					     TGListTreeItem *item,
					     char *signal) {
  
  // Bool_t yesno = kFALSE;
  Bool_t yesno = kTRUE;
  
  const TGWindow *win  = cl->GetClient()->GetRoot();

  TList *connections = GetObject()->FindSignal(signal);

  TIter next(connections);
  
  // Loop over the Connections
  //
  while(TQConnection *s = (TQConnection*)next()) {
    
    
    char *str  = GetObject()->ConnectionToString(signal, s);

    // This part is not exacly correct - fix later ...
    
    TQQObject* rec = (TQQObject*)s->GetReceiver();
    const char *name = rec->GetName();

    // the object must be inherit from ...
    
    //if (rec->InheritsFrom(fVisibleBaseClass)) yesno=kTRUE; else continue;
    Info("UpdateConnections","fVisibleBaseClass disabled !");

    if (!cl->FindChildByName(item,str)) {
      // Add a Popup menu to each Connection
      TGPopupMenu       *p = new TGPopupMenu(win);
      TGListTreeItem *i = cl->AddItem(item,str);
  
      p->AddLabel(name);
      p->AddSeparator();
      p->AddEntry("Disconnect",kDelConnection);
      p->Connect("Activated(Int_t)","TQQObjectGControl",
		 this,"HandlePopup(Int_t)");
      cl->ContextMenu(i,p);
    }
  }
  
  return yesno;

}

// Put the send-rec-list from one delegation
// into a ControlList "cl" starting at item "item"
//
void  TQQObjectGControl::UpdateDelegation(TGControlList  *cl,
					  TGListTreeItem *item,
					  char *delegation) {
  
  const TGWindow *win  = cl->GetClient()->GetRoot();

  TQQDelegation *Delegation = GetObject()->FindDelegation(delegation);
  
  // Create the Sender list
  //
  char *chSenders = "Senders";
  TGListTreeItem *senders;
  
  if (!(senders = cl->FindChildByName(item,chSenders)))
    senders = cl->AddItem(item,chSenders);
  
  // Loop over the Senders

  TIter next(Delegation->senders);  
  while(TQQDelegated_Sender *s = (TQQDelegated_Sender*)next()) {
        
    char *name = s->ToString();
    
    if (!cl->FindChildByName(senders,name)) {
      // Add a Popup menu to each Sender
      TGPopupMenu       *p = new TGPopupMenu(win);
      TGListTreeItem *i = cl->AddItem(senders,name);
      p->AddLabel(name);
      p->AddSeparator();
      //p->AddEntry("Disconnect",kDelConnection);
      p->Connect("Activated(Int_t)","TQQObjectGControl",
		 this,"HandlePopup(Int_t)");
      cl->ContextMenu(i,p);
    }

    delete name;
  } 

  // Create the Receivers list
  //
  char *chReceivers = "Receivers";
  TGListTreeItem *receivers;
  
  if (!(receivers = cl->FindChildByName(item,chReceivers)))
    receivers = cl->AddItem(item,chReceivers);
  
  // Loop over the Receivers

  TIter next1(Delegation->receivers);  
  while(TQQDelegated_Receiver *s = (TQQDelegated_Receiver*)next1()) {
        
    char *name = s->ToString();
    
    if (!cl->FindChildByName(senders,name)) {
      // Add a Popup menu to each Receiver
      TGPopupMenu       *p = new TGPopupMenu(win);
      TGListTreeItem *i = cl->AddItem(receivers,name);
      p->AddLabel(name);
      p->AddSeparator();
      //p->AddEntry("Disconnect",kDelConnection);
      p->Connect("Activated(Int_t)","TQQObjectGControl",
		 this,"HandlePopup(Int_t)");
      cl->ContextMenu(i,p);
    }

    delete name;
  }
}


void  TQQObjectGControl::Update() {
  
  TQQObject   *o       = NULL;
  TGControlList  *cl   = GetControlList();
  TGListTreeItem *top  = GetFirstItem();
  const TGWindow *win  = cl->GetClient()->GetRoot();
  
  o=GetObject();
  
  if (o) {
    

    TString name = o->GetName();
    if (name.IsNull()||(name.Length()==0)) name = "(no name)";
    ////////////////////////////////////////////////////////
    //
    // the Signals
    //
    

    if (!fSignalsListTreeItem) {
      fSignalsListTreeItem = cl->AddItem(top,"Signals");
    }
    cl->SetToolTipItem(fSignalsListTreeItem,"The list of signals \""+name+"\" can emit");
    TIter next1(o->GetListOfSignals());
    while(TList *s = (TList*)next1()) {
      char* name = (char*)s->GetName();
      if (!cl->FindChildByName(fSignalsListTreeItem,name)) {
	TGListTreeItem *item = cl->AddItem(fSignalsListTreeItem,name);
	cl->HighlightItem(item);
   	TGPopupMenu       *p = new TGPopupMenu(win);
	p->AddLabel(name);
	p->AddSeparator();
	p->AddEntry("Edit",kEditSignal);
	p->AddEntry("Connect to",kAddConnection);
	p->AddEntry("Emit",kEmitSignal);
	p->Connect("Activated(Int_t)","TQQObjectGControl",
		   this,"HandlePopup(Int_t)");
	cl->ContextMenu(item,p);
	if (!UpdateConnections(cl,item,name)) cl->DeleteItem(item);
      } 

    }
    
    // Because the popups containing the object name
    // rebuild them every update

    if (fSignalsListPopup) delete fSignalsListPopup;
    fSignalsListPopup = new TGPopupMenu(win);
    fSignalsListPopup->AddLabel("\""+name+"\" Signal list");
    fSignalsListPopup->AddSeparator();
    fSignalsListPopup->AddEntry("Add Signal",kAddSignal);
    fSignalsListPopup->DisableEntry(kAddSignal); // Disabled (not implemented)
    fSignalsListPopup->Connect("Activated(Int_t)","TQQObjectGControl",
			       this,"HandlePopup(Int_t)");
    cl->ContextMenu(fSignalsListTreeItem,fSignalsListPopup);
    


  
    ////////////////////////////////////////////////////////
    //
    // the Slots
    //
    if (!fSlotsListTreeItem) {
      fSlotsListTreeItem   = cl->AddItem(top,"Slots");  
    }
    cl->SetToolTipItem(fSlotsListTreeItem,"The list of slots on which \""+name+"\" can listen and receive data");
    TIter next(o->GetListOfSlots());
    while(TList *s = (TList*)next()) {
      const char* name = s->GetName();
      if (!cl->FindChildByName(fSlotsListTreeItem,name)) {
	TGListTreeItem *item = cl->AddItem(fSlotsListTreeItem,name);
	TGPopupMenu       *p = new TGPopupMenu(win);
	p->AddLabel(name);
	p->AddSeparator();
	p->AddEntry("Edit",kEditSlot);
	p->AddEntry("Call",kCallSlot);
	p->Connect("Activated(Int_t)","TQQObjectGControl",
		   this,"HandlePopup(Int_t)");
	cl->ContextMenu(item,p);
      }
    }
    // Because the popups containing the object name
    // rebuild them every update

    if (fSlotsListPopup) delete fSlotsListPopup;
    fSlotsListPopup = new TGPopupMenu(win);
    fSlotsListPopup->AddLabel("\""+name+"\" Slot list");
    fSlotsListPopup->AddSeparator();
    fSlotsListPopup->AddEntry("Add Slot",kAddSlot);
    fSlotsListPopup->DisableEntry(kAddSlot); // Disabled (not implemented)
    fSlotsListPopup->Connect("Activated(Int_t)","TQQObjectGControl",
			     this,"HandlePopup(Int_t)");
    cl->ContextMenu(fSlotsListTreeItem,fSlotsListPopup);
    
    
    ////////////////////////////////////////////////////////
    //
    // the Delegations
    //
    if (!fDelegationsListTreeItem) {
      fDelegationsListTreeItem    = cl->AddItem(top,"Delegations");  
    }
    cl->SetToolTipItem(fDelegationsListTreeItem,"The list of Delegations of \""+name+"\"");
    
    TIter next2(o->GetListOfDelegations());
    while(TQQDelegation *s = (TQQDelegation*)next2()) {
      const char* name = s->GetName();
      if (!cl->FindChildByName(fDelegationsListTreeItem,name)) {
	TGListTreeItem *item = cl->AddItem(fDelegationsListTreeItem,s->GetName());
	TGPopupMenu       *p = new TGPopupMenu(win);
	p->AddLabel(name);
	p->AddSeparator();
	p->AddEntry("Edit",kEditDelegation);
	p->Connect("Activated(Int_t)","TQQObjectGControl",
		   this,"HandlePopup(Int_t)");
	cl->ContextMenu(item,p);
	UpdateDelegation(cl,item,(char*)name);
      }
    }

    // Because the popups containing the object name
    // rebuild them every update
    
    if (fDelegationsListPopup) delete fDelegationsListPopup;
    
    fDelegationsListPopup = new TGPopupMenu(win);
    fDelegationsListPopup->AddLabel("\""+name+"\" Delegation list");
    fDelegationsListPopup->AddSeparator();
    fDelegationsListPopup->AddEntry("Add Delegation",kAddDelegation);
    fDelegationsListPopup->Connect("Activated(Int_t)","TQQObjectGControl",
				   this,"HandlePopup(Int_t)");
    
    cl->ContextMenu(fDelegationsListTreeItem,fDelegationsListPopup);

    cl->GetClient()->NeedRedraw(cl);
    
  }
  

}



void TQQObjectGControl::ListTree(TGControlList  *list,
				    TGListTreeItem *parent) {

  
  TQQGControl::ListTree(list,parent);
  Update();
  
}

TQQObjectGControl::TQQObjectGControl(const TGWindow *parent,
					   UInt_t w, UInt_t h,
					   UInt_t options) 
  : TQQGControl(parent,w,h,options)
{
  
  fSignalsListTreeItem      = NULL;
  fSlotsListTreeItem        = NULL;
  fDelegationsListTreeItem  = NULL;

  fSignalsListPopup         = NULL;
  fSlotsListPopup           = NULL;
  fDelegationsListPopup     = NULL;

  // Show all TQQObjects

  AddVisibleBaseClass("TQQObject"); 

}


TQQObjectGControl::~TQQObjectGControl(){
}


// Slots


// Handle actions from the PopUp Menus
//
void TQQObjectGControl::HandlePopup(Int_t id) {

  TGControlList   *cl  = GetControlList();
  TGListTreeItem  *sel = cl->GetSelected();
  const char *name     = sel->GetText();

  TGTabDialog       *dialog = NULL;

  cl->OpenItem(sel);

  switch(id) {

  case kAddDelegation:
    name = NULL;
  case kEditDelegation:
    
    dialog = new DelegationDialog(this,
				  (char*)name,
				  cl->GetClient()->GetRoot(), 
				  cl->GetClient()->GetRoot());
   
    break;
    
  case kAddSignal:
    name  = NULL;
  case kEditSignal:
    dialog = new SignalDialog(this,
			      (char*)name,
			      cl->GetClient()->GetRoot(), 
			      cl->GetClient()->GetRoot());
    break;

  case kEmitSignal:
    
    // Checking Arguments !!

    GetObject()->Emit(name);
    break;
    
  case kAddSlot:
    name = NULL;
  case kEditSlot:
    dialog = new SlotDialog(this,
			    (char*)name,
			    cl->GetClient()->GetRoot(), 
			    cl->GetClient()->GetRoot());
    break;

  case kCallSlot:
    Connect("TempSignal",GetObject(),name);
    TempSignal();
    Disconnect("TempSignal",GetObject(),name);
    
    // Some GUI Updates

    NeedPaint();
    if (gPad) gPad->Update();
    
    break;

  case kDelConnection:
    // Get the corresponding signal
    {
      //const char *signal = sel->GetParent()->GetText();
      //TQConnection *c    = GetObject()->StringToConnection((char*)signal,(char*)name);
      
      /*
	if (!GetObject()->Disconnect(signal,c->GetReceiver(),c->GetName())) {
	fprintf(stderr,"Error during disconnect of signal \"%s\" on connection \"%s\"!\n",signal,name);
	} else {
	
	Update();
	
	}
      */
    }
    break;

  case kAddConnection:

    dialog = new ConnectionDialog(this,
				  (char*)name,
				  NULL,
				  cl->GetClient()->GetRoot(), 
				  cl->GetClient()->GetRoot());
    break;
    
  case kEditConnection:

    break;
				  

  }
  if (dialog) {
    dialog->Connect("Apply()",
		    "TQQObjectGControl",
		    this,
		    "Update()");
    dialog->Connect("HandlePopup(Int_t)",
		    "TQQObjectGControl",
		    this,
		    "HandlePopup(Int_t)");
  }
}

///////////////////////////////////////////////////////////////
//
// This makes the Dialog for a connection "connection" of the
// signal "signal"
//
///////////////////////////////////////////////////////////////

ConnectionDialog::ConnectionDialog(TQQObjectGControl   *oc,
				   char*           signal,
				   char*           connection,
				   const TGWindow *p, 
				   const TGWindow *main, UInt_t w, UInt_t h,
				   UInt_t options) :
  TGTabDialog(p,main,w,h,options) {
  
  fObjectControl      = oc;
  fItem               = oc->GetControlList()->GetSelected();
  
  if ((connection)&&strlen(connection)) 
    fConnection           = connection;  // Starting on an existing one
  else 
    fConnection           = NULL;        // Creating a new one
    

  TGTab *tab = GetTab();
  
  TGCompositeFrame *tf  = tab->AddTab("General");
  
  TGCompositeFrame *fF1 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
  
  // widgets for the Delegation name

  TGLayoutHints *fL3    = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);
  fF1->AddFrame(new TGLabel(tf,"Name"),fL3);
  fF1->AddFrame(fTxt1   = new TGTextEntry(tf, new TGTextBuffer(100)), fL3);
  if (fConnection) fTxt1->SetText(fConnection);
  fTxt1->Home();
  tf->AddFrame(fF1, fL3);
  fTxt1->Resize(150, fTxt1->GetDefaultHeight());

  // Set the Window Name

  SetWindowName("Connection");

  // Draw It

  DrawDialog();


}

ConnectionDialog::~ConnectionDialog() {
}

void ConnectionDialog::Apply() {
  
  const char* name = fTxt1->GetText();
  
  if (fConnection&&(strcmp(fConnection,name))) {
    
    //fObjectControl->GetObject()->RenameDelegation((char*)fDelegation,(char*)name);
    // Name changed 
    //if (fItem)        fItem->Rename(name);
    //fObjectControl->GetControlList()->GetClient()->NeedRedraw(fObjectControl->GetControlList());

  } else {

    if (!fConnection) {
      
      // New Delegation
      //if (!fObjectControl->GetObject()->AddDelegation((char*)name)) {
      //fprintf(stderr,"Error : Can not add delegation%s\n",name);
      //}

      // Emit apply so that all other Object can react on
      // the changed delegation
    
      //Emit("Apply()");
    } else { 

      // Only Delegation Properties have been changed

    }
  }
}

///////////////////////////////////////////////////////////////
//
// This makes the Dialog for a delegation "delegation" on the
// the Object "op"
//
///////////////////////////////////////////////////////////////

DelegationDialog::DelegationDialog(TQQObjectGControl   *oc,
				   char*           delegation,
				   const TGWindow *p, 
				   const TGWindow *main, UInt_t w, UInt_t h,
				   UInt_t options) :
  TGTabDialog(p,main,w,h,options) {
  
  fObjectControl      = oc;
  fItem               = oc->GetControlList()->GetSelected();
  
  if ((delegation)&&strlen(delegation)) 
    fDelegation           = delegation;  // Starting on an existing one
  else 
    fDelegation           = NULL;        // Creating a new one
    

  TGTab *tab = GetTab();
  
  TGCompositeFrame *tf  = tab->AddTab("General");
  
  TGCompositeFrame *fF1 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
  
  // widgets for the Delegation name

  TGLayoutHints *fL3    = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);
  fF1->AddFrame(new TGLabel(tf,"Name"),fL3);
  fF1->AddFrame(fTxt1   = new TGTextEntry(tf, new TGTextBuffer(100)), fL3);
  if (fDelegation) fTxt1->SetText(fDelegation);
  fTxt1->Home();
  tf->AddFrame(fF1, fL3);
  fTxt1->Resize(150, fTxt1->GetDefaultHeight());

  // Set the Window Name

  SetWindowName("Delegation");

  // Draw It

  DrawDialog();


}

DelegationDialog::~DelegationDialog() {
}

void DelegationDialog::Apply() {
  
  const char* name = fTxt1->GetText();
  
  if (fDelegation&&(strcmp(fDelegation,name))) {
    
    fObjectControl->GetObject()->RenameDelegation((char*)fDelegation,(char*)name);
    // Name changed 
    if (fItem)        fItem->Rename(name);
    fObjectControl->GetControlList()->GetClient()->NeedRedraw(fObjectControl->GetControlList());

  } else {

    if (!fDelegation) {
      
      // New Delegation
      if (!fObjectControl->GetObject()->AddDelegation((char*)name)) {
        Error("Apply","Error : Can not add delegation \"%s\"",name);
      }
      
    } else { 

      // Only Delegation Properties have been changed

    }
  }

  // Emit apply so that all other Object can react on
  // the changed delegation
  
  Emit("Apply()");
}

///////////////////////////////////////////////////////////////
//
// This makes the Dialog for a signal "signal" on the
// the Object "op"
//
///////////////////////////////////////////////////////////////

SignalDialog::SignalDialog(TQQObjectGControl *oc,
			   char*           signal,
			   const TGWindow *p, 
			   const TGWindow *main, UInt_t w, UInt_t h,
			   UInt_t options) :
  TGTabDialog(p,main,w,h,options) {

  TGCompositeFrame *tf;

  fObjectControl      = oc;
  fItem               = oc->GetControlList()->GetSelected();

  if ((signal)&&strlen(signal)) 
    fSignal           = signal;      // Starting on an existing one
  else 
    fSignal           = NULL;        // Creating a new one
  
  
  TGTab *tab = GetTab();

  // The General Tab
  //
  
  tf  = tab->AddTab("General");
 
  TGCompositeFrame *fF1 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
  
  // widgets for the Signal name
  //

  TGLayoutHints *fL3    = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);
  fF1->AddFrame(new TGLabel(tf,"Name"),fL3);
  fF1->AddFrame(fTxt1            = new TGTextEntry(tf, new TGTextBuffer(100)), fL3);
  if (fSignal) fTxt1->SetText(fSignal);
  //  fTxt1->SetEnabled(kFALSE);
  fTxt1->Home();
  tf->AddFrame(fF1, fL3);
  fTxt1->Resize(150, fTxt1->GetDefaultHeight());

  // The Connections Tab
  //
  
  tf  = tab->AddTab("Connections");
  TGCompositeFrame *fF4 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
  TGLayoutHints    *fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
					    kLHintsExpandY, 5, 5, 5, 5);
  
  if (fSignal) {
    TGControlList *cl1 = new TGControlList(fF4);
    oc->UpdateConnections(cl1,
			  NULL,
			  fSignal);
    tf->AddFrame(fF4,fL4);
    fF4->Resize(cl1->GetDefaultWidth(),
		cl1->GetDefaultHeight());

  }

  
  // Set the Window Name
  //
  SetWindowName("Signal");
  
  // Draw It
  //
  DrawDialog();


}

SignalDialog::~SignalDialog() {
}

void SignalDialog::Apply() {

  const char* name = fTxt1->GetText();

 
  if (fSignal&&(strcmp(fSignal,name))) {
    
    // Name changed 

    Emit("Rename(char*)",name);
    Error("Apply","Signal Renaming not yet implemented ! ");

  } else {

    if (!fSignal) {
      
      // New Signal
      
      if (!fObjectControl->GetObject()->AddSignal((char*)name)) {
	Error("Apply","Error : Can not add signal %s",name);
      }

    } else {

      // Only Signal Properties have been changed

    }

  }

  // Emit apply so that all other Object can react on
  // the changed delegation

  Emit("Apply()");
}
///////////////////////////////////////////////////////////////
//
// This makes the Dialog for a slot "slot" on the
// the Object "op"
//
///////////////////////////////////////////////////////////////

SlotDialog::SlotDialog(TQQObjectGControl *oc,
		       char*           slot,
		       const TGWindow *p, 
		       const TGWindow *main, UInt_t w, UInt_t h,
		       UInt_t options) :
  TGTabDialog(p,main,w,h,options) {
  
  fObjectControl      = oc;
  fItem               = oc->GetControlList()->GetSelected();

  if ((slot)&&strlen(slot)) 
    fSlot           = slot;  // Starting on an existing one
  else 
    fSlot           = NULL;  // Creating a new one
    

  TGTab *tab = GetTab();
  
  TGCompositeFrame *tf  = tab->AddTab("General");
  
  TGCompositeFrame *fF1 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
  
  // widgets for the Slot name

  TGLayoutHints *fL3    = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);
  fF1->AddFrame(new TGLabel(tf,"Name"),fL3);
  fF1->AddFrame(fTxt1            = new TGTextEntry(tf, new TGTextBuffer(100)), fL3);
  if (fSlot) fTxt1->SetText(fSlot);
  fTxt1->SetEnabled(kFALSE);
  fTxt1->Home();
  tf->AddFrame(fF1, fL3);
  fTxt1->Resize(150, fTxt1->GetDefaultHeight());

  // Set the Window Name

  SetWindowName("Slot");

  // Draw It

  DrawDialog();


}

SlotDialog::~SlotDialog() {
}

void SlotDialog::Apply() {
  
  const char* name = fTxt1->GetText();
  
  if (fSlot&&(strcmp(fSlot,name))) {
    
    // Name changed 
    Emit("Rename(char*)",name);
    Error("Apply","Slot Renaming not yet implemented !");

  } else {

    if (!fSlot) {
      
      // New Slot
      if (!fObjectControl->GetObject()->AddSlot((char*)name)) {
	Error("Apply","Error : Can not add slot %s",name);
      }

    } else {

      // only slot properties have been changed
    }
    
  }

  // Emit apply so that all other Object can react on
  // the changed delegation

  Emit("Apply()");
}
