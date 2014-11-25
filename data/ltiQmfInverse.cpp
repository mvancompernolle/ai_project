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
 * file .......: ltiQmfInverse.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 23.1.2001
 * revisions ..: $Id: ltiQmfInverse.cpp,v 1.4 2006/02/08 11:42:20 ltilib Exp $
 */

#include "ltiQmfInverse.h"

namespace lti {

  // --------------------------------------------------
  // qmfInverse
  // --------------------------------------------------

  // default constructor
  qmfInverse::qmfInverse()
    : filter() {
    parameters defaultParam;
    setParameters(defaultParam);
  }

  // copy constructor
  qmfInverse::qmfInverse(const qmfInverse& other)
    : filter() {
    copy(other);
  }

  // destructor
  qmfInverse::~qmfInverse() {
  }

  // returns the name of this type
  const char* qmfInverse::getTypeName() const {
    return "qmfInverse";
  }

  // copy member
  qmfInverse&
    qmfInverse::copy(const qmfInverse& other) {
      filter::copy(other);
    return (*this);
  }

  // clone member
  functor* qmfInverse::clone() const {
    return new qmfInverse(*this);
  }

  // return parameters
  const qmfInverse::parameters&
    qmfInverse::getParameters() const {
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
  bool qmfInverse::apply(channel& srcdest) {

    initializeUpsamplers(false); // false -> column upsamplers too!

    const parameters& param = getParameters();

    if (param.levels>0) {
      int i;
      point old;
      vector<point> borders(param.levels);

      old = borders.at(0) = srcdest.size();
      borders.at(0).x--;
      borders.at(0).y--;

      for (i=1;i<param.levels;++i) {
        old = ((old/2)+(old%2));
        borders.at(i) = old;
        borders.at(i).subtract(point(1,1));
      }

      for (i=param.levels-1;i>=0;--i) {
        mergeTwo(srcdest,0,borders.at(i).y,0,borders.at(i).x);

        mergeTwoCol(srcdest,0,borders.at(i).y,0,borders.at(i).x);

      }

    }

    return true;
  };

  // On place apply for type vector<float>!
  bool qmfInverse::apply(vector<float>& srcdest) {

    initializeUpsamplers();

    const parameters& param = getParameters();

    if (param.levels>0) {
      int i,old;
      vector<int> borders(param.levels);

      old = borders.at(0) = srcdest.lastIdx();
      old++; // old contains the size of the subvector and not the last idx

      for (i=1;i<param.levels;++i) {
        old = ((old/2)+(old%2));
        borders.at(i) = old-1;
      }

      for (i=param.levels-1;i>=0;--i) {
        mergeTwo(srcdest,0,borders.at(i));
      }

    }

    return true;
  };

  // On copy apply for type channel!
  bool qmfInverse::apply(const channel& src,channel& dest) {
    dest.copy(src);

    return apply(dest);
  };

  // On copy apply for type vector<float>!
  bool qmfInverse::apply(const vector<float>& src,
                               vector<float>& dest) {
    dest.copy(src);

    return apply(dest);
  };

  void qmfInverse::mergeTwo(vector<float>& src,
                            const int& from,
                            const int& to) const {

    const int blockSize = to-from+1;
    const int boundary = (blockSize/2)+(blockSize%2)+from;

    // const_cast should not be used, but in this case is not as bad due
    // to the fact that lpvct and hpvct are also const vectors used as
    // wrappers for some subvectors of src.

    const vector<float> lpvct(boundary,const_cast<float*>(&src.at(0)),true);
    const vector<float> hpvct(blockSize-boundary,
                              const_cast<float*>(&src.at(boundary)),true);

    vector<float> lpvct2,hpvct2;

    lowUpsampler.apply(lpvct,lpvct2);
    highUpsampler.apply(hpvct,hpvct2);

    const int minSize = min(lpvct2.size(),hpvct2.size());

    // make the sum of the terms that can be added:

    int i,j;
    const float *lp,*hp,*ep;
    float* dest;

    dest = &src.at(from);
    lp = &lpvct2.at(0);
    hp = &hpvct2.at(0);
    ep = lp+minSize;

    for (;lp!=ep;++lp,++hp,++dest) {
      *dest = *lp + *hp;
    }

    // copy the rest of the elements

    if (lpvct2.size() == minSize) {
      // copy the high vector
      for (i=minSize,j=from+i;i<blockSize;++i,++j) {
        src.at(j)=hpvct2.at(i);
      }
    } else {
      // copy the low vector
      for (i=minSize,j=from+i;i<blockSize;++i,++j) {
        src.at(j)=lpvct2.at(i);
      }
    }
  }

  void qmfInverse::mergeTwo(channel& src,
                            const int& fromRow,
                            const int& toRow,
                            const int& fromCol,
                            const int& toCol) const {

    int i;
    for (i=fromRow;i<=toRow;++i) {
      mergeTwo(src.getRow(i),fromCol,toCol);
    }
  }

  void qmfInverse::mergeTwoCol(channel& src,
                               const int& fromRow,
                               const int& toRow,
                               const int& fromCol,
                               const int& toCol) const {

    const int blockSize = toRow-fromRow+1;
    const int boundary = (blockSize/2)+(blockSize%2)+fromRow;

    // const_cast should not be used, but in this case is not as bad due
    // to the fact that lpvct and hpvct are also const channels used as
    // wrappers for some subimages of src.

    const channel lpvct(false,*const_cast<channel*>(&src),
                        fromRow,boundary-1,
                        fromCol,toCol);
    const channel hpvct(false,*const_cast<channel*>(&src),
                        boundary,toRow,
                        fromCol,toCol);

    channel lpvct2,hpvct2;

    lowColUpsampler.apply(lpvct,lpvct2);
    highColUpsampler.apply(hpvct,hpvct2);

    const int minSize = min(lpvct2.rows(),hpvct2.rows());

    // make the sum of the terms that can be added:

    int i,j;
    const float *lp,*hp,*ep;
    float* dest;

    for (j=0;j<minSize;j++) {
      for (dest = &src.at(j+fromRow,fromCol),
             lp = &lpvct2.at(j,0),
             hp = &hpvct2.at(j,0),
             ep = lp+lpvct2.columns();
           lp!=ep;
           ++lp,++hp,++dest) {
        *dest = *lp + *hp;
      }
    }

    // copy the rest of the elements

    if (lpvct2.rows() == minSize) {
      // copy the high vector
      for (i=minSize,j=fromRow+i;i<blockSize;++i,++j) {
        for (dest = &src.at(j,fromCol),
               hp = &hpvct2.at(i,0),
               ep = hp+hpvct2.columns();
             hp!=ep;
             ++hp,++dest) {
          *dest = *hp;
        }
      }
    } else {
      // copy the low vector
      for (i=minSize,j=fromRow+i;i<blockSize;++i,++j) {
        for (dest = &src.at(j,fromCol),
               lp = &lpvct2.at(i,0),
               ep = lp+lpvct2.columns();
             lp!=ep;
             ++lp,++dest) {
          *dest = *lp;
        }
      }
    }
  }

  void qmfInverse::initializeUpsamplers(const bool onlyRows) {

    const parameters& param = getParameters();
    kernel1D<float> lpCoef,hpCoef;
    int s,i;

    // the wavelet coefficients for the low-pass (h(n)) and high-pass (g(n))
    // are related with g(n) = (-1)^n h(1-n).  And the kernels are related
    // with the coefficients with hh(n) = h(-n) and gg(n) = g(-n).
    // => gg(n) = (-1)^n hh(-n-1)
    //

    if (notNull(dynamic_cast<const kernel1D<float>*>(&param.getKernel()))) {
      lpCoef.mirror(*dynamic_cast<const kernel1D<float>*>(&param.getKernel()));
    } else {
      throw invalidParametersException("kernel not set in qmfInverse object");
    }

    // generate the hpCoef from the lpCoef:

    hpCoef.resize(1-lpCoef.lastIdx(),1-lpCoef.firstIdx(),false,false);

    if (hpCoef.firstIdx() % 2 != 0) {
      s = -1;
    } else {
      s = +1;
    }

    for (i=hpCoef.firstIdx();i<=hpCoef.lastIdx();++i) {
      hpCoef.at(i) = s*lpCoef.at(1-i);
      s *= -1;
    }

    // the row upsamplers
    upsampling::parameters upParam;
    upParam.boundaryType =  param.boundaryType;
    upParam.factor = point(2,1);

    upParam.setKernel(lpCoef);
    lowUpsampler.setParameters(upParam);

    upParam.setKernel(hpCoef);
    highUpsampler.setParameters(upParam);

    // the column upsamplers
    if (!onlyRows) {
      sepKernel<float> colKernel(0,0,1.0f); // 1x1 kernel initialized with 1.0
      upParam.factor = point(1,2);

      colKernel.getColFilter(0).copy(lpCoef);
      upParam.setKernel(colKernel);
      lowColUpsampler.setParameters(upParam);

      colKernel.getColFilter(0).copy(hpCoef);
      upParam.setKernel(colKernel);
      highColUpsampler.setParameters(upParam);
    }
  }
}
