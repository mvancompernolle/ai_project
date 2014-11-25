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

 
/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: lticomprehensiveColourNormalization.cpp
 * authors ....: Marius Wolf	
 * organization: LTI, RWTH Aachen
 * creation ...: 19.3.2003
 * revisions ..: $Id: ltiComprehensiveColourNormalization.cpp,v 1.11 2006/09/05 10:07:09 ltilib Exp $
 */


#include "ltiComprehensiveColourNormalization.h"
#include "ltiSplitImageToRGB.h"
#include "ltiMergeRGBToImage.h"


namespace lti {
  // --------------------------------------------------
  // comprehensiveColourNormalization::parameters
  // --------------------------------------------------

  // default constructor
  comprehensiveColourNormalization::parameters::parameters() 
    : colorNormalizationBase::parameters() {      
    
    maxSteps=5;
    maxDist=1.f;
  }

  // copy constructor
  comprehensiveColourNormalization::parameters::parameters(const parameters& other)    
    : colorNormalizationBase::parameters() {      
    copy(other);
  }
  
  // destructor
  comprehensiveColourNormalization::parameters::~parameters() {    
  }

  // get type name
  const char* comprehensiveColourNormalization::parameters::getTypeName() const {
    return "comprehensiveColourNormalization::parameters";
  }
  
  // copy member

  comprehensiveColourNormalization::parameters& 
  comprehensiveColourNormalization::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    colorNormalizationBase::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    colorNormalizationBase::parameters& (colorNormalizationBase::parameters::* p_copy)
      (const colorNormalizationBase::parameters&) = 
      colorNormalizationBase::parameters::copy;
    (this->*p_copy)(other);
# endif
    
    maxDist = other.maxDist;
    maxSteps = other.maxSteps;

