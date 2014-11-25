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
 * file .......: ltioptimalThresholding.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 13.12.2000
 * revisions ..: $Id: ltiOptimalThresholding.cpp,v 1.9 2006/09/05 10:24:01 ltilib Exp $
 */

#include "ltiOptimalThresholding.h"

namespace lti {

  // --------------------------------------------------
  // optimalThresholding::parameters
  // --------------------------------------------------

  // default constructor
  optimalThresholding::parameters::parameters()
    : thresholding::parameters() {
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    precision = 0.003915f;
  }

  // copy constructor
  optimalThresholding::parameters::parameters(const parameters& other)
    : thresholding::parameters()  {
    copy(other);
  }

  // destructor
  optimalThresholding::parameters::~parameters() {
  }

  // get type name
  const char* optimalThresholding::parameters::getTypeName() const {
    return "optimalThresholding::parameters";
  }

  // copy member

  optimalThresholding::parameters&
    optimalThresholding::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    thresholding::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    thresholding::parameters&
      (thresholding::parameters::* p_copy)
      (const thresholding::parameters&) =
      thresholding::parameters::copy;
    (this->*p_copy)(other);
# endif


      precision = other.precision;

    return *this;
  }

  // alias for copy member
  optimalThresholding::parameters&
    optimalThresholding::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* optimalThresholding::parameters::clone() const {
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
  bool optimalThresholding::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool optimalThresholding::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"precision",precision);
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
  bool optimalThresholding::parameters::write(ioHandler& handler,
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
  bool optimalThresholding::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool optimalThresholding::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"precision",precision);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && thresholding::parameters::read(handler,false);
