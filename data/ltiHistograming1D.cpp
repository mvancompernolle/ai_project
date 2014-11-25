/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiHistograming1D.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 12.4.2001
 * revisions ..: $Id: ltiHistograming1D.cpp,v 1.11 2006/09/05 10:15:59 ltilib Exp $
 */

#include "ltiHistograming1D.h"
#include "ltiGaussKernels.h"
#include "ltiConvolution.h"

namespace lti {
  // --------------------------------------------------
  // histograming1D::parameters
  // --------------------------------------------------

  // default constructor
  histograming1D::parameters::parameters()
    : globalFeatureExtractor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    kernel = gaussKernel1D<double>(3);
    smooth = false;
    cells = int(256);
    mode = Normal;
    normalize = false;
    intervalLow = 0.0f;
    intervalHigh = 1.0f;
    ignoreValue = -1.0f;
  }

  // copy constructor
  histograming1D::parameters::parameters(const parameters& other)
    : globalFeatureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  histograming1D::parameters::~parameters() {
  }

  // get type name
  const char* histograming1D::parameters::getTypeName() const {
    return "histograming1D::parameters";
  }

  // copy member

  histograming1D::parameters&
    histograming1D::parameters::copy(const parameters& other) {
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

    kernel.copy(other.kernel);
    smooth = other.smooth;
    cells = other.cells;
    mode = other.mode;
    normalize = other.normalize;
    intervalLow = other.intervalLow;
    intervalHigh = other.intervalHigh;
    ignoreValue = other.ignoreValue;

    return *this;
  }

  // alias for copy member
  histograming1D::parameters&
    histograming1D::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* histograming1D::parameters::clone() const {
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
  bool histograming1D::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool histograming1D::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"kernel",kernel);
      lti::write(handler,"smooth",smooth);
      lti::write(handler,"cells",cells);

      if (mode == Extremes) {
        lti::write(handler,"mode","Extremes");
      } else if (mode == Interval) {
        lti::write(handler,"mode","Interval");
      } else {
        lti::write(handler,"mode","Normal");
      }

      lti::write(handler,"normalize",normalize);
      lti::write(handler,"intervalLow",intervalLow);
      lti::write(handler,"intervalHigh",intervalHigh);
      lti::write(handler,"ignoreValue",ignoreValue);
    }

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
  bool histograming1D::parameters::write(ioHandler& handler,
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
  bool histograming1D::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool histograming1D::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"kernel",kernel);
      lti::read(handler,"smooth",smooth);
      lti::read(handler,"cells",cells);

      std::string modeStr;
      lti::read(handler,"mode",modeStr);
      if (modeStr == "Extremes") {
        mode = Extremes;
      } else if (modeStr == "Interval") {
        mode = Interval;
      } else {
        mode = Normal;
      }

      lti::read(handler,"normalize",normalize);
      lti::read(handler,"intervalLow",intervalLow);
      lti::read(handler,"intervalHigh",intervalHigh);
      lti::read(handler,"ignoreValue",ignoreValue);
    }

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
  bool histograming1D::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // histograming1D
  // --------------------------------------------------

  // default constructor
  histograming1D::histograming1D()
    : globalFeatureExtractor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // constructor with parameters
  histograming1D::histograming1D(const parameters& par)
    : globalFeatureExtractor(){
    // set the default parameters
    setParameters(par);
  }

  // copy constructor
  histograming1D::histograming1D(const histograming1D& other)
    : globalFeatureExtractor()  {
    copy(other);
  }

  // destructor
  histograming1D::~histograming1D() {
  }

  // returns the name of this type
  const char* histograming1D::getTypeName() const {
    return "histograming1D";
  }

  // copy member
  histograming1D&
    histograming1D::copy(const histograming1D& other) {
      globalFeatureExtractor::copy(other);
    return (*this);
  }

  // clone member
  functor* histograming1D::clone() const {
    return new histograming1D(*this);
  }

  // return parameters
  const histograming1D::parameters&
    histograming1D::getParameters() const {
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


  // On copy apply for type vector<float>!
  bool histograming1D::apply(const channel& src,vector<double>& dest) const {

    if (src.empty()) {
      dest.clear();
      setStatusString("input channel empty");
      return false;
    }

    const parameters& param = getParameters();

    float theMin(0),theMax(1.0f);

    if (param.mode == parameters::Extremes) {
      src.getExtremes(theMin,theMax);
    } else if (param.mode == parameters::Interval) {
      theMin = param.intervalLow;
      theMax = param.intervalHigh;
    }

    const int lastIdx = param.cells-1;

    const float m = float(lastIdx)/(theMax-theMin);
    const float b = -m*theMin;
    int y;
    int idx;
    int entries;

    fvector::const_iterator it,eit;

    dest.resize(param.cells,0.0,false,true); // initialize with 0

    entries = 0;

    // if b too small, it's possible to calculate everything faster...
    if (abs(b) < (0.1f/param.cells)) {

      if (param.mode == parameters::Normal) {
        for (y=0;y<src.rows();++y) {
          const fvector& vct = src.getRow(y);
          for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
            if ((*it) != param.ignoreValue) {
              idx = static_cast<int>((*it)*m);
              if (idx<0) {
                idx = 0;
              } else if (idx>=param.cells) {
                idx = lastIdx;
              }
              dest.at(idx)++;
              entries++;
            }
          }
        }
      } else {
        for (y=0;y<src.rows();++y) {
          const fvector& vct = src.getRow(y);
          for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
            if ((*it) != param.ignoreValue) {
              idx = static_cast<int>((*it)*m);
              if ( (idx>=0) && (idx<param.cells) )  {
                dest.at(idx)++;
                entries++;
              }
            }
          }
        }
      }
    } else {
      // b is not neglectible...

      if (param.mode == parameters::Normal) {
        for (y=0;y<src.rows();++y) {
          const fvector& vct = src.getRow(y);
          for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
            if ((*it) != param.ignoreValue) {
              idx = static_cast<int>((*it)*m+b);

              if (idx<0) {
                idx = 0;
              } else if (idx>=param.cells) {
                idx = lastIdx;
              }
              dest.at(idx)++;
              entries++;
            }
          }
        }
      } else {
        for (y=0;y<src.rows();++y) {
          const fvector& vct = src.getRow(y);
          for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
            if ((*it) != param.ignoreValue) {
              idx = static_cast<int>((*it)*m+b);

              if ( (idx>=0) && (idx < param.cells)) {
                dest.at(idx)++;
                entries++;
              }
            }
          }
        }
      }

    }

    if (param.normalize) {
      if (entries > 0) {
        dest.divide(entries);
      }
    }

    if (param.smooth) {
      convolution convolver;
      convolution::parameters cpar;
      cpar.boundaryType = lti::Mirror;
      cpar.setKernel(param.kernel);
      convolver.setParameters(cpar);
      convolver.apply(dest);
    }

    return true;
  };

  // On copy apply for type vector<float>!
  bool histograming1D::apply(const channel8& src,vector<double>& dest) const {
    if (src.empty()) {
      dest.clear();
      setStatusString("input channel empty");
      return false;
    }

    const parameters& param = getParameters();

    int theMin(0),theMax(255);

    if (param.mode == parameters::Extremes) {
      ubyte tmpMin,tmpMax;
      src.getExtremes(tmpMin,tmpMax);
      theMin = tmpMin;
      theMax = tmpMax;
    } else if (param.mode == parameters::Interval) {
      theMin = static_cast<int>(param.intervalLow);
      theMax = static_cast<int>(param.intervalHigh);
    }

    const int lastIdx = param.cells-1;

    const float m = float(lastIdx)/(theMax-theMin);
    const float b = -m*theMin;
    int y;
    int idx;
    int entries;

    vector<channel8::value_type>::const_iterator it,eit;

    dest.resize(param.cells,0.0,false,true); // initialize with 0

    entries = 0;

    // if b too small, it's possible to calculate everything faster...
    if (abs(b) < (0.1f/param.cells)) {

      if (param.mode == parameters::Normal) {
        for (y=0;y<src.rows();++y) {
          const vector<channel8::value_type>& vct = src.getRow(y);
          for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
            if ((*it) != param.ignoreValue) {
              idx = static_cast<int>((*it)*m);
              if (idx<0) {
                idx = 0;
              } else if (idx>=param.cells) {
                idx = lastIdx;
              }
              dest.at(idx)++;
              entries++;
            }
          }
        }
      } else {
        for (y=0;y<src.rows();++y) {
          const vector<channel8::value_type>& vct = src.getRow(y);
          for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
            if ((*it) != param.ignoreValue) {
              idx = static_cast<int>((*it)*m);
              if ( (idx>=0) && (idx<param.cells) )  {
                dest.at(idx)++;
                entries++;
              }
            }
          }
        }
      }
    } else {
      // b is not neglectible...

      if (param.mode == parameters::Normal) {
        for (y=0;y<src.rows();++y) {
          const vector<channel8::value_type>& vct = src.getRow(y);
          for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
            if ((*it) != param.ignoreValue) {
              idx = static_cast<int>((*it)*m+b);

              if (idx<0) {
                idx = 0;
              } else if (idx>=param.cells) {
                idx = lastIdx;
              }
              dest.at(idx)++;
              entries++;
            }
          }
        }
      } else {
        for (y=0;y<src.rows();++y) {
          const vector<channel8::value_type>& vct = src.getRow(y);
          for (it=vct.begin(),eit=vct.end();it!=eit;++it) {
            if ((*it) != param.ignoreValue) {
              idx = static_cast<int>((*it)*m+b);

              if ( (idx>=0) && (idx < param.cells)) {
                dest.at(idx)++;
                entries++;
              }
            }
          }
        }
      }

    }

    if (param.normalize) {
      dest.divide(entries);
    }

    if (param.smooth) {
      convolution convolver;
      convolution::parameters cpar;
      cpar.boundaryType = lti::Mirror;
      cpar.setKernel(param.kernel);
      convolver.setParameters(cpar);
      convolver.apply(dest);
    }

    return true;
  };

  // On copy apply for type vector<float>!
  bool histograming1D::apply(const channel& src,histogram1D& dest) const {
    dvector tmp;
    if (apply(src,tmp)) {
      dest.resize(tmp.size());
      for (int i=0;i<tmp.size();++i) {
        dest.put(i,tmp.at(i));
      }
    } else {
      return false;
    }

    return true;
  };

  // On copy apply for type vector<float>!
  bool histograming1D::apply(const channel8& src,histogram1D& dest) const {
    dvector tmp;
    if (apply(src,tmp)) {
      dest.resize(tmp.size());
      for (int i=0;i<tmp.size();++i) {
        dest.put(i,tmp.at(i));
      }
    } else {
      return false;
    }

    return true;
  };

}
