//////////////////////////////////////////////////////////////////////////////
// $Id: PermutationGroup.cc,v 1.5 2000/10/12 04:45:32 nthiery Exp $
// Name:	PermutationGroup.cc
// Purpose:	Class PermutationGroup for groups of permutations
//		(Implementation)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/04
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <functional>
#include "GLIP/Partitions.h"
#include "GLIP/PermutationGroup.h"
#include "GLIP/PermutationGroup_tmpl.h"

using GLIP::MemoryCache;
using GLIP::PermutationGroup;
using GLIP::BinaryList;
using GLIP::SmallList;


//////////////////////////////////////////////////////////////////////////////
// Private helper functions
//////////////////////////////////////////////////////////////////////////////

namespace {
  BinaryList
  flip(const BinaryList &l) {
    BinaryList result(l.size());
    transform(l.begin(),
	      l.end(),
	      result.begin(),
	      logical_not<bool>()
	      );
    return result;
  }
}

////////////////////////////////////////////////////////////////////////////
// Implementation of the MemoryCache strategy

MemoryCache::MemoryCache(size_t nn) : n(nn), cache() {
  cache.reserve(ceil(n/2)+1);
};

bool
MemoryCache::have(size_t k) {
  return (min(k,n-k)<cache.size());
}

list<BinaryList>
MemoryCache::get(size_t k) {
  assert(min(k,n-k)<cache.size());		// Should throw exception
  if(k<=n-k) {
    return cache[k];
  } else {
    list<BinaryList> result;
    transform(cache[n-k].begin(), cache[n-k].end(),
	      back_inserter(result),
	      flip);
    return(result);
  }
}

void
MemoryCache::set(size_t k, list<BinaryList> l) {
  if(k>n-k) return;
  cache.resize(k+1);
  cache[k]=l;
}

////////////////////////////////////////////////////////////////////////////
// Implementation of the member functions

list<BinaryList> PermutationGroup::canonics (size_t k) {
  if (cache.have(k)) {
    return(cache.get(k));
  }
  list<BinaryList> result;
  if (k==0) {
    result.push_back(BinaryList(card(),0));
    cache.set(k,result);
    return result;
  }
  // generates all canonic lists with k ones and card()-k zero
  //clog << "Generating all canonic binary lists with " << k << " ones\n";
  list<BinaryList> prev=canonics(k-1);
  list<BinaryList>::const_iterator i;
  for (i=prev.begin(); i!=prev.end(); i++) {
    list<BinaryList> added = augment(*i,0,1);
    result.splice(result.end(), added);
  }
  //clog << "Found: " << result.size() << endl;
  cache.set(k,result);
  return result;
}

list<BinaryList> PermutationGroup::canonics () {
  list<BinaryList> result;
  for (size_t k=0; k<=card(); k++) {
    list<BinaryList> l=canonics(k);
    result.splice(result.end(), l);
  }
  return result;
}

list<SmallList<> > PermutationGroup::canonic_compositions (size_t d) {
  list<SmallList<> > result;
  Partitions generator(d,n);
  Partitions::pointer shape;
  while ((shape=generator())) {
    list<SmallList<> > l=canonics_shape_rec(*shape);
    result.splice(result.end(), l);
  }
  return result;
}
