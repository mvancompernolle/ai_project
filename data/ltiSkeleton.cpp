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
 * file .......: ltiSkeleton.cpp
 * authors ....: Daniel Ruijters
 * organization: LTI, RWTH Aachen
 * creation ...: 3.8.2000
 * revisions ..: $Id: ltiSkeleton.cpp,v 1.7 2006/02/08 11:50:42 ltilib Exp $
 */

#define VIEW_ON 0

#include "ltiSkeleton.h"
#include "ltiBinaryKernels.h"
#include "ltiErosion.h"
#include "ltiDilation.h"

#if VIEW_ON
#include "ltiViewer.h"
#include <iostream>

using std::cout;
using std::endl;

#endif

namespace lti {

  // --------------------------------------------------
  // skeleton::parameters
  // --------------------------------------------------

  // default constructor
  skeleton::parameters::parameters()
    : transform::parameters() {

    kernelType = CityBlock;
    formPointsValue = 255;
    jPointsValue = 255;
  }

  // copy constructor
  skeleton::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  skeleton::parameters::~parameters() {
  }

  // get type name
  const char* skeleton::parameters::getTypeName() const {
    return "skeleton::parameters";
  }

  // copy member

  skeleton::parameters&
    skeleton::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    transform::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    transform::parameters& (transform::parameters::* p_copy)
      (const transform::parameters&) =
      transform::parameters::copy;
    (this->*p_copy)(other);
# endif

    kernelType = other.kernelType;
//    threshold = other.threshold;
//    structuringElementSize = other.structuringElementSize;

    formPointsValue = other.formPointsValue;
    jPointsValue = other.jPointsValue;

