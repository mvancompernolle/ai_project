//////////////////////////////////////////////////////////////////////////////
// $Id: Byte-test.cc,v 1.7 2000/09/04 18:59:24 nthiery Exp $
// Name:	Byte-test.cc
// Purpose:	Class Byte for very small non negative integers
//		(Test suite)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/08
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include <strstream>
#include <string>

#include "TestsUtils.h"
#include "TestsUtils_tmpl.h"
#include "GLIP/Byte.h"

using GLIP::ByteBinOp;
using GLIP::ByteConv;
using GLIP::Byte;

template<class Byte>
void test_arithmetic();

int main () {
  test_start("class ByteBinOp");
  test_arithmetic<ByteBinOp>();
  
  test_start("class ByteConv");
  test_arithmetic<ByteConv>();

  test(ByteBinOp(1)+ByteConv(1),	2,	"Full Mix");
  
  return(test_summary());
}

template<class Byte>
void test_arithmetic() {
  Byte a;
  test(a,			0,		"Constructor 1");
  test(Byte(255),		255,		"Constructor 2");

  a=Byte(255);
  test(a,			255,		"Assignment 1");
  // Should this operation be allowed, when int(0)=1 is forbidden ?
  // Any way, I don't know how to forbid it.
  test(Byte(0)=1,		1,		"Assignment 2");
 
  test(Byte(7)+Byte(3),		Byte(10),	"Operator + 1");
  test(Byte(7)+255,		262,		"Operator + 2");
  test(Byte(7)-Byte(3),		Byte(4),	"Operator - 1");
  test(Byte(7)-10,		-3,		"Operator - 2");
  test(Byte(7)*Byte(3),		Byte(21),	"Operator *");
  test(Byte(7)/Byte(3),		Byte(2),	"Operator / 1");
  test(Byte(7)/3,		2,		"Operator / 2");
  test(Byte(7)/float(2),	float(3.5),	"Operator / 3");
  test(Byte(7)%Byte(3),		Byte(1),	"Operator %");
 
  a=Byte(0);
  test(++a,			Byte(1),	"Operator left ++ 1");
  test(a,			Byte(1),	"Operator left ++ 2");
  test(++(++a),			Byte(3),	"Operator left ++ 3");
  test(a,			Byte(3),	"Operator left ++ 4");
  test(--(--a),			Byte(1),	"Operator left -- 1");
  test(a,			Byte(1),	"Operator left -- 2");
  test(a++,			Byte(1),	"Operator right ++ 1");
  test(a,			Byte(2),	"Operator right ++ 2");
  test(a--,			Byte(2),	"Operator right -- 1");
  test(a,			Byte(1),	"Operator right -- 2");
  
  test(Byte(10) == 10,		true,		"Operator ==");
  test(Byte(10) == Byte(10),	true,		"Operator ==");
  test(Byte(10) == Byte(11),	false,		"Operator ==");
  test(Byte(10) != Byte(10),	false,		"Operator !=");
  test(Byte(10) != Byte(11),	true,		"Operator !=");
  test(Byte(10) <  Byte(11),	true,		"Operator < 1");
  test(Byte(11) <  Byte(11),	false,		"Operator < 2");
  test(Byte(12) <  Byte(11),	false,		"Operator < 3");
  test(Byte(10) >  Byte(11),	false,		"Operator > 1");
  test(Byte(11) >  Byte(11),	false,		"Operator > 2");
  test(Byte(12) >  Byte(11),	true,		"Operator > 3");
  test(Byte(10) <= Byte(11),	true,		"Operator <= 1");
  test(Byte(11) <= Byte(11),	true,		"Operator <= 2");
  test(Byte(12) <= Byte(11),	false,		"Operator <= 3");
  test(Byte(10) >= Byte(11),	false,		"Operator >= 1");
  test(Byte(11) >= Byte(11),	true,		"Operator >= 2");
  test(Byte(12) >= Byte(11),	true,		"Operator >= 3");
  ostrstream o;
  o << Byte(64);
  test(o.str(),			string("64"),	"Operator <<");
  istrstream is("17 ");
  is >> a;
  test(a,			Byte(17),	"Operator >>");
}
