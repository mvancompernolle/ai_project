/** TDSP *******************************************************************
                          TQQObject.cpp  -  description
                             -------------------
    begin                : Fri Nov 2 2001
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

#include <iostream>
#include <stdarg.h>
#include <string.h>

#include <TROOT.h>
#include <TList.h>
#include <TCollection.h>
#include <TQConnection.h>
#include <TNamed.h>
#include <TRegexp.h>
#include "TQQObject.h"
#include <TQQGControl.h>
#include <TQQObjectGControl.h>
#include <TQQEditor.h>

using namespace std;

ClassImpQ(TQQDelegation)

ClassImpQ(TQQDelegated_Sender)

char*   TQQDelegated_Sender::ToString() {
  char *s = new char[100];
  //sprintf(s,"%p.%s [%s]",sender,signal,sender->ClassName());

  sprintf(s,"%s.%s [%s] %p",sender->GetName(),signal,sender->ClassName(), sender);
  return s;
}


ClassImpQ(TQQDelegated_Receiver);

char*   TQQDelegated_Receiver::ToString() {
  char *s = new char[100];
  sprintf(s,"%s.%s [%s] %p",receiver->GetName(),slot,receiver->ClassName(), receiver);
  return s;
}


ClassImpQ(TQQObject)

TQQObject::TQQObject() : TQObjSender(), TNamed() {

  fSender = this; // by default there is no delegation
  fListOfSlots=NULL;
  fListOfDelegations=NULL;    
  RegisterSignalsAndSlots();

}
TQQObject::~TQQObject(){
}


char*       TQQObject::ConnectionToString(char *signal, TQConnection*c) {
  char   *s = new char[100];

  TNamed    *t = (TNamed*)c->GetReceiver();
  
  sprintf(s,"%s.%s [%s] %p",t->GetName(),c->GetName(),t->ClassName(),t);
  
  return s;
}

TQConnection* TQQObject::StringToConnection(char *signal, char*c) {
  
  TQConnection *con;
  
  TList *s = FindSignal(signal);
  
  if (!s) return NULL;
  char classname[100];
  void *rec;
  
  TString str(c);

  TRegexp e(".+\\s\\(.+\\)\\s.+");
  Int_t j;
  Int_t i=str.Index(e,&j);
  if (i==-1) return NULL;
  str.Remove(i);
  const char *slot=str.Data();
  return NULL;

  
  return NULL;
  TIter next(s);
  while((con = (TQConnection*)next())) {
    if (con->GetReceiver()!=rec) continue;
    if (strcmp(slot,con->GetName())) continue;
    if (strcmp(classname,con->ClassName())) continue;
  }

  return con;

}
void TQQObject::PrintSignals() {


  TList *s = GetListOfSignals();


  TIter next(s);                           
  while (TList *obj = (TList*)next()) {                                
    cout << ClassName() << " " << this << "." << obj->GetName() << endl; 
    TIter next1(obj);
    while (TQConnection *obj1 = (TQConnection*)next1()) {
      TNamed* rec = (TNamed*)obj1->GetReceiver();
      cout << " +-- " << rec->ClassName() << " " << rec << "." << obj1->GetName() << endl; 
    }
  }
}
void TQQObject::PrintSlots() {


  TList *s = GetListOfSlots();


  TIter next(s);                           
  while (TList *obj = (TList*)next()) {                                
    cout << ClassName() << " " << this << "." << obj->GetName() << endl; 
    TIter next1(obj);

    // this list is not filled - for this we have to change more
    // TQObject/TQConnection - code - but in principle this could
    // be a list of signals connected to this slot 

    while (TQConnection *obj1 = (TQConnection*)next1()) {
      TNamed* rec = (TNamed*)obj1->GetReceiver();
      cout << " +-- " << rec->ClassName() << " " << rec << "." << obj1->GetName() << endl; 
    }
  }
}
void TQQObject::PrintDelegations() {


  TList *s = GetListOfDelegations();


  TIter next(s);                           
  while (TQQDelegation *obj = (TQQDelegation*)next()) {                                
    cout << ClassName() << " " << this << "." << obj->GetName() << endl; 
    // Loop over the senders
    //
    TIter next1(obj->senders);
    Int_t first=1;
    while (TQQDelegated_Sender *obj1 = (TQQDelegated_Sender*)next1()) {
      if (first) { cout << " <senders>" << endl;first=0;};
      char *s;
      cout << "  +-- " << (s=obj1->ToString()) << endl; 
      delete s;
    }

    // Loop over the receivers
    //
    TIter next2(obj->receivers);
    first=1;
    while (TQQDelegated_Receiver *obj1 = (TQQDelegated_Receiver*)next2()) {
      if (first) { cout << " <receivers>" << endl;first=0;};
      char *s;
      cout << "  +-- " << (s=obj1->ToString()) << endl;
      //obj1->slot << " " << rec->ClassName() << " " << rec << "." << obj1->GetName() << endl; 
    }
  }
}

char* TQQObject::AddSignals(Int_t num,...) {

  va_list Signals;
  char   *Signal;
  va_start(Signals,num);      

  for (int i = 0 ; i < num ; i++) {
    Signal = va_arg(Signals, char*);
    AddSignal(Signal);
  }
  va_end(Signals);
 
  // return the last signal name

  return Signal;
 
}


char* TQQObject::AddSignal(char *Signal) {

  TList  *obj = NULL;
  
  // now loop over all Signals we have already in the list
    
  obj = FindSignal(Signal);

  if (obj) return NULL;
  
  // add the signal if it was not found
  
  TList *NewSignal = new TList();
  NewSignal->SetName(Signal);
  if (!fListOfSignals) fListOfSignals = new TList();
  fListOfSignals->Add(NewSignal);
  
  return Signal;
}


char* TQQObject::AddSlots(Int_t num,...) {

  va_list Slots;
  char   *Slot;

  va_start(Slots,num);      

  for (int i = 0 ; i < num ; i++) {
    Slot = va_arg(Slots, char*);
    AddSignal(Slot);
  }
  va_end(Slots);
 
  // return the last signal name

  return Slot;
 
}


char* TQQObject::AddSlot(char *Slot) {

  TList  *obj = NULL;
  
  obj = FindSlot(Slot);

  if (obj) return NULL;
  
  // add the signal if it was not found
  
  TList *NewSlot = new TList();
  NewSlot->SetName(Slot);
  if (!fListOfSlots) fListOfSlots = new TList();
  fListOfSlots->Add(NewSlot);
  
  return Slot;
}

char * TQQObject::AddDelegation(char *d) {
  
  TQQDelegation* del = NULL;

  if (!fListOfDelegations) fListOfDelegations=new TList();

  // Does this Delegation already exists ?

  del = FindDelegation(d);

  // return if this name is already registered

  if (del) return NULL;

  // register this name

  del = new TQQDelegation();
  del->SetName(d);
  del->senders   = NULL;
  del->receivers = NULL;
  
  fListOfDelegations->Add(del);
  return d;

}

char * TQQObject::RenameDelegation(char *name, char* newname) {
  
  TQQDelegation* del = NULL;

  if (!fListOfDelegations) return NULL;

  del = FindDelegation(name);

  // return if this name not exists

  if (!del) return NULL;

  // register this name

  del->SetName(newname);

  return newname;

}

// Add multiple Delegations

char* TQQObject::AddDelegations(Int_t num,...) {

  va_list Delegations;
  char   *Delegation;

  va_start(Delegations,num);      

  for (int i = 0 ; i < num ; i++) {
    Delegation = va_arg(Delegations, char*);
    AddDelegation(Delegation);
  }
  va_end(Delegations);
 
  // return the last signal name

  return Delegation;
 
}


// find the Delegation

TQQDelegation* TQQObject::FindDelegation(const char *name) {
  TIter          next(fListOfDelegations);
  TQQDelegation *d;
  
  while((d = (TQQDelegation*)next())) { 
    if (!strcmp(d->GetName(),name)) break;
  }
  return d;
}

// find the Signal

TList* TQQObject::FindSignal(const char *name) {
  TIter          next(fListOfSignals);
  TList *d;

  while((d = (TList*)next())) 
    if (!strcmp(d->GetName(),name)) break;

  return d;
}

// find the Slot

TList* TQQObject::FindSlot(const char *name) {
  TIter          next(fListOfSlots);
  TList *d;

  while((d = (TList*)next())) 
    if (!strcmp(d->GetName(),name)) break;

  return d;
}

Bool_t TQQObject::Connect(const char *signal,
			  const char *receiver_class,
			  void       *receiver,
			  const char *slot) {

  

  TQQDelegation *Slot_Delegation   = NULL;
  TQQDelegation *Signal_Delegation = NULL;
  

  // Check for a Slot Delegation
  //
  TClass *rcv_cl = gROOT->GetClass(receiver_class);
  if (rcv_cl) {
    // Check that the Class inherits from TQQObject
    //
    if (rcv_cl->InheritsFrom(TQQObject::Class())) {

      // Cast Receiver to TQQObject
      //
      TQQObject *tqqreceiver = (TQQObject*)rcv_cl->DynamicCast(TQQObject::Class(),receiver,kTRUE);
      
      // Execute FindDelegation on that Object
      //
      if (tqqreceiver) {
	Slot_Delegation = (TQQDelegation*)tqqreceiver->FindDelegation(slot);
      } else {
	Error("Connect","Error during up cast to TQQObject");
      }
    }
  }
  
  // Check for a Signal Delegation
  Signal_Delegation = FindDelegation(signal);

  // the normal case 

  if ((!Slot_Delegation)&&(!Signal_Delegation))
    return TQObject::Connect(signal,receiver_class, receiver, slot);

  
  if (Slot_Delegation) {
  
    // in the case of Slot_Delegations
    // we have to add a new sender and connect it to all
    // registered receivers
    
    if (!Slot_Delegation->senders) Slot_Delegation->senders = new TList();
    
    // first make sure that we are not already on the list
    
    if (!Slot_Delegation->senders->FindObject(this)) {
      
      // Add me onto the list
      //
      TQQDelegated_Sender *tmp = new TQQDelegated_Sender();
      tmp->signal = strdup(signal);
      tmp->sender = this;
      Slot_Delegation->senders->Add(tmp);
      
      if (Slot_Delegation->receivers) {
	
	// Loop over all receivers and make connections
	//
	TIter next(Slot_Delegation->receivers);
	while(TQQDelegated_Receiver* delegated_receiver = (TQQDelegated_Receiver*)next()){
	  // Here we use the same connect method
	  // because the slot is already expanded but not
	  // the signal !!
	  Connect(signal,
		  delegated_receiver->receiver_class, 
		  delegated_receiver->receiver, 
		  delegated_receiver->slot);
	}	
      }
    }
    return kTRUE;
  }
   
  
  
  // in the case of Signal Delegations
  // we have to add a new receiver and connect it to all
  // registered senders
    
  if (!Signal_Delegation->receivers) Signal_Delegation->receivers = new TList();
    
  // first make sure that we are not already on the list
    
  if (!Signal_Delegation->receivers->FindObject(this)) {
      
    // Add me onto the list
    //
    TQQDelegated_Receiver *tmp = new TQQDelegated_Receiver();
    tmp->receiver_class = strdup(receiver_class);
    tmp->receiver       = (TQQObject*)receiver;
    tmp->slot           = strdup(slot);
    
    Signal_Delegation->receivers->Add(tmp);
    
    if (Signal_Delegation->senders) {
      
      // Loop over all senders and make connections
      //
      TIter next(Signal_Delegation->senders);
      while(TQQDelegated_Sender* delegated_sender = (TQQDelegated_Sender*)next()){
	delegated_sender->sender->Connect(delegated_sender->signal,
					  receiver_class, 
					  receiver, 
					  slot);
      }	
    }
  }
  return kTRUE; 
}

// Later ...

Bool_t  TQQObject::Connect(const char *sender_class,
				  const char *signal,
				  const char *receiver_class,
				  void *receiver,
				  const char *slot) {
  return TQObject::Connect(sender_class,signal,receiver_class, receiver, slot);
}


void TQQObject::Browse(TBrowser *b) {
  if (b) {
    b->Add(fListOfSignals,    "fListOfSignals");
    b->Add(fListOfSlots,      "fListOfSlots");
    b->Add(fListOfDelegations,"fListOfDelegations");
  } else 
    TNamed::Browse(b);
}

TQQGControl* TQQObject::NewGControl() {
  TQQObjectGControl *o = new TQQObjectGControl();
  o->SetObject(this);
  return  o;
}


void TQQObject::Edit() {
  // Opens a standard Viewer
  TQQEditor*         e = new TQQEditor(NewGControl(),gClient->GetRoot(),500,500);
}