    return *this;
  }

  // clone member
  functor::parameters* skeleton::parameters::clone() const {
    return new parameters(*this);
  }

  // --------------------------------------------------
  // skeleton
  // --------------------------------------------------

  // default constructor
  skeleton::skeleton()
    : transform(){
    // set default parameters
    parameters tmp;
    setParameters(tmp);
  }

  // copy constructor
  skeleton::skeleton(const skeleton& other)
    : transform()  {
    copy(other);
  }

  // destructor
  skeleton::~skeleton() {
  }

  // returns the name of this type
  const char* skeleton::getTypeName() const {
    return "skeleton";
  }

  // copy member
  skeleton&
    skeleton::copy(const skeleton& other) {
      transform::copy(other);
    return (*this);
  }

  // clone member
  functor* skeleton::clone() const {
    return new skeleton(*this);
  }

  // return parameters
  const skeleton::parameters&
    skeleton::getParameters() const {
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
  channel& skeleton::apply(channel& srcdest) const {

    channel tmp;
    apply(srcdest,tmp);
    tmp.detach(srcdest);

    return srcdest;
  };

  // On place apply for type channel8!
  channel8& skeleton::apply(channel8& srcdest) const {
    channel8 tmp;
    apply(srcdest,tmp);
    tmp.detach(srcdest);

    return srcdest;
  };


  // On copy apply for type channel!
  channel& skeleton::apply(const channel& src,channel& dest) const {
    channel8 chnl;
    chnl.castFrom(src);

    apply(chnl);

    dest.castFrom(chnl);

    return dest;
  };

  // On copy apply for type channel8!
  channel8& skeleton::apply(const channel8& src,channel8& dest) const {
    const parameters& par = getParameters();

    //I(i+1), S(i), M(i)
    erosion eroder;
    erosion::parameters erosionParam;
    dilation dilator;
    dilation::parameters dilationParam;
    channel8 I1, I2, S, M, J, temp, SoX;  //H
    int i = 1;

    //Kernels
    cityBlockKernel<channel8::value_type>  D(3);  //Dilation-kernel
    //Erosion-kernel:
    mathObject* E = 0;
    switch (par.kernelType) {
      case parameters::CityBlock:
        E = new cityBlockKernel<channel8::value_type>(3);
        break;
      case parameters::ChessBoard:
        E = new chessBoardKernel<channel8::value_type>(3);
        break;
      case parameters::Euclidean:
        E = new euclideanKernel<channel8::value_type>(3);
    }

#if VIEW_ON
    static viewer view1, view2;
#endif

    //init dest
    //dest has same Dim as src, filled with 0
    dest.resize(src.size(),0,false,true);

    erosionParam.mode = erosion::parameters::Binary;
    erosionParam.setKernel(*E);
    eroder.setParameters(erosionParam);

    dilationParam.mode = dilation::parameters::Binary;
    dilationParam.setKernel(D);
    dilator.setParameters(dilationParam);

    I1.copy(src);
    eroder.apply(I1);  //first erosion to remove small edge features
    while (I1.maximum() != 0) {
      //set all non-zero elements to formPointsValue
      visualize(I1, par.formPointsValue);
#if VIEW_ON
      view1.show(I1);
      cout << "Skeleton iteration: " << i << endl;
      //            if (getchar() == 'q') break;;
#endif

      if (par.kernelType == par.Euclidean) {
        // non-seperable sequence of E[i]
        // I[i+1] = I.Erosion(E[i])
        ((euclideanKernel<channel8::value_type>*)E)->generate(1+2*i);
        erosionParam.setKernel(*E);
        eroder.setParameters(erosionParam);
        eroder.apply(src,I2);
      } else { //separable sequence of E[i]
        // I[i+1] = I[i].Erosion(E)
        eroder.apply(I1,I2);
      }

      dilator.apply(I2, temp);
      difference(I1, temp, M);   //M[i] = I[i] \ I[i+1].Dilation(D)

      difference(I1, I2, S);     //S[i] = I[i] \ I[i+1]
      difference(S, M, temp);    //temp = S[i] \ M[i]
      SoX.copy(dest);
      img_or(S, SoX);            //SoX = S[i] U X[i-1]
      calc_jpoints(temp, SoX, J);

      // temp.copy(dest);
      // img_or(I1, temp);          //temp = I[i] U X[i-1]
      // calc_hpoints(S, temp, SoX, H);

      img_or(J, M);              //add skeleton points for preserving homotopy
      //            img_or(H, M);
      img_or(M, dest);           //add X to skeleton

      I1.copy(I2);  //for next iteration
      i++;

#if VIEW_ON
      //view2.show(dest);
      view2.show(img_or(dest,S));
#endif
    }

#if VIEW_ON
    cout << "Skeleton completed!!!" << endl;
    cout << "Iterations: " << i-1 << endl << endl;
#endif
    delete E;
    return dest;
  };

  channel8& skeleton::img_or(const channel8& src1, channel8& srcdest) const {
    // assume both channels have the same size.  This should always be true!
    assert(src1.size()==srcdest.size());

    channel8::const_iterator cit,ce;
    channel8::iterator it;

    for (cit=src1.begin(),ce=src1.end(),it=srcdest.begin();
         cit!=ce;
         ++cit,++it) {
      if ((*cit)>(*it)) {
        (*it)=(*cit);
      }
    }

    return srcdest;
  };

  // dest = img \ stamp
  channel8& skeleton::difference(const channel8& img,
                                 const channel8& stamp,
                                 channel8& dest) const {
    // assume both channels have the same size.  This should always be true
    assert(img.size()==stamp.size());

    channel8::const_iterator cit,ce;
    channel8::iterator it;

    dest.copy(img);

    for (cit=stamp.begin(),ce=stamp.end(),it=dest.begin();
         cit!=ce;
         ++cit,++it) {
      if ((*cit) != 0) {
        (*it)=0;
      }
    }

    return dest;
  };


  channel8& skeleton::calc_jpoints(const channel8& src,
                                   const channel8& match,
                                   channel8& dest) const {
    const parameters& par = getParameters();

    point p;
    int a,b,c;
    int N4_1[] = { 1,  0,  1,  0, 2,  3, 5, 4};
    int N4_2[] = { 3,  2,  4,  5, 7,  6, 7, 6};
    int N8_y[] = {-1, -1, -1,  0, 0,  1, 1, 1};
    int N8_x[] = {-1,  0,  1, -1, 1, -1, 0, 1};

    //init dest
    //dest has same Dim as src, filled with 0
    dest.resize(src.size(),0,false,true);

    const int lastRow = src.lastRow();
    const int lastCol = src.lastColumn();

    for (p.y=1; p.y < lastRow; p.y++) {
      for (p.x=1; p.x < lastCol; p.x++) {
        if (src.at(p) != 0) {
          for (a=0; a<6; a++)
            if (match.at(p.y+N8_y[a],p.x+N8_x[a]) != 0) for (b=a+1; b<7; b++)
              if (match.at(p.y+N8_y[b],p.x+N8_x[b]) != 0)for (c=b+1; c<8; c++)
                if (match.at(p.y+N8_y[c],p.x+N8_x[c]) != 0
                    && N4_1[a] != b  //not N4 connected
                    && N4_2[a] != b
                    && N4_1[b] != c
                    && N4_2[b] != c
                    && N4_1[c] != a
                    && N4_2[c] != a
                    )
                  dest.at(p) = par.jPointsValue;
        }
      }
    }

    return dest;
  };

  channel8& skeleton::calc_hpoints(const channel8& src,
                                   const channel8& match1,
                                   const channel8& match2,
                                   channel8& dest) const {
    point p;
    int n,m,pos(0);

    const int lastRow = src.lastRow();
    const int lastCol = src.lastColumn();

    int N4_1[] = { 1,  0,  1,  0, 2,  3, 5, 4};
    int N4_2[] = { 3,  2,  4,  5, 7,  6, 7, 6};
    int N8_x[] = {-1,  0,  1, -1, 1, -1, 0, 1};
    int N8_y[] = {-1, -1, -1,  0, 0,  1, 1, 1};

    //init dest
    //dest has same Dim as src, filled with 0
    dest.resize(src.size(),0,false,true);

    for (p.y=1; p.y < lastRow; p.y++) {
      for (p.x=1; p.x < lastCol; p.x++) {
        if (src.at(p) != 0) {
          for (m=0, n=0; n<8; n++) {
            if (match1.at(p.y+N8_y[n],p.x+N8_x[n]) == 0) {
//                            && match2.at(x+N8_x[n], y+N8_y[n]) == 0)
              pos = n;
              m++;
            }
          }

          if (m == 1  //just one background pixel
              && match2.at(p.y + N8_y[ N4_1[pos]],p.x + N8_x[ N4_1[pos]]) != 0
              && match2.at(p.y + N8_y[ N4_2[pos]],p.x + N8_x[ N4_2[pos]]) != 0
              )
            dest.at(p) = 127;
        }
      }
    }

    return dest;
  };


  channel8& skeleton::visualize(channel8& srcdest, const int& val) const {
    channel8::iterator it,e;

    for (it=srcdest.begin(),e=srcdest.end();it!=e;it++) {
      if ((*it) != 0) {
        (*it)=val;
      }
    }

    return srcdest;
  };

}
