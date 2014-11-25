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
 * file .......: ltiRelativeThresholding.cpp
 * authors ....: Arnd Hannemann
 * organization: LTI, RWTH Aachen
 * creation ...: 4.1.2004
 * revisions ..: $Id: ltiRelativeThresholding.cpp,v 1.7 2006/09/05 10:29:17 ltilib Exp $
 */

#include "ltiRelativeThresholding.h"
#include <limits>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1

#ifdef _LTI_DEBUG
  #include "ltiViewer.h"
  #include <cstdio>
  #include "ltiTimer.h"
#endif

#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // relativeThresholding::parameters
  // --------------------------------------------------

  // default constructor
  relativeThresholding::parameters::parameters()
    : thresholding::parameters() {
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters
    
    bins = 256;
  }

  // copy constructor
  relativeThresholding::parameters::parameters(const parameters& other)
    : thresholding::parameters() {
    copy(other);
  }

  // destructor
  relativeThresholding::parameters::~parameters() {
  }

  // get type name
  const char* relativeThresholding::parameters::getTypeName() const {
    return "relativeThresholding::parameters";
  }

  // copy member

  relativeThresholding::parameters&
    relativeThresholding::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    thresholding::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    thresholding::parameters& (thresholding::parameters::* p_copy)
      (const thresholding::parameters&) =
      thresholding::parameters::copy;
    (this->*p_copy)(other);
# endif

    
      bins = other.bins;

    return *this;
  }

  // alias for copy member
  relativeThresholding::parameters&
    relativeThresholding::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* relativeThresholding::parameters::clone() const {
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
  bool relativeThresholding::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool relativeThresholding::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"bins",bins);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && thresholding::parameters::write(handler,false);
# else
    bool (thresholding::parameters::* p_writeMS)(ioHandler&,const bool) const =
      thresholding::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool relativeThresholding::parameters::write(ioHandler& handler,
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
  bool relativeThresholding::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool relativeThresholding::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"bins",bins);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && thresholding::parameters::read(handler,false);
