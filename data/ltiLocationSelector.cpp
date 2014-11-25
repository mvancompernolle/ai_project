/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiLocationSelector.cpp
 * authors ....: Axel Berner, Xinghan Yu
 * organization: LTI, RWTH Aachen
 * creation ...: 28.2.2002
 * revisions ..: $Id: ltiLocationSelector.cpp,v 1.8 2006/09/05 10:21:11 ltilib Exp $
 */

#include "ltiLocationSelector.h"

namespace lti {
  // --------------------------------------------------
  // locationSelector::parameters
  // --------------------------------------------------

  // default constructor
  locationSelector::parameters::parameters()
    : functor::parameters() {
    eraseEntry = ubyte(0);
  }

  // copy constructor
  locationSelector::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  locationSelector::parameters::~parameters() {
  }

  // get type name
  const char* locationSelector::parameters::getTypeName() const {
    return "locationSelector::parameters";
  }

  // copy member

  locationSelector::parameters&
    locationSelector::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif

    eraseEntry = other.eraseEntry;

    return *this;
  }

  // alias for copy member
  locationSelector::parameters&
    locationSelector::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* locationSelector::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool locationSelector::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool locationSelector::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"eraseEntry",eraseEntry);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool locationSelector::parameters::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool locationSelector::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool locationSelector::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"eraseEntry",eraseEntry);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool locationSelector::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // locationSelector
  // --------------------------------------------------

  // default constructor
  locationSelector::locationSelector()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  locationSelector::locationSelector(const locationSelector& other)
    : functor()  {
    copy(other);
  }

  // destructor
  locationSelector::~locationSelector() {
  }

  // returns the name of this type
  const char* locationSelector::getTypeName() const {
    return "locationSelector";
  }

  // copy member
  locationSelector&
    locationSelector::copy(const locationSelector& other) {
      functor::copy(other);

    return (*this);
  }

  // alias for copy member
  locationSelector&
    locationSelector::operator=(const locationSelector& other) {
    return (copy(other));
  }


  // clone member
  functor* locationSelector::clone() const {
    return new locationSelector(*this);
  }

  // return parameters
  const locationSelector::parameters&
    locationSelector::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool locationSelector::apply(const channel8& mask,
			       std::list<location>& locs) const {

    const ubyte eraseEntry = getParameters().eraseEntry;
    point p;
    std::list<location>::iterator it = locs.begin();
    while(it != locs.end()) {
      p.castFrom((*it).position); //float->int
      if(mask.at(p) == eraseEntry) {
	it = locs.erase(it);//remove location from list
      } else
	it++;
    }
    return true;
  };


  bool
  locationSelector::apply(const std::list<areaPoints>& objs,
                          const std::list<location>& locs,
                          std::vector<std::list<location> >& filteredLocs
                         ) const {

    // create a mask
    matrix<int> mask;

    // guess a size for the mask
    // use the location list for it
    std::list<location>::const_iterator lit;
    point size;

    for (lit=locs.begin();lit!=locs.end();++lit) {
      size.x = max(size.x,iround((*lit).position.x+(*lit).radius));
      size.y = max(size.y,iround((*lit).position.y+(*lit).radius));
    }

    mask.resize(size,0,false,true); // fill the mask with 0

    int label = 1;
    std::list<areaPoints>::const_iterator it;
    areaPoints::const_iterator ait;

    for (it=objs.begin();it!=objs.end();++it,++label) {
      for (ait=(*it).begin();ait!=(*it).end();++ait) {
        if (((*ait).x < mask.columns()) &&
            ((*ait).y < mask.rows())) {
          mask.at(*ait) = label;
        }
      }
    }

    return apply(mask,locs,filteredLocs);
  }


  bool
  locationSelector::apply(const matrix<int>& mask,
                          const std::list<location>& locs,
                          std::vector<std::list<location> >& filteredLocs
                         ) const {

    const int maxValue = mask.maximum();
    std::list<location> myList;

    filteredLocs.resize(maxValue+1);
    point locationPosition;

    // erase all locations lists...
    std::vector<std::list<location> >::iterator vit;
    for (vit =  filteredLocs.begin();
         vit != filteredLocs.end();
         ++vit) {
      (*vit).clear();
    }

    // sort the locations in smaller lists...
    std::list<location>::const_iterator it = locs.begin();
    while (it != locs.end()) {
      locationPosition.castFrom((*it).position); // float -> int
      filteredLocs[mask.at(locationPosition)].push_back(*it);
      it++;
    }

    return true;
  };

}
