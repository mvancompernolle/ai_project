/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiThresholding.cpp
 * authors ....: Alexandros Matsikis, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 26.04.2000
 * revisions ..: $Id: ltiThresholding.cpp,v 1.7 2006/09/05 10:32:37 ltilib Exp $
 */

#include "ltiThresholding.h"
#include <limits>

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // thresholding::parameters
  // --------------------------------------------------

  // static member definition

  //const float thresholding::parameters::Original = float(-1.0E-30);

  // default constructor
  thresholding::parameters::parameters()
    : modifier::parameters() {

     // default parameters
      inRegionValue = 1.f;
      outRegionValue = 0.0f;
      keepInRegion = true;
      keepOutRegion = false;

      highThreshold = std::numeric_limits<float>::max();
      lowThreshold = 0.5f;
  }

  // copy constructor
  thresholding::parameters::parameters(const parameters& other)
    : modifier::parameters() {
    copy(other);
  }

  // destructor
  thresholding::parameters::~parameters() {
  }

  // get type name
  const char* thresholding::parameters::getTypeName() const {
    return "thresholding::parameters";
  }

  // copy member

  thresholding::parameters&
    thresholding::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    modifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    modifier::parameters& (modifier::parameters::* p_copy)
      (const modifier::parameters&) =
      modifier::parameters::copy;
    (this->*p_copy)(other);
