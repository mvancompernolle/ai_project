/** TDSP *******************************************************************
                          TDSPInt.cpp  -  description
                             -------------------
    begin                : Wed Dec 12 2001
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
#include <TROOT.h>
#include <TError.h>
#include "TDSPVector.h"
#include "TDSPfft.h"
#include "TDSPInt.h"

#ifndef WIN32
# include <sys/poll.h>
#endif

#ifdef WIN32
# include "../../config.h"
#else
# include "config.h"
#endif


ClassImp(TDSPInt)

TDSPInt::TDSPInt(const char* appClassName, 
		 int* argc, char** argv,
		 void* options, 
		 int numOptions,
		 Bool_t noLogo) : TRint(appClassName, argc, argv, options, numOptions, kTRUE) {

  SetPrompt("tdsp [%d] > ");
  if (!noLogo) PrintLogo();
}
TDSPInt::~TDSPInt(){
}

void TDSPInt::PrintLogo() {
  cout << "Welcome to TDSP " << GetVersion().Data() << " - based on ROOT v" << gROOT->GetVersion() << " (jung@hhi.de)" << endl;
  cout << "----- compiled with : ";
  if (TDSPfft::UsingFFTW()) 
    cout << "fftw ";

#ifdef HAVE_MATLAB
  cout << "matlab ";
#endif
#ifdef HAVE_OCTAVE
# include <octave/version.h>
  cout << "octave-" << OCTAVE_VERSION << " ";
#endif
  if (TDSPVector::UsingATLAS()) 
    cout << "atlas ";
#ifdef HAVE_CBLAS
  cout << "cblas ";
#endif
  cout << "----" << endl;
}

TString TDSPInt::fVersionStr = TString(VERSION);

Bool_t CheckEnter() {

#ifndef WIN32  
  struct pollfd d;
  //d.fd=STDIN_FILENO;
  d.fd=0;
  d.events=POLLIN|POLLPRI;
  d.revents=0;
  poll(&d,1,0);
  
  return d.revents&(POLLPRI|POLLIN);
#else
  Error("CheckEnter","Not yet implemented for Win32");
  return kFALSE;
#endif
} 
