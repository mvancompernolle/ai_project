//////////////////////////////////////////////////////////////////////////////
// $Id: SmallList.cc,v 1.6 2000/09/06 05:06:06 nthiery Exp $
// Name:	SmallList.cc
// Purpose:	Class SmallList for efficient small lists of small objects
//		(Implementation)
// Author:	Nicolas M. Thiéry <nthiery@mines.edu>
// Created:	2000/04
// Copyright:	LGPL
//////////////////////////////////////////////////////////////////////////////

#include "GLIP/SmallList.h"
#include "GLIP/SmallList_tmpl.h"

namespace GLIP {    // Explicit instantiation for the default values
  template SmallList<>;
  template SmallList<char>;

  // Non inline templates members and helper functions
  // They need to be explicitely instanciated
  template SmallList<>::SmallList(int *, int*);
  template std::ostream &operator << (std::ostream &, const SmallList<> &);
  template std::istream &operator >> (std::istream &,       SmallList<> &);
}
