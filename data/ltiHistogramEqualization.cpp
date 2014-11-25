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
 * file .......: ltiHistogramEqualization.cpp
 * authors ....: Thomas Erger
 * organization: LTI, RWTH Aachen
 * creation ...: 25.7.2000
 * revisions ..: $Id: ltiHistogramEqualization.cpp,v 1.13 2006/09/05 10:15:39 ltilib Exp $
 */

#include "ltiHistogram.h"
#include "ltiMath.h"
#include "ltiHistogramEqualization.h"

namespace lti {
  // --------------------------------------------------
  // histogramEqualization::parameters
  // --------------------------------------------------

  // default constructor
  histogramEqualization::parameters::parameters(const float upperLimit)
    : modifier::parameters() {

    lowerInputLimit = float(0);
    lowerOutputLimit = float(0);
    cells = int(256);
    upperInputLimit = upperLimit;
    upperOutputLimit = upperLimit;
  }

  // copy constructor
  histogramEqualization::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  histogramEqualization::parameters::~parameters() {
  }

  // get type name
  const char* histogramEqualization::parameters::getTypeName() const {
    return "histogramEqualization::parameters";
  }

  // copy member

  histogramEqualization::parameters&
    histogramEqualization::parameters::copy(const parameters& other) {
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

    lowerInputLimit = other.lowerInputLimit;
    lowerOutputLimit = other.lowerOutputLimit;
    cells = other.cells;
    upperInputLimit = other.upperInputLimit;
    upperOutputLimit = other.upperOutputLimit;

    return *this;
  }

  // clone member
  functor::parameters* histogramEqualization::parameters::clone() const {
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
  bool histogramEqualization::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool histogramEqualization::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"lowerInputLimit",lowerInputLimit);
      lti::write(handler,"lowerOutputLimit",lowerOutputLimit);
      lti::write(handler,"cells",cells);
      lti::write(handler,"upperInputLimit",upperInputLimit);
      lti::write(handler,"upperOutputLimit",upperOutputLimit);
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
  bool histogramEqualization::parameters::write(ioHandler& handler,
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
  bool histogramEqualization::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool histogramEqualization::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"lowerInputLimit",lowerInputLimit);
      lti::read(handler,"lowerOutputLimit",lowerOutputLimit);
      lti::read(handler,"cells",cells);
      lti::read(handler,"upperInputLimit",upperInputLimit);
      lti::read(handler,"upperOutputLimit",upperOutputLimit);
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
  bool histogramEqualization::parameters::read(ioHandler& handler,
                                    const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // histogramEqualization
  // --------------------------------------------------

  // default constructor
  histogramEqualization::histogramEqualization(const float upperLimit)
    : modifier(){
    parameters tmp(upperLimit);
    setParameters(tmp);
  }

  // copy constructor
  histogramEqualization::histogramEqualization(const histogramEqualization& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  histogramEqualization::~histogramEqualization() {
  }

  // returns the name of this type
  const char* histogramEqualization::getTypeName() const {
    return "histogramEqualization";
  }

  // copy member
  histogramEqualization&
    histogramEqualization::copy(const histogramEqualization& other) {
      modifier::copy(other);
    return (*this);
  }

  // clone member
  functor* histogramEqualization::clone() const {
    return new histogramEqualization(*this);
  }

  // return parameters
  const histogramEqualization::parameters&
    histogramEqualization::getParameters() const {
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
  bool histogramEqualization::apply(channel& srcdest) const {

    channel tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest); // srcdest.copy(tmp);
      return true;
    }
    return false;
  };

  // On place apply for type channel8!
  bool histogramEqualization::apply(channel8& srcdest) const {
    channel8 tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest); // srcdest.copy(tmp);
      return true;
    }

