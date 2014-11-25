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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiSparseMatrix.cpp
 * authors ....: Bastian Ibach
 * organization: LTI, RWTH Aachen
 * creation ...: 26.04.02
 * revisions ..: $Id: ltiSparseMatrix.cpp,v 1.8 2006/02/08 12:45:44 ltilib Exp $
 */

#include "ltiMacroSymbols.h"
#ifdef _LTI_MSC_6
// a weird warning from MSVC++ is a false alarm!
#pragma warning(disable:4660)
#pragma warning(disable:4800) // warnings for sparseMatrix<bool> disabled
#pragma warning(disable:4804) // warnings for sparseMatrix<bool> disabled
#endif


#undef _LTI_DEBUG
// #define _LTI_DEBUG 4
#include "ltiDebug.h"


#include "ltiSparseMatrix.h"
#include "ltiSparseMatrix_template.h"
#include "ltiRGBPixel.h"

namespace lti {
  // explicit instantiations

  // bool does not work
  //template class sparseMatrix<bool>; divide does not work

  template class sparseMatrix<ubyte>;
  template class sparseMatrix<byte>;
  template class sparseMatrix<int>;
  template class sparseMatrix<unsigned int>;
  template class sparseMatrix<float>;
  template class sparseMatrix<double>;
  template class sparseMatrix<rgbPixel>;
  template class sparseMatrix<point>;
  template class sparseMatrix< tpoint<float> >;
//   template class sparseMatrix< tpoint3D<float> >;
}

namespace std {
  //template ostream& operator<<(ostream& outStr,
  //                             const lti::sparseMatrix<bool>&);
  template ostream& operator<<(ostream& outStr,
                               const lti::sparseMatrix<lti::ubyte>&);
  template ostream& operator<<(ostream& outStr,
                               const lti::sparseMatrix<lti::byte>&);
  template ostream& operator<<(ostream& outStr,
                               const lti::sparseMatrix<int>&);
  template ostream& operator<<(ostream& outStr,
                               const lti::sparseMatrix<unsigned int>&);
  template ostream& operator<<(ostream& outStr,
                               const lti::sparseMatrix<float>&);
  template ostream& operator<<(ostream& outStr,
                               const lti::sparseMatrix<double>&);
  template ostream& operator<<(ostream& outStr,
                               const lti::sparseMatrix<lti::rgbPixel>&);
  template ostream& operator<<(ostream& outStr,
                               const lti::sparseMatrix<lti::point>&);
  template ostream& operator<<(ostream& outStr,
                               const lti::sparseMatrix<lti::tpoint<float> >&);
//   template ostream& operator<<(ostream& outStr,
//                               const lti::sparseMatrix<lti::tpoint3D<float> >&);
}
