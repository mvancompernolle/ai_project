/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiQrDecomposition.cpp
 * authors ....: Arnd Hannemann
 * organization: LTI, RWTH Aachen
 * creation ...: 26.1.2004
 * revisions ..: $Id: ltiQrDecomposition.cpp,v 1.6 2006/09/11 17:05:00 ltilib Exp $
 */

#include "ltiQrDecomposition.h"
#include "ltiQrDecomposition_template.h"

#ifdef HAVE_LAPACK
  #include "clapack.h"
#endif

namespace lti {

  // explicit instantiations
  template class qrDecomposition<float>;
  template class qrDecomposition<double>;

#ifdef HAVE_LAPACK
  template<>
  int qrDecomposition<float>::geqrf(integer* rows, integer* cols, float* a, 
                                    integer* lda,float* tau,
                                    float * work, integer* lwork, 
                                    integer* info) const {
    lockInterface();
    int tmp=LA_SGEQRF(rows,cols,a,lda,tau,work,lwork,info);
    unlockInterface();
    return tmp;
  }

  template<>
  int qrDecomposition<double>::geqrf(integer* rows, integer* cols, double* a,
                                     integer* lda, double* tau,
                                     double *work, integer* lwork,
                                     integer* info) const {
    lockInterface();
    int tmp=LA_DGEQRF(rows,cols,a,lda,tau,work,lwork,info);
    unlockInterface();
    return tmp;
  }

  template<>
  int qrDecomposition<float>::orgqr(integer* rows, integer* cols, integer* k,
                                    float* a, integer* lda,
                                    float* tau, float* work,
                                    integer* lwork, integer* info) const {
    lockInterface();
    int tmp=LA_SORGQR(rows,cols,k,a,lda,tau,work,lwork,info);
    unlockInterface();
    return tmp;
  }

  template<>
  int qrDecomposition<double>::orgqr(integer* rows, integer* cols, integer* k,
                                     double* a, integer* lda,
                                     double* tau, double* work, integer* lwork,
                                     integer* info) const {
    lockInterface();
    int tmp=LA_DORGQR(rows,cols,k,a,lda,tau,work,lwork,info);
    unlockInterface();
    return tmp;
  }
#endif

}
