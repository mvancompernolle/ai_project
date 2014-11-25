/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiMatrix.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 18.05.01
 * revisions ..: $Id: ltiMatrix.cpp,v 1.13 2006/02/08 12:34:33 ltilib Exp $
 */

#include "ltiMacroSymbols.h"
#ifdef _LTI_MSC_6
// a weird warning from MSVC++ is a false alarm!
#pragma warning(disable:4660)
#pragma warning(disable:4800) // warnings for matrix<bool> disabled
#pragma warning(disable:4804) // warnings for matrix<bool> disabled
#pragma warning(disable:4275) // warnings for matrix< complex<T> > 
                              // disabled (dll stuff)
#endif

#include "ltiRGBPixel.h"
#include "ltiPoint.h"
#include "ltiComplex.h"
#include "ltiGenericMatrix.h" // provides implementation of generic matrix
#include "ltiMatrix.h"
#include "ltiMatrix_template.h"


namespace lti {
  // explicit instantiations for the parent class
  template class genericMatrix<ubyte>;
  template class genericMatrix<byte>;
  template class genericMatrix<char>;
  template class genericMatrix<int>;
  template class genericMatrix<long>;
  template class genericMatrix<unsigned int>;
  template class genericMatrix<float>;
  template class genericMatrix<double>;
  template class genericMatrix<rgbPixel>;
  template class genericMatrix<ipoint>;
  template class genericMatrix<fpoint>;
  template class genericMatrix<dpoint>;
  template class genericMatrix< complex<float> >;
  template class genericMatrix< complex<double> >;
  template class genericMatrix<fpoint3D>;
  template class genericMatrix<dpoint3D>;


  // explicit instantiation of the class
  template class matrix<ubyte>;
  template class matrix<byte>;
  template class matrix<char>;
  template class matrix<int>;
  template class matrix<long>;
  template class matrix<unsigned int>;
  template class matrix<float>;
  template class matrix<double>;
  template class matrix<rgbPixel>;
  template class matrix<ipoint>;
  template class matrix<fpoint>;
  template class matrix<dpoint>;
  template class matrix< complex<float> >;
  template class matrix< complex<double> >;
  template class matrix<fpoint3D>;
  template class matrix<dpoint3D>;
}

namespace std {
  // explicit instantiations for operator<<
  // for ubyte and byte there was already an specialization
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<lti::ubyte>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<lti::byte>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<char>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<int>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<unsigned int>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<float>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<double>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<lti::rgbPixel>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<lti::point>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericMatrix<lti::tpoint<float> >&);
  template ostream&
    operator<<(ostream& s,const lti::genericMatrix<lti::tpoint<double> >&);
  template ostream&
    operator<<(ostream& s,const lti::genericMatrix<lti::tpoint3D<float> >&);
  template ostream&
    operator<<(ostream& s,const lti::genericMatrix<lti::complex<float> >&);
  template ostream&
    operator<<(ostream& s,const lti::genericMatrix<lti::complex<double> >&);

}
