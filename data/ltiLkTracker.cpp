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
 * file .......: ltiLkTracker.cpp
 * authors ....: Frederik Lange
 * organization: LTI, RWTH Aachen
 * creation ...: 26.4.2001
 * revisions ..: $Id: ltiLkTracker.cpp,v 1.13 2006/09/05 17:02:34 ltilib Exp $
 */


#include "ltiLkTracker.h"
#include "ltiPointList.h"

#include "ltiCornerDetectorFactory.h"

#include "ltiGradientKernels.h"
#include "ltiConvolution.h"

#include "ltiDownsampling.h"
#include "ltiGaussKernels.h"

#include "ltiClassName.h"

//#define _LTI_DEBUG

#ifdef _LTI_DEBUG
#include <iostream>
#include <cstdio>

using std::cout;
using std::endl;
#endif // _LTI_DEBUG

#ifdef _LTI_MSC_VER
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif

namespace lti {
  // static attributes
  const tpoint<float> lkTracker::outOfRange = tpoint<float>(-50.0f,-50.0f);

  // --------------------------------------------------
  // lkTracker::parameters
  // --------------------------------------------------

  // default constructor
  lkTracker::parameters::parameters()
    : modifier::parameters() {

    windowSize = int(4);
    numLevels = int(5);

    autoCorners = bool(true);

    cornerDetectorFactory cdf;
#if defined(HAVE_SUSAN) && (HAVE_SUSAN != 0)
    cornerFinder = cdf.newInstance("lti::susanCorners");
#else
    cornerFinder = cdf.newInstance("lti::harrisCorners");
#endif

    maxStepsPerLevel = 10;
    eta2Threshold   = 0.0009f;
    errorThreshold = 0.99f;
  }

  // copy constructor
  lkTracker::parameters::parameters(const parameters& other)
    : modifier::parameters(), cornerFinder(0)  {
    copy(other);
  }

  // destructor
  lkTracker::parameters::~parameters() {
    delete cornerFinder;
    cornerFinder=0;
  }

  // get type name
  const char* lkTracker::parameters::getTypeName() const {
    return "lkTracker::parameters";
  }

  // copy member

  lkTracker::parameters&
    lkTracker::parameters::copy(const parameters& other) {
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

    windowSize = other.windowSize;
    numLevels = other.numLevels;

    autoCorners = other.autoCorners;
    delete cornerFinder;
    cornerFinder = dynamic_cast<cornerDetector*>(other.cornerFinder->clone());

    maxStepsPerLevel = other.maxStepsPerLevel;
    eta2Threshold   = other.eta2Threshold;
    errorThreshold = other.errorThreshold;

    return *this;
  }

  // alias for copy member
  lkTracker::parameters&
    lkTracker::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* lkTracker::parameters::clone() const {
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
  bool lkTracker::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool lkTracker::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"windowSize",windowSize);
      lti::write(handler,"numLevels",numLevels);
      lti::write(handler,"maxStepsPerLevel",maxStepsPerLevel);
      lti::write(handler,"eta2Threshold",eta2Threshold  );
      lti::write(handler,"errorThreshold",errorThreshold);
      if (autoCorners && notNull(cornerFinder)) {
        className namer;
        lti::write(handler,"autoCorners",autoCorners);
        lti::write(handler,"cornerFinder",namer.get(*cornerFinder));
        lti::write(handler,"cornerFinderParameters",*cornerFinder);
      } else {
        lti::write(handler,"autoCorners",false);
      }
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
  bool lkTracker::parameters::write(ioHandler& handler,
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
  bool lkTracker::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool lkTracker::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"windowSize",windowSize);
      lti::read(handler,"numLevels",numLevels);
      lti::read(handler,"maxStepsPerLevel",maxStepsPerLevel);
      lti::read(handler,"eta2Threshold",eta2Threshold  );
      lti::read(handler,"errorThreshold",errorThreshold);

      delete cornerFinder;
      cornerFinder = 0;

