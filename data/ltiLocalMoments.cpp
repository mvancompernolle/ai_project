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
 * file .......: ltiLocalMoments.cpp
 * authors ....: Arnd Hannemann
 * organization: LTI, RWTH Aachen
 * creation ...: 28.4.2004
 * revisions ..: $Id: ltiLocalMoments.cpp,v 1.6 2006/09/05 10:20:59 ltilib Exp $
 */

//TODO: include files
#include "ltiLocalMoments.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // localMoments::parameters
  // --------------------------------------------------

  // default constructor
  localMoments::parameters::parameters()
    : localFeatureExtractor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters
    
    scale = 1.0f;
    gridRadius = 3;
    numRays = 24;
    numCircles = 3;
    samplingMethod = FixedRaySampling;
    calculateMean = true;
    calculateVariance = true;
    calculateSkew = true;
    calculateKurtosis = true;
  }

  // copy constructor
  localMoments::parameters::parameters(const parameters& other)
    : localFeatureExtractor::parameters() {
    copy(other);
  }

  // destructor
  localMoments::parameters::~parameters() {
  }

  // get type name
  const char* localMoments::parameters::getTypeName() const {
    return "localMoments::parameters";
  }

  // copy member

  localMoments::parameters&
    localMoments::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    localFeatureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    localFeatureExtractor::parameters& (localFeatureExtractor::parameters::* p_copy)
      (const localFeatureExtractor::parameters&) =
      localFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif

    
      scale = other.scale;
      gridRadius = other.gridRadius;
      numRays = other.numRays;
      numCircles = other.numCircles;
      samplingMethod = other.samplingMethod;
      calculateMean = other.calculateMean;
      calculateVariance = other.calculateVariance;
      calculateSkew = other.calculateSkew;
      calculateKurtosis = other.calculateKurtosis;
    return *this;
  }

  // alias for copy member
  localMoments::parameters&
    localMoments::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* localMoments::parameters::clone() const {
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
  bool localMoments::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool localMoments::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      std::string str;
      switch (samplingMethod) {
          case FixedRaySampling: {
            str = "FixedRaySampling";
            break;
          }
          case FixedGridSampling: {
            str = "FixedGridSampling";
            break;
          }
          default: {
            str = "FixedRaySampling";
          }
      }
      lti::write(handler,"samplingMethod",str);
      lti::write(handler,"calculateMean",calculateMean);
      lti::write(handler,"calculateVariance",calculateVariance);
      lti::write(handler,"calculateSkew",calculateSkew);
      lti::write(handler,"calculateKurtosis",calculateKurtosis);
      lti::write(handler,"scale",scale);
      lti::write(handler,"gridRadius",gridRadius);
      lti::write(handler,"numRays",numRays);
      lti::write(handler,"numCircles",numCircles);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && localFeatureExtractor::parameters::write(handler,false);
# else
    bool (localFeatureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      localFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool localMoments::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool localMoments::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool localMoments::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str;
      lti::read(handler,"samplingMethod",str);
      lti::read(handler,"calculateMean",calculateMean);
      lti::read(handler,"calculateVariance",calculateVariance);
      lti::read(handler,"calculateSkew",calculateSkew);
      lti::read(handler,"calculateKurtosis",calculateKurtosis);
      lti::read(handler,"scale",scale);
      lti::read(handler,"gridRadius",gridRadius);
      lti::read(handler,"numRays",numRays);
      lti::read(handler,"numCircles",numCircles); 
      if (str=="FixedRaySampling") {
        samplingMethod=FixedRaySampling;
      } else if (str=="FixedGridSampling") {
        samplingMethod=FixedGridSampling;
     // default
      } else {
        samplingMethod=FixedRaySampling;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && localFeatureExtractor::parameters::read(handler,false);
# else
    bool (localFeatureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      localFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool localMoments::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // localMoments
  // --------------------------------------------------

  // default constructor
  localMoments::localMoments()
    : localFeatureExtractor(){

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  localMoments::localMoments(const parameters& par)
    : localFeatureExtractor() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  localMoments::localMoments(const localMoments& other)
    : localFeatureExtractor() {
    copy(other);
  }

  // destructor
  localMoments::~localMoments() {
  }

  // returns the name of this type
  const char* localMoments::getTypeName() const {
    return "localMoments";
  }

  // which apply is faster
 //  functor::eFasterApply localMoments::theFaster() const {
//     return OnCopy;
//   }

  // copy member
  localMoments&
    localMoments::copy(const localMoments& other) {
      localFeatureExtractor::copy(other);

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    return (*this);
  }

  // alias for copy member
  localMoments&
    localMoments::operator=(const localMoments& other) {
    return (copy(other));
  }
 

  // clone member
  functor* localMoments::clone() const {
    return new localMoments(*this);
  }

  // return parameters
  const localMoments::parameters&
    localMoments::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


  // ------------
  // own setParameters() method
  // ------------
  bool localMoments::setParameters(const parameters& theParam) {
    bool rc = true;
    rc = rc && (localFeatureExtractor::setParameters(theParam));
    const parameters& par = getParameters();
    // if fixedGridSampling is used the circular mask is calculated here
    if (par.samplingMethod == parameters::FixedGridSampling) {
      rc = rc && initCircle(par.gridRadius,circPoints);
    }
    return rc;
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  bool localMoments::apply(const lti::channel& src,
                           const location& loc,
                           dvector& dest) const {

    bool rc = true;

    const parameters& par = getParameters();

    // first extract circle "vector" from channel
    fvector circle;
    switch (par.samplingMethod) {
        case parameters::FixedRaySampling: {
          rc = rc && fixedRaySampling(src,loc,circle,
                                      par.numRays,par.numCircles,
                                      par.scale,false);
          break;
        }
        case parameters::FixedGridSampling: {
          // circPoints are initialized in setParameters()
          rc = rc && fixedGridSampling(src,loc,circle,circPoints,
                                       par.scale,false);
          break;
        }
    }
   // copy parameters for faster access
    const bool calculateMean = par.calculateMean;
    const bool calculateVariance = par.calculateVariance;
    const bool calculateSkew = par.calculateSkew;
    const bool calculateKurtosis = par.calculateKurtosis;

    // something went wrong if the sample is empty...
    const int size = circle.size();
    const float fsize = static_cast<float>(size);
    if (size==0) {
      setStatusString("No pixels found. Check sampling parameters and location.");
      rc = false;
    }  else {
      // position of the estimated values in the destination vector
      int pos = 0;

      // calculating meand and standard deviation
      //  as it is used for all types
      fvector::const_iterator itb = circle.begin();
      fvector::const_iterator it = itb;
      fvector::const_iterator ite = circle.end();
      float mean = *it;

      // if only mean is demanded 
      if (((calculateMean) && (!(calculateVariance))) && 
          ((!(calculateSkew))   && (!(calculateKurtosis)))) {
        for (it= it+1;it != ite;it++) {
          mean += *it;
        }
        _lti_debug("Only calulating mean...\n");
        dest.resize(1,0.0f,false,false);
        dest[0] = static_cast<double>(mean /fsize);
        return true;
      }

      float devi = *it**it;
      for (it=it+1;it != ite;it++) {
        devi += *it**it;
        mean += *it;
      }
      mean /= fsize;
      devi /= fsize;
      devi -= mean*mean;
      devi *= fsize/(fsize-1.f);
      devi = sqrt(devi);
      _lti_debug("mean = "<<mean<<"\n");
      _lti_debug("devi = "<<devi<<"\n");

     // how many space is needed in the resulting vector ?
      int space=0;
      if(calculateMean) { space++; };
      if(calculateVariance) { space++; };
      if(calculateSkew) { space++;};
      if(calculateKurtosis) { space++;};
      dest.resize(space,0.0f,false,false);

      if (calculateMean) {
        dest[pos++] = static_cast<double>(mean);
      }
      if (calculateVariance) {
        dest[pos++] = static_cast<double>(devi*devi);
      }
      if (calculateSkew) {
        const float b = (*itb)-mean;
        float skew = b*b*b;
        for (it=itb+1;it != ite;it++) {
          const float a = (*it)-mean;
          skew += a*a*a;
        }
        skew /= devi*devi*devi;
        skew *= fsize;
        skew /= (fsize-1.f)*(fsize-2.f);
        dest[pos++]  = static_cast<double>(skew);
      }
      if (calculateKurtosis) {
        const float b = (*itb)-mean;
        float kurt = b*b*b*b;
        // as size is constant for several locations this could be optimized..
        const float c = (fsize-1.f)*(fsize-1.f)*3.f/((fsize-2.f)*(fsize-3.f));
        for (it=itb+1; it != ite;it++) {
          const float a = (*it)-mean;
          kurt += a*a*a*a;
        }
        kurt *= fsize*(fsize+1.f);
        kurt /= (fsize-1.f)*(fsize-2.f)*(fsize-3.f);
        kurt -= c;
        dest[pos++] = static_cast<double>(kurt);
      }

    } // else
    return rc;
  }


  bool localMoments::apply(const lti::channel& src,
                           const std::list<location>& locs,
                           std::list<dvector>& dest) const {

    return (localFeatureExtractor::apply(src,locs,dest));
  }

  bool localMoments::apply(const lti::image& src,
                           const std::list<location>& locs,
                           std::list<dvector>& dest) const {

    return (localFeatureExtractor::apply(src,locs,dest));
  }

  bool localMoments::apply(const lti::image& src,
                           const location& loc,
                           dvector& dest) const {
    return (localFeatureExtractor::apply(src,loc,dest));
  }

}

#include "ltiUndebug.h"
