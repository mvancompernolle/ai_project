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
 * file .......: ltiBrightRGB.cpp
 * authors ....: Axel Berner
 * organization: LTI, RWTH Aachen
 * creation ...: 28.2.2002
 * revisions ..: $Id: ltiBrightRGB.cpp,v 1.9 2006/09/05 10:03:46 ltilib Exp $
 */

#include "ltiBrightRGB.h"

namespace lti {
  // --------------------------------------------------
  // brightRGB::parameters
  // --------------------------------------------------

  // default constructor
  brightRGB::parameters::parameters()
    : globalFeatureExtractor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    type = Average;
    transparent = true;
    transColor = lti::Black;
  }

  // copy constructor
  brightRGB::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  brightRGB::parameters::~parameters() {
  }

  // get type name
  const char* brightRGB::parameters::getTypeName() const {
    return "brightRGB::parameters";
  }

  // copy member

  brightRGB::parameters&
    brightRGB::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    globalFeatureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    globalFeatureExtractor::parameters& 
      (globalFeatureExtractor::parameters::* p_copy)
      (const globalFeatureExtractor::parameters&) =
      globalFeatureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif

    type = other.type;
    transparent = other.transparent;
    transColor = other.transColor;

    return *this;
  }

  // alias for copy member
  brightRGB::parameters&
    brightRGB::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* brightRGB::parameters::clone() const {
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
  bool brightRGB::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool brightRGB::parameters::writeMS(ioHandler& handler,
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
    default:
      b = b && lti::write(handler,"type","Average");
    }

    b = b && lti::write(handler,"transparent",transparent);
    b = b && lti::write(handler,"transColor",transColor);

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::write(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_writeMS)
      (ioHandler&,const bool) const =
      globalFeatureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool brightRGB::parameters::write(ioHandler& handler,
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
  bool brightRGB::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool brightRGB::parameters::readMS(ioHandler& handler,
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
    } else if (str == "Median") {
      type = Median;
    } else {
      type = Average;
    }

    b = b && lti::read(handler,"transparent",transparent);
    b = b && lti::read(handler,"transColor",transColor);

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && globalFeatureExtractor::parameters::read(handler,false);
# else
    bool (globalFeatureExtractor::parameters::* p_readMS)
      (ioHandler&,const bool) =
      globalFeatureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool brightRGB::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // brightRGB
  // --------------------------------------------------

  // default constructor
  brightRGB::brightRGB()
    : globalFeatureExtractor(){

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

  // copy constructor
  brightRGB::brightRGB(const brightRGB& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  brightRGB::~brightRGB() {
  }

  // returns the name of this type
  const char* brightRGB::getTypeName() const {
    return "brightRGB";
  }

  // copy member
  brightRGB& brightRGB::copy(const brightRGB& other) {
    globalFeatureExtractor::copy(other);

    return (*this);
  }

  // alias for copy member
  brightRGB&
    brightRGB::operator=(const brightRGB& other) {
    return (copy(other));
  }


  // clone member
  functor* brightRGB::clone() const {
    return new brightRGB(*this);
  }

  // return parameters
  const brightRGB::parameters&
    brightRGB::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool brightRGB::getMin(const image& img,dvector& dest) const{

    // image empty?
    if (img.empty()) {
      setStatusString("image empty");
      dest.resize(0);
      return false;
    }

    const rgbPixel transColor = getParameters().transColor;
    ivector minV(3,32768-1); //2^15-1
    image::const_iterator it = img.begin();
    if(getParameters().transparent) {
      while(it != img.end()) {
	if(*it != transColor) {
	  if((*it).getRed() < minV.at(0))
	    minV.at(0) = (*it).getRed();
	  if((*it).getGreen() < minV.at(1))
	    minV.at(1) = (*it).getGreen();
	  if((*it).getBlue() < minV.at(2))
	    minV.at(2) = (*it).getBlue();
	}
	it++;
      }
      // only transparent pixels?
      if (minV.at(0)==32768-1) {
        setStatusString("only transparent pixels");
        dest.resize(0);
        return false;
      }
    } else { // no transparent color
      while(it != img.end()) {
	if((*it).getRed() < minV.at(0))
	  minV.at(0) = (*it).getRed();
	if((*it).getGreen() < minV.at(1))
	  minV.at(1) = (*it).getGreen();
	if((*it).getBlue() < minV.at(2))
	  minV.at(2) = (*it).getBlue();
	it++;
      }
    }
    dest.castFrom(minV);
    // normalize to 0..1
    dest.divide(255);
    return true;
  };

  bool brightRGB::getMax(const image& img,dvector& dest) const{

    // image empty?
    if (img.empty()) {
      setStatusString("image empty");
      dest.resize(0);
      return false;
    }

    const rgbPixel transColor = getParameters().transColor;
    ivector maxV(3,-1);
    image::const_iterator it = img.begin();
    if(getParameters().transparent) {
      while(it != img.end()) {
	if(*it != transColor) {
	  if((*it).getRed() > maxV.at(0))
	    maxV.at(0) = (*it).getRed();
	  if((*it).getGreen() > maxV.at(1))
	    maxV.at(1) = (*it).getGreen();
	  if((*it).getBlue() > maxV.at(2))
	    maxV.at(2) = (*it).getBlue();
	}
	it++;
      }
      // only transparent pixels?
      if (maxV.at(0)==-1) {
        setStatusString("only transparent pixels");
        dest.resize(0);
        return false;
      }
    } else { // no transparent color
      while(it != img.end()) {
	if((*it).getRed() > maxV.at(0))
	  maxV.at(0) = (*it).getRed();
	if((*it).getGreen() > maxV.at(1))
	  maxV.at(1) = (*it).getGreen();
	if((*it).getBlue() > maxV.at(2))
	  maxV.at(2) = (*it).getBlue();
	it++;
      }
    }
    if(maxV.at(0) == -1)
      return false;
    dest.castFrom(maxV);
    // normalize to 0..1
    dest.divide(255);
    return true;
  };

  bool brightRGB::getAverage(const image& img,dvector& dest) const{

    const rgbPixel transColor = getParameters().transColor;
    dvector avg(3,0.0);
    image::const_iterator it = img.begin();
    // check for empty image
    if (img.columns()==0 || img.rows()==0) {
      setStatusString("image empty");
      dest.resize(0);
      return false;
    }
    if(getParameters().transparent) {
      int counter = 0;
      while(it != img.end()) {
	if(*it != transColor) {
	  avg.at(0) += (*it).getRed();
	  avg.at(1) += (*it).getGreen();
	  avg.at(2) += (*it).getBlue();
	  ++counter;
	}
	it++;
      }
      // check for completely transparent image
      if (counter==0) {
        setStatusString("only transparent pixels");
        dest.resize(0);
        return false;
      }
      avg.divide(counter);
    } else { // no transparent color
      while(it != img.end()) {
	avg.at(0) += (*it).getRed();
	avg.at(1) += (*it).getGreen();
	avg.at(2) += (*it).getBlue();
	it++;
      }
      avg.divide(img.columns()*img.rows());
    }
    // values between 0 and 1
    dest.divide(avg, 255.);
    return true;
  };

  bool brightRGB::getMedian(const image& img,dvector& dest) const{


    // image empty?
    if (img.empty()) {
      setStatusString("image empty");
      dest.resize(0);
      return false;
    }

    const rgbPixel transColor = getParameters().transColor;
    dest.resize(3);
    ivector hist0(256,0);
    ivector hist1(256,0);
    ivector hist2(256,0);
    image::const_iterator it = img.begin();
    if(getParameters().transparent) {
      while(it != img.end()) {
  	if(*it != transColor) {
	  ++hist0.at((*it).getRed());
	  ++hist1.at((*it).getGreen());
	  ++hist2.at((*it).getBlue());
	}
	it++;
      }
      const int counterHalf = hist0.sumOfElements()/2;
      // check for complete image transparent
      if (counterHalf==0) {
        setStatusString("only transparent pixels");
        dest.resize(0);
        return false;
      }

      int i,s;
      i=-1,s=0;
      while(++i<256 && s<counterHalf) {
	s += hist0.at(i);
      }
      dest.at(0) = i-1;
      i=-1,s=0;
      while(++i<256 && s<counterHalf) {
	s += hist1.at(i);
      }
      dest.at(1) = i-1;
      i=-1,s=0;
      while(++i<256 && s<counterHalf) {
	s += hist2.at(i);
      }
      dest.at(2) = i-1;
    } else { // no transparent color
      while(it != img.end()) {
	  ++hist0.at((*it).getRed());
	  ++hist1.at((*it).getGreen());
	  ++hist2.at((*it).getBlue());
	it++;
      }
      const int counterHalf = img.columns()*img.rows()/2;
      int i,s;
      i=-1,s=0;
      while(++i<256 && s<counterHalf) {
	s += hist0.at(i);
      }
      dest.at(0) = i-1;
      i=-1,s=0;
      while(++i<256 && s<counterHalf) {
	s += hist1.at(i);
      }
      dest.at(1) = i-1;
      i=-1,s=0;
      while(++i<256 && s<counterHalf) {
	s += hist2.at(i);
      }
      dest.at(2) = i-1;
    }

    // normalize to 0..1
    dest.divide(255);

    return true;
  };

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool brightRGB::apply(const image& src,
			dvector& dest) const {
    std::vector<channel8> ch;

    switch (getParameters().type) {
      case parameters::Minimum: {
	return getMin(src,dest);
      }
      case parameters::Maximum: {
	return getMax(src,dest);
      }
      case parameters::Average: {
	return getAverage(src,dest);
      }
      case parameters::Median: {
	return getMedian(src,dest);
      }
      default:{
	setStatusString("Kind of filter-type doesn't exist");
	return false;
      }
    }
    return true;
  };


}
