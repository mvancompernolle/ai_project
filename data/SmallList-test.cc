//////////////////////////////////////////////////////////////////////////////
// $Id: SmallList-test.cc,v 1.13 2000/09/06 04:58:45 nthiery Exp $
// Name:	SmallList-test.cc
// Purpose:	Class SmallList for efficient small lists of small objects
//		(Test suite)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/08
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include "TestsUtils.h"
#include "TestsUtils_tmpl.h"
#include "GLIP/SmallList.h"
// Uncomment if you get an error about missing symbols.
// #include "GLIP/SmallList_tmpl.h"

using GLIP::SmallList;

int main () {
  test_start("class SmallList");

  SmallList<> l1(3);
  l1[0]=1; l1[1]=0; l1[2]=2;
  int l[]={1,0,2};

  clog << "Size of SmallList's:\n";
  clog << "\tn\t* sizeof(char)\n";
  for (size_t i=0; i!=20; i+=5) {
    clog << "\t" << i << "\t" << SmallList<>(i).fullsizeof() << endl;
  }
  
  test(l1,		SmallList<>(l, l+3),		"Constructor 1");
  test(l1,		SmallList<>("[1,0,2]"),		"Constructor 2");
  test(l1.size(),	(unsigned char)(3),		"Method size");

  SmallList<> l2=l1;
  test(l2,		SmallList<>("[1,0,2]"),		"Assignment 1");
  test(l1.is_clone(l2),	true,				"Assignment 2");
  test(l1,		l1,				"Equality 1");
  test(l1,		l2,				"Equality 2");

  test(l1[0],		GLIP::Byte(1),			"Operator [] RO 1");
  test(l1[1],		GLIP::Byte(0),			"Operator [] RO 2");
  test(l1[2],		GLIP::Byte(2),			"Operator [] RO 3");
  test(l1(2),		GLIP::Byte(2),			"Operator [] RO 3");
  test(l1.is_clone(l2),	true,				"Copy on write 1");

  l2[0]=0;
  test(l1.is_clone(l2),	false,				"Copy on Write 2");
  test(l1,		SmallList<>("[1,0,2]"),		"Copy on write 3");
  test(l2,		SmallList<>("[0,0,2]"),		"Operator [] RW 1");
  l2[1]=l1[0];
  test(l2,		SmallList<>("[0,1,2]"),		"Operator [] RW 2");
  l2[l2[2]]=l2[0];
  test(l2,		SmallList<>("[0,1,0]"),		"Operator [] RW 2");
  test(l1==l2,		false,				"Equality 3");
  test(l1!=l2,		true,				"Equality 4");

  test(l1 < l1,						false, "Inequality 1");
  test(SmallList<>("[1,0,2]") < SmallList<>("[1,0,2]"),	false, "Inequality 2");
  test(SmallList<>("[1,0,2]") < SmallList<>("[0,1,2]"),	false, "Inequality 3");
  test(SmallList<>("[1,0,2]") < SmallList<>("[2,1,0]"),	true,  "Inequality 4");
  test(SmallList<>("[1,0,2]") > SmallList<>("[0,1,2]"),	true,  "Inequality 4");

  SmallList<>::const_iterator i=l1.begin();
  test(*i,		1,				"Const iterator 1");
  test(*(++i),		0,				"Const iterator 2");
  test(*(++i),		2,				"Const iterator 3");
  test(++i,		l1.end(),			"Const iterator 4");

  return(test_summary());
}
