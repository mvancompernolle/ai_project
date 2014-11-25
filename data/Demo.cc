//////////////////////////////////////////////////////////////////////////////
// $Id: Demo.cc,v 1.1 2000/09/26 00:21:56 nthiery Exp $
// Name:	PermutationGroup-test.cc
// Purpose:	Class PermutationGroup for groups of permutations
//		(Test suite)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/08
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include "GLIP/SmallList.h"
#include "GLIP/SmallList_tmpl.h"
#include "GLIP/Permutation.h"
#include "GLIP/Permutation_tmpl.h"
#include "GLIP/PermutationGroup.h"
#include "GLIP/PermutationGroup_tmpl.h"

using GLIP::SmallList;
using GLIP::Permutation;
using GLIP::PermutationGroup;
using GLIP::closure;

template <class L> void print(const L &l) {
  for(typename L::const_iterator i=l.begin();i!=l.end(); i++) {
    cout << *i << endl;
  }
}

PermutationGroup ReadGroup(std::istream &i=cin) {
  vector<Permutation> generators(0);
  Permutation l;
  while(1) {
    i>>l;
    if (i.eof()) break;
    l.MuPAD_to_C();
    generators.push_back(l);
  }
  PermutationGroup G(closure(generators.begin(),generators.end()));
  return G;
}

int main (int argc, char * argv[]) {
  PermutationGroup G=ReadGroup();

  print(G.canonic_compositions(atoi(argv[1])));
}
