//////////////////////////////////////////////////////////////////////////////
// $Id: Partitions-test.cc,v 1.7 2000/09/24 18:31:50 nthiery Exp $
// Name:	Partitions-test.cc
// Purpose:	Class Partitions for generating partitions
//		(Test suite)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/07
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <strstream>
#include "TestsUtils.h"
#include "TestsUtils_tmpl.h"
#include "GLIP/Partitions.h"

using GLIP::Partitions;

int main(int argc, char **argv) {

  if (argc>1) {
    Partitions gen(0,0);
    if (argc==3) {
      gen=Partitions(atoi(argv[1]), atoi(argv[2]));
    } else if (argc==4) {
      gen=Partitions(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    } else {
      cerr << "Usage: echo n k [m] | " << argv[0] << endl;
      cerr << "Generates all partitions of n in k parts." << endl;
      cerr << "If m is given, each part will be smaller than m" << endl;
      exit(-1);
    }
    while(Partitions::pointer p=gen()) {
      cout << *p << endl;
    }
    exit(0);
  }

  typedef Partitions::value_type Partition;
  
  test_start("class Partitions");
  Partitions gen,gen2;

  gen=Partitions(0,0);
  test(*   (gen()),	Partition(0),		"Partitions(0,0) 1");
  test(bool(gen()),	false,			"Partitions(0,0) 2");

  gen=Partitions(1,0);
  test(bool(gen()),	false,			"Partitions(1,0)");

  gen=Partitions(1,1);
  test(*   (gen()),	Partition("[1]"),	"Partitions(1,1) 1");
  test(bool(gen()),	false,			"Partitions(1,1) 2");

  gen=Partitions(2,2);
  test(*   (gen()),	Partition("[2,0]"),	"Partitions(2,2) 1");
  test(*   (gen()),	Partition("[1,1]"),	"Partitions(2,2) 2");
  test(bool(gen()),	false,			"Partitions(2,2) 3");

  gen=Partitions(5,2);
  test(*   (gen()),	Partition("[5,0]"),	"Partitions(5,2) 1");
  test(*   (gen()),	Partition("[4,1]"),	"Partitions(5,2) 2");
  test(*   (gen()),	Partition("[3,2]"),	"Partitions(5,2) 3");
  test(bool(gen()),	false,			"Partitions(5,2) 4");

  gen=Partitions(3,2,1);
  test(bool(gen()),	false,			"Partitions(3,2,1)");

  gen=Partitions(3,3,1);
  test(*   (gen()),	Partition("[1,1,1]"),	"Partitions(3,3,1) 1");
  test(bool(gen()),	false,			"Partitions(3,3,1) 2");

  gen=Partitions(5,4,3);
  gen2=gen;
  test(*   (gen()),	Partition("[3,2,0,0]"),	"Partitions(5,4,3) 1");
  test(*   (gen()),	Partition("[3,1,1,0]"),	"Partitions(5,4,3) 2");
  gen2=gen;
  test(*   (gen()),	Partition("[2,2,1,0]"),	"Partitions(5,4,3) 3");
  test(*   (gen()),	Partition("[2,1,1,1]"),	"Partitions(5,4,3) 4");
  test(bool(gen()),	false,			"Partitions(5,4,3) 5");

  gen=gen2;
  test(*   (gen()),	Partition("[2,2,1,0]"),	"Assignment 2");
  test(*   (gen()),	Partition("[2,1,1,1]"),	"Assignment 3");
  test(bool(gen()),	false,			"Assignment 4");

  return(test_summary());
}
