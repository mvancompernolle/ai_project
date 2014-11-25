//////////////////////////////////////////////////////////////////////////////
// $Id: Permutation-test.cc,v 1.5 2000/09/05 23:55:33 nthiery Exp $
// Name:	Permutation-test.cc
// Purpose:	Class Permutation for efficient small permutations
//		(Test suite)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/07
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include "TestsUtils.h"
#include "TestsUtils_tmpl.h"
#include "GLIP/SmallList.h"
#include "GLIP/SmallList_tmpl.h"
#include "GLIP/Permutation.h"
#include "GLIP/Permutation_tmpl.h"

using GLIP::SmallList;
using GLIP::Permutation;

int main () {
  test_start("class Permutation");

  test(Permutation::id(4),	Permutation("[0,1,2,3]"), "Identity");
  test(Permutation("[4,2,3,1,0]")*Permutation("[0,4,2,1,3]"),
       Permutation("[4,0,3,2,1]"), 			"Operator * 1");
  test(Permutation("[0,4,2,1,3]")*Permutation("[4,2,3,1,0]"),
       Permutation("[3,2,1,4,0]"),			"Operator * 2");
  test(Permutation("[4,2,3,1,0]")(SmallList<char>("[a,b,c,d,e]")),
       SmallList<char>("[e,d,b,c,a]"),			"Operator () 1");

  Permutation p("[1,0,2]");
  Permutation::const_iterator i=p.begin();
  test(*i,		1,				"Const iterator 1");
  test(*(++i),		0,				"Const iterator 2");
  test(*(++i),		2,				"Const iterator 3");
  test(++i,		p.end(),			"Const iterator 4");

  return(test_summary());
}
  
bool time_action(int n, int repeat) {
  cout << "Card: " << n << ", Repeat: " << repeat << endl;
  
  for (int i=0; i<repeat; i++) {
    //    vector<index_t>::iterator i;
    //for (i=v.begin();i!=v.end(); i++) {
    //*i=6;
    //}
    vector<Permutation::size_type> v(n);
  }
  return true;
}
