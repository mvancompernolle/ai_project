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
 * file .......: ltiThresholdSegmentation.cpp
 * authors ....: Alexandros Matsikis, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 26.04.2000
 * revisions ..: $Id: ltiThresholdSegmentation.cpp,v 1.10 2006/09/05 10:32:24 ltilib Exp $
 */

#define _LTI_IGNORE_THRESH_DEP_WARNING 1

#include "ltiThresholdSegmentation.h"

namespace lti {
  // --------------------------------------------------
  // thresholdSegmentation::parameters
  // --------------------------------------------------

  // static member definition

  const float thresholdSegmentation::parameters::Original = float(-1.0E-30);

  // default constructor
  thresholdSegmentation::parameters::parameters(const bool& useChannel8)
    : segmentation::parameters() {

    if (useChannel8) {
      inRegionValue = Original;
      outRegionValue = 0.0f;

      highThreshold = 255.0f;
      lowThreshold = 127.0f;
    } else {
      inRegionValue = Original;
      outRegionValue = 0.0f;

      highThreshold = 1.0e+6f;
      lowThreshold = 0.5f;
    }
  }

  // copy constructor
  thresholdSegmentation::parameters::parameters(const parameters& other)
    : segmentation::parameters() {
    copy(other);
  }

  // destructor
  thresholdSegmentation::parameters::~parameters() {
  }

  // get type name
  const char* thresholdSegmentation::parameters::getTypeName() const {
    return "thresholdSegmentation::parameters";
  }

  // copy member

  thresholdSegmentation::parameters&
    thresholdSegmentation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    segmentation::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    segmentation::parameters& (segmentation::parameters::* p_copy)
      (const segmentation::parameters&) =
      segmentation::parameters::copy;
    (this->*p_copy)(other);
# endif

      inRegionValue  = other.inRegionValue;
      outRegionValue = other.outRegionValue;
      highThreshold  = other.highThreshold;
      lowThreshold   = other.lowThreshold;