      lti::read(handler,"autoCorners",autoCorners);
      if (autoCorners) {
        std::string str;
        lti::read(handler,"cornerFinder",str);
        cornerDetectorFactory cdf;
        cornerFinder = cdf.newInstance(str);
        if (notNull(cornerFinder)) {
          lti::read(handler,"cornerFinderParameters",*cornerFinder);
        }
      }

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
  bool lkTracker::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  bool lkTracker::parameters::setCornerDetector(const cornerDetector& cdet) {
    delete cornerFinder;
    functor* tmp = cdet.clone();
    cornerFinder = dynamic_cast<cornerDetector*>(tmp);
    return (notNull(cornerFinder));
  }

  /*
   * get a reference to the corner detector functor being used
   */
  cornerDetector& lkTracker::parameters::getCornerDetector() {
    return *cornerFinder;
  }

  /*
   * get a read only reference to the corner detector object being used
   */
  const cornerDetector& lkTracker::parameters::getCornerDetector() const {
    return *cornerFinder;
  }


  // --------------------------------------------------
  // lkTracker
  // --------------------------------------------------

  // default constructor
  lkTracker::lkTracker()
    : modifier() {
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    pyramidA = new gaussianPyramid<channel>;
    pyramidB = new gaussianPyramid<channel>;
    initialised = false;

  }

