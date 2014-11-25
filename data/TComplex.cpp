//----------Author's Name:Francois-Xavier Gentit  DAPNIA/SPP CEN Saclay
//----------Modified:1/12/99
//----------Copyright:Those valid for CNRS sofware

#include <Riostream.h>
#include <stdlib.h>
#include "TComplex.h"


ClassImp(TComplex)
//______________________________________________________________________________
//
// Implement complex numbers in ROOT, with many operations using complex
//numbers. Look at all the methods for documentation.
//
//______________________________________________________________________________

void TComplex::Print() {
//Prints the complex number
  cout << *this << endl;;
}
void TComplex::Dump() {
  cout << "{ fRe=" << fRe << ", fIm=" << fIm << endl;  
}
 
void TComplex::Test(Double_t x,const TComplex &z2) {
  TComplex z4(0,0);
  z4 = x + z2;
  cout << '(' << z4.fRe << ',' << z4.fIm << ')' << endl;
}
istream &operator>>(istream &is,TComplex &z) {
//Allows to input a complex number
#define MAXLEN 100
  char buf[MAXLEN];
  char *b=buf,*end;

  char *Re=NULL;
  char *Im=NULL;

  // read until a space or newline
  //
  while((!is.eof())&&((b-buf)<MAXLEN)) {
    *b=is.get();
    if ((*b=='\n')||
	(*b==' ')) 
      break;
    b++;
  }
  // Finalize the string
  //
  *b=0;

  // thats the real part
  //
  Re = buf;

  // Find a comma
  //
  b=buf;
  while((*b)&&(*b!=',')) b++;
  
  // thats the Imag part
  //
  if (*b) {*b=0;Im = b+1;} 
  
  // This was at least a real number in stream
  //
  z.fRe = strtod(Re,&end);
  if (end == Re) {
    // This was not a complex number in input
    //
#ifdef WIN32
#else
    is.setstate(ios::failbit); // msvc++ does not like ANSI
#endif 
 } else {
    if (Im) z.fIm = atof(Im); else z.fIm=0;
  
  }
  return ( is );
}

ostream &operator<<(ostream &os,TComplex &z) {
//Allows to output a complex number
#ifdef __GNUC__
# if (__GNUC__>=3)
  std::_Ios_Fmtflags old_flags = os.flags(ios::fixed|ios::showpos);
# else
  long old_flags = os.flags(ios::fixed|ios::showpos);
# endif
#endif
  os.precision(3);
  os.width(7);
  os.unsetf(ios::showpos);
  os << z.fRe;
  os.width(6);
  os.setf(ios::showpos);
  if (z.fIm) { 
    os << z.fIm << "i";
  } else os << "" << " ";
#ifdef __GNUC__
  os.flags(old_flags);
#endif
  return ( os );
}

void TComplex::Streamer(TBuffer &R__b) {
  // Stream an object of class TComplex.
  
  UInt_t R__s, R__c;
  if (R__b.IsReading()) {
    Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
    R__b >> fRe;
    R__b >> fIm;
    R__b.CheckByteCount(R__s, R__c, TComplex::IsA());
  } else {
    R__c = R__b.WriteVersion(TComplex::IsA(), kTRUE);
    R__b << fRe;
    R__b << fIm;
    R__b.SetByteCount(R__c, kTRUE);
  }
}


TComplex       ComplexZero = TComplex(0,0);
TComplex       ComplexOne  = TComplex(1,0);
TComplex       I           = TComplex(0,1);
