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
 * file .......: ltiRegionMerge.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 18.3.2002
 * revisions ..: $Id: ltiRegionMerge.cpp,v 1.8 2006/09/05 10:28:27 ltilib Exp $
 */

//TODO: include files
#include "ltiRegionMerge.h"

namespace lti {
  // --------------------------------------------------
  // regionMerge::parameters
  // --------------------------------------------------

  // default constructor
  regionMerge::parameters::parameters()
    : functor::parameters() {
    threshold = double(0.001);
  }

  // copy constructor
  regionMerge::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  regionMerge::parameters::~parameters() {
  }

  // get type name
  const char* regionMerge::parameters::getTypeName() const {
    return "regionMerge::parameters";
  }

  // copy member

  regionMerge::parameters&
    regionMerge::parameters::copy(const parameters& other) {
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


    threshold = other.threshold;

    return *this;
  }

  // alias for copy member
  regionMerge::parameters&
    regionMerge::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* regionMerge::parameters::clone() const {
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
  bool regionMerge::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool regionMerge::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"threshold",threshold);
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
  bool regionMerge::parameters::write(ioHandler& handler,
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
  bool regionMerge::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool regionMerge::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"threshold",threshold);
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
  bool regionMerge::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // regionMerge
  // --------------------------------------------------

  // default constructor
  regionMerge::regionMerge()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  regionMerge::regionMerge(const regionMerge& other)
    : functor()  {
    copy(other);
  }

  // destructor
  regionMerge::~regionMerge() {
  }

  // returns the name of this type
  const char* regionMerge::getTypeName() const {
    return "regionMerge";
  }

  // copy member
  regionMerge&
    regionMerge::copy(const regionMerge& other) {
      functor::copy(other);

    return (*this);
  }

  // alias for copy member
  regionMerge&
    regionMerge::operator=(const regionMerge& other) {
    return (copy(other));
  }


  // clone member
  functor* regionMerge::clone() const {
    return new regionMerge(*this);
  }

  // return parameters
  const regionMerge::parameters&
    regionMerge::getParameters() const {
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

  /*
   * operates on a copy of the given %parameters.
   * @param srcmask source mask.  Each object must be represented by one
   *                              label.
   * @param simMat The similarity matrix.  The size of the matrix must
   *               be at least equal the number of labels plus one.
   * @param destMask resulting mask with merged objects.
   * @return true if apply successful or false otherwise.
   */
  bool regionMerge::apply(const imatrix& srcmask,
                          const dmatrix& simMat,
                          imatrix& destmask) const {
    int i,j;
    ivector eLab(simMat.rows());
    const double thr = getParameters().threshold;

    for (i=0;i<eLab.size();++i) {
      eLab.at(i) = i;
    }

    for (j=0;j<simMat.rows();++j) {
      for (i=0;i<simMat.columns();++i) {
        if (simMat.at(j,i) > thr) {
          if (eLab.at(j)>eLab.at(i)) {
            eLab.at(j)=eLab.at(i);
          } else {
            eLab.at(i)=eLab.at(j);
          }

        }
      }
    }

    // now correct the labels
    for (j=eLab.size()-1;j>=0;--j) {
      i = j;
      while (eLab.at(i) != i) {
        i=eLab.at(i);
      }
      eLab.at(j) = i;
    }

    destmask.resize(srcmask.size(),0,false,false);

    for (j=0;j<srcmask.rows();++j)
      for (i=0;i<srcmask.columns();++i) {
        destmask.at(j,i) = eLab.at(srcmask.at(j,i));
      }

    return true;
  };

  bool regionMerge::apply(const imatrix& srcmask,
                          const dmatrix& simMat,
                          const dvector& thresholds,
                          imatrix& destmask) const {
    int i,j;
    const dvector& thrs = thresholds;

    ivector eLab(simMat.rows());
    for (i=0;i<eLab.size();++i)
      eLab.at(i) = i;

    double a;
    for (j=0;j<simMat.rows();++j)
      for (i=0;i<simMat.columns();++i) {
        if (simMat.at(j,i) > (a=max(thrs.at(j),thrs.at(i))) ) {
//          cout<<j<<" "<<i<<" "<<simMat.at(j,i)<<" "<<a<<endl;
          if (eLab.at(j)>eLab.at(i)) {
            eLab.at(j)=eLab.at(i);
          } else {
            eLab.at(i)=eLab.at(j);
          }

        }
      }


    // now correct the labels
    for (j=eLab.size()-1;j>=0;--j) { //todo
      i = j;
      while (eLab.at(i) != i) {
        i=eLab.at(i);
      }
      eLab.at(j) = i;
    }

    destmask.resize(srcmask.size(),0,false,false);
    for (j=0;j<srcmask.rows();++j)
      for (i=0;i<srcmask.columns();++i) {
        destmask.at(j,i) = eLab.at(srcmask.at(j,i));
      }

    return true;
 };


}
