//////////////////////////////////////////////////////////////////////////////
// $Id: Partitions.cc,v 1.7 2000/09/24 18:31:51 nthiery Exp $
// Name:	Partitions.cc
// Purpose:	Class Partitions for generating partitions
//		(Implementation)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/07
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include "GLIP/Partitions.h"
#include "GLIP/SmallList_tmpl.h"

using GLIP::Partitions;

// Invariants when the number of parts is > 0:
//
//  - Before the first run of opeartor(),  b==begin
//    (1)*begin is set to n, or to the maximal value allowed m
//  - After any further run of operator(), b==end
//    (2)*begin, ..., *(end-1) is a valid partition
//  - Any time during operator():
//    (2b) b==end and (2), or:
//    (3) *begin >= ... >= *b
//    (4) *begin + ... + *b + n is the original value of n
//    (5) There exists a partition with k parts starting with *begin, ..., *b
//	  i.e., there is enough room after *b for the remaining n
//
// The special case with 0 parts is treated separately

Partitions::Partitions() : n(0) {
  end=b=begin=new cell_type[0];
}

Partitions::Partitions(int n_, int k) :
  n(n_) {
  begin=new cell_type[k];
  b=begin;
  end=begin+k;
  if (k>0) {
    *begin=n;
    n=0;
  }
}

Partitions::Partitions(int n_, int k, int m) :
  n(n_) {
  begin=new cell_type[k];
  b=begin;
  end=begin+k;
  if (k>0) {
    *begin=m;
    n-=*begin;
  }
}

Partitions::~Partitions() {
  delete [] begin;
}

Partitions& Partitions::operator= (const Partitions & p) {
  delete [] begin;
  n=p.n;
  begin=new cell_type[distance(p.begin, p.end)];
  end=begin+distance(p.begin, p.end);
  b=begin+distance(p.begin, p.b);
  copy(p.begin, p.end, begin);
  return *this;
}
  
Partitions::pointer Partitions::operator() () {
  if (begin==end) {
    if (n>0) {
      return 0;
    } else {
      n=1;
      return new value_type(0);
    }
  }
  if (b==begin) {	// First run
    if ((*b) * (end-(b+1)) <n) return 0;
  } else {		// Following runs
    // Backtrack until you find a position where it's possible to
    // decrease the element without invalidating (5)
    b--;
    while(true) {
      if (*b>0) {
	--(*b);
	++n;
	if ((*b) * (end-(b+1)) >=n) {
	  break;
	}
      }
      if (b==begin) { return 0; }
      n+=*b;
      b--;
    }
  }
  while((++b)!=end) {
    *b=min(*(b-1),n);
    n=n-*b;
  }
  return new value_type(begin,end);
}