# else
    bool (thresholding::parameters::* p_readMS)(ioHandler&,const bool) =
      thresholding::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool relativeThresholding::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // relativeThresholding
  // --------------------------------------------------

  // default constructor
  relativeThresholding::relativeThresholding()
    : thresholding (){

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
  relativeThresholding::relativeThresholding(const parameters& par)
    : thresholding () {

    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  relativeThresholding::relativeThresholding(const relativeThresholding& other)
    : thresholding () {
    copy(other);
  }

  // destructor
  relativeThresholding::~relativeThresholding() {
  }

  // returns the name of this type
  const char* relativeThresholding::getTypeName() const {
    return "relativeThresholding";
  }

  // copy member
  relativeThresholding&
  relativeThresholding::copy(const relativeThresholding& other) {
    thresholding ::copy(other);

    return (*this);
  }

  // alias for copy member
  relativeThresholding&
    relativeThresholding::operator=(const relativeThresholding& other) {
    return (copy(other));
  }


  // clone member
  functor* relativeThresholding::clone() const {
    return new relativeThresholding(*this);
  }

  // return parameters
  const relativeThresholding::parameters&
    relativeThresholding::getParameters() const {
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

  
  // On place apply for type channel!
  bool relativeThresholding::apply(channel& srcdest) const {
    channel tmp;
    bool b = apply(srcdest,tmp);
    tmp.detach(srcdest);
    return b;
  };

  // On place apply for type channel8!
  bool relativeThresholding::apply(channel8& srcdest) const {
    channel8 tmp;
    bool b = apply(srcdest,tmp);
    tmp.detach(srcdest);
    return b;
  };

  // On copy apply for type channel!
  bool relativeThresholding::apply(const channel& src,channel& dest) const {
    parameters par = getParameters();

    // storage for the histogramm
    int* binAry = new int[par.bins];

    // extremes
    float min,max;
    src.getExtremes(min,max);
    float diff = (max-min);
    // init binary
    int i=0;
    for (i=0;i<par.bins;++i) {
      binAry[i] = 0;
    }
    vector<float>::const_iterator cit,e;

    // compute histogramm
    const float bd = static_cast<float>(par.bins-1)/diff;
    int y;
    for (y=0;y<src.rows();y++) {
      const vector<float>& vct = src.getRow(y);
      for (cit=vct.begin(),e=vct.end();
           cit!=e;
           cit++) {
        binAry[iround(bd*(*cit-min))]++;
      }
    }


    // how many pixels are in the source channel
    const int chSize = src.rows()*src.columns();

    // get the absolute thresholds
    int high = static_cast<int>(par.highThreshold*chSize);
    int low = static_cast<int>(par.lowThreshold*chSize);
    float highThreshold = max;
    float lowThreshold = min;
    const float db = static_cast<float>(diff)/(par.bins-1);
    for (i=1;i<par.bins;++i) {
      // add rows together
      binAry[i]+=binAry[i-1];
      if (binAry[i-1]<low) { 
        lowThreshold=(i*db + min );
      }
      if (binAry[i-1]<high) {
        highThreshold=(i*db + min);
      }
    }

    _lti_debug("relative thresholds: low="<<par.lowThreshold
               <<" high="<<par.highThreshold<<"\n");
    _lti_debug("absolute thresholds: low="<<lowThreshold<<" high="
               <<highThreshold<<"\n");


#ifdef _LTI_DEBUG
    _lti_debug("show Histogramm...\n");
      
    channel8 ch(200,par.bins,ubyte(255));
    int maximum = binAry[par.bins-1];
    int p;
    for (i=0;i<par.bins;i++) {
     // _lti_debug("i="<<i<<" binAry[i]="<<binAry[i]<<"\n");
      p = static_cast<int>(binAry[i]*200/maximum);
      for (y=199;y>(200-p);y--) {
        ch.at(y,i)=ubyte(0);
      }
    }
    viewer view("Histogramm");
    view.show(ch);
    _lti_debug("Press Enter to continue\n");
    std::getchar();
#endif   

	delete[] binAry;

    // now do the thresholding
    return thresholding::apply(src,dest,lowThreshold,highThreshold,
                               par.keepInRegion,par.keepOutRegion,
                               par.inRegionValue,par.outRegionValue);
  };
  /****************************************************/



  // On copy apply for type channel8!
  bool relativeThresholding::apply(const channel8& src,channel8& dest) const {
    parameters par = getParameters();

    // storage for the histogramm
    int* binAry = new int[par.bins];
    
    // extremes
    ubyte min,max;
    src.getExtremes(min,max);
    ubyte diff = (max-min);

    // init binAry
    int i=0;
    for (i=0;i<par.bins;++i) {
      binAry[i] = 0;
    }

    // TODO: max=min, max-min < bins
    vector<ubyte>::const_iterator cit,e;


#ifdef _LTI_DEBUG
    static int ___count=0;
    static double mytime;
    timer time1;
    time1.start();
#endif

    // compute histogramm
    const float bd = static_cast<float>(par.bins-1)/diff;
    int y;
    for (y=0;y<src.rows();++y) {
      const vector<ubyte>& vct = src.getRow(y);
      for (cit=vct.begin(),e=vct.end();
           cit!=e;
           ++cit) {        

          binAry[iround(bd*(*cit-min))]++;
      }
    }
    // how many pixels are in the source channel
    const int chSize = src.rows()*src.columns();

    // get the absolute thresholds
    int high = static_cast<int>(par.highThreshold*chSize);
    int low = static_cast<int>(par.lowThreshold*chSize);
    float highThreshold = max;
    float lowThreshold = min;
    const float db = static_cast<float>(diff)/(par.bins-1);
    for (i=1;i<par.bins;++i) {
      // add rows together
      binAry[i]+=binAry[i-1];
      if (binAry[i-1]<low) { 
        lowThreshold=(i*db + min );
      }
      if (binAry[i-1]<high) {
        highThreshold=(i*db + min);
      }
    }

#ifdef _LTI_DEBUG
    time1.stop();
    mytime+=time1.getTime();
    _lti_debug("Time: "<<mytime/(++___count)<<"\n");
#endif

    // divide through 255
    lowThreshold = lowThreshold / 255;
    highThreshold = highThreshold / 255;

    _lti_debug("relative thresholds: low="<<par.lowThreshold<<" high="
                <<par.highThreshold<<"\n");
    _lti_debug("absolute thresholds: low="<<lowThreshold<<" high="
                <<highThreshold<<"\n");
    
    // Draw histogramm
#ifdef _LTI_DEBUG
    _lti_debug("show Histogramm...\n");
      
    channel8 ch(200,par.bins,ubyte(255));
    int maximum = binAry[par.bins-1];
    int p;
    for (i=0;i<par.bins;i++) {
//      _lti_debug("i="<<i<<" binAry[i]="<<binAry[i]<<"\n");
      p = static_cast<int>(binAry[i]*200/maximum);
      for (y=199;y>(200-p);y--) {
        ch.at(y,i)=ubyte(0);
      }
    }
    viewer view("Histogramm");
    view.show(ch);
    _lti_debug("Press Enter to continue\n");
    std::getchar();
#endif

	delete[] binAry;

// now do the thresholding
    return thresholding::apply(src,dest,
                               lowThreshold,highThreshold,
                               par.keepInRegion,par.keepOutRegion);
  };

}
#include "ltiUndebug.h"
