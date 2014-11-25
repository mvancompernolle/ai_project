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
 * project ....: LTI Computer Vision Library
 * file .......: ltiSparseHistogram.pp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.2001
 * revisions ..: $Id: ltiSparseHistogram.cpp,v 1.7 2006/02/08 12:45:26 ltilib Exp $
 */

#include "ltiObject.h"
#include <limits>
#include <cstdio>
#include "ltiBoundsFunctor.h"
#include "ltiSTLIoInterface.h"
#include "ltiSparseHistogram.h"
#include "ltiVector.h"

// This stupid so-called C++ compiler from MS sucks!
#ifdef _LTI_MSC_6
#undef min
#undef max
#endif

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  // --------------------------------------------------
  // sparseHistogram
  // --------------------------------------------------

  const char* sparseHistogram::codeChars=
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!?";

  const int  sparseHistogram::maxN = strlen(codeChars);
  const float sparseHistogram::zero = 0;

  inline void sparseHistogram::checkTmpBuffer(const ivector& d) const {
    if (d.size() != tmpBufSize) {
      tmpBufSize=d.size();
      delete[] tmpBuffer;
      if (tmpBufSize > 0) {
        tmpBuffer=new char[tmpBufSize+1];
      } else {
        tmpBuffer=0;
      }
    }
  }

  sparseHistogram::sparseHistogram()
    : mathObject(),tmpBuffer(0) {
    resize(0,1);
  }


  sparseHistogram::sparseHistogram(const int dim, const int n)
    : mathObject(),tmpBuffer(0) {
    resize(dim,n);
  }

  sparseHistogram::sparseHistogram(const int n,
                                   const dvector& min,
                                   const dvector& max)
    : mathObject(),tmpBuffer(0) {
    resize(n,min,max);
  }


  sparseHistogram::sparseHistogram(const ivector& bn)
    : mathObject(),tmpBuffer(0) {
    resize(bn);
  }

  sparseHistogram::sparseHistogram(const ivector& bn,
                                   const dvector& min,
                                   const dvector& max)
    : mathObject(),tmpBuffer(0) {
    resize(bn,min,max);
  }

  sparseHistogram::~sparseHistogram() {
    clear();
    delete[] tmpBuffer;
    tmpBuffer=0;
    tmpBufSize=0;
  }

  sparseHistogram::sparseHistogram(const sparseHistogram& other)
  : mathObject(),tmpBuffer(0) {
    copy(other);
  }

  /*
   * Clear the previous content of the histogram and resize it
   * to the given dimensions and bins per dimensions.
   *
   * The maximum number of bins per dimension is limited to 64.
   *
   * @param dim number of dimensions
   * @param numberOfBins number of bins per dimension
   */
  void sparseHistogram::resize(const int dim, const int numberOfBins) {
    bins.resize(dim,numberOfBins,false,true);
    dvector min(dim,0.0);
    dvector max(dim,double(numberOfBins));
    resize(bins,min,max);
  }

  /*
   * Clear and resize the histogram to the number of dimensions equal to
   * the size of the vector bins, and having at each dimension the number
   * of bins given at each component of bins vector.
   *
   * The maximum number of bins per dimension is limited to 64.
   */
  void sparseHistogram::resize(const ivector& bn) {
    dvector min(bn.size(),0.0);
    dvector max;
    max.castFrom(bn);
    resize(bins,min,max);
  }

  /*
   * Clear and resize the histogram.
   *
   * The new histogram will have the given numbers of bins per dimension, for
   * a number of dimensions equal min.size() or max.size() (which must have
   * the same number of elements).
   * Furthermore, you must give the lower and upper bounds
   * of the hyperbox which is supposed to be occupied by the
   * histogram.
   * This will be used to access the histogram by vectors with
   * double elements.
   *
   * The maximum number of bins per dimension is limited to 64.
   */
  void sparseHistogram::resize(const int n,
                               const dvector& min,
                               const dvector& max) {
    bins.resize(min.size(),n,false,true);
    resize(bins,min,max);
  }

  /*
   * Clear and resize the histogram
   *
   * The first vector gives the number of bins for each dimension.
   * The second and third vector give the lower and upper bounds of
   * the hyperbox which is supposed to be occupied by the histogram.
   * This will be used to access the histogram by vectors with
   * double elements.
   *
   * The maximum number of bins per dimension is limited to 64.
   */
  void sparseHistogram::resize(const ivector& bn,
                               const dvector& min,
                               const dvector& max) {
    assert(bn.maximum() < maxN);
    assert(min.size() == max.size());
    assert(bn.size() == max.size());

    clear();

    bins=bn;
    // compute transform to get an index vector from a value vector
    offset=min;
    dvector tmp=max;
    tmp.subtract(min);
    scale.castFrom(bins);
    dvector::iterator i;
    for (i=tmp.begin(); i != tmp.end(); i++) {
      // avoid division by zero error
      if (lti::abs(*i) < std::numeric_limits<double>::min()) {
        (*i)=1.0;
      }
    }
    scale.edivide(tmp);
    minIndex.resize(min.size(),0,false,true);
    maxIndex=bins;
    maxIndex.add(-1);

    _lti_debug("MinIndex = " << minIndex << "\n");
    _lti_debug("MaxIndex = " << maxIndex << "\n");
    _lti_debug("Offset = " << offset << "\n");
    _lti_debug("Scale = " << scale << "\n");

    checkTmpBuffer(bins);
  }

  sparseHistogram& sparseHistogram::copy(const sparseHistogram& other) {

    clear();

    bins=other.bins;
    scale=other.scale;
    offset=other.offset;
    // copy the core
    for (mapType::const_iterator i=other.core.begin();
         i != other.core.end();
         ++i) {
      char *buf=new char[strlen(i->first)+1];
      strcpy(buf,i->first);
      core[buf]=i->second;
    }

    checkTmpBuffer(bins);

    return *this;
  }

  void sparseHistogram::clear() {
    for (mapType::iterator i=core.begin(); i != core.end(); ++i) {
      const char* key=i->first;
      delete[] const_cast<char*>(key);
    }
    core.clear();
  }

  const float& sparseHistogram::at(const ivector& a) const {
    makeKey(a,tmpBuffer);
    mapType::const_iterator i=core.find(tmpBuffer);
    if (i != core.end()) {
      return i->second;
    } else {
      return zero;
    }
  }

  float& sparseHistogram::at(const ivector& a) {
    makeKey(a,tmpBuffer);
    mapType::iterator i=core.find(tmpBuffer);
    if (i != core.end()) {
      return (*i).second;
    } else {
      const char *tmp=makeNewKey(a);
      core[tmp]=zero;
      return core[tmp];
    }
  }


  float sparseHistogram::get(const ivector& a) const {
    makeKey(a,tmpBuffer);
    _lti_debug("Key is " << tmpBuffer << "\n");
    mapType::const_iterator i=core.find(tmpBuffer);
    if (i != core.end()) {
      return i->second;
    } else {
      return zero;
    }
  }

  void sparseHistogram::put(const ivector& a, float v) {
    const char *buf=makeNewKey(a);
    mapType::iterator i=core.find(buf);
    if (i == core.end()) {
      core[buf]=v;
    } else {
      i->second=v;
      delete[] const_cast<char*>(buf);
    }
  }

  void sparseHistogram::clear(const ivector& a) {
    makeKey(a,tmpBuffer);
    mapType::iterator i=core.find(tmpBuffer);
    if (i != core.end()) {
      delete[] const_cast<char*>((*i).first);
      core.erase(i);
    }
  }

  void sparseHistogram::add(const ivector& a, float v) {
    makeKey(a,tmpBuffer);
    mapType::iterator i=core.find(tmpBuffer);
    if (i == core.end()) {
      core[makeNewKey(a)]=v;
    } else {
      i->second+=v;
    }
  }

  void sparseHistogram::multiply(const ivector& a, const float& v) {
    makeKey(a,tmpBuffer);
    mapType::iterator i=core.find(tmpBuffer);
    if (i != core.end()) {
      i->second*=v;
    }
  }

  void sparseHistogram::divide(const ivector& a, const float& v) {
    makeKey(a,tmpBuffer);
    mapType::iterator i=core.find(tmpBuffer);
    if (i != core.end()) {
      i->second/=v;
    }
  }

  void sparseHistogram::divide(const float& sum) {
    for (mapType::iterator i=core.begin(); i != core.end(); i++) {
      i->second/=sum;
    }
  }

  mathObject* sparseHistogram::clone() const {
    return new sparseHistogram(*this);
  }

  bool sparseHistogram::write(ioHandler& handler,
                              const bool complete) const {
    bool b=true;
    if (complete) {
      b=handler.writeBegin();
    }
    if (b) {
      b=b && lti::write(handler, "bins", bins);
      b=b && lti::write(handler, "offset", offset);
      b=b && lti::write(handler, "scale", scale);
      b=b && lti::write(handler, "core", core.size());
      b=b && handler.writeBegin();
      for (mapType::const_iterator i=core.begin(); i != core.end(); i++) {
        b=b && handler.writeBegin();
        b=b && handler.write(i->first);
        b=b && handler.writeKeyValueSeparator();
        b=b && handler.write(i->second);
        b=b && handler.writeEnd();
      }
      b=b && handler.writeEnd();
    }
    if (complete) {
      b=b && handler.writeEnd();
    }
    return b;
  }

  bool sparseHistogram::read(ioHandler& handler,
                             const bool complete) {

    clear();
    bool b=true;
    if (complete) {
      b=handler.readBegin();
    }
    if (b) {
      b=b && lti::read(handler, "bins", bins);
      b=b && lti::read(handler, "offset", offset);
      b=b && lti::read(handler, "scale", scale);
      int n;
      b=b && lti::read(handler, "core", n);
      clear();
      b=b && handler.readBegin();
      for (int i=0; i<n; i++) {
        b=b && handler.readBegin();
        std::string tmp;
        // read index string and copy it
        b=b && handler.read(tmp);
        char *buf=new char[tmp.length()+1];
        strcpy(buf,tmp.c_str());
        float value;
        b=b && handler.read(value);
        core[buf]=value;
        b=b && handler.readEnd();
      }
      minIndex.resize(bins.size());
      minIndex.fill(0);
      maxIndex=bins;
      maxIndex.add(-1);

      b=b && handler.readEnd();
    }
    if (complete) {
      b=handler.readEnd();
    }
    return b;
  }

}
