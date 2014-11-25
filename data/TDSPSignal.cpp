/** TDSP *******************************************************************
                          TDSPSignal.cpp  -  description
                             -------------------
    begin                : Thu Nov 1 2001
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


#include <TROOT.h>
#include <TVirtualPad.h>
#include "TDSPSignal.h"

//_____________________________________________________________
// TDSPSignal
//
// This is base implementation of a complex signal
//_____________________________________________________________

ClassImp(TDSPSignal)

TDSPSignal::TDSPSignal(Int_t len) : TDSPMatrix(1,len) {
}

TDSPSignal::TDSPSignal(Int_t rows, Int_t cols) : TDSPMatrix(rows,cols) {
}

TDSPSignal::TDSPSignal(char *name) : fName(name), TDSPMatrix() {
}

TDSPSignal::~TDSPSignal() {
}


TDSPSignal& TDSPSignal::operator=(TDSPFifo& f) {
  SetLen(f.GetDepth());
  for(register Int_t i=0;i<Num;i++) fVec[i] = f(i);
  return (*this);
}

istream &operator>>(istream &is,TDSPSignal &z) {
  is >> static_cast<TDSPMatrix*>(&z);
  return ( is );
}

ostream &operator<<(ostream &os,TDSPSignal &z) {
  os << static_cast<TDSPMatrix*>(&z);
  return ( os );
}
  

istream &operator>>(istream &is,TDSPSignal *z) {
  is >> *z;
  return ( is );
}
ostream &operator<<(ostream &os,TDSPSignal *z) {
  os << *z;
  return ( os );
}


void TDSPSignal::SetName(const char *name)
{
   // Change (i.e. set) the name of the TDSPSignal.
   // WARNING: if the object is a member of a THashTable or THashList container
   // the container must be Rehash()'ed after SetName(). For example the list
   // of objects in the current directory is a THashList.

   fName = name;
   if (gPad && TestBit(kMustCleanup)) gPad->Modified();
}

//______________________________________________________________________________
void TDSPSignal::SetNameTitle(const char *name, const char *title)
{
   // Change (i.e. set) all the TDSPSignal parameters (name and title).
   // WARNING: if the name is changed and the object is a member of a
   // THashTable or THashList container the container must be Rehash()'ed
   // after SetName(). For example the list of objects in the current
   // directory is a THashList.

   fName  = name;
   fTitle = title;
   if (gPad && TestBit(kMustCleanup)) gPad->Modified();
}

//______________________________________________________________________________
void TDSPSignal::SetTitle(const char *title)
{
   // Change (i.e. set) the title of the TDSPSignal.

   fTitle = title;
   if (gPad && TestBit(kMustCleanup)) gPad->Modified();
}


//______________________________________________________________________________
void TDSPSignal::ls(Option_t *) const
{
   // List TDSPSignal name and title.

  TROOT::IndentLevel();
  cout <<"OBJ: " << IsA()->GetName() << "\t" << GetName() << "\t" << GetTitle() << " : "
       << Int_t(TestBit(kCanDelete)) << " at: "<<this<< endl;
}

//______________________________________________________________________________
void TDSPSignal::Print(Option_t *) {
   // Print TDSPSignal name and title.

   cout <<"OBJ: " << IsA()->GetName() << "\t" << GetName() << "\t" << GetTitle() << endl;
   this->TDSPMatrix::Print();
}
