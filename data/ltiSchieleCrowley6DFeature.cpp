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
 * file .......: ltiSchieleCrowley6DFeature.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 27.3.2002
 * revisions ..: $Id: ltiSchieleCrowley6DFeature.cpp,v 1.12 2006/09/05 10:30:17 ltilib Exp $
 */

#include "ltiSchieleCrowley6DFeature.h"
#include "ltiOgdFilter.h"


namespace lti {
  // --------------------------------------------------
  // schieleCrowley6DFeature::parameters
  // --------------------------------------------------

  // default constructor
  schieleCrowley6DFeature::parameters::parameters()
    : localFeatureExtractor::parameters(),
      globalFeatureExtractor::parameters() {

    sigma = double(2);
    numResolutions = 3;
  }

  // copy constructor
  schieleCrowley6DFeature::parameters::parameters(const parameters& other)
    : localFeatureExtractor::parameters(),
      globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  schieleCrowley6DFeature::parameters::~parameters() {
  }

  // get type name
  const char* schieleCrowley6DFeature::parameters::getTypeName() const {
    return "schieleCrowley6DFeature::parameters";
  }

  // copy member

  schieleCrowley6DFeature::parameters&
    schieleCrowley6DFeature::parameters::copy(const parameters& other) {
// We need to use the old workaround here for .NET, as well, since the compiler
// has runtime problems with calling the second copy function.
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    localFeatureExtractor::parameters::copy(other);
    globalFeatureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    localFeatureExtractor::parameters& 
      (localFeatureExtractor::parameters::* p_copy)
      (const localFeatureExtractor::parameters&) =
      localFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);
    globalFeatureExtractor::parameters& 
      (globalFeatureExtractor::parameters::* p_copy2)
      (const globalFeatureExtractor::parameters&) =
      globalFeatureExtractor::parameters::copy;
    (this->*p_copy2)(other);
# endif


    sigma = other.sigma;
    numResolutions = other.numResolutions;

    return *this;
  }

  // alias for copy member
  schieleCrowley6DFeature::parameters&
    schieleCrowley6DFeature::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* schieleCrowley6DFeature::parameters::clone() const {
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
  bool schieleCrowley6DFeature::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool schieleCrowley6DFeature::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"sigma",sigma);
      lti::write(handler,"numResolutions",numResolutions);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && localFeatureExtractor::parameters::write(handler,false);
    b = b && globalFeatureExtractor::parameters::write(handler,false);
# else
    bool (localFeatureExtractor::parameters::* p_writeMS)
      (ioHandler&,const bool) const =
      localFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false); 
    bool (globalFeatureExtractor::parameters::* p_writeMS2)
      (ioHandler&,const bool) const =
      globalFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS2)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool schieleCrowley6DFeature::parameters::write(ioHandler& handler,
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
  bool schieleCrowley6DFeature::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool schieleCrowley6DFeature::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      b=b && lti::read(handler,"sigma",sigma);
      b=b && lti::read(handler,"numResolutions",numResolutions);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && localFeatureExtractor::parameters::read(handler,false);
    b = b && globalFeatureExtractor::parameters::read(handler,false);
# else
    bool (localFeatureExtractor::parameters::* p_readMS)
      (ioHandler&,const bool) =
      localFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
    bool (globalFeatureExtractor::parameters::* p_readMS2)
      (ioHandler&,const bool) =
      globalFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS2)(handler,false);

# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool schieleCrowley6DFeature::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // schieleCrowley6DFeature
  // --------------------------------------------------

  // default constructor
  schieleCrowley6DFeature::schieleCrowley6DFeature()
    : localFeatureExtractor(),
      globalFeatureExtractor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    chnls = 0;
  }

  // copy constructor
  schieleCrowley6DFeature::schieleCrowley6DFeature(const schieleCrowley6DFeature& other)
    : localFeatureExtractor(),
      globalFeatureExtractor()  {
    chnls = 0;
    copy(other);
  }

  // destructor
  schieleCrowley6DFeature::~schieleCrowley6DFeature() {
    delete[] chnls;
  }

  // returns the name of this type
  const char* schieleCrowley6DFeature::getTypeName() const {
    return "schieleCrowley6DFeature";
  }

  // copy member
  schieleCrowley6DFeature&
  schieleCrowley6DFeature::copy(const schieleCrowley6DFeature& other) {
    localFeatureExtractor::copy(other);
    globalFeatureExtractor::copy(other);

    const parameters& param = getParameters();

    delete[] chnls;
    chnls = new channel[param.numResolutions*2];
    for (int i=0;i<param.numResolutions*2;++i) {
      chnls[i].copy(other.chnls[i]);
    }

    return (*this);
  }

  // alias for copy member
  schieleCrowley6DFeature&
  schieleCrowley6DFeature::operator=(const schieleCrowley6DFeature& other) {
    return (copy(other));
  }

  // clone member
  functor* schieleCrowley6DFeature::clone() const {
    return new schieleCrowley6DFeature(*this);
  }

  // return parameters
  const schieleCrowley6DFeature::parameters&
    schieleCrowley6DFeature::getParameters() const {
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
   * @param src original channel with the source data
   * @param mask only the pixels in src, for which the respective pixel in
   *             the mask is different than zero will be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const channel& src,
                                     const channel8& mask,
                                     std::list<dvector>& vcts) const {

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;
    channel* chnls = new channel[dim];
    int i;
    double k = 1.0;
    for (i=0;i<dim;i+=2) {
      getBasisChannels(src,k*param.sigma,chnls[i],chnls[i+1]);
      k*=2.0;
    }

    vcts.clear();

    int x,y;
    dvector fv(dim,0.0);

    for (y=0;y<src.rows();++y) {
      for (x=0;x<src.columns();++x) {
        if (mask.at(y,x) != 0) {
          for (i=0;i<dim;++i) {
            fv.at(i) = chnls[i].at(y,x);
          }
          vcts.push_back(fv);
        }
      }
    }

    delete[] chnls;
    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in the mask (areaPoints list) will
   *             be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const channel& src,
                                     const areaPoints& mask,
                                     std::list<dvector>& vcts) const {
    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;
    channel* chnls = new channel[dim];
    areaPoints::const_iterator it;
    int i;
    double k=1.0;

    for (i=0;i<dim;i+=2) {
      getBasisChannels(src,k*param.sigma,chnls[i],chnls[i+1]);
      k*=2.0;
    }

    vcts.clear();

    dvector fv(dim,0.0);

    for (it=mask.begin();it != mask.end();++it) {
      for (i=0;i<dim;++i) {
        fv.at(i) = chnls[i].at(*it);
      }
      vcts.push_back(fv);
    }
    delete[] chnls;
    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in the mask (areaPoints list) will
   *             be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const channel& src,
                                     const std::list<location>& mask,
                                     std::list<dvector>& vcts) const {

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;
    channel* chnls = new channel[dim];
    std::list<location>::const_iterator it;
    int i;
    double k = 1.0;

    for (i=0;i<dim;i+=2) {
      getBasisChannels(src,k*param.sigma,chnls[i],chnls[i+1]);
      k*=2.0;
    }

    vcts.clear();

    point p;
    dvector fv(dim,0.0);

    for (it=mask.begin();it != mask.end();++it) {
      p.castFrom((*it).position);
      for (i=0;i<dim;++i) {
        fv.at(i) = chnls[i].at(p);
      }
      vcts.push_back(fv);
    }

    delete[] chnls;
    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in src, for which the respective pixel in
   *             the mask is different than zero will be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const channel& src,
                                     const double angle,
                                     const channel8& mask,
                                     std::list<dvector>& vcts) const {

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;

    double stmp, ctmp;
    sincos(angle, stmp, ctmp);
    const double cosa(ctmp);
    const double sina(stmp);

    channel* chnls = new channel[dim];
    int i;
    double k=1.0;

    for (i=0;i<dim;i+=2) {
      getBasisChannels(src,k*param.sigma,chnls[i],chnls[i+1]);
      k*=2.0;
    }

    vcts.clear();

    int x,y;
    float a,b;
    dvector fv(dim,0.0);

    for (y=0;y<src.rows();++y) {
      for (x=0;x<src.columns();++x) {
        if (mask.at(y,x) != 0) {
          for (i=0;i<dim;i+=2) {
            a = chnls[i].at(y,x);
            b = chnls[i+1].at(y,x);
            fv.at(i)   =  cosa*a - sina*b;
            fv.at(i+1) =  sina*a + cosa*b;
          }
          vcts.push_back(fv);
        }
      }
    }

    delete[] chnls;
    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in the mask (areaPoints list) will
   *             be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const channel& src,
                                     const double angle,
                                     const areaPoints& mask,
                                     std::list<dvector>& vcts) const {
    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;
    double stmp, ctmp;
    sincos(angle, stmp, ctmp);
    const double cosa(ctmp);
    const double sina(stmp);
    channel* chnls = new channel[dim];
    areaPoints::const_iterator it;
    int i;
    float a,b;
    double k=1.0;

    for (i=0;i<dim;i+=2) {
      getBasisChannels(src,k*param.sigma,chnls[i],chnls[i+1]);
      k*=2.0;
    }

    vcts.clear();

    dvector fv(dim,0.0);

    for (it=mask.begin();it != mask.end();++it) {
      for (i=0;i<dim;i+=2) {
        a = chnls[i].at(*it);
        b = chnls[i+1].at(*it);
        fv.at(i)   =  cosa*a - sina*b;
        fv.at(i+1) =  sina*a + cosa*b;
      }
      vcts.push_back(fv);
    }

    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in the mask (areaPoints list) will
   *             be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const channel& src,
                                     const double angle,
                                     const std::list<location>& mask,
                                     std::list<dvector>& vcts) const {

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;
    channel* chnls = new channel[dim];
    std::list<location>::const_iterator it;
    double stmp, ctmp;
    sincos(angle, stmp, ctmp);
    const double cosa(ctmp);
    const double sina(stmp);
    int i;
    float a,b;
    double k = 1.0;

    for (i=0;i<dim;i+=2) {
      getBasisChannels(src,k*param.sigma,chnls[i],chnls[i+1]);
      k*=2;
    }

    vcts.clear();

    point p;
    dvector fv(dim,0.0);

    for (it=mask.begin();it != mask.end();++it) {
      p.castFrom((*it).position);
      for (i=0;i<dim;i+=2) {
        a = chnls[i].at(p);
        b = chnls[i+1].at(p);
        fv.at(i)   =  cosa*a - sina*b;
        fv.at(i+1) =  sina*a + cosa*b;
      }
      vcts.push_back(fv);
    }

    return true;
  }

  bool schieleCrowley6DFeature::use(const channel& chnl) {
    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;

    delete[] chnls;
    chnls = new channel[dim];

    if (notNull(chnls)) {
      int i;
      double k = 1.0;
      for (i=0;i<dim;i+=2) {
        getBasisChannels(chnl,k*param.sigma,chnls[i],chnls[i+1]);
        k*=2.0;
      }
      return true;
    }

    setStatusString("Not enough memory available");
    return false;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in src, for which the respective pixel in
   *             the mask is different than zero will be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const channel8& mask,
                                     std::list<dvector>& vcts) const {

    if (isNull(chnls)) {
      setStatusString("No channel was specified (see method use())");
      return false;
    }

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;
    int i;

    vcts.clear();

    int x,y;
    dvector fv(dim,0.0);

    const int lastx = chnls[0].columns();
    const int lasty = chnls[0].rows();

    for (y=0;y<lasty;++y) {
      for (x=0;x<lastx;++x) {
        if (mask.at(y,x) != 0) {
          for (i=0;i<dim;++i) {
            fv.at(i) = chnls[i].at(y,x);
          }
          vcts.push_back(fv);
        }
      }
    }

    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in the mask (areaPoints list) will
   *             be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const areaPoints& mask,
                                     std::list<dvector>& vcts) const {
    if (isNull(chnls)) {
      setStatusString("No channel was specified (see method use())");
      return false;
    }

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;

    areaPoints::const_iterator it;
    int i;

    vcts.clear();

    dvector fv(dim,0.0);

    for (it=mask.begin();it != mask.end();++it) {
      for (i=0;i<dim;++i) {
        fv.at(i) = chnls[i].at(*it);
      }
      vcts.push_back(fv);
    }

    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in the mask (areaPoints list) will
   *             be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const std::list<location>& mask,
                                     std::list<dvector>& vcts) const {

    if (isNull(chnls)) {
      setStatusString("No channel was specified (see method use())");
      return false;
    }

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;

    std::list<location>::const_iterator it;
    int i;

    vcts.clear();

    point p;
    dvector fv(dim,0.0);

    for (it=mask.begin();it != mask.end();++it) {
      p.castFrom((*it).position);
      for (i=0;i<dim;++i) {
        fv.at(i) = chnls[i].at(p);
      }
      vcts.push_back(fv);
    }

    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in src, for which the respective pixel in
   *             the mask is different than zero will be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const double angle,
                                     const channel8& mask,
                                     std::list<dvector>& vcts) const {

    if (isNull(chnls)) {
      setStatusString("No channel was specified (see method use())");
      return false;
    }

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;

    double stmp, ctmp;
    sincos(angle, stmp, ctmp);
    const double cosa(ctmp);
    const double sina(stmp);

    int i;

    vcts.clear();

    int x,y;
    float a,b;
    dvector fv(dim,0.0);

    const int lastx = chnls[0].columns();
    const int lasty = chnls[0].rows();

    for (y=0;y<lasty;++y) {
      for (x=0;x<lastx;++x) {
        if (mask.at(y,x) != 0) {
          for (i=0;i<dim;i+=2) {
            a = chnls[i].at(y,x);
            b = chnls[i+1].at(y,x);
            fv.at(i)   =  cosa*a - sina*b;
            fv.at(i+1) =  sina*a + cosa*b;
          }
          vcts.push_back(fv);
        }
      }
    }

    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in the mask (areaPoints list) will
   *             be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const double angle,
                                     const areaPoints& mask,
                                     std::list<dvector>& vcts) const {
    if (isNull(chnls)) {
      setStatusString("No channel was specified (see method use())");
      return false;
    }

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;
    double stmp, ctmp;
    sincos(angle, stmp, ctmp);
    const double cosa(ctmp);
    const double sina(stmp);
    areaPoints::const_iterator it;
    int i;
    float a,b;

    vcts.clear();

    dvector fv(dim,0.0);

    for (it=mask.begin();it != mask.end();++it) {
      for (i=0;i<dim;i+=2) {
        a = chnls[i].at(*it);
        b = chnls[i+1].at(*it);
        fv.at(i)   =  cosa*a - sina*b;
        fv.at(i+1) =  sina*a + cosa*b;
      }
      vcts.push_back(fv);
    }

    return true;
  }

  /*
   * operates on a copy of the given %parameters.
   * @param src original channel with the source data
   * @param mask only the pixels in the mask (areaPoints list) will
   *             be considered.
   * @param vcts the list of all feature vectors for all masked pixels
   *
   * @return true if apply successful or false otherwise.
   */
  bool schieleCrowley6DFeature::apply(const double angle,
                                     const std::list<location>& mask,
                                     std::list<dvector>& vcts) const {

    if (isNull(chnls)) {
      setStatusString("No channel was specified (see method use())");
      return false;
    }

    const parameters& param = getParameters();
    const int dim = 2*param.numResolutions;

    channel* chnls = new channel[dim];
    std::list<location>::const_iterator it;
    double stmp, ctmp;
    sincos(angle, stmp, ctmp);
    const double cosa(ctmp);
    const double sina(stmp);
    int i;
    float a,b;

    vcts.clear();

    point p;
    dvector fv(dim,0.0);

    for (it=mask.begin();it != mask.end();++it) {
      p.castFrom((*it).position);
      for (i=0;i<dim;i+=2) {
        a = chnls[i].at(p);
        b = chnls[i+1].at(p);
        fv.at(i)   =  cosa*a - sina*b;
        fv.at(i+1) =  sina*a + cosa*b;
      }
      vcts.push_back(fv);
    }

    return true;
  }


  void schieleCrowley6DFeature::getBasisChannels(const channel& chnl,
                                                const double sigma,
                                                channel& Dx,
                                                channel& Dy) const {

    // ogd filter
    ogdFilter ogd;
    ogdFilter::parameters ogdPar;

    ogdPar.order = 1;
    ogdPar.size = iround(5*sigma);
    ogdPar.variance = sigma*sigma;
    ogdPar.angle = 0;

    ogd.setParameters(ogdPar);

    // get the basis channels
    ogd.generateBasisOgd1(chnl,Dx,Dy);
  }


}