  // copy constructor
  lkTracker::lkTracker(const lkTracker& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  lkTracker::~lkTracker() {
    delete pyramidA;
    delete pyramidB;
  }

  // returns the name of this type
  const char* lkTracker::getTypeName() const {
    return "lkTracker";
  }

  // copy member
  lkTracker&
    lkTracker::copy(const lkTracker& other) {
      modifier::copy(other);
    return (*this);
  }

  // clone member
  functor* lkTracker::clone() const {
    return new lkTracker(*this);
  }

  // return parameters
  const lkTracker::parameters&
    lkTracker::getParameters() const {
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

  inline float lkTracker::getBilinear(const float x, const float y,
                                      const channel& map) {

    const int truncY = static_cast<int>(y);
    const float fractY = y - static_cast<float>(truncY);
    const int truncX = static_cast<int>(x);
    const float fractX = x - static_cast<float>(truncX);
    const float cFractX = 1.0f - fractX;
    const float cFractY = 1.0f - fractY;

    // the interpolation:
    return ( (cFractX*cFractY*map[truncY][truncX]) +
             ( fractX*cFractY*map[truncY][truncX + 1]) +
             (cFractX* fractY*map[truncY + 1][truncX]) +
             ( fractX* fractY*map[truncY + 1][truncX + 1]) );
  }


  bool lkTracker::apply(const channel& src, pointList& featurePoints) {
    tpointList<float> tmp;
    int validPts;
    if (!initialised) {
      // the featurePoints can be required only the first time...
      tmp.castFrom(featurePoints);
    }
    apply(src,tmp,validPts);
    featurePoints.castFrom(tmp);
    return true;
  }

  bool lkTracker::apply(const channel& src, pointList& featurePoints,
                        int& validPoints) {
    tpointList<float> tmp;
    if (!initialised) {
      // the featurePoints can be required only the first time...
      tmp.castFrom(featurePoints);
    }
    apply(src,tmp,validPoints);
    featurePoints.castFrom(tmp);
    return true;
  }

  /*
   * Resets the internal state but not the parameters
   */
  bool lkTracker::reset() {
    initialised = false;
    return true;
  }

  /*
   * Resets some points only
   */
  bool lkTracker::reset(const tpointList<float>& whichPoints) {

    tpointList<float>::const_iterator it,eit;
    std::list<vec2d>::iterator fvit;

    for (it = whichPoints.begin(),eit = whichPoints.end(),
           fvit = flowVectors.begin();
         (it != whichPoints.end()) && (fvit != flowVectors.end());
         ++it,++fvit) {

      // only consider the valid points
      if (((*it).x >= 0) && ((*it).y >= 0)) {
        (*fvit).p = (*it); // revive the point!
      }
    }

    return true;
  }

  /*
   * Resets some points only
   */
  bool lkTracker::reset(const pointList& whichPoints) {

    pointList::const_iterator it,eit;
    std::list<vec2d>::iterator fvit;

    for (it = whichPoints.begin(),eit = whichPoints.end(),
           fvit = flowVectors.begin();
         (it != whichPoints.end()) && (fvit != flowVectors.end());
         ++it,++fvit) {

      // only consider the valid points
      if (((*it).x >= 0) && ((*it).y >= 0)) {
        (*fvit).p.castFrom(*it); // revive the point!
      }
    }
    return true;

  }

  // //////////////////////////////////////////////////////////////
  bool lkTracker::apply(const channel& src,
                        tpointList<float>& featurePoints) {
    int tmp;
    return apply(src,featurePoints,tmp);
  }

  // //////////////////////////////////////////////////////////////
  bool lkTracker::apply(const channel& src,
                        tpointList<float>& featurePoints,
                        int& validPoints) {
    const parameters& par = getParameters();

    // ----------------------------------
    // Initialisation
    // ----------------------------------
    if (!initialised) {
      flowVectors.clear();   // make the container empty
      //  actualPoints.clear();

      if (par.autoCorners) {

        pointList initialPoints;

        findInitialPoints(src,initialPoints);  // determine the corners

        pointList::iterator it;
        vec2d tmpVec;

        featurePoints.clear(); // discard given values!

        // convert to float and put into vector list
        for (it = initialPoints.begin();
             it != initialPoints.end();
             it++) {

          tmpVec.p.castFrom(*it);

          flowVectors.push_back(tmpVec);
          featurePoints.push_back(tmpVec.p);
        }
      }
      else {
        vec2d tmpVec;
        tpointList<float>::iterator it;

        for (it = featurePoints.begin();
             it != featurePoints.end();
             it++) {

          // only consider the point in the image
          if (((*it).x >= 0) && ((*it).x < src.columns()) &&
              ((*it).y >= 0) && ((*it).y < src.rows())) {
            tmpVec.p = (*it);
          } else {
            tmpVec.p = outOfRange;
          }
          flowVectors.push_back(tmpVec);
        }
      }

      buildPyramid(src);      // build the pyramid of the first image

      initialised = true;
      return true;            // ok we are ready to perform the next steps
    }

    // ----------------------------------
    // Tracker
    // ----------------------------------

    // these are candidates to be in the parameters
    const int maxStepsPerLevel = par.maxStepsPerLevel;
    const float eta2Threshold   = par.eta2Threshold;
    const float errorThreshold = par.errorThreshold;

    float gxx,gyy,gxy; // the spatial gradient matrix
    tpoint<float> b;   // image mismatch vector

    float valX, valY, valA, valB; // values for gradient components and
                                  // images A and B

    trectangle<float> absBorders;  // gives absolute position of
                                   // integration window in the image

    float x,y;
    float delta;

    // MSVC++ doesn't like if this channel arrays are created in the stack,
    // so we build them in the heap...
    channel* gradX = new channel[par.numLevels];
    channel* gradY = new channel[par.numLevels];

    int j, k;

    // build the pyramid for the actual image -> pyramidB, old is then pyramidA
    buildPyramid(src);

    // generate the image gradients in x and y for each level
    buildGradientMaps(*pyramidB, gradX, gradY);

    std::list<vec2d>::iterator it;
    tpointList<float>::iterator oit; // output vector iterator

    tpoint<float> pa, pb;
    tpoint<float> r,d;
    tpoint<float> fl;

    // window size
    const float ws = float(par.windowSize);

    // indicates if the tracking was successfull
    bool trackedToGround;

    // the size of the feature points must be equal to the size
    // of the internal flowVectors!
    if ((x = (float)featurePoints.size()) != (y = (float)flowVectors.size())) {
      if (x > y) {
        // too many points:  delete some of them
        for (;y<x;++y) {
          featurePoints.pop_back();
        }
      } else {
        // more points are required!
        pa.x = 0; pa.y = 0;
        for (;x<y;++x) {
          featurePoints.push_back(pa);
        }
      }
    }

    // main loop over all points being tracked
    for (it = flowVectors.begin(),oit = featurePoints.begin(),validPoints = 0;
         it != flowVectors.end(); ++it,++oit) {
      // flow vector being analysed:
      vec2d& fv = (*it);

      trackedToGround = true;

      // outside of the image
      if ( fv.p.x < 0.0) {
        trackedToGround = false;
        (*oit) = outOfRange;
        continue;
      }

      // the pyramidal tracking algorithm:

      fv.d.x = 0.0f;
      fv.d.y = 0.0f;

#     ifdef _LTI_DEBUG
      cout << "Tracking point " << fv.p << ":" << endl;
#     endif

      for (j = par.numLevels-1; j >= 0; j--) { // go down the pyramid

        fl.x = 0.0f;
        fl.y = 0.0f;

        // faster than pow(2,j):
        const float twoPowj(static_cast<float>(0x00000001 << j));

        // original point:
        pa = fv.p / twoPowj;

        r.x = float(gradX[j].lastColumn());
        r.y = float(gradX[j].lastRow());

#       ifdef _LTI_DEBUG
        cout << "  Level " << j
             << " (" << r.x+1 << "," << r.y+1 << ")" << endl;
#       endif

        for (k = 0; k < maxStepsPerLevel; k++) {  // maximum steps per level

          // estimated point on actual image
          // ((it->d) corresponds to the guess term (g) in the paper
          // fl corresponds to the actual mismatch nu
          pb = ((fv.p + fv.d) / twoPowj) + fl;

#         ifdef _LTI_DEBUG
          cout << "    Iteration " << k
               << " \tTracking point " << pa
               << " \tActual guess " << pb << endl;
#         endif

          // if outside the image, feature is not trackable
          if ((pb.x >= r.x)  ||
              (pb.y >= r.y)  ||
              (pb.x <= 0.0f) ||
              (pb.y <= 0.0f)) {
            trackedToGround = false;
            (*oit) = outOfRange;
            break;
          }

          d = pb - pa;

          // determine the borders
          delta = (float)ceil(max(0.0f,-pa.x+ws,-pb.x+ws));
          absBorders.ul.x = pb.x - ws + delta;

          delta = (float)ceil(max(0.0f,-pa.y+ws,-pb.y+ws));
          absBorders.ul.y = pb.y - ws + delta;

          delta = (float)ceil(max(0.0f,
                                  pa.x+ws-r.x+1.0f,
                                  pb.x+ws-r.x+1.0f));
          absBorders.br.x = pb.x + ws - delta;

          delta = (float)ceil(max(0.0f,
                                  pa.y+ws-r.y+1.0f,
                                  pb.y+ws-r.y+1.0f));
          absBorders.br.y = pb.y + ws - delta;

          if ((absBorders.ul.x >= absBorders.br.x) ||
              (absBorders.ul.y >= absBorders.br.y)) {
            trackedToGround = false;
            (*oit) = outOfRange;
            break;
          }

          // initialise spatial gradient matrix and image
          gxx = 0.0f;
          gxy = 0.0f;
          gyy = 0.0f;

          // mismatch vector to zero
          b.x = 0;
          b.y = 0;

          // generate the matrix and the vector with bilinear interpolation
          for (y = absBorders.ul.y;  y <= absBorders.br.y; y+=1.0f) {
            for (x = absBorders.ul.x; x <= absBorders.br.x; x+=1.0f) {

              valX = getBilinear(x,y,gradX[j]);
              valY = getBilinear(x,y,gradY[j]);

              gxx += valX * valX;
              gxy += valX * valY;
              gyy += valY * valY;

              valA = getBilinear(x-d.x, y-d.y, pyramidA->at(j));
              valB = getBilinear(x,y, pyramidB->at(j));

              delta = (valA - valB);

              b.x +=  delta * valX;
              b.y +=  delta * valY;
            }
          }

          if (!trackedToGround) {
            fv.p = outOfRange;
            fv.d.x = 0.0;
            fv.d.y = 0.0;
            fl.x = 0;
            fl.y = 0;

            (*oit) = fv.p;
            break;
          } else {
            float det1;
            tpoint<float> eta;

            // inverse of determinant
            det1 = 1.0f/(gxx*gyy - gxy*gxy);

            if (abs(det1) < 1.0E+30f) {
              // the inverse of g[][] is
              // g1[0][0] =  g[1][1] * det1;
              // g1[0][1] = -g[1][0] * det1;
              // g1[1][0] = -g[0][1] * det1;
              // g1[1][1] =  g[0][0] * det1;

              eta.x = (gyy*b.x - gxy*b.y)*det1;
              eta.y = (gxx*b.y - gxy*b.x)*det1;

              fl.x += eta.x;
              fl.y += eta.y;

#             ifdef _LTI_DEBUG
              cout << "    Eta: " << eta
                   << "  Fehler nu:  " << fl << endl;
#             endif

              if ( (eta.x*eta.x + eta.y*eta.y) < eta2Threshold ) {
                break;
              }
            }
            else {
              // determinant is to small!
              fl.x = 0.0;
              fl.y = 0.0;
              fv.p = outOfRange;

              (*oit) = fv.p;

              break;
            }
          }
        }  // iterations

        if (j == 0) {
          fv.d.x = (fv.d.x + fl.x);
          fv.d.y = (fv.d.y + fl.y);
        } else {
          fv.d.x = 2.0f * (fv.d.x + fl.x);
          fv.d.y = 2.0f * (fv.d.y + fl.y);
        }
      }  // levels

      float error;

      error = 0.0;
      // calculate the absolute quadratic errors
      for (y = absBorders.ul.y; y <= absBorders.br.y; y+=1.0) {
        for (x = absBorders.ul.x;x <= absBorders.br.x; x+=1.0){
          delta = getBilinear(x-d.x, y-d.y, pyramidA->at(0)) -
                  getBilinear(x, y, pyramidB->at(0));

          error += delta*delta;
        }
      }

      fv.error =(float)sqrt(error);

      if (error < errorThreshold) {
        b.x = round(fv.p.x + fv.d.x);
        b.y = round(fv.p.y + fv.d.y);
        validPoints++;
      }
      else {
        b = outOfRange;
      }

      // set the result for the actual point
      (*oit) = b;

      // reset the vectors for next iteration
      fv.p.x = b.x;
      fv.p.y = b.y;

    }  // each point

    delete[] gradX;
    delete[] gradY;

    return true;
  }

  void lkTracker::findInitialPoints(const channel& img,
                                    pointList& initialPoints) {

    const parameters& par = getParameters();
    initialPoints.clear();

    if (notNull(&par.getCornerDetector())) {
      par.getCornerDetector().apply(img, initialPoints);
    }
  }

  void lkTracker::buildPyramid(const channel& img) {
    const parameters& par = getParameters();

    gaussianPyramid<channel> *tmpPyramid;


    tmpPyramid = pyramidA;

    pyramidA = pyramidB;
    pyramidB = tmpPyramid;

    // convolution size , variance
    pyramidB->setKernelParameters(3);
    pyramidB->generate(img, par.numLevels);
  }


  void lkTracker::buildGradientMaps(const gaussianPyramid<channel>& src,
                                    channel*& gradX,
                                    channel*& gradY) {
    int i;
    convolution convolver;
    convolution::parameters convolverParameters;
    convolverParameters.boundaryType = lti::Constant;

    gradientKernelY<float> yKernel(3);
    gradientKernelX<float> xKernel(3);

    // create and overgive the parameters
    convolverParameters.setKernel(yKernel);
    convolver.setParameters(convolverParameters);

    // perform the gradient generation
    for (i=0;i<src.size();++i) {
      convolver.apply(src.at(i), gradY[i]);
    }

    // create and overgive the parameters
    convolverParameters.setKernel(xKernel);
    convolver.setParameters(convolverParameters);

    // perform the gradient generation
    for (i=0;i<src.size();++i) {
      convolver.apply(src.at(i), gradX[i]);
    }
  }

}
