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
 * file .......: ltiVector.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 18.05.01
 * revisions ..: $Id: ltiVector.cpp,v 1.15 2006/02/08 12:52:45 ltilib Exp $
 */

#include "ltiMacroSymbols.h"
#ifdef _LTI_MSC_6
// a weird warning from MSVC++ is a false alarm!
#pragma warning(disable:4660)
#pragma warning(disable:4275) // warnings for matrix< complex<T> > 
                              // disabled (dll stuff)
#endif

#include "ltiPoint.h"
#include "ltiRGBPixel.h"
#include "ltiComplex.h"
#include "ltiGenericVector.h" // provides implementation of generic vector
#include "ltiVector.h"
#include "ltiVector_template.h"

namespace lti {
  // explicit instantiations for the parent class, which were explicitely
  // deactivated until now
  template class genericVector<ubyte>;
  template class genericVector<byte>;
  template class genericVector<char>;
  template class genericVector<uint16>;
  template class genericVector<int16>;
  template class genericVector<int>;
  template class genericVector<long>;
  template class genericVector<unsigned int>;
  template class genericVector<float>;
  template class genericVector<double>;
  template class genericVector<rgbPixel>;
  template class genericVector<trgbPixel<float> >;
  template class genericVector<point>;
  template class genericVector<tpoint<float> >;
  template class genericVector<tpoint<double> >;
  template class genericVector<tpoint3D<float> >;
  template class genericVector<complex<float> >;
  template class genericVector<complex<double> >;
  template class genericVector<dpoint3D>;


  // explicit instantiations for the class, which were explicitely
  // deactivated until now
  template class vector<ubyte>;
  template class vector<byte>;
  template class vector<char>;
  template class vector<uint16>;
  template class vector<int16>;
  template class vector<int>;
  template class vector<long>;
  template class vector<unsigned int>;
  template class vector<float>;
  template class vector<double>;
  template class vector<rgbPixel>;
  template class vector<trgbPixel<float> >;
  template class vector<point>;
  template class vector<tpoint<float> >;
  template class vector<tpoint<double> >;
  template class vector<tpoint3D<float> >;
  template class vector<complex<float> >;
  template class vector<complex<double> >;
  template class vector<dpoint3D>;
}

namespace std {
  // explicit instantiations for operator<<
  // for ubyte and byte there was already an specialization
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<lti::ubyte>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<lti::byte>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<char>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<int>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<unsigned int>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<float>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<double>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<lti::rgbPixel>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<lti::point>&);
  template ostream& operator<<(ostream& s,
                               const lti::genericVector<lti::tpoint<float> >&);
  template ostream&
    operator<<(ostream& s,const lti::genericVector<lti::tpoint<double> >&);
  template ostream&
    operator<<(ostream& s,const lti::genericVector<lti::tpoint3D<float> >&);
  template ostream&
    operator<<(ostream& s,const lti::genericVector<lti::complex<float> >&);
  template ostream&
    operator<<(ostream& s,const lti::genericVector<lti::complex<double> >&);

}