      return *this;
  }

  // clone member
  functor::parameters* thresholdSegmentation::parameters::clone() const {
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
  bool thresholdSegmentation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool thresholdSegmentation::parameters::writeMS(ioHandler& handler,
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
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::write(handler,false);
# else
    bool (segmentation::parameters::* p_writeMS)(ioHandler&,const bool) const =
      segmentation::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool thresholdSegmentation::parameters::write(ioHandler& handler,
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
  bool thresholdSegmentation::parameters::read(ioHandler& handler,
                                               const bool complete)
# else
  bool thresholdSegmentation::parameters::readMS(ioHandler& handler,
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
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::read(handler,false);
# else
    bool (segmentation::parameters::* p_readMS)(ioHandler&,const bool) =
      segmentation::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool thresholdSegmentation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // thresholdSegmentation
  // --------------------------------------------------

  // default constructor
  thresholdSegmentation::thresholdSegmentation()
    : segmentation(){
  }

  // copy constructor
  thresholdSegmentation::thresholdSegmentation(const thresholdSegmentation& other)
    : segmentation()  {
    copy(other);
  }

  // destructor
  thresholdSegmentation::~thresholdSegmentation() {
  }

  // returns the name of this type
  const char* thresholdSegmentation::getTypeName() const {
    return "thresholdSegmentation";
  }

  // copy member
  thresholdSegmentation&
  thresholdSegmentation::copy(const thresholdSegmentation& other) {
    segmentation::copy(other);
    return (*this);
  }

  // clone member
  functor* thresholdSegmentation::clone() const {
    return new thresholdSegmentation(*this);
  }

  // return parameters
  const thresholdSegmentation::parameters&
    thresholdSegmentation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // set the parameters
  bool thresholdSegmentation::updateParameters() {

    const parameters& param = getParameters();
    int i,low,high;

    ubyte out,in;

    // initialize lookup table:
    low  = int(param.lowThreshold);
    high = int(param.highThreshold);
    out = ubyte(param.outRegionValue);
    in  = ubyte(param.inRegionValue);

    // look-up tables can be initialized if and only if the values
    // are in the valid range for the channel8;

    if ((high>255.0) || (high<0.0) ||
        (low>255.0) || (low<0.0)) {
      setStatusString("high or low values in the parameters out of range");
      return false;
    }

    // initialize out of range values

    if (param.outRegionValue != parameters::Original) {
      // if original channel value must NOT be keeped
      for (i=0;i<low;i++) {
        lut[i]=out;
        mask[i] = 0;
      }
      for (i=high+1;i<256;i++) {
        lut[i]=out;
        mask[i] = 0;
      }
    } else {
      // if original channel value must BE keeped
      for (i=0;i<low;i++) {
        lut[i]=ubyte(i);
        mask[i] = 0;
      }
      for (i=high+1;i<256;i++) {
        lut[i]=ubyte(i);
        mask[i] = 0;
      }
    }

    // initialize in range values

    if (param.inRegionValue != parameters::Original) {
      // if original channel value must NOT be keeped
      for (i=low;i<=high;i++) {
        lut[i]=in;
        mask[i] = 1;
      }
    } else {
      // if original channel value must BE keeped
      // if original channel value must NOT be keeped
      for (i=low;i<=high;i++) {
        lut[i]=ubyte(i);
        mask[i] = 1;
      }
    }

    return true;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  // On place apply for type channel8!
  channel8& thresholdSegmentation::apply(channel8& srcdest) const
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

    return srcdest;
  }

  // On place apply for type areaPoints with input a single band image!
  areaPoints& thresholdSegmentation::apply(const channel8& src,
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

    return dest;
  }

  // On place apply for type areaPoints with input a single band image!
  areaPoints& thresholdSegmentation::apply(const channel& src,
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

    return dest;
  }


  // On copy apply for type channel8!
  channel8& thresholdSegmentation::apply(const channel8& src,
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

    return dest;
  }

  // On copy apply for type channel!
  channel& thresholdSegmentation::apply(channel& srcdest) const {

    channel tmp;

    apply(srcdest,tmp);
    tmp.detach(srcdest);

    return srcdest;
  }


  // On copy apply for type channel!
  channel& thresholdSegmentation::apply(const channel& src,
                                              channel& dest) const {

    const parameters& param = getParameters();

    float low,high;

    int y;
    vector<channel::value_type>::iterator it;
    vector<channel::value_type>::const_iterator cit,e;

    // prepare background

    if (param.outRegionValue != parameters::Original) {
      dest.resize(src.rows(),src.columns(),param.outRegionValue,false,true);
    } else {
      dest.copy(src);
    }

    low = param.lowThreshold;
    high = param.highThreshold;

    if (param.inRegionValue != parameters::Original) {

      for (y=0;y<src.rows();y++) {
        const vector<channel::value_type>& vct = src.getRow(y);
        for (cit=vct.begin(),e=vct.end(),it=dest.getRow(y).begin();
             cit!=e;
             cit++,it++) {

          if (((*cit)>=low) && ((*cit)<=high)) {
            (*it) = param.inRegionValue;
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
            (*it) = (*cit);
          }
        }
      }

    }

    return dest;
  }

  // On copy apply for type channel!
  channel8& thresholdSegmentation::apply(const channel& src,
                                               channel8& dest) const {

    const parameters& param = getParameters();

    float low,high;

    int y;
    vector<channel8::value_type>::iterator it;
    vector<channel::value_type>::const_iterator cit,e;

    // prepare background

    if (param.outRegionValue != parameters::Original) {
      dest.resize(src.rows(),src.columns(),
                  ubyte(param.outRegionValue),false,true);
    } else {
      dest.castFrom(src);
    }

    low = param.lowThreshold;
    high = param.highThreshold;

    if (param.inRegionValue != parameters::Original) {

      for (y=0;y<src.rows();y++) {
        const vector<channel::value_type>& vct = src.getRow(y);
        for (cit=vct.begin(),e=vct.end(),it=dest.getRow(y).begin();
             cit!=e;
             cit++,it++) {

          if (((*cit)>=low) && ((*cit)<=high)) {
            (*it) = ubyte(param.inRegionValue);
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
            (*it) = ubyte(255.0f*(*cit));
          }
        }
      }

    }

    return dest;
  }

  // On copy apply for type areaPoints with input a single band image and a
  // areaPoints to make the search space smaller !
  areaPoints& thresholdSegmentation::apply(const channel8& src,
                                        const areaPoints& comp,
                                              areaPoints& dest) const {

    // check if the comp areaPoints lies within the src channel

    dest.clear();

    lti::rectangle compbounds;
    compbounds=comp.getBoundary();
    point size = compbounds.br;
//    size.add(point(2,2));

    if ((src.columns()<size.x)||(src.rows()<size.y)) {
      throw exception("ThresholdSegmentation: Image smaller than the areaPoints");
    };

    areaPoints::const_iterator it;

    for (it = comp.begin();it != comp.end();it++) {
      if (mask[src.at(*it)]!=0) {
        dest.push_back(*it);
      }
    }

    return dest;
  };

  // On copy apply for type areaPoints with input a single band image and a
  // areaPoints to make the search space smaller !
  areaPoints& thresholdSegmentation::apply(const channel& src,
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
      throw exception("ThresholdSegmentation: Image smaller than the areaPoints");
    };

    areaPoints::const_iterator it;

    for (it = comp.begin();it != comp.end();it++) {
      if ((src.at(*it)>=param.lowThreshold)&&
          (src.at(*it)<=param.highThreshold)) {
        dest.push_back(*it);
      }
    }

    return dest;
  };


}
