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
 * file .......: ltiCurvatureScaleSpace.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 24.6.2001
 * revisions ..: $Id: ltiCurvatureScaleSpace.cpp,v 1.15 2006/09/05 10:08:47 ltilib Exp $
 */

#include "ltiCurvatureScaleSpace.h"
#include "ltiBilinearInterpolator.h"
#include "ltiGaussKernels.h"

#undef _LTI_DEBUG
// #define _LTI_DEBUG 2
#include "ltiDebug.h"

#ifdef _LTI_DEBUG

#include "ltiFastViewer.h"
#include "ltiViewer.h"
#include <iostream>
#include <cstdio>
#include "ltiDraw.h"

using std::cout;
using std::endl;

#endif

namespace lti {
  // --------------------------------------------------
  // curvatureScaleSpace::parameters
  // --------------------------------------------------

  // default constructor
  curvatureScaleSpace::parameters::parameters()
    : transform::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    geometricDelta = false;
    deltaSigma = double(0.1);
    startSigma = double(1.0);
    gaussianSize = int(9);
    normLength = int(256);
    sigmaAxisBlock = int(128);
    fastMethod = bool(true);

    posNegCross = ubyte(0);
    negPosCross = ubyte(255);
    nonCross = ubyte(128);
  }

  // copy constructor
  curvatureScaleSpace::parameters::parameters(const parameters& other)
    : transform::parameters()  {
    copy(other);
  }

  // destructor
  curvatureScaleSpace::parameters::~parameters() {
  }

  // get type name
  const char* curvatureScaleSpace::parameters::getTypeName() const {
    return "curvatureScaleSpace::parameters";
  }

  // copy member

  curvatureScaleSpace::parameters&
    curvatureScaleSpace::parameters::copy(const parameters& other) {
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


    geometricDelta = other.geometricDelta;
    deltaSigma = other.deltaSigma;
    startSigma = other.startSigma;
    gaussianSize = other.gaussianSize;
    normLength = other.normLength;
    sigmaAxisBlock = other.sigmaAxisBlock;
    fastMethod = other.fastMethod;

    posNegCross = other.posNegCross;
    negPosCross = other.negPosCross;
    nonCross = other.nonCross;

    return *this;
  }

  // alias for copy member
  curvatureScaleSpace::parameters&
    curvatureScaleSpace::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* curvatureScaleSpace::parameters::clone() const {
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
  bool curvatureScaleSpace::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool curvatureScaleSpace::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"geometricDelta",geometricDelta);
      lti::write(handler,"deltaSigma",deltaSigma);
      lti::write(handler,"startSigma",startSigma);
      lti::write(handler,"gaussianSize",gaussianSize);
      lti::write(handler,"normLength",normLength);
      lti::write(handler,"sigmaAxisBlock",sigmaAxisBlock);
      lti::write(handler,"fastMethod",fastMethod);

      lti::write(handler,"posNegCross",posNegCross);
      lti::write(handler,"negPosCross",negPosCross);
      lti::write(handler,"nonCross",nonCross);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::write(handler,false);
# else
    bool (transform::parameters::* p_writeMS)(ioHandler&,const bool) const =
      transform::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool curvatureScaleSpace::parameters::write(ioHandler& handler,
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
  bool curvatureScaleSpace::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool curvatureScaleSpace::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"geometricDelta",geometricDelta);
      lti::read(handler,"deltaSigma",deltaSigma);
      lti::read(handler,"startSigma",startSigma);
      lti::read(handler,"gaussianSize",gaussianSize);
      lti::read(handler,"normLength",normLength);
      lti::read(handler,"sigmaAxisBlock",sigmaAxisBlock);
      lti::read(handler,"fastMethod",fastMethod);

      lti::read(handler,"posNegCross",posNegCross);
      lti::read(handler,"negPosCross",negPosCross);
      lti::read(handler,"nonCross",nonCross);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && transform::parameters::read(handler,false);
