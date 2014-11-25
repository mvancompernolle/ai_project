/** TDSP *******************************************************************
                          TMatlab.cpp  -  description
                             -------------------
    begin                : Mon Jan 7 2002
    copyright            : (C) 2002 by Peter Jung
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


#ifdef WIN32
#  include "../../config.h"
#else
#  include "config.h"
#endif

#ifdef HAVE_MATLAB
# define V5_COMPAT
# include <engine.h>
#else
typedef void Engine;
#endif

#include <Riostream.h>
#include "TMatlab.h"

ClassImpQ(TMatlab)

TMatlab::TMatlab(){
#ifdef HAVE_MATLAB
  if (!(fEngine = engOpen("\0"))) {
    Error("TMatlab","Can not start Matlab engine\n");
    fEngine = NULL;
  }

  fOutputBuffer = NULL;
#else
  Error("TMatlab","not compiled with matlab");
#endif
}
TMatlab::~TMatlab(){
#ifdef HAVE_MATLAB
  if (fEngine) engClose((Engine*)fEngine);
  if (fOutputBuffer) delete [] fOutputBuffer;
#endif
}

Int_t    TMatlab::Put(TmxArray *a) {
#ifdef HAVE_MATLAB
  return engPutArray((Engine*)fEngine,(mxArray*)a->Getmx());
#else
  return NULL;
#endif
}

TmxArray* TMatlab::Get(const char *name, TmxArray *a) {
#ifdef HAVE_MATLAB
  if (!a) a = new TmxArray(name);
  a->Setmx(engGetArray((Engine*)fEngine,name));
  return a;
#else
  return NULL;
#endif
}

Bool_t    TMatlab::EvalString(const char* va_(fmt), ...)
{
  
#ifdef HAVE_MATLAB
  char buf[1024];
  
  if (!fEngine) return 0;
  
  va_list ap;
  va_start(ap,va_(fmt));
  vsprintf(buf, fmt, ap); 
  
  return ((fEvalReturn=engEvalString((Engine*)fEngine,buf)) == 0);
#else
  return kFALSE;
#endif
}

Int_t TMatlab::OutputBuffer(char* buffer, Int_t buflen) { 
#ifdef HAVE_MATLAB
  if (!fEngine) return 0;
 
  if (fOutputBuffer) delete [] fOutputBuffer;

  if (!buffer) {
    buffer = new char[buflen];
  }
  
  fOutputBuffer = buffer;

  return engOutputBuffer((Engine*)fEngine, buffer,buflen);
#else
  return 0;
#endif
}

Bool_t TMatlab::Eval(const char* va_(fmt), ...) { 
#ifdef HAVE_MATLAB
  char buf[1024];

  if (!fEngine) return 0;
  
  va_list ap;
  va_start(ap,va_(fmt));
  vsprintf(buf, fmt, ap); 
  
  if (!fOutputBuffer) OutputBuffer();
  fEvalReturn = engEvalString((Engine*)fEngine,buf);
  cout << fOutputBuffer+2 << endl;
  return (fEvalReturn == 0);
#else
  return kFALSE;
#endif
}


#define IN_BUF 256
istream &operator>>(istream &is,TMatlab &z) {
#ifdef HAVE_MATLAB

  char buf[IN_BUF];

  while(!is.eof()) {
    is.getline(buf, IN_BUF);
    z.Eval(buf);
  }
#endif  
  return ( is );

}

TMatlab* TMatlab::GlobalEngine() {
#ifdef HAVE_MATLAB
  if (!gMatlab) 
    gMatlab = new TMatlab();
  
  gMatlab->EvalString("clear;");

  return gMatlab;
#else
  return NULL;
#endif
}

TMatlab *gMatlab = NULL;