# endif

      inRegionValue  = other.inRegionValue;
      outRegionValue = other.outRegionValue;
      highThreshold  = other.highThreshold;
      lowThreshold   = other.lowThreshold;
      keepInRegion = other.keepInRegion;
      keepOutRegion = other.keepOutRegion;

      return *this;
  }

  // clone member
  functor::parameters* thresholding::parameters::clone() const {
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
  bool thresholding::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool thresholding::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"inRegionValue",inRegionValue);
      lti::write(handler,"outRegionValue",outRegionValue);
      lti::write(handler,"highThreshold",highThreshold);
      lti::write(handler,"lowThreshold",lowThreshold);
      lti::write(handler,"keepInRegion",keepInRegion);
      lti::write(handler,"keepOutRegion",keepOutRegion);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool thresholding::parameters::write(ioHandler& handler,
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
  bool thresholding::parameters::read(ioHandler& handler,
                                               const bool complete)
# else
  bool thresholding::parameters::readMS(ioHandler& handler,
                                                 const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"inRegionValue",inRegionValue);
      lti::read(handler,"outRegionValue",outRegionValue);
      lti::read(handler,"highThreshold",highThreshold);
      lti::read(handler,"lowThreshold",lowThreshold);
      lti::read(handler,"keepInRegion",keepInRegion);
      lti::read(handler,"keepOutRegion",keepOutRegion);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool thresholding::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // thresholding
  // --------------------------------------------------

  // default constructor
  thresholding::thresholding()
    : modifier(){
    // set Default parameters
    parameters DefaultParameters = parameters();
    setParameters(DefaultParameters);

  }

  // default constructor
  thresholding::thresholding(const parameters& par) {
    setParameters(par);
  }

  // copy constructor
  thresholding::thresholding(const thresholding& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  thresholding::~thresholding() {
  }

  // returns the name of this type
  const char* thresholding::getTypeName() const {
    return "thresholding";
  }

  // copy member
  thresholding& thresholding::copy(const thresholding& other) {
    modifier::copy(other);
    return (*this);
  }

  // clone member
  functor* thresholding::clone() const {
    return new thresholding(*this);
  }

  // return parameters
  const thresholding::parameters&
    thresholding::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // set the parameters
  bool thresholding::updateParameters() {
    const parameters& param = getParameters();

    return initLut(lut,mask,
                   param.lowThreshold,param.highThreshold,
                   param.keepInRegion,param.keepOutRegion,
                   param.inRegionValue,param.outRegionValue);
  }

  // init lut for faster channel8 processing
  bool thresholding::initLut(ubyte *lut, ubyte *mask, 
               const float lowThreshold, const float highThreshold, 
               const bool keepInRegion, const bool keepOutRegion,
               const float inRegionValue, const float outRegionValue) const {    
    
    int i,low,high;
    ubyte out,in;

    // initialize lookup table for channel8
    // scale the values from 0-1 to 0-255
    low  = static_cast<int>(lowThreshold*255);

    // for channel8 cut off too high threshold
    high = highThreshold>1.0f ? 255 : static_cast<int>(highThreshold*255);
    out = static_cast<ubyte>(outRegionValue*255);
    in  = static_cast<ubyte>(inRegionValue*255);

    // look-up tables can be initialized if and only if the values
    // are in the valid range for the channel8;

    if ((high>255) || (high<0) ||
        (low>255) || (low<0)) {
      setStatusString("high or low values in the parameters out of range");
      _lti_debug("ouf of range: low="<<low <<" high="<<high << std::endl);
      return false;
    }

    // initialize out of range values
    _lti_debug("initialize out of range values\n");
    if (keepOutRegion) {
      // if original channel value must BE kept
      for (i=0;i<low;i++) {
        lut[i]=ubyte(i);
        mask[i] = 0;
      }
      for (i=high+1;i<256;i++) {
        lut[i]=ubyte(i);
        mask[i] = 0;
      }
    } else {
      // if original channel value must NOT be kept
      for (i=0;i<low;i++) {
        lut[i]=out;
        mask[i] = 0;
      }
      for (i=high+1;i<256;i++) {
        lut[i]=out;
        mask[i] = 0;
      }
    }

    // initialize in range values
    _lti_debug("initialize in range values\n");
    if (keepInRegion) {
      // if original channel value must BE kept
      for (i=low;i<=high;i++) {
        lut[i]=static_cast<ubyte>(i);
        mask[i] = 1;
      }
    } else {
      // if original channel value must NOT be kept
      for (i=low;i<=high;i++) {
        lut[i]=in;
        mask[i] = 1;
      }
    }

    return true;
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  // On place apply for type channel8!
  bool thresholding::apply(channel8& srcdest) const
  {

    int y;
    vector<ubyte>::iterator it,e;

    // lookup table initialized in "setParameters":

    for (y=0;y<srcdest.rows();y++) {
      vector<ubyte>& vct = srcdest.getRow(y);
      for (it=vct.begin(),e=vct.end();it!=e;it++) {
        (*it) = lut[*it];
      }
    }

    return true;
  }


  // On place apply for type areaPoints with input a single band image!
  bool thresholding::apply(const channel8& src,
                           areaPoints& dest) const {


    point p;

    dest.clear();

    // lookup table initialized in "setParameters":

    for (p.y=0;p.y<src.rows();p.y++) {
      for (p.x=0;p.x<src.columns();p.x++) {
        if (mask[src.at(p)] != 0) {
          dest.push_back(p);
        }
      }
    }

    return true;
  }

  // On place apply for type areaPoints with input a single band image!
  bool thresholding::apply(const channel& src,
                           areaPoints& dest) const {

    const parameters& param = getParameters();
    point p;
    dest.clear();

    // lookup table initialized in "setParameters":

    for (p.y=0;p.y<src.rows();p.y++) {
      for (p.x=0;p.x<src.columns();p.x++) {
        if ((src.at(p)>=param.lowThreshold) &&
            (src.at(p)<=param.highThreshold)) {
          dest.push_back(p);
        }
      }
    }

    return true;
  }


  // On copy apply for type channel8!
  bool thresholding::apply(const channel8& src,
                           channel8& dest) const {

    int y;
    vector<ubyte>::iterator it;
    vector<ubyte>::const_iterator cit,e;

    dest.resize(src.rows(),src.columns(),0,false,false);

    // lookup table initialized in "setParameters":

    for (y=0;y<src.rows();y++) {
      const vector<ubyte>& vct = src.getRow(y);
      for (cit=vct.begin(),e=vct.end(),it=dest.getRow(y).begin();
           cit!=e;
           cit++,it++) {
        (*it) = lut[*cit];
      }
    }

    return true;
  }

  // On copy apply for type channel8 with parameters
  bool thresholding::apply(const channel8& src, channel8& dest, 
                           const float lowThreshold, const float highThreshold,
                           const bool keepInRegion, const bool keepOutRegion) const {


    int y;
    vector<ubyte>::iterator it;
    vector<ubyte>::const_iterator cit,e;
    parameters param = getParameters();

    dest.resize(src.rows(),src.columns(),0,false,false);

    // initialize lookuptable
    ubyte myLut[256];
    ubyte myMask[256];
    if (!(initLut(myLut,myMask,
                  lowThreshold,highThreshold,
                  keepInRegion,keepOutRegion,
                  param.inRegionValue,param.outRegionValue))) 
      return false;

    for (y=0;y<src.rows();y++) {
      const vector<ubyte>& vct = src.getRow(y);
      for (cit=vct.begin(),e=vct.end(),it=dest.getRow(y).begin();
           cit!=e;
           cit++,it++) {
        (*it) = lut[*cit];
      }
    }

    return true;

  }

  // On place apply for type channel!
  bool thresholding::apply(channel& srcdest) const {

    channel tmp;

    bool b = apply(srcdest,tmp);
    tmp.detach(srcdest);

    return b;
  }


  // On copy apply for type channel!
   bool thresholding::apply(const channel& src,
                                  channel& dest) const {
    _lti_debug("apply(channel,channel)\n");

    const parameters& param = getParameters();     
    return apply(src,dest,param.lowThreshold,param.highThreshold,
                 param.keepInRegion, param.keepOutRegion,
                 param.inRegionValue, param.outRegionValue);
  }

  // on copy apply for type channel with parameters
  bool thresholding::apply(const channel& src,
                           channel& dest,
                           const float lowThreshold, const float highThreshold,
                           const bool keepInRegion, const bool keepOutRegion,
                           const float inRegionValue, const float outRegionValue) const {
    float low,high;

    int y;
    vector<channel::value_type>::iterator it;
    vector<channel::value_type>::const_iterator cit,e;

   
    // prepare background
    if (keepOutRegion) {
      _lti_debug("copy src -> dest \n");
      dest.copy(src);
    } else {
      _lti_debug("fill dest with "<<outRegionValue<<"\n");
      dest.resize(src.rows(),src.columns(),outRegionValue,false,true);
    }

    low = lowThreshold;
    high = highThreshold;

    if (!(keepInRegion)) {
      _lti_debug("replace inRegionPixels with "<<inRegionValue<<"\n");
      for (y=0;y<src.rows();y++) {
        const vector<channel::value_type>& vct = src.getRow(y);
        for (cit=vct.begin(),e=vct.end(),it=dest.getRow(y).begin();
             cit!=e;
             cit++,it++) {

          if (((*cit)>=low) && ((*cit)<=high)) {
            (*it) = inRegionValue;
          }

        }
      }

    } else {
      _lti_debug("copy inRegion Pixels from src to dest\n");
      for (y=0;y<src.rows();y++) {
        const vector<channel::value_type>& vct = src.getRow(y);
        for (cit=vct.begin(),e=vct.end(),it=dest.getRow(y).begin();
             cit!=e;
             cit++,it++) {

          if (((*cit)>=low) && ((*cit)<=high)) {
            (*it) = (*cit);
          }
        }
      }

    }
    return true;
  }

  

  // On copy apply for type channel with destination channel8
  bool thresholding::apply(const channel& src,
                                 channel8& dest) const {
    _lti_debug("on copy apply for type channel");
    const parameters& param = getParameters();

    float low,high;

    int y;
    vector<channel8::value_type>::iterator it;
    vector<channel::value_type>::const_iterator cit,e;

    // prepare background

    if (!(param.keepOutRegion)) {
      dest.resize(src.rows(),src.columns(),
                  static_cast<ubyte>(param.outRegionValue*255),false,true);
    } else {
      dest.castFrom(src);
    }

    low = param.lowThreshold;
    high = param.highThreshold;

    if (!(param.keepInRegion)) {

      for (y=0;y<src.rows();y++) {
        const vector<channel::value_type>& vct = src.getRow(y);
        for (cit=vct.begin(),e=vct.end(),it=dest.getRow(y).begin();
             cit!=e;
             cit++,it++) {

          if (((*cit)>=low) && ((*cit)<=high)) {
            (*it) = static_cast<ubyte>(param.inRegionValue*255);
          }
        }
      }

    } else {

      for (y=0;y<src.rows();y++) {
        const vector<channel::value_type>& vct = src.getRow(y);
        for (cit=vct.begin(),e=vct.end(),it=dest.getRow(y).begin();
             cit!=e;
             cit++,it++) {

          if (((*cit)>=low) && ((*cit)<=high)) {
            (*it) = static_cast<ubyte>(255.0f*(*cit));
          }
        }
      }

    }

    return true;
  }

  // On copy apply for type areaPoints with input a single band image and a
  // areaPoints to make the search space smaller !
  bool thresholding::apply(const channel8& src,
                           const areaPoints& comp,
                                 areaPoints& dest) const {

    // check if the comp areaPoints lies within the src channel

    dest.clear();

    lti::rectangle compbounds;
    compbounds=comp.getBoundary();
    point size = compbounds.br;
//    size.add(point(2,2));

    if ((src.columns()<size.x)||(src.rows()<size.y)) {
      throw exception("Thresholding: Image smaller than the areaPoints");
    };

    areaPoints::const_iterator it;

    for (it = comp.begin();it != comp.end();it++) {
      if (mask[src.at(*it)]!=0) {
        dest.push_back(*it);
      }
    }

    return true;
  };

  // On copy apply for type areaPoints with input a single band image and a
  // areaPoints to make the search space smaller !
  bool thresholding::apply(const channel& src,
                           const areaPoints& comp,
                                 areaPoints& dest) const {

    const parameters& param = getParameters();

    // check if the comp areaPoints lies within the src channel

    dest.clear();

    lti::rectangle compbounds;
    compbounds=comp.getBoundary();
    point size = compbounds.br;
//    size.add(point(2,2));

    if ((src.columns()<size.x)||(src.rows()<size.y)) {
      throw exception("Thresholding: Image smaller than the areaPoints");
    };

    areaPoints::const_iterator it;

    for (it = comp.begin();it != comp.end();it++) {
      if ((src.at(*it)>=param.lowThreshold)&&
          (src.at(*it)<=param.highThreshold)) {
        dest.push_back(*it);
      }
    }

    return true;
  };


}

#include "ltiUndebug.h"