    return false;
  };

  // On place apply for type fvector!
  bool histogramEqualization::apply(fvector& srcdest) const {
      fvector tmp;
      if (apply(srcdest,tmp)) {
        tmp.detach(srcdest);
        return true;
      }

      return false;
  };

  // On place apply for type vector<ubyte>!
  bool histogramEqualization::apply(vector<ubyte>& srcdest) const {
      vector<ubyte> tmp;
      if (apply(srcdest,tmp)) {
        tmp.detach(srcdest);
        return true;
      }

      return false;
  };

  // On copy apply for type channel!
  bool histogramEqualization::apply(const channel& src,
                                          channel& dest) const {
    int y,i;
    float cst;

    dest.resize(src.rows(),src.columns(),0,false,false);

    const parameters& param = getParameters();

    histogram1D histo(param.cells);
    vector<channel::value_type>::const_iterator it,e;

    if (param.upperInputLimit <= param.lowerInputLimit) {
      setStatusString("Wrong parameter input interval");
      return false;
    }

    cst = (param.cells-1)/(param.upperInputLimit-param.lowerInputLimit);

    // create histogram of the src channel

    for (y=0;y<src.rows();y++) {
      e=src.getRow(y).end();
      for(it=src.getRow(y).begin();it!=e;++it) {
        if ((*it>=param.lowerInputLimit) &&        // check whether value
            (*it<=param.upperInputLimit)) {        // within input range
          // *cst because the cells of the histogram have even values, -1
          // because otherwise the term in brackets could become equal to
          // param.cells which would create an error
          histo.put(int(0.5f+(*it-param.lowerInputLimit)*cst));
        }
      }
    }

    // generate cumulative histogram
    for(i=1;i<param.cells;i++) {
      histo.at(i) += histo.at(i-1);
    }

    // generate look-up-table
    lti::vector<channel::value_type> lut(param.cells);

    float ncst;
    if (histo.getNumberOfEntries() > 0) {
      ncst = static_cast<float>((param.upperOutputLimit-
                                 param.lowerOutputLimit)/
                                 histo.getNumberOfEntries());
    } else {
      ncst = 0.0f;
    }

    for(i=0;i<param.cells;i++) {
      // number of gray levels: cells1
      // the gray level saved at histoc.at(i)
      // divide by cst to get back to values in range 0.0-1.0
      lut.at(i)=static_cast<float>(param.lowerOutputLimit+ncst*histo.at(i));
    } // map the values to outputrange

    // write into the destination channel
    lti::channel::iterator dit;
    for(dit=dest.begin(),y=0; y<src.rows(); ++y) {
      e=src.getRow(y).end();
      for(it=src.getRow(y).begin();it!=e;++dit,++it) {
        // compare whether value is to be altered
        if ((*it>=param.lowerInputLimit) && (*it<=param.upperInputLimit)) {
          // use look-up-table
          *dit = lut.at(int(0.5f+(*it-param.lowerInputLimit)*cst));
        } else {
          // if value is not to be altered, it keeps its value
          *dit = *it;
        }
      }
    }

    return true;
  };

  // On copy apply for type channel8!
  bool histogramEqualization::apply(const channel8& src,
                                          channel8& dest) const {

    int i,y;
    float cst;

    dest.resize(src.rows(),src.columns(),0,false,false);

    const parameters& param = getParameters();

    if (param.upperInputLimit <= param.lowerInputLimit) {
      setStatusString("Wrong parameter input interval");
      return false;
    }

    lti::histogram1D histo(param.cells);
    lti::vector<channel8::value_type>::const_iterator it,e;

    cst = (param.cells-1)/(param.upperInputLimit-param.lowerInputLimit);

    // create histogram of the src channel

    ubyte histLut[256];
    for ( i=int(0.5f+param.lowerInputLimit); i<=int(param.upperInputLimit); i++ ) {
        histLut[i] = int(0.5f+(i-param.lowerInputLimit)*cst);
    }

    // durch das ganze Bild laufen ...
		int numberOfPixels = 0;
    for (y=0;y<src.rows();y++) {
      e=src.getRow(y).end();
      for(it=src.getRow(y).begin();it!=e;it++) {
        if ((*it>=param.lowerInputLimit) &&
            (*it<=param.upperInputLimit)) 
				{
          histo.put(histLut[*it]);
					numberOfPixels++;
        }
      }
    }

    // generate cumulative histogram
    for(i=1;i<param.cells;i++) {
//			std::cerr << i-1 << "\t" << histo.at(i-1) << "\n";
      histo.at(i) += histo.at(i-1);
    }

    // no quantization, so lut can be used for all values
    ubyte lut[256];
    const int maxLLimit = min(256,int(param.lowerInputLimit));

    for(i=0;i<maxLLimit;i++) {
      lut[i] = i;
    }

    float ncst;
    if (histo.getNumberOfEntries() > 0) {
      ncst = static_cast<float>((param.upperOutputLimit-
                                 param.lowerOutputLimit)/
                                histo.getNumberOfEntries());
    } else {
      ncst = 0.0f;
    }

    // apply the algorithm and afterwards map to output range

    const int maxULimit = min(256,int(param.upperInputLimit));

//    float hcst = (param.cells - 1) / (param.upperInputLimit - param.lowerInputLimit);
		float hcst = static_cast<float>(param.cells - 1) / static_cast<float>(maxULimit-maxLLimit);

    for(;i<=maxULimit;++i) {
      lut[i]=	static_cast<ubyte>(ncst*histo.at(static_cast<int>(0.5f + (i - maxLLimit) * hcst)));
    }

    // no quantization, so lut can be used for all values
    for(;i<256;i++) {
      lut[i] = i;
    }

    // write information into the destination channel8
    lti::channel8::iterator dit;
    for(dit=dest.begin(),y=0; y<src.rows(); ++y) {
      e=src.getRow(y).end();
      for(it=src.getRow(y).begin();it!=e;++dit,++it) {

        if ((*it>=param.lowerInputLimit) &&
            (*it<=param.upperInputLimit)) 
				{
					*dit = lut[*it];
				}
				else
					*dit = *it;
      }
    }

    return true;
  };

  // On copy apply for type fvector!
  bool histogramEqualization::apply(const fvector& src,
                                          fvector& dest) const {
    channel tmpsrc;
    channel tmpdest;

    //quite bad code here, because of const input, to be changed
    //the src vector will not be changed,
    tmpsrc.useExternData(1,src.size(),const_cast<float*>(&src.at(0)));

    apply(tmpsrc,tmpdest);

    dest.copy(tmpdest.getRow(0));

    return true;
  };

  // On copy apply for type vector<ubyte>!
  bool histogramEqualization::apply(const vector<ubyte>& src,
                                          vector<ubyte>& dest) const {

      channel8 tmpsrc;
      channel8 tmpdest;

      // here as well, for the same reason, to be changed
      tmpsrc.useExternData(1,src.size(),const_cast<ubyte*>(&src.at(0)));

      apply(tmpsrc,tmpdest);

      dest.copy(tmpdest.getRow(0));

      return true;
  };

}
