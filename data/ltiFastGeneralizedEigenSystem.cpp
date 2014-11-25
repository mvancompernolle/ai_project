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

/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiFastGeneralizedEigenSystem.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 09.05.2003
 * revisions ..: $Id: ltiFastGeneralizedEigenSystem.cpp,v 1.3 2006/09/11 17:02:49 ltilib Exp $
 */

#include "ltiFastGeneralizedEigenSystem.h"
#ifdef HAVE_LAPACK
#include "ltiFastGeneralizedEigenSystem_template.h"
#include "clapack.h"

namespace lti {


  template<>
  int fastGeneralizedEigenSystem<float>::sygv(integer* itype, char* jobz, 
                                              char* uplo, integer* n, 
                                              float* a, integer* lda,
                                              float* b, integer* ldb,
                                              float* w,
                                              float* work, integer* lwork,
                                              integer* info) const {
    lockInterface();
    int tmp=LA_SSYGV(itype,jobz,uplo,n,a,lda,b,ldb,w,work,lwork,info);
    unlockInterface();
    return tmp;
  }

  template<>
  int fastGeneralizedEigenSystem<double>::sygv(integer* itype, char* jobz, 
                                               char* uplo, integer* n, 
                                               double* a, integer* lda,
                                               double* b, integer* ldb,
                                               double* w,
                                               double* work, integer* lwork,
                                               integer* info) const {
    lockInterface();
    int tmp=LA_DSYGV(itype,jobz,uplo,n,a,lda,b,ldb,w,work,lwork,info);
    unlockInterface();
    return tmp;
  }

  template<>
  int fastGeneralizedEigenSystem<float>::sygvd(integer* itype, char* jobz, 
                                               char* uplo, integer* n, 
                                               float* a, integer* lda,
                                               float* b, integer* ldb,
                                               float* w,
                                               float* work, integer* lwork,
                                               integer* iwork, integer* liwork,
                                               integer* info) const {
    lockInterface();
    int tmp=LA_SSYGVD(itype,jobz,uplo,n,a,lda,b,ldb,w,
                      work,lwork,iwork,liwork,info);
    unlockInterface();
    return tmp;
  }

  template<>
  int fastGeneralizedEigenSystem<double>::sygvd(integer* itype, char* jobz, 
                                                char* uplo, integer* n, 
                                                double* a, integer* lda,
                                                double* b, integer* ldb,
                                                double* w,
                                                double* work, integer* lwork,
                                                integer* iwork, integer* liwork,
                                                integer* info) const {
    lockInterface();
    int tmp=LA_DSYGVD(itype,jobz,uplo,n,a,lda,b,ldb,w,
                      work,lwork,iwork,liwork,info);
    unlockInterface();
    return tmp;
  }

  template<>
  int fastGeneralizedEigenSystem<float>::sygvx(integer* itype, char* jobz, 
                                               char* range, char* uplo, 
                                               integer* n, 
                                               float* a, integer* lda,
                                               float* b, integer* ldb,
                                               float* vl, float* vu,
                                               integer* il, integer* iu,
                                               float* abstol,
                                               integer* m, float* w, 
                                               float* z, integer* ldz,
                                               float* work, integer* lwork,
                                               integer* iwork, integer* ifail,
                                               integer* info) const {

    lockInterface();
    int tmp=LA_SSYGVX(itype,jobz,range,uplo,n,a,lda,b,ldb,
                      vl,vu,il,iu,abstol,m,w,z,ldz,
                      work,lwork,iwork,ifail,info);
    unlockInterface();
    return tmp;
  }

  template<>
  int fastGeneralizedEigenSystem<double>::sygvx(integer* itype, char* jobz, 
                                                char* range, char* uplo, 
                                                integer* n, 
                                                double* a, integer* lda,
                                                double* b, integer* ldb,
                                                double* vl, double* vu,
                                                integer* il, integer* iu,
                                                double* abstol,
                                                integer* m, double* w, 
                                                double* z, integer* ldz,
                                                double* work, integer* lwork,
                                                integer* iwork, integer* ifail,
                                                integer* info) const {
    
    lockInterface();
    int tmp=LA_DSYGVX(itype,jobz,range,uplo,n,a,lda,b,ldb,
                      vl,vu,il,iu,abstol,m,w,z,ldz,
                      work,lwork,iwork,ifail,info);
    unlockInterface();
    return tmp;
  }

  // explicit instantiations
  template class fastGeneralizedEigenSystem<double>;
  template class fastGeneralizedEigenSystem<float>;
  
  
}

#endif
