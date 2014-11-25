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


/* -----------------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiScalarValuedInterpolatorFactory.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.2002
 * revisions ..: $Id: ltiScalarValuedInterpolatorFactory.cpp,v 1.4 2006/02/08 11:47:19 ltilib Exp $
 */

#include "ltiScalarValuedInterpolatorFactory.h"
#include "ltiClassName.h"
#include "ltiNearestNeighborInterpolator.h"
#include "ltiBilinearInterpolator.h"
#include "ltiBicubicInterpolator.h"
#include "ltiTypeInfo.h"

namespace lti {
  // --------------------------------------------------
  // static members
  // --------------------------------------------------

  template<class T>
  const scalarValuedInterpolation<T> *const 
  scalarValuedInterpolatorFactory<T>::scalarValuedInterpolators[] = {
    new nearestNeighborInterpolator<T>,
    new bilinearInterpolator<T>,
    new bicubicInterpolator<T>,
    0
  };

  template<class T>
  objectFactory< scalarValuedInterpolation<T> >*
  scalarValuedInterpolatorFactory<T>::factory = 0;

  // --------------------------------------------------
  // scalarValuedInterpolatorFactory
  // --------------------------------------------------

  // default constructor
  template<class T>
  scalarValuedInterpolatorFactory<T>::scalarValuedInterpolatorFactory() {
    if (isNull(factory)) {
      factory=
          new objectFactory< scalarValuedInterpolation<T> >(scalarValuedInterpolators);
    }
  }

  // destructor
  template<class T>
  scalarValuedInterpolatorFactory<T>::~scalarValuedInterpolatorFactory() {
  }

  // returns the name of this type
  template<class T>
  const char*
  scalarValuedInterpolatorFactory<T>::getTypeName() const {
    return "scalarValuedInterpolatorFactory";
  }

  template<class T>
  scalarValuedInterpolation<T>* 
  scalarValuedInterpolatorFactory<T>::newInstance(const std::string& name)
    const {
    
    // replace the <whatsoever> type by the proper one.
    std::string::size_type pos = name.find('<');
    std::string str=name.substr(0,pos);
    str+="<";
    str+=typeInfo<T>::name();
    str+=">";

    return factory->newInstance(str.c_str());
  }

}
