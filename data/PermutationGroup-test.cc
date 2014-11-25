//////////////////////////////////////////////////////////////////////////////
// $Id: PermutationGroup-test.cc,v 1.7 2000/09/26 00:01:08 nthiery Exp $
// Name:	PermutationGroup-test.cc
// Purpose:	Class PermutationGroup for groups of permutations
//		(Test suite)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/08
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include "TestsUtils.h"
#include "TestsUtils_tmpl.h"
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

int main () {
  test_start("class PermutationGroup");

  // Construct G4, the symmetric group S4 acting on the pairs of {1,2,3,4}
  Permutation gen[]={
    Permutation("[1, 3, 2, 5, 4, 6]").MuPAD_to_C(),
    Permutation("[3, 5, 6, 1, 2, 4]").MuPAD_to_C()
  };
  PermutationGroup G4(closure(gen,gen+2));

  typedef SmallList<> L;
  
  test(G4.size(),			size_t(24),	"Method size");

  test(G4.canonic(L("[0,10,8,6,4,2]")), L("[10,8,0,2,6,4]"), "Method canonic");
  test(G4.is_canonic(L("[0,10,8,6,4,2]")), false,	"Method is_canonic");
  test(G4.is_canonic(L("[10,8,0,2,6,4]")), true,	"Method is_canonic");

  test(G4.orbit_set(L("[1,1,1,1,1,1]")).size(), size_t(1),
       "Method orbit_set 1");
  test(G4.orbit_set(L("[1,0,0,0,0,1]")).size(), size_t(3),
       "Method orbit_set 2");
  test(G4.orbit_set(L("[1,0,0,0,0,0]")).size(), size_t(6),
       "Method orbit_set 3");
  test(G4.orbit_set(L("[1,2,0,0,0,0]")).size(), size_t(24),
       "Method orbit_set 4");

  test(G4.orbit_size(L("[1,1,1,1,1,1]")), size_t(1),	"Method orbit_size 1");
  test(G4.orbit_size(L("[1,0,0,0,0,1]")), size_t(3),	"Method orbit_size 2");
  test(G4.orbit_size(L("[1,0,0,0,0,0]")), size_t(6),	"Method orbit_size 3");
  test(G4.orbit_size(L("[1,2,0,0,0,0]")), size_t(24),	"Method orbit_size 4");
  
  test(G4.automorphism_group(L("[1,1,1,1,1,1]")).size(),size_t(24),
       "Method automorphism_group 1");
  test(G4.automorphism_group(L("[1,0,0,0,0,1]")).size(),size_t(8),
       "Method automorphism_group 2");
  test(G4.automorphism_group(L("[1,0,0,0,0,0]")).size(),size_t(4),
       "Method automorphism_group 3");
  test(G4.automorphism_group(L("[1,2,0,0,0,0]")).size(),size_t(1),
       "Method automorphism_group 4");
  
  test(G4.automorphism_group_size(L("[1,1,1,1,1,1]")),	size_t(24),
       "Method automorphism_group_size 1");
  test(G4.automorphism_group_size(L("[1,0,0,0,0,1]")),	size_t(8),
       "Method automorphism_group_size 2");
  test(G4.automorphism_group_size(L("[1,0,0,0,0,0]")),	size_t(4),
       "Method automorphism_group_size 3");
  test(G4.automorphism_group_size(L("[1,2,0,0,0,0]")),	size_t(1),
       "Method automorphism_group_size 4");
  
  test(G4.canonics().size(),		size_t(11),	"Method canonics 1");
  test(G4.canonics(0).size(),		size_t(1),	"Method canonics 2");
  test(G4.canonics(1).size(),		size_t(1),	"Method canonics 3");
  test(G4.canonics(2).size(),		size_t(2),	"Method canonics 4");
  test(G4.canonics(3).size(),		size_t(3),	"Method canonics 5");
  test(G4.canonics(4).size(),		size_t(2),	"Method canonics 6");
  test(G4.canonics(5).size(),		size_t(1),	"Method canonics 7");
  test(G4.canonics(6).size(),		size_t(1),	"Method canonics 8");

  test(G4.canonics_shape(L("[3,2,1,0,0,0]")).size(),		size_t(5),       "Method canonics_shape");
  test(G4.canonics_shape_rec(L("[3,2,1,0,0,0]")).size(),	size_t(5),       "Method canonics_shape_rec");

  //cout << G.weight_automorphism_group(v2,1).size() << endl;
  //print(G.weight_automorphism_group(v2,1));
  //list<List > c=G.split_value_canonics(v2,1,2);
  //cout << c.size() << endl;
  //print(c);
  
  //list<SmallList<char> >::iterator i;
  //  for(i=canonics.begin(); i!=canonics.end(); i++) {
  //cout << *i << endl;
  //}
  test(G4.canonic_compositions(4).size(), 			size_t(11),
       "Method canonic_compositions");
  
  return(test_summary());
}

//////////////////////////////////////////////////////////////////////////////
// Old stuff for debugging
//////////////////////////////////////////////////////////////////////////////

template <class L> void print(const L &l) {
  for(typename L::const_iterator i=l.begin();i!=l.end(); i++) {
    cout << *i << endl;
  }
}

template <class ForwardIterator> 
iterator_traits<ForwardIterator>::value_type average(ForwardIterator begin,
						     ForwardIterator end) {
  typedef typename iterator_traits<ForwardIterator>::value_type value_type;
  value_type a=0;
  value_type i=0;
  for(;begin!=end; begin++) {
    a+=*begin;
    i++;
  }
  return a/i;
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


PermutationGroup SymGroup(int n) {
  // The transposition (0,1)
  Permutation t1=Permutation::id(n);
  t1[0]=1; t1[1]=0;

  cout << "elementary tranposition (0,1): " <<  t1 << endl;

  // The full cycle (0,1,2,3,...)
  Permutation cycle(n);
  for (int i=0;i<n;i++) {
    cycle[i]=(i+1)%n;
  }
  cout << "full cycle (0,1,2,3,...): " <<  cycle << endl;

  // generators of the symmetric group
  vector<Permutation> generators(2);
  generators[0]=cycle;
  generators[1]=t1;

  // The symmetric group
  PermutationGroup Sn(closure(generators.begin(),generators.end()));
  return Sn;
}
