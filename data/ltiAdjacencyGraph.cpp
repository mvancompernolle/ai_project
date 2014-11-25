/*
 * Copyright (C) 2003, 2004, 2005, 2006
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


/*--------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiAdjacencyGraph.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 01.07.2003
 * revisions ..: $Id: ltiAdjacencyGraph.cpp,v 1.3 2006/02/08 12:11:04 ltilib Exp $
 */

#include "ltiAdjacencyGraph.h"

// only for compilers different than VC++ 6.0 available
#ifndef _LTI_MSC_6

#include <limits>

namespace lti {

  // ------------------------------------------------------------------------
  // --                        Edges
  // ------------------------------------------------------------------------

  template<class T>
  const T symmetricEdgeTraits<T>::Invalid = typeInfo<T>::min();

  template<class T>
  const bool symmetricEdgeTraits<T>::Symmetric = true;

  template<class T>
  const T asymmetricEdgeTraits<T>::Invalid = typeInfo<T>::min();

  template<class T>
  const bool asymmetricEdgeTraits<T>::Symmetric = false;

  // explicit instantiations
  template class symmetricEdgeTraits<char>;
  template class symmetricEdgeTraits<short>;
  template class symmetricEdgeTraits<int>;
  template class symmetricEdgeTraits<float>;
  template class symmetricEdgeTraits<double>;

  template class asymmetricEdgeTraits<char>;
  template class asymmetricEdgeTraits<short>;
  template class asymmetricEdgeTraits<int>;
  template class asymmetricEdgeTraits<float>;
  template class asymmetricEdgeTraits<double>;

} // end of namespace lti

#endif