# else
    bool (transform::parameters::* p_readMS)(ioHandler&,const bool) =
      transform::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool curvatureScaleSpace::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // curvatureScaleSpace
  // --------------------------------------------------

  // default constructor
  curvatureScaleSpace::curvatureScaleSpace()
    : transform(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  curvatureScaleSpace::curvatureScaleSpace(const curvatureScaleSpace& other)
    : transform()  {
    copy(other);
  }

  // destructor
  curvatureScaleSpace::~curvatureScaleSpace() {
  }

  // returns the name of this type
  const char* curvatureScaleSpace::getTypeName() const {
    return "curvatureScaleSpace";
  }

  // copy member
  curvatureScaleSpace&
  curvatureScaleSpace::copy(const curvatureScaleSpace& other) {
    transform::copy(other);
    return (*this);
  }

  // clone member
  functor* curvatureScaleSpace::clone() const {
    return new curvatureScaleSpace(*this);
  }

  // return parameters
  const curvatureScaleSpace::parameters&
    curvatureScaleSpace::getParameters() const {
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

  bool curvatureScaleSpace::apply(const borderPoints& src,
                                        channel8& dest,
                                        float& maxSigma,
                                        int& maxRow) const {

    if (src.size() < 9) { // smallest circle has 8 points
      dest.clear();
      maxSigma = -1;
      maxRow = -1;
      setStatusString("unsufficient points to compute CSS-image");
      return false;
    }

    pointList rows;

    const parameters& param = getParameters();

    if (param.fastMethod) {
      return cssFast(src,dest,maxSigma,maxRow,rows);
    } else {
      return cssOriginal(src,dest,maxSigma,maxRow,rows);
    }
  }

  bool curvatureScaleSpace::apply(const borderPoints& src,
                                        channel8& dest,
                                        pointList& rows) const {

    if (src.size() < 9) { // smallest circle has 8 points
      dest.clear();
      rows.clear();
      setStatusString("unsufficient points to compute CSS-image");
      return false;
    }

    float maxSigma;
    int maxRow;

    const parameters& param = getParameters();

    if (param.fastMethod) {
      return cssFast(src,dest,maxSigma,maxRow,rows);
    } else {
      return cssOriginal(src,dest,maxSigma,maxRow,rows);
    }

  }

  // On copy apply for type channel8!
  bool curvatureScaleSpace::apply(const borderPoints& src,
                                        channel8& dest) const {
    float maxSigma;
    int maxRow;

    return apply(src,dest,maxSigma,maxRow);
  }

  bool curvatureScaleSpace::apply(const borderPoints& src,
                                        channel8& dest,
                                        dmatrix& vcts) const {
    pointList rows;
    return apply(src,dest,vcts,rows);

  }

  bool curvatureScaleSpace::apply(const borderPoints& src,
                                        channel8& dest,
                                        dmatrix& vcts,
                                        pointList& rows) const {
    matrix<int> blobs;
    int numFeatures;

    if (apply(src,dest,rows)) {
      numFeatures = extractBlobs(dest,rows,blobs,vcts);

      if (numFeatures < vcts.rows()) {
        vcts.resize(numFeatures,vcts.columns(),0.0,true,true);
      }

#     ifdef _LTI_DEBUG
      const parameters& param = getParameters();
      static viewer view("blobs");
      image img(blobs.size(),rgbPixel(0,0,0));
      channel chnl;

      draw<rgbPixel> drawer;
      drawer.use(img);

      chnl.castFrom(blobs);
      chnl.apply(signum0);
      chnl.add(1.0f);
      chnl.divide(2.2f);

      drawer.setColor(rgbPixel(240,240,255));
      drawer.overlay(chnl);
      int i;
      double medpos;
      rectangle r;

      cout << "Computed features: " << endl;

      for (i=0;i<numFeatures;++i) {
        cout << i << ": Max ("
             << vcts.at(i,MaxX) << ","
             << vcts.at(i,MaxY) << ") ";
        cout << "W: " << vcts.at(i,Width)
             << "  O: " << vcts.at(i,Offset) << endl;


        int theMaxY;

        if (param.geometricDelta) {
          theMaxY = static_cast<int>(round(log(vcts.at(i,MaxY)/
                                               param.startSigma)/
                                           log(param.deltaSigma)));
        } else {
          theMaxY = static_cast<int>(round((vcts.at(i,MaxY)-
                                            param.startSigma)/
                                           param.deltaSigma));
        }

        // draw the rectangle

        medpos = vcts.at(i,MaxX)+vcts.at(i,Offset);
        r.ul.x = static_cast<int>(round(medpos - (vcts.at(i,Width)/2.0)));
        r.ul.y = 0;
        r.br.x = static_cast<int>(round(r.ul.x + vcts.at(i,Width)));
        r.br.y = theMaxY;

        if (r.ul.x < 0) {
          int tmp;
          tmp = r.ul.x;
          r.ul.x = r.br.x;
          r.br.x = img.columns();
          drawer.rectangle(r);

          r.ul.x = 0;
          r.br.x = tmp;
        } else if (r.br.x >= img.columns()) {
          drawer.rectangle(r);

          r.br.x -= img.columns();
          r.ul.x = 0;
        }

        drawer.setColor(rgbPixel(255,96,64));
        drawer.rectangle(r);

        // draw the maxima
        drawer.setColor(rgbPixel(255,255,192));
        drawer.set(iround(vcts.at(i,MaxX)),theMaxY);
        drawer.setColor(rgbPixel(64,255,128));
        drawer.circle(point(iround(vcts.at(i,MaxX)),theMaxY),3);

      }

      view.show(img);
#     endif

      return true;
    }

    return false;
  }

  bool curvatureScaleSpace::apply(const borderPoints& src,
                                  dmatrix& dest) const {
    channel8 css;
    return apply(src,css,dest);
  }

  /*
   * extract the x and y components of the borderPoints object and
   * resize (normalize the length) of the vectors if required.
   */
  bool curvatureScaleSpace::borderToXY(const borderPoints& src,
                                       vector<float>& theX,
                                       vector<float>& theY) const {

    const parameters& param = getParameters();

    int length = param.normLength;
    if (length <= 0) {
      length = src.size();
    }

    // first extract the x and y components of the border points
    // in the tmp and theY vectors.  After that, normalize the length
    // to the value given in the parameters.
    vector<float> tmp;

    tmp.resize(src.size(),0.0f,false,false);
    theY.resize(src.size(),0.0f,false,false);

    bilinearInterpolator<float> biInterp;
    bilinearInterpolator<float>::parameters biInterpPar;
    biInterpPar.boundaryType = lti::Periodic;
    biInterp.setParameters(biInterpPar);

    borderPoints::const_iterator it,eit;
    int i;
    for (it=src.begin(),eit=src.end(),i=0;
         it!=eit;
         ++it,++i) {
      tmp.at(i) = static_cast<float>((*it).x);
      theY.at(i) = static_cast<float>((*it).y);
    };

    // normalize the size of x
    theX.resize(length,0.0f,false,false);
    float step = float(tmp.lastIdx())/float(theX.lastIdx());
    for (i=0;i<theX.size();++i) {
      theX.at(i) = biInterp.apply(tmp,step*i);
    }

    // normalize the size of y
    theY.detach(tmp);
    theY.resize(length,0.0f,false,false);
    for (i=0;i<theY.size();++i) {
      theY.at(i) = biInterp.apply(tmp,step*i);
    }

    return true;
  }


  void curvatureScaleSpace::calcKernels(const float& sigma,
                                        kernel1D<float>& first,
                                        kernel1D<float>& second) const {
    const parameters& param = getParameters();
    int newlength = static_cast<int>(0.5 + sigma * param.gaussianSize);
    if ((newlength % 2) == 0) { // new length must be an odd number
      newlength++;
    }

    int i;
    float i2;
    const int n = newlength/2;
    float tmp1,sigma2;
    sigma2 = sigma*sigma;
    first.resize(-n,n,0);
    second.resize(-n,n,0);
    for (i=-n;i<=n;++i) {
      i2 = float(i*i);
      tmp1 = static_cast<float>(exp(-i2/(2.0*sigma2))/
                                (sqrt(2*Pi)*sigma2*sigma));
      first.at(i) = -float(i)*tmp1;
      second.at(i) = tmp1*(i2-sigma2)/sigma2;
    }

  }


  int curvatureScaleSpace::curvature(const float& sigma,
                                     const vector<float>& theX,
                                     const vector<float>& theY,
                                           vector<float>& curv,
                                           vector<ubyte>& zeroCross) const {

    const parameters& param = getParameters();

    kernel1D<float> first,second;
    calcKernels(sigma,first,second);

    vector<float> xu,yu,xuu,yuu;

    convolution convolver;
    convolution::parameters convPar;

    // first derivatives;

    convPar.boundaryType = lti::Periodic;

    convPar.setKernel(first);
    convolver.setParameters(convPar);
    convolver.apply(theX,xu);
    convolver.apply(theY,yu);

    // second derivatives;
    convPar.setKernel(second);
    convolver.setParameters(convPar);
    convolver.apply(theX,xuu);
    convolver.apply(theY,yuu);

    curv.resize(theX.size(),0.0,false,false);

    int i,zc;
    float x,y;
    int lastNonZeroIdx = -1;
    float lastNonZero = 0.0f;
    float a;
    const float threshold = 0.0f;

    for (i=0,zc=0;i<curv.size();++i) {
      x = xu.at(i);
      y = yu.at(i);

      a = curv.at(i) = (x*yuu.at(i) - xuu.at(i)*y);

      // zero crossing detection
      if (a != 0.0f) {
        if (((lastNonZero * a) < threshold) &&
            (lastNonZeroIdx >= 0)) {
          // zero crossing detected:
          ++zc;
          zeroCross.at((i+lastNonZeroIdx)/2) =
            ((lastNonZero<0) ? param.negPosCross : param.posNegCross);
        }

        lastNonZeroIdx = i;
        lastNonZero = a;
      }
    }

    return zc;
  }

  void curvatureScaleSpace::calcKernels(float& sigma,
                                        gaussKernel1D<float>& gaussian) const {

    const parameters& param = getParameters();

    float newSigma2;
    int newlength;

    if (sigma > param.startSigma) {
      float oldSigma;
      if (param.geometricDelta) {
        oldSigma = static_cast<float>(sigma/param.deltaSigma);
        newSigma2 = static_cast<float>((param.deltaSigma*
                                        param.deltaSigma-1.0f)*
                                        oldSigma*oldSigma);
      } else {
        oldSigma = static_cast<float>(sigma - param.deltaSigma);
        newSigma2 = static_cast<float>(param.deltaSigma*(2*oldSigma + 
                                                         param.deltaSigma));
      }

      newlength = iround(0.5 + sqrt(newSigma2) * param.gaussianSize/
                               param.startSigma);

      if ((newlength % 2) == 0) { // new length must be an odd number
        newlength++;
      }
    } else {
      newlength = param.gaussianSize;
      newSigma2 = static_cast<float>(param.startSigma*param.startSigma);
    }

    gaussian.generate(newlength,newSigma2);

    if (param.geometricDelta) {
      sigma*=static_cast<float>(param.deltaSigma);
    } else {
      sigma += static_cast<float>(param.deltaSigma);
    }

  }

  // curvature computation for cssFast
  int curvatureScaleSpace::curvature(const gaussKernel1D<float>& gaussian,
                                           vector<float>& theX,
                                           vector<float>& theY,
                                           vector<float>& curv,
                                           vector<ubyte>& zeroCross,
                                     const int& lastZC) const {

    const parameters& param = getParameters();

    // temporary data
    vector<float> xu,yu,xuu,yuu;

    // resize output data
    curv.resize(theX.size(),0.0f,false,false);
    zeroCross.resize(theX.size(),param.nonCross,false,false);

    // prepare the new x(t) and y(t) (gauss filter)
    convolution convolver;
    convolution::parameters convPar;
    convPar.boundaryType = lti::Periodic;
    convPar.setKernel(gaussian);

    convolver.setParameters(convPar);

    convolver.apply(theX);
    convolver.apply(theY);

    // derivatives;
    derive(theX,xu,xuu);
    derive(theY,yu,yuu);

    int i,zc;
    float x,y;
    int lastNonZeroIdx = -1;
    float lastNonZero = 0.0f;
    float a;
    const float threshold = 0.0f;

    // float d;
    for (i=0,zc=0;i<curv.size();++i) {
      x = xu.at(i);
      y = yu.at(i);

      // d = (x*x + y*y); // for the zero crossings this is irrelevant
      a = curv.at(i) = (x*yuu.at(i) - xuu.at(i)*y); // /(sqrt(d)*d);

      // zero crossing detection
      if (a != 0.0f) {
        if (((lastNonZero * a) < threshold) &&
            (lastNonZeroIdx >= 0)) {
          // zero crossing detected:
          ++zc;
          zeroCross.at((i+lastNonZeroIdx)/2) =
            ((lastNonZero<0) ? param.negPosCross : param.posNegCross);
        }

        lastNonZeroIdx = i;
        lastNonZero = a;
      }
    }

    while (zc > lastZC) { // number of zero crossing cannot become greater!

      convolver.apply(curv);
      zeroCross.fill(param.nonCross);

      lastNonZeroIdx = -1;
      lastNonZero = 0.0f;
      for (i=0,zc=0;i<curv.size();++i) {
        a = curv.at(i);

        // zero crossing detection
        if (a != 0.0f) {
          if (((lastNonZero * a) < threshold) &&
              (lastNonZeroIdx >= 0)) {
            // zero crossing detected:
            ++zc;
            zeroCross.at((i+lastNonZeroIdx)/2) =
              ((lastNonZero<0) ? param.negPosCross : param.posNegCross);
          }

          lastNonZeroIdx = i;
          lastNonZero = a;
        }
      }

    }

#   if defined(_LTI_DEBUG) && (_LTI_DEBUG>=4)
    static viewer view("curve");
    view.show(curv);
#   endif

    return zc;
  }

  // calculate the first and second derivatives of the given vector.
  void curvatureScaleSpace::derive(const vector<float>& theX,
                                   vector<float>& xu,
                                   vector<float>& xuu) const {

    xu.resize(theX.size(),0.0f,false,false);
    xuu.resize(theX.size(),0.0f,false,false);

    if (theX.size()<3) {
      // if the vectors have less than 3 elements
      xu.fill(0.0f); // the first derivative is always zero
      if (theX.size() == 1) {
        xuu.fill(0.0f); // the second derivative is also zero if only one elem.
      } else {
        // compute the second derivative if the array has two elements
        xuu.at(0) = 2.0f*(theX.at(1)-theX.at(0));
        xuu.at(1) = 2.0f*(theX.at(0)-theX.at(1));
      }
      return;
    }

    const float *nm1, *np1, *n, *endn;
    float *fn, *sn;

    endn = (&theX.at(0))+theX.size();

    // compute the first and second derivatives using some pointers
    for (nm1=&theX.at(0),np1=&theX.at(2),n = &theX.at(1),
           fn = &xu.at(1),sn = &xuu.at(1);
         np1 != endn;
         ++nm1,++np1,++n,++fn,++sn) {
      *fn = ((*np1)-(*nm1))/2.0f;
      *sn = ((*np1)+(*nm1)-2.0f*(*n));
    }

    // boundaries of the first derivative
    xu.at(0) = (theX.at(1)-theX.at(theX.lastIdx()))/2.0f;
    xu.at(xu.lastIdx()) = (theX.at(0)-theX.at(theX.lastIdx()-1))/2.0f;

    // boundaries of the second derivative
    xuu.at(0) = (theX.at(1)+theX.at(theX.lastIdx())-2.0f*theX.at(0));
    xuu.at(xu.lastIdx()) = (theX.at(0)+theX.at(theX.lastIdx()-1)
                            -2.0f*theX.at(theX.lastIdx()));

  }

  // On copy apply for type channel8!
  bool curvatureScaleSpace::cssOriginal(const borderPoints& src,
                                        channel8& dest,
                                        float& maxSigma,
                                        int& maxRow,
                                        pointList& rows) const {


#   if defined(_LTI_DEBUG) && (_LTI_DEBUG>=4)
    static fastViewer view("css");
#   endif

    const parameters& param = getParameters();
    rows.clear();

    // first extract the x and y components of the border points
    vector<float> theX,theY;

    borderToXY(src,theX,theY);

    // predict the number of evolutions of the curve:
    int startn = param.sigmaAxisBlock;
    if (startn <= 0) {
      startn = 1;
    }

    dest.resize(startn,theX.size(),param.nonCross,false,true);
    float sigma = static_cast<float>(param.startSigma);
    vector<float> curv;

    int i(0),zc(0);
    int lastNumberOfZeroCrossings = -1;
    point pt;

    do {
      if (i>=dest.rows()) {
        dest.resize(dest.rows()+startn,dest.columns(),
                    param.nonCross,true,true);
      }

      maxRow = i;
      maxSigma = sigma;

      // "exact" method to calculate curvature and zero crossings
      zc = curvature(sigma,theX,theY,curv,dest.getRow(i));

      // remember the rows with maxima!
      if ((lastNumberOfZeroCrossings > 0) &&
          (lastNumberOfZeroCrossings > zc)) {
        pt.x = (lastNumberOfZeroCrossings - zc) / 2;
        pt.y = i-1;
        rows.push_front(pt);

        _lti_debug(pt);

      }

      lastNumberOfZeroCrossings = zc;

#     if defined(_LTI_DEBUG) && (_LTI_DEBUG>=4)
      view.show(dest);
#     endif

      if (param.geometricDelta) {
        sigma *= static_cast<float>(param.deltaSigma);
      } else {
        sigma += static_cast<float>(param.deltaSigma);
      }

      i++;
    } while (zc > 0);

    return true;
  };

  // On copy apply for type channel8!
  bool curvatureScaleSpace::cssFast(const borderPoints& src,
                                    channel8& dest,
                                    float& maxSigma,
                                    int& maxRow,
                                    pointList& rows) const {


    const parameters& param = getParameters();

    rows.clear();

    // first extract the x and y components of the border points
    vector<float> theX,theY;
    borderToXY(src,theX,theY);

    // predict the number of evolutions of the curve:
    int startn = param.sigmaAxisBlock;
    if (startn <= 0) {
      startn = 1;
    }

    // fill the destination with the "nonCross" flag!
    dest.resize(startn,theX.size(),param.nonCross,false,true);

    float sigma = static_cast<float>(param.startSigma);
    vector<float> curv,zeroCross;

    int i(0),zc;
    int lastNumberOfZeroCrossings = -1;
    point pt;
    gaussKernel1D<float> gaussian;

    zc = theX.size();
    do {

      maxRow = i;
      maxSigma = sigma;

      // if the destination image is full, resize it:
      if (i>=dest.rows()) {
        dest.resize(dest.rows()+startn,dest.columns(),
                    param.nonCross,true,true);
      }

      // compute optimized gaussian (to filter the last curvature)
      calcKernels(sigma,gaussian);

      // compute the new curvature, through the x and y components of the
      // boundary.  Also compute the line of the css image and compute the
      // number of zero crossings.  The last number of zero crossings is also
      // expected by the function, which uses it to detect noise effects:
      // it is not allowed that the new number of zero crossings is greater
      // than the last zero crossings
      zc = curvature(gaussian,theX,theY,curv,dest.getRow(i),zc);

      // remember the rows with maxima!
      if ((lastNumberOfZeroCrossings > 0) &&
          (lastNumberOfZeroCrossings > zc)) {
        pt.x = (lastNumberOfZeroCrossings - zc) / 2;
        pt.y = i-1;
        rows.push_front(pt);
      }

      lastNumberOfZeroCrossings = zc;

#     if defined(_LTI_DEBUG) && (_LTI_DEBUG>=4)
      static fastViewer view("css");
      view.show(dest);
#     endif

      i++;
    } while (zc > 0);

    return true;
  };



  /*
   * extract blobs
   */
  int curvatureScaleSpace::extractBlobs(const channel8& css,
                                        const pointList& rows,
                                        matrix<int>& blobs,
                                        dmatrix& features) const {

    const parameters& param = getParameters();
    const int col = css.columns();
    const int maxDeltaCol = (css.columns()/2)+1;

    // accumulator of the X position values for a blob edge
    vector<double> blobSinXAcc(col,0.0f);
    vector<double> blobCosXAcc(col,0.0f);

    // number of points in the given blob edge
    vector<int> blobMaxY(col,-1);
    vector<int> blobMaxX(col,-1);

    // consistency map
    static const ubyte unused = 0;
    static const ubyte ok = 1;
    static const ubyte inconsistent = 2;

    vector<ubyte> consistency(col,unused);

    channel8 csstmp(css);

    blobs.resize(css.size(),0,false,true);

    pointList::const_iterator it;
    ubyte ocol;
    int x,y,xx,yy,xt,xc;
    int blobNumber = 1;
    int numberOfInconsistentBlobs = 0;
    int value;
    bool probInconsistent;
    int i,j,k;
    double pos1,pos2,medpos;
    
    const double fac = 2.0*Pi/static_cast<double>(col);
    double sina, cosa;

    it = rows.begin();

    while (it != rows.end()) {
      // get the first point of the rows
      y = (*it).y;

      // security check:  there are some cases, where due to the extra high
      // number of inconsistencies, the blob number grows too fast.  If we
      // come here, these data are irrelevant, and we should skip till end.
      if (blobNumber >= col) {
        it = rows.end();
        continue;
      }

      // track down all points found
      for (x = 0; x<col; ++x) {
        if ((csstmp.at(y,x)) != param.nonCross) {
          ocol = csstmp.at(y,x);

          value = (ocol == param.negPosCross) ? blobNumber : -blobNumber;

          // something found...
          blobs.at(y,x) = value;
          csstmp.at(y,x) = param.nonCross;

          const double angle = static_cast<double>(x)*fac;
          sincos(angle, sina, cosa);
          blobSinXAcc.at(blobNumber) += sina;
          blobCosXAcc.at(blobNumber) += cosa;
          blobMaxY.at(blobNumber) = y;
          blobMaxX.at(blobNumber) = x;

          consistency.at(blobNumber) = ok; // assume everything is ok

          // try to track down the blob edge
          yy = y-1;
          xt = x;
          xx = x;

          while (yy >= 0) {
            xc = 1;

            probInconsistent = false;

            // try to find the continuation:
            while (csstmp.at(yy,xx) != ocol) {
              probInconsistent = probInconsistent || (blobs.at(yy,xx) != 0);
              xx = (xt+xc+col)%col;
              xc = (xc>0) ? -xc : 1-xc;
	      if (xc > maxDeltaCol) {
		probInconsistent = true;
		break;
	      }
            }

            // if the found point already belongs to a blob, there might be
            // an inconsistency
            if (probInconsistent) {
              int xtmp = xt;
              // check if there is really an inconsistency
              if (xc<=1) {
                for (xtmp=(xtmp+1)%col;xtmp != xx; xtmp=(xtmp+1)%col) {
                  if (blobs.at(yy,xtmp) != 0) {
                    if (consistency.at(blobNumber) != inconsistent) {
                      consistency.at(blobNumber) = inconsistent;
                      numberOfInconsistentBlobs++;
                    }
                    yy = -1;
                    xtmp = (xx-1+col)%col;
                  }
                }
              } else {
                for (xtmp=(xtmp-1+col)%col;xtmp != xx; xtmp=(xtmp-1+col)%col) {
                  if (blobs.at(yy,xtmp) != 0) {
                    if (consistency.at(blobNumber) != inconsistent) {
                      consistency.at(blobNumber) = inconsistent;
                      numberOfInconsistentBlobs++;
                    }
                    yy = -1;
                    xtmp = (xx+1)%col;
                  }
                }
              }

#             ifdef _LTI_DEBUG
              if (consistency.at(blobNumber) == inconsistent) {
                cout << "Inconsistency detected at (" << yy << ","
                     << xx << "); blob id " << value << endl;
              }
#             endif
            } else {

              xt = xx;

              blobs.at(yy,xt) = value;

              const double angle = static_cast<double>(xt)*fac;
              sincos(angle, sina, cosa);
              blobSinXAcc.at(blobNumber) += sina;
              blobCosXAcc.at(blobNumber) += cosa;

              csstmp.at(yy,xt) = param.nonCross;

              --yy;
            }
          } // endif (yy>=0)

          blobNumber++;
          if (blobNumber >= col) {
            // too many inconsistencies!!!  this isn't really possible
            x = col;
          }
        }
      }

      ++it;
    }

    features.resize((blobNumber-1-numberOfInconsistentBlobs)/2,4);

    i=j=0;
    while ((i<features.rows()) && (j<consistency.lastIdx())) {
      // find two halfs of a blob to create one
      // search for the next valid one
      while ((j<consistency.lastIdx()) &&
             (consistency.at(j) != ok)) {
        ++j;
      };

      // search for the second one
      if ((j<consistency.lastIdx()) &&
          (consistency.at(j) == ok) &&
          (consistency.at(j+1) == ok) &&
          (blobMaxY.at(j) == blobMaxY.at(j+1))) {
        // one pair found!
        k=j+1;

        if (param.geometricDelta) {
          features.at(i,MaxY) =
            param.startSigma*pow(param.deltaSigma,blobMaxY.at(j));
        } else {
          features.at(i,MaxY) =
            (param.deltaSigma*blobMaxY.at(j)) + param.startSigma;
        }

        // compute average positions for the half-blobs
        pos1 = double(col)*atan2(blobSinXAcc.at(j),blobCosXAcc.at(j))/
          (2*Pi);
        pos2 = double(col)*atan2(blobSinXAcc.at(k),blobCosXAcc.at(k))/
          (2*Pi);

        if (abs(blobMaxX.at(j)-blobMaxX.at(k)) <= (col/2)) {
          features.at(i,MaxX) = double(blobMaxX.at(k)+blobMaxX.at(j))/2.0;
        } else {
          features.at(i,MaxX) = double(blobMaxX.at(k)+blobMaxX.at(j)+col)/2.0;

          if (features.at(i,MaxX)>=double(col)) {
            features.at(i,MaxX)-=double(col);
          }
        }

        medpos = (pos1+pos2)/2.0;

        // compute width
        features.at(i,Width) = abs(pos1-pos2);

        if (features.at(i,Width) >= (col/2.0)) {
          features.at(i,Width) = col+min(pos1,pos2)-max(pos1,pos2);
          medpos += col/2.0;
        }

        features.at(i,Offset) = medpos - features.at(i,MaxX);
        if (features.at(i,Offset) < (-col/2.0)) {
          features.at(i,Offset) += col;
        } else if (features.at(i,Offset) > (col/2.0)) {
          features.at(i,Offset) -= col;
        }

        // continue
        j=k+1;
        ++i;
      } else {
        ++j;
      }
    }

    return i;
  }

  /*
   * calculate the zero crossings.  The output vector MUST have the
   * same size as the input vector
   */
  int curvatureScaleSpace::zeroCross(const vector<float>& curv,
                                           vector<ubyte>& dest,
                                     const ubyte& posNeg,
                                     const ubyte& negPos) const {

    int i,lastNonZeroIdx;
    float a,lastNonZero(0);
    int zc = 0;
    lastNonZeroIdx = -1;
    static const float threshold = 0;

    for (i=0;i<curv.size();++i) {
      a = curv.at(i);
      if (a != 0.0f) {
        if (lastNonZeroIdx >= 0) {
          if ((lastNonZero * a) < threshold) {
            // zero crossing detected:
            dest.at((i+lastNonZeroIdx)/2) = ((lastNonZero<0) ?
                                             negPos : posNeg);
            zc++;
          }
        }
        lastNonZeroIdx = i;
        lastNonZero = a;
      }
    }

    // TODO:  zero crossing at the borders is not exact!
    a = curv.at(curv.lastIdx());
    if (curv.at(0) == 0) {
      if ((a*curv.at(1))<threshold) {
        dest.at(0) = ((a<0) ? negPos : posNeg);
        zc++;
      }
    }

    if (a == 0) {
      if ((curv.at(curv.lastIdx()-1)*curv.at(0)) < threshold) {
        dest.at(curv.lastIdx()) = ((curv.at(0)>0) ? negPos : posNeg);
        zc++;
      }
    } else {
      if ((a * curv.at(0)) < threshold) {
        dest.at(curv.lastIdx()) = ((a<0) ? negPos : posNeg);
        zc++;
      }
    }

    return zc;
  }


}
