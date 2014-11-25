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
 * file .......: ltiChannelStatistics.cpp
 * authors ....: Axel Berner, Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 28.2.2002
 * revisions ..: $Id: ltiChannelStatistics.cpp,v 1.11 2006/09/05 10:04:42 ltilib Exp $
 */

#include "ltiChannelStatistics.h"
#include "ltiQuickMedian.h"
#include "ltiSplitImageFactory.h"
#include "ltiSplitImageToRGB.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // channelStatistics::parameters
  // --------------------------------------------------

  // default constructor
  channelStatistics::parameters::parameters()
    : globalFeatureExtractor::parameters(), splitter(0) {
    
    _lti_debug("Normal constructor\n");
    type = Average;
    ignoreColor=Black;
    setSplitter(new splitImageToRGB());
  }

  // copy constructor
  channelStatistics::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters(), splitter(0)  {
    copy(other);
  }

  // destructor
  channelStatistics::parameters::~parameters() {
    if (ownsSplitter) {
      delete splitter;
      splitter=0;
    }
  }

  // get type name
  const char* channelStatistics::parameters::getTypeName() const {
    return "channelStatistics::parameters";
  }

  // copy member

  channelStatistics::parameters&
    channelStatistics::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    globalFeatureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    globalFeatureExtractor::parameters& (globalFeatureExtractor::parameters::* p_copy)
      (const globalFeatureExtractor::parameters&) =
      globalFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif

    type = other.type;
    ignoreColor=other.ignoreColor;
    setSplitter(dynamic_cast<splitImage*>(other.splitter->clone()));

    return *this;
  }

  // alias for copy member
  channelStatistics::parameters&
    channelStatistics::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* channelStatistics::parameters::clone() const {
    return new parameters(*this);
  }

  void channelStatistics::parameters::setSplitter(splitImage* theSplitter,
                                                  const bool own) {
    if (ownsSplitter) {
      delete splitter;
      splitter=0;
    }

    splitter = theSplitter;
    ownsSplitter = own;
  }

  const splitImage& channelStatistics::parameters::getSplitter() const {
    return *splitter;
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool channelStatistics::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool channelStatistics::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    switch (type) {
    case Minimum:
      b = b && lti::write(handler,"type","Minimum");
      break;
    case Maximum:
      b = b && lti::write(handler,"type","Maximum");
      break;
    case Median:
      b = b && lti::write(handler,"type","Median");
      break;
    case Average:
      b = b && lti::write(handler,"type","Average");
      break;
    case Gaussian:
      b = b && lti::write(handler,"type","Gaussian");
      break;
    default:
      b = b && lti::write(handler,"type","Average");
    }

    if (splitter!=0) {
      std::string str;
      className cn;
      cn.get(splitter, str);

      b = b && lti::write(handler, "splitter", str);
    } else {
      b=false;
    }

    b=b && lti::write(handler, "ignoreColor", ignoreColor);

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::write(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      globalFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool channelStatistics::parameters::write(ioHandler& handler,
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
  bool channelStatistics::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool channelStatistics::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    std::string str;
    b = b && lti::read(handler,"type",str);

    if (str == "Minimum") {
      type = Minimum;
    } else if (str == "Maximum") {
      type = Maximum;
    } else if (str == "Average") {
      type = Average;
    } else if (str == "Gaussian") {
      type = Gaussian;
    } else if (str == "Median") {
      type = Median;
    } else {
      type = Average;
    }

    b = b && lti::read(handler, "splitter", str);
    if (ownsSplitter) {
      delete splitter;
      splitter=0;
    }

    splitImageFactory theFactory;  
    setSplitter(theFactory.newInstance(str));

    if (splitter==0) {
      b = false;
      std::string serr="Could not instantiate ";
      serr+=str;
    }
    
    b=b && lti::read(handler, "ignoreColor", ignoreColor);
    
# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::read(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      globalFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool channelStatistics::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // channelStatistics
  // --------------------------------------------------

  // default constructor
  channelStatistics::channelStatistics()
    : globalFeatureExtractor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  channelStatistics::channelStatistics(const channelStatistics& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  channelStatistics::~channelStatistics() {
  }

  // returns the name of this type
  const char* channelStatistics::getTypeName() const {
    return "channelStatistics";
  }

  // copy member
  channelStatistics&
  channelStatistics::copy(const channelStatistics& other) {
    globalFeatureExtractor::copy(other);    
    
    return (*this);
  }

  // alias for copy member
  channelStatistics&
    channelStatistics::operator=(const channelStatistics& other) {
    return (copy(other));
  }


  // clone member
  functor* channelStatistics::clone() const {
    return new channelStatistics(*this);
  }

  // return parameters
  const channelStatistics::parameters&
  channelStatistics::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool channelStatistics::getMin(const channel& ch, const areaPoints& obj,
                                 double& dest) const{

    areaPoints::const_iterator it = obj.begin();
    areaPoints::const_iterator eit = obj.end();
    
    // only non-object pixels?
    if (it==eit) {
      setStatusString("only non-object pixels");
      dest=0.;
      return false;
    }

    float min=ch.at(*it);
    ++it;
    while(it != eit) {
      if(ch.at(*it) < min) {
        min = ch.at(*it);
      }
      ++it;
    }

    dest=min;
    return true;
  };

  bool channelStatistics::getMax(const channel& ch, const areaPoints& obj,
                                 double& dest) const{

    areaPoints::const_iterator it = obj.begin();
    areaPoints::const_iterator eit = obj.end();

    // only non-object pixels?
    if (it==eit) {
      setStatusString("only non-object pixels");
      dest=0.;
      return false;
    }

    float max=ch.at(*it);
    ++it;
    while(it != eit) {
      if(ch.at(*it) > max) {
        max = ch.at(*it);
      }
      ++it;
    }

    dest=max;
    return true;
  };

  bool channelStatistics::getAverage(const channel& ch, const areaPoints& obj,
                                     double& dest) const{

    areaPoints::const_iterator it = obj.begin();
    areaPoints::const_iterator eit = obj.end();

    // only non-object pixels?
    if (it==eit) {
      setStatusString("only non-object pixels");
      dest=0.;
      return false;
    }

    float mean=ch.at(*it);
    float sum=1.0f;
    ++it;
    while(it != eit) {
      mean += ch.at(*it);
      sum++;
      ++it;
    }

    dest=mean/sum;
    return true;
  };

  bool channelStatistics::getGaussian(const channel& ch,
                                      const areaPoints& obj,
                                      double& m, double& v) const {

    areaPoints::const_iterator it = obj.begin();
    areaPoints::const_iterator eit = obj.end();

    // only non-object pixels?
    if (it==eit) {
      setStatusString("only non-object pixels");
      m=0.;
      v=0.;
      return false;
    }

    float mean=ch.at(*it);
    float var=mean*mean;
    float sum=1.0f;
    ++it;
    float tmp;
    while(it != eit) {
      tmp = ch.at(*it);
      mean += tmp;
      var += tmp*tmp;
      sum++;
      ++it;
    }

    m=mean/sum;
    v=var/sum - m*m;
    return true;
  };

  bool channelStatistics::getMedian(const channel& ch, 
                                    const areaPoints& obj,
                                    double& dest) const{

    _lti_debug("gM1");
    areaPoints::const_iterator it = obj.begin();
    areaPoints::const_iterator eit = obj.end();

    // only non-object pixels?
    if (it==eit) {
      setStatusString("only non-object pixels");
      dest=0.;
      return false;
    }

    vector<float> tmp(obj.size());
    vector<float>::iterator vit=tmp.begin();
    _lti_debug("gM2:" << obj.size() << ":");
    _lti_debug(ch.rows() << "x" << ch.columns() << ":");
    while(it != eit) {
      (*vit)=ch.at(*it);
      ++it;
      ++vit;
    }

    _lti_debug("gM3");
    quickMedian<float> qMed;

    dest=qMed.apply(tmp);
    _lti_debug("gM4\n");
    return true;
  };

  // the methods without contour...

  bool channelStatistics::getMin(const channel& ch, 
                                 const channel8& flags, 
                                 double& dest) const{

    channel::const_iterator it = ch.begin();
    channel::const_iterator eit = ch.end();
    channel8::const_iterator f=flags.begin();
    
    assert(flags.size() == ch.size());

    // only non-object pixels?
    if (it==eit) {
      setStatusString("empty input channnel");
      dest=0.;
      return false;
    }

    float min=*it;
    ++it;
    while (it != eit) {
      if (*f++ && *it < min) {
        min = *it;
      }
      ++it;
    }

    dest=min;
    return true;
  };

  bool channelStatistics::getMax(const channel& ch,
                                 const channel8& flags, 
                                 double& dest) const{

    channel::const_iterator it = ch.begin();
    channel::const_iterator eit = ch.end();
    channel8::const_iterator f=flags.begin();

    assert(flags.size() == ch.size());

    // only non-object pixels?
    if (it==eit) {
      setStatusString("only non-object pixels");
      dest=0.;
      return false;
    }

    float max=*it;
    ++it;
    while(it != eit) {
      if((*f++)!=0 && *it > max) {
        max = *it;
      }
      ++it;
    }

    dest=max;
    return true;
  };

  bool channelStatistics::getAverage(const channel& ch, 
                                     const channel8& flags, 
                                     double& dest) const{

    channel::const_iterator it = ch.begin();
    channel::const_iterator eit = ch.end();
    channel8::const_iterator f=flags.begin();

    assert(flags.size() == ch.size());

    // only non-object pixels?
    if (it==eit) {
      setStatusString("only non-object pixels");
      dest=0.;
      return false;
    }

    float mean=*it;
    float sum=1.0f;
    ++it;
    while(it != eit) {
      if ((*f++)!=0) {
        mean += *it;
        sum++;
      }
      ++it;
    }

    dest=mean/sum;
    return true;
  };

  bool channelStatistics::getGaussian(const channel& ch,
                                      const channel8& flags, 
                                      double& m, double& v) const {

    channel::const_iterator it = ch.begin();
    channel::const_iterator eit = ch.end();
    channel8::const_iterator f=flags.begin();

    assert(flags.size() == ch.size());

    // only non-object pixels?
    if (it==eit) {
      setStatusString("only non-object pixels");
      m=0.;
      v=0.;
      return false;
    }

    float mean=*it;
    float var=mean*mean;
    float sum=1.0f;
    ++it;
    float tmp;
    while(it != eit) {
      if ((*f++)!=0) {
        tmp = *it;
        mean += tmp;
        var += tmp*tmp;
        sum++;
      }
      ++it;
    }

    m=mean/sum;
    v=var/sum - m*m;
    return true;
  };

  bool channelStatistics::getMedian(const channel& ch, 
                                    const channel8& flags, 
                                    double& dest) const{

    _lti_debug("hM1");
    channel::const_iterator it = ch.begin();
    channel::const_iterator eit = ch.end();
    channel8::const_iterator f=flags.begin();
    channel8::const_iterator fe=flags.end();

    assert(flags.size() == ch.size());

    // only non-object pixels?
    if (it==eit) {
      setStatusString("only non-object pixels");
      dest=0.;
      return false;
    }

    int count=0;
    while (f != fe) {
      if ((*f++)!=0) {
        count++;
      }
    }

    vector<float> tmp(count);
    vector<float>::iterator vit=tmp.begin();
    _lti_debug("hM2:" << tmp.size() << ":");
    _lti_debug(ch.rows() << "x" << ch.columns() << ":");
    f=flags.begin();
    while(it != eit) {
      if ((*f++)!=0) {
        *vit=*it;
        ++vit;
      }
      ++it;
    }

    _lti_debug("hM3");
    quickMedian<float> qMed;

    dest=qMed.apply(tmp);
    _lti_debug("hM4\n");
    return true;
  };

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool channelStatistics::apply(const image& src, const areaPoints& obj,
                                dvector& dest) const {

    // image empty?
    if (src.empty()) {
      setStatusString("image empty");
      dest.resize(0);
      return false;
    }

    const parameters& param=getParameters();

    channel c1,c2,c3;

    param.getSplitter().apply(src,c1,c2,c3);
    bool b = true;

    switch (param.type) {
      case parameters::Minimum: {
        dest.resize(3);
        b = b && getMin(c1, obj, dest.at(0));
        b = b && getMin(c2, obj, dest.at(1));
        b = b && getMin(c3, obj, dest.at(2));
        return b;
      }
      case parameters::Maximum: {
        dest.resize(3);
        b = b && getMax(c1, obj, dest.at(0));
        b = b && getMax(c2, obj, dest.at(1));
        b = b && getMax(c3, obj, dest.at(2));
        return b;
      }
      case parameters::Average: {
        dest.resize(3);
        b = b && getAverage(c1, obj, dest.at(0));
        b = b && getAverage(c2, obj, dest.at(1));
        b = b && getAverage(c3, obj, dest.at(2));
        return b;
      }
      case parameters::Gaussian: {
        dest.resize(6);
        b = b && getGaussian(c1, obj, dest.at(0), dest.at(3));
        b = b && getGaussian(c2, obj, dest.at(1), dest.at(4));
        b = b && getGaussian(c3, obj, dest.at(2), dest.at(5));
        return b;
      }
      case parameters::Median: {
        dest.resize(3);
        b = b && getMedian(c1, obj, dest.at(0));
        b = b && getMedian(c2, obj, dest.at(1));
        b = b && getMedian(c3, obj, dest.at(2));
        return b;
      }
      default:{
        setStatusString("Kind of filter-type doesn't exist");
        return false;
      }
    }
    return true;
  };

  bool channelStatistics::apply(const image& src, dvector& dest) const {

    // image empty?
    if (src.empty()) {
      setStatusString("image empty");
      dest.resize(0);
      return false;
    }

    const parameters& param=getParameters();

    channel c1,c2,c3;

    channel8 mask(src.size());

    image::const_iterator i=src.begin();
    image::const_iterator ie=src.end();
    channel8::iterator f=mask.begin();
    while (i != ie) {
      *f++ = (*i++ != param.ignoreColor) ? 255 : 0;
    }

    param.getSplitter().apply(src,c1,c2,c3);
    bool b = true;

    switch (param.type) {
      case parameters::Minimum: {
        dest.resize(3);
        b = b && getMin(c1, mask, dest.at(0));
        b = b && getMin(c2, mask, dest.at(1));
        b = b && getMin(c3, mask, dest.at(2));
        return b;
      }
      case parameters::Maximum: {
        dest.resize(3);
        b = b && getMax(c1, mask, dest.at(0));
        b = b && getMax(c2, mask, dest.at(1));
        b = b && getMax(c3, mask, dest.at(2));
        return b;
      }
      case parameters::Average: {
        dest.resize(3);
        b = b && getAverage(c1, mask, dest.at(0));
        b = b && getAverage(c2, mask, dest.at(1));
        b = b && getAverage(c3, mask, dest.at(2));
        return b;
      }
      case parameters::Gaussian: {
        dest.resize(6);
        b = b && getGaussian(c1, mask, dest.at(0), dest.at(3));
        b = b && getGaussian(c2, mask, dest.at(1), dest.at(4));
        b = b && getGaussian(c3, mask, dest.at(2), dest.at(5));
        return b;
      }
      case parameters::Median: {
        dest.resize(3);
        b = b && getMedian(c1, mask, dest.at(0));
        b = b && getMedian(c2, mask, dest.at(1));
        b = b && getMedian(c3, mask, dest.at(2));
        return b;
      }
      default:{
        setStatusString("Kind of filter-type doesn't exist");
        return false;
      }
    }
    return true;
  };


}