    return *this;
  }

  // alias for copy member
  comprehensiveColourNormalization::parameters& 
  comprehensiveColourNormalization::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* comprehensiveColourNormalization::parameters::clone() const {
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
  bool comprehensiveColourNormalization::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool comprehensiveColourNormalization::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && colorNormalizationBase::parameters::write(handler,false);
# else
    bool (colorNormalizationBase::parameters::* p_writeMS)(ioHandler&,const bool) const = 
      colorNormalizationBase::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (b) {
      lti::write(handler,"maxDist",maxDist);
      lti::write(handler,"maxSteps",maxSteps);
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool comprehensiveColourNormalization::parameters::write(ioHandler& handler,
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
  bool comprehensiveColourNormalization::parameters::read(ioHandler& handler,
                                                         const bool complete) 
# else
  bool comprehensiveColourNormalization::parameters::readMS(ioHandler& handler,
                                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && colorNormalizationBase::parameters::read(handler,false);
# else
    bool (colorNormalizationBase::parameters::* p_readMS)(ioHandler&,const bool) = 
      colorNormalizationBase::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (b) {
      lti::read(handler,"maxDist",maxDist);
      lti::read(handler,"maxSteps",maxSteps);
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool comprehensiveColourNormalization::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // comprehensiveColourNormalization
  // --------------------------------------------------

  // default constructor
  comprehensiveColourNormalization::comprehensiveColourNormalization()
    : colorNormalizationBase() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  comprehensiveColourNormalization::comprehensiveColourNormalization(const parameters& par)
    : colorNormalizationBase() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  comprehensiveColourNormalization::comprehensiveColourNormalization(const comprehensiveColourNormalization& other)
    : colorNormalizationBase() {
    copy(other);
  }

  // destructor
  comprehensiveColourNormalization::~comprehensiveColourNormalization() {
  }

  // returns the name of this type
  const char* comprehensiveColourNormalization::getTypeName() const {
    return "comprehensiveColourNormalization";
  }

  // copy member
  comprehensiveColourNormalization&
  comprehensiveColourNormalization::copy(const comprehensiveColourNormalization& other) {

    colorNormalizationBase::copy(other);
    return (*this);
  }

  // alias for copy member
  comprehensiveColourNormalization&
  comprehensiveColourNormalization::operator=(const comprehensiveColourNormalization& other) {
    return (copy(other));
  }


  // clone member
  functor* comprehensiveColourNormalization::clone() const {
    return new comprehensiveColourNormalization(*this);
  }

  // return parameters
  const comprehensiveColourNormalization::parameters&
  comprehensiveColourNormalization::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }
  
  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------
  
  // On place apply for type image!
  bool comprehensiveColourNormalization::apply(image& srcdest) const {

    image tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }
    return false;
  };

  // On copy apply for type image!
  bool comprehensiveColourNormalization::apply(const image& src,
                                               image& dest) const {
    
    channel sr,sg,sb,dr,dg,db;
    splitImageToRGB splitter;
    mergeRGBToImage merger;

    splitter.apply(src,sr,sg,sb);

    const parameters& param = getParameters();          
    int steps=0;
    bool success = true;
    float dist;

    do {
      if (step(sr,sg,sb,dr,dg,db,dist)) {
        if (dist<param.maxDist) {
          // we're ready!  change was small enough!
          steps=param.maxSteps;  // flag to indicate end!
        } else {
          // prepare next iteration
          dr.detach(sr);
          dg.detach(sg);
          db.detach(sb);
        }
      } else {
        steps = param.maxSteps;
        success = false;
      }

      ++steps;
    } while (steps<param.maxSteps);

    if (success) {
      merger.apply(dr,dg,db,dest);
    }

    return success;
  };

  bool comprehensiveColourNormalization::step(const channel& srcred,
                                              const channel& srcgreen,
                                              const channel& srcblue,
                                              channel &destred,
                                              channel &destgreen,
                                              channel &destblue,
                                              float& dist) const {
    
    // resize the destination but do not initialize, this resize()
    // also ensures that dest is continous
    const int n = srcred.columns()*srcred.rows();

    destred.resize(srcred.size(),float(),false,false);
    destgreen.resize(srcgreen.size(),float(),false,false);
    destblue.resize(srcblue.size(),float(),false,false);

    // --------------------------------------------------------------
    // first step, computation of the chromaticity normalization R(I)

    // access all channels with the vector access operator, assuming that
    // all channels are connected.
    int i;
    float sum,cr,cg,cb;
    trgbPixel<float> colSum(0.0f,0.0f,0.0f);

    for (i=0;i<n;++i) {
      sum = ((cr=srcred.at(i))+
             (cg=srcgreen.at(i))+
             (cb=srcblue.at(i)));
      
      if (sum==0.0f) {
        sum=1.0f;
      }
      
      destred.at(i)  = (cr/=sum);
      destgreen.at(i)= (cg/=sum);
      destblue.at(i) = (cb/=sum);
      
      colSum += trgbPixel<float>(cr,cg,cb);    
    }
    
    // normalize the values from 0.0 to 1.0 instead of 0 to 255
    colSum.multiply(3.0f/n);

    // colSum is now 1/3 of the mean values at each channel. Invert it:
    if (colSum.red != 0.0f) {
      colSum.red=1.0f/colSum.red;
    }
    if (colSum.green != 0.0f) {
      colSum.green=1.0f/colSum.green;
    }
    if (colSum.blue != 0.0f) {
      colSum.blue=1.0f/colSum.blue;
    }

    // --------------------------------------------------------------
    // now intensity normalization C(R(I))
    
    dist=0;
    for (i=0;i<n;++i) {
      cr=(destred.at(i)  *= colSum.red)   - srcred.at(i);
      cg=(destgreen.at(i)*= colSum.green) - srcgreen.at(i);
      cb=(destblue.at(i) *= colSum.blue)  - srcblue.at(i);
      if ((sum=cr*cr+cg*cg+cb*cb)>dist) {
        dist=sum;
      }
    }

    return true;  
  }

}
