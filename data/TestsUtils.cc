//////////////////////////////////////////////////////////////////////////////
// $Id: TestsUtils.cc,v 1.4 2000/08/31 22:56:50 nthiery Exp $
// Name:	TestsUtils.cc
// Purpose:	GLIP test suite, C++ utilities
//		(Implementation)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/08
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "TestsUtils.h"

char buffer[80];

istrstream *read_command() {
  cout << "Test> ";
  if (cin.getline(buffer,sizeof(buffer))) {
    return new istrstream (buffer);
  } else {
    return 0;
  }
}

string testname("noname");
bool verbose= true;
int failed  = 0;
int xfailed = 0;
int passed  = 0;
int xpassed = 0;

void pass() {
  passed++;
}
void xpass() {
  xpassed++;
}
void fail() {
  failed++;
}
void xfail() {
  xfailed++;
}

bool test (bool t, string s) {
  if (t) {
    cout << "PASS: " << s << endl;
    pass();
  } else {
    cout << "FAIL: " << s << endl;
    fail();
  }
}

bool xtest (bool t, string s) {
  if (t) {
    cout << "XPASS: " << s << endl;
    xpass();
  } else {
    cout << "XFAIL: " << s << endl;
    xfail();
  }
}

void test_start(string s) {
  testname=s;
  clog << "		=== " << testname << " tests === \n";
}

int test_summary() {
  clog << "		=== " << testname << " tests summary === \n";
  clog << "# of expected passes " << passed << endl;
  if (xfailed>0) {
    clog << "# of expected failures " << xfailed << endl;
  }
  if (failed>0) {
    clog << "# of unexpected failures " << failed << endl;
  }
  if (xpassed>0) {
    clog << "# of unexpected passes " << xpassed << endl;
  }
  if (failed+xpassed>0) {
    return(1);
  } else {
    return 0;
  }
}
