//////////////////////////////////////////////////////////////////////////////
// $Id: Permutation.cc,v 1.5 2000/09/06 04:54:48 nthiery Exp $
// Name:	Permutation.cc
// Purpose:	Class Permutation for efficient small permutations
//		(Implementation)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/04
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include "GLIP/Permutation.h"
#include "GLIP/Permutation_tmpl.h"

using GLIP::SmallList;
using GLIP::Permutation;

namespace GLIP {    // Explicit instantiation of templates for default class
  template SmallList<> Permutation::operator()(const SmallList<>&) const;
  template bool        Permutation::stabilize (const SmallList<>&) const;
  template bool        Permutation::decrease  (const SmallList<>&) const;
}

Permutation& Permutation::MuPAD_to_C () {
  for (Permutation::iterator i=this->begin(); i!=end(); i++) {
    (*i)--;
  }
  return *this;
}

Permutation& Permutation::C_to_MuPAD () {
  for (Permutation::iterator i=begin(); i!=end(); i++) {
    (*i)++;
  }
  return *this;
}

Permutation Permutation::id(size_type n) {
  Permutation p(n);
  for(size_type i=0;i<n;i++) {
    p[i]=i;
  }
  return p;
}

Permutation GLIP::operator *(const Permutation &p1, const Permutation &p2) {
  assert(p1.size()==p2.size());
  Permutation result(p1.size());
  Permutation::const_iterator ip2    (p2    .begin());
  Permutation::iterator       iresult(result.begin());	
  for(; ip2!=p2.end(); ip2++,iresult++) {
    *iresult=p1[*ip2];
  }
  return result;
}