# else
    bool (thresholding::parameters::* p_readMS)(ioHandler&,
                                                         const bool) =
      thresholding::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool optimalThresholding::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // optimalThresholding
  // --------------------------------------------------

  // default constructor
  optimalThresholding::optimalThresholding()
    : thresholding(){
    parameters param;
    setParameters(param);
  }

  // copy constructor
  optimalThresholding::optimalThresholding(const optimalThresholding& other)
    : thresholding()  {
    copy(other);
  }

  // destructor
  optimalThresholding::~optimalThresholding() {
  }

  // returns the name of this type
  const char* optimalThresholding::getTypeName() const {
    return "optimalThresholding";
  }

  // copy member
  optimalThresholding&
    optimalThresholding::copy(const optimalThresholding& other) {
      thresholding::copy(other);
    return (*this);
  }

  // clone member
  functor* optimalThresholding::clone() const {
    return new optimalThresholding(*this);
  }

  // return parameters
  const optimalThresholding::parameters&
    optimalThresholding::getParameters() const {
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
  bool optimalThresholding::apply(channel& srcdest) {
    parameters param = getParameters();

    param.lowThreshold = getThreshold(srcdest);
    //std::cout << param.lowThreshold;
    param.highThreshold = 1.0f;

    setParameters(param);

    return thresholding::apply(srcdest);
  };

  // On place apply for type channel8!
  bool optimalThresholding::apply(channel8& srcdest) {
    parameters param = getParameters();

    param.lowThreshold = static_cast<float>(getThreshold(srcdest));
    // needs to be set to 1.0 according
    // to documentation of thresholding;
    param.highThreshold = 1.0f;

    thresholding::setParameters(param);

    return thresholding::apply(srcdest);
  };

  // On copy apply for type channel!
  bool optimalThresholding::apply(const channel& src,channel& dest) {
    parameters param = getParameters();

    param.lowThreshold = getThreshold(src);
    param.highThreshold = 1.0f;

    thresholding::setParameters(param);

    return thresholding::apply(src, dest);
  };

  // On copy apply for type channel8!
  bool optimalThresholding::apply(const channel8& src,channel8& dest) {
    parameters param = getParameters();

    param.lowThreshold = static_cast<float>(getThreshold(src));
    // needs to be set to 1.0 according
    // to documentation of thresholding;
    param.highThreshold = 1.0f;

    thresholding::setParameters(param);

    return thresholding::apply(src, dest);
  };

  // get iterative optimal threshold value
  // Background at iteration t: mB(t)
  // Objects at iteration t: mO(t)
  // optimal Threshold at iteration t: T(t)
  // algorithm:
  // t=0: mB(0) = colors in the four corners of the image
  //      mO(0) = all except mB(0)
  // t+1: mB(t+1) = (sum f(i,j) where (i,j) is in mB(t)) / #background_pixels
  //      mO(t+1) = (sum f(i,j) where (i,j) is in mO(t)) / #object_pixels
  //      T(t+1) = (mB(t)+mO(t))/2
  // when T(t+1)==T(t) return T
  int optimalThresholding::getThreshold(const channel8& src) const {
    int rows, columns;
    float mBackground = 0.0f;         // background distribution
    float mObject = 0.0f;             // object distribution
    int threshold, oldThreshold;      // save last threshold

    int precision = static_cast<int>(getParameters().precision * 255);

    // no valid threshold: make sure iteration is run at least once
    oldThreshold = -500;

    rows = src.rows();
    columns = src.columns();

    // first step: all pixels are object pixels ...
    int rowCount, colCount;
    for (rowCount = 0; rowCount < rows; rowCount++) {
      vector<ubyte> actRow = src.getRow(rowCount);
      for (colCount = 0; colCount < columns; colCount++) {
        mObject += actRow[colCount];
      }
    }
    // ... except for the four corner pixels
    mBackground = static_cast<float>(src[0][0] + src[0][columns-1] +
                                     src[rows-1][0] + src[rows-1][columns-1]);
    mObject -= mBackground;

    // calculate median value
    mBackground /= 4;
    mObject /= (rows*columns - 4);

    threshold = static_cast<int>((mObject + mBackground) / 2);


    int epsilon = 256;

    // iteration
    while (epsilon > precision) {

      // test equality
      if (precision <= 0.0f) {
        if (threshold == oldThreshold)
          break;
      }

      // calculate new distributions
      int objectPixels = 0;
      mBackground = 0.0f;
      mObject = 0.0f;
      for (rowCount = 0; rowCount < rows; rowCount++) {
        vector<ubyte> actRow = src.getRow(rowCount);
        for (colCount = 0; colCount < columns; colCount++) {
          int value = actRow[colCount];
          if (value < threshold) {
            mBackground += value;
          }
          else {
            mObject += value;
            objectPixels++;
          }
        }
      }

      mObject /= objectPixels;
      mBackground /= (rows*columns - objectPixels);

      // calculate threshold
      oldThreshold = threshold;
      threshold = static_cast<int>((mObject + mBackground) / 2);

      // variance
      epsilon = (threshold > oldThreshold ?
                threshold - oldThreshold : oldThreshold - threshold);
    };

    return threshold;
  }

  // get optimal threshold value for channel
  float optimalThresholding::getThreshold(const channel& src) const {
    int rows, columns;
    float mBackground = 0.0f;               // background distribution
    float mObject = 0.0f;                   // object distribution
    float threshold, oldThreshold;          // save last threshold

    float precision = getParameters().precision;

    oldThreshold = -1.0f;                   // make sure iteration is run
                                            // at least once

    rows = src.rows();
    columns = src.columns();

    // first step: all pixels are object pixels ...
    int rowCount, colCount;
    for (rowCount = 0; rowCount < rows; rowCount++) {
      vector<float> actRow = src.getRow(rowCount);
      for (colCount = 0; colCount < columns; colCount++) {
        mObject += actRow[colCount];
      }
    }
    // ... except for the four corner pixels
    mBackground = src[0][0] + src[0][columns-1] + src[rows-1][0] +
                  src[rows-1][columns-1];
    mObject -= mBackground;

    // calculate median value
    mBackground /= 4;
    mObject /= (rows*columns - 4);

    threshold = (mObject + mBackground) / 2;

    float epsilon = 1.1f;

    float last_epsilon = epsilon * 2;  // always convergent when testing
                                       // for the first time!

    // iteration
    while (epsilon > precision) {

      // test convergence
      if (precision <= 0.0f) {
        if (epsilon >= last_epsilon)
          break;
      }

      // calculate new distributions
      int objectPixels = 0;
      mBackground = 0.0f;
      mObject = 0.0f;
      for (rowCount = 0; rowCount < rows; rowCount++) {
        vector<float> actRow = src.getRow(rowCount);
        for (colCount = 0; colCount < columns; colCount++) {
          float value = actRow[colCount];
          if (value < threshold) {
            mBackground += value;
          }
          else {
            mObject += value;
            objectPixels++;
          }
        }
      }

      mObject /= objectPixels;
      mBackground /= (rows*columns - objectPixels);

      // threshold for next iteration
      oldThreshold = threshold;
      threshold = (mObject + mBackground) / 2;

      // variance
      last_epsilon = epsilon;
      epsilon = (threshold > oldThreshold ?
                threshold - oldThreshold : oldThreshold - threshold);
    };

    return threshold;
  }

  int optimalThresholding::getThreshold(const vector<double>& histo) const {

    // first suggestion based on the mean
    double mean(0),entries(0);

    int i;


    for (i=0;i<histo.size();++i) {
      mean += i*histo.at(i);
      entries+=histo.at(i);
    }

    if (entries <= 0) {
      // no entries... assume optimal threshold the the half of the histogram
      return histo.size()/2;
    }

    mean /= entries;

    int t,tnew;
    t = int(mean);
    tnew = -1;

    double meanB(0); // mean of "background"
    double meanO(0); // mean of "object"
    double entriesB(0);
    double entriesO(0);

    double tmp;
    for (i=0;i<t;++i) {
      meanB += i*histo.at(i);
      entriesB+=histo.at(i);
    }

    for (;i<histo.size();++i) {
      meanO += i*histo.at(i);
      entriesO+=histo.at(i);
    }

    if (entriesB == 0)  {
      tnew = int((meanO/entriesO)/2.0);
    } else if (entriesO == 0) {
      tnew = int((meanB/entriesB)/2.0);
    } else {
      tnew = int(((meanB/entriesB)+(meanO/entriesO))/2.0);
    }

    while (t!=tnew) {

      // update mean values
      if (tnew>t) {
        for (i=t;i<tnew;++i) {
          tmp = histo.at(i);

          meanB += i*tmp;
          entriesB += tmp;
          meanO -= i*tmp;
          entriesO -= tmp;
        }
      } else {
        for (i=t-1;i>=tnew;--i) {
          tmp = histo.at(i);
          meanB -= i*tmp;
          entriesB -= tmp;
          meanO += i*tmp;
          entriesO += tmp;
        }
      }

      t = tnew;
      if (entriesB == 0)  {
        tnew = int((meanO/entriesO)/2.0);
      } else if (entriesO == 0) {
        tnew = int((meanB/entriesB)/2.0);
      } else {
        tnew = int(((meanB/entriesB)+(meanO/entriesO))/2.0);
      }

    }

    return t;

  }

  int optimalThresholding::getThreshold(const vector<float>& histo) const {
    vector<double> vct;
    vct.castFrom(histo);
    return getThreshold(vct);
  }

  int optimalThresholding::getThreshold(const vector<int>& histo) const {
    vector<double> vct;
    vct.castFrom(histo);
    return getThreshold(vct);
  }

}
