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
 * file .......: ltiColorACASegmentation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 6.1.2004
 * revisions ..: $Id: ltiColorACASegmentation.cpp,v 1.7 2006/09/22 17:01:51 ltilib Exp $
 */

#include "ltiColorACASegmentation.h"
#include "ltiSplitImageToRGB.h"
#include "ltiMergeRGBToImage.h"
#include "ltiKMColorQuantization.h"
#include "ltiL2Distance.h"
#include "ltiGaussianPyramid.h"
#include "ltiFilledUpsampling.h"

#include <limits>

#undef _LTI_DEBUG
// #define _LTI_DEBUG 4
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include <iostream>
#include "ltiViewer.h"
#include "ltiTimer.h"
#include <cstdio>
#endif



namespace lti {
  // --------------------------------------------------
  // colorACASegmentation::parameters
  // --------------------------------------------------

  // default constructor
  colorACASegmentation::parameters::parameters()
    : segmentation::parameters() {
    
    levels = int(4);
    kMeansParam.numberOfColors = 16;
    beta = float(0.5f);
    alpha.clear();
    sigma = float(5.0f);
    convergenceCriterion = float(0.0004f);
    tMin            = float(1.0f);
    nMax            = int(10);
    windowOverlap   = float(0.5f);
    firstWindowSize = float(1.0f);
    lastWindowSize  = int(7);
    windowSizeStep  = float(0.5f);
  }

  // copy constructor
  colorACASegmentation::parameters::parameters(const parameters& other)
    : segmentation::parameters() {
    copy(other);
  }

  // destructor
  colorACASegmentation::parameters::~parameters() {
  }

  // get type name
  const char* colorACASegmentation::parameters::getTypeName() const {
    return "colorACASegmentation::parameters";
  }

  // copy member

  colorACASegmentation::parameters&
    colorACASegmentation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    segmentation::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    segmentation::parameters& (segmentation::parameters::* p_copy)
      (const segmentation::parameters&) =
      segmentation::parameters::copy;
    (this->*p_copy)(other);
# endif

    
    levels = other.levels;
    kMeansParam.copy(other.kMeansParam);
    beta = other.beta;
    alpha.copy(other.alpha);
    sigma = other.sigma;
    convergenceCriterion = other.convergenceCriterion;
    tMin = other.tMin;
    nMax = other.nMax;
    windowOverlap   = other.windowOverlap;
    firstWindowSize = other.firstWindowSize;
    lastWindowSize  = other.lastWindowSize;
    windowSizeStep  = other.windowSizeStep;

    return *this;
  }

  // alias for copy member
  colorACASegmentation::parameters&
  colorACASegmentation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* colorACASegmentation::parameters::clone() const {
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
  bool colorACASegmentation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool colorACASegmentation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"levels",levels);
      lti::write(handler,"kMeansParam",kMeansParam);
      lti::write(handler,"beta",beta);
      lti::write(handler,"alpha",alpha);
      lti::write(handler,"sigma",sigma);
      lti::write(handler,"convergenceCriterion",convergenceCriterion);
      lti::write(handler,"tMin",tMin);
      lti::write(handler,"nMax",nMax);
      lti::write(handler,"windowOverlap",windowOverlap);
      lti::write(handler,"firstWindowSize",firstWindowSize);
      lti::write(handler,"lastWindowSize",lastWindowSize);
      lti::write(handler,"windowSizeStep",windowSizeStep);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::write(handler,false);
# else
    bool (segmentation::parameters::* p_writeMS)(ioHandler&,const bool) const =
      segmentation::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorACASegmentation::parameters::write(ioHandler& handler,
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
   * @return true if read was successful
   */
# ifndef _LTI_MSC_6
  bool colorACASegmentation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool colorACASegmentation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"levels",levels);
      lti::read(handler,"kMeansParam",kMeansParam);
      lti::read(handler,"beta",beta);
      lti::read(handler,"alpha",alpha);
      lti::read(handler,"sigma",sigma);
      lti::read(handler,"convergenceCriterion",convergenceCriterion);
      lti::read(handler,"tMin",tMin);
      lti::read(handler,"nMax",nMax);
      lti::read(handler,"windowOverlap",windowOverlap);
      lti::read(handler,"firstWindowSize",firstWindowSize);
      lti::read(handler,"lastWindowSize",lastWindowSize);
      lti::read(handler,"windowSizeStep",windowSizeStep);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::read(handler,false);
# else
    bool (segmentation::parameters::* p_readMS)(ioHandler&,const bool) =
      segmentation::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool colorACASegmentation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // colorACASegmentation
  // --------------------------------------------------

  // default constructor
  colorACASegmentation::colorACASegmentation()
    : segmentation(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  colorACASegmentation::colorACASegmentation(const parameters& par)
    : segmentation() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  colorACASegmentation::colorACASegmentation(const colorACASegmentation& other)
    : segmentation() {
    copy(other);
  }

  // destructor
  colorACASegmentation::~colorACASegmentation() {
  }

  // returns the name of this type
  const char* colorACASegmentation::getTypeName() const {
    return "colorACASegmentation";
  }

  // copy member
  colorACASegmentation&
  colorACASegmentation::copy(const colorACASegmentation& other) {
    segmentation::copy(other);

    return (*this);
  }

  // alias for copy member
  colorACASegmentation&
  colorACASegmentation::operator=(const colorACASegmentation& other) {
    return (copy(other));
  }


  // clone member
  functor* colorACASegmentation::clone() const {
    return new colorACASegmentation(*this);
  }

  // return parameters
  const colorACASegmentation::parameters&
  colorACASegmentation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool colorACASegmentation::setParameters(const functor::parameters& param) {
    if (segmentation::setParameters(param)) {
      const parameters& par = getParameters();
      alpha.resize(par.kMeansParam.numberOfColors,0.0f,false,true);
      if (!par.alpha.empty()) {
        alpha.fill(par.alpha);
      }
      beta = par.beta;

      return true;
    }

    return false;
  } 

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  
  // On place apply for type image!
  bool colorACASegmentation::apply(image& srcdest) const {
    image tmp;
    if (apply(srcdest,tmp)) {
      tmp.detach(srcdest);
      return true;
    }

    return false;
  };

  // On copy apply for type image!
  bool colorACASegmentation::apply(const image& src,image& dest) const {
    imatrix mask;
    return apply(src,mask,dest);
  };

  bool colorACASegmentation::apply(const image& src,
                                         imatrix& mask,
                                         image& means) const {
    return aca(src,mask,means,true);
  }

  bool colorACASegmentation::apply(const image& src,
                                         imatrix& mask) const {
    image means;
    return aca(src,mask,means,false);
  }



  bool colorACASegmentation::aca(const image& src,
                                 imatrix& mask,
                                 image& means,
                                 const bool computeMeans) const {

    const parameters& par = getParameters();

    filledUpsampling upsampler(2);

    std::vector<image> lmeans(par.kMeansParam.numberOfColors);
    int n=0;
    int cycles=0;
    float gfactor;

    if (par.levels < 2) {
      int windowSize = iround(par.firstWindowSize*min(src.rows(),
                                                      src.columns()));
      int gridSize = max(1,min(windowSize,
                               iround((1-windowSize)*par.windowOverlap +
                                      windowSize)));
      gfactor = -1.0f/(2.0f*par.sigma*par.sigma);
      // just one level desired, i.e. no pyramid computation
      kMeansInit(src,mask);
      do {
        n=0;
        cycles=0;
        do {
          estimateMeans(src,mask,lmeans,windowSize,gridSize,
                        iround(par.tMin*windowSize));
          estimateSegmentation(src,lmeans,gfactor,mask,cycles);
        } while((cycles > 1) && (++n < par.nMax));
        
        windowSize = iround(windowSize*par.windowSizeStep);
        gridSize = max(1,min(windowSize,
                             iround((1-windowSize)*par.windowOverlap +
                                    windowSize)));
      } while (windowSize >= par.lastWindowSize);
      
    } else {
      
      gaussianPyramid<image> pyramid(par.levels);
      pyramid.generate(src);
      int level = pyramid.size()-1;
      int f = 0x01 << level;
      gfactor = -f*f/(2.0f*par.sigma*par.sigma);
      kMeansInit(pyramid.at(level),mask);

      int windowSize = iround(par.firstWindowSize*
                              min(pyramid.at(level).rows(),
                                  pyramid.at(level).columns()));
      int gridSize = max(1,min(windowSize,
                               iround((1-windowSize)*par.windowOverlap +
                                      windowSize)));

      do {
        const image& img = pyramid.at(level);
        
        if (mask.size() != img.size()) {
          // fix upsampled size
          const point old = mask.size();
          mask.resize(img.size(),0,true,false);
          if (old.x < mask.columns()) {
            // copy the last column
            mask.fill(mask,
                      0,mask.lastColumn(),
                      mask.lastRow(),mask.lastColumn(),
                      0,old.x-1);
          }
          if (old.y < mask.rows()) {
            // copy last row
            mask.getRow(mask.lastRow()).fill(mask.getRow(old.y-1));            
          }
        }
        do {
          n=0;
          cycles=0;
          do {
            estimateMeans(img,mask,lmeans,windowSize,gridSize,
                          iround(par.tMin*windowSize));
            estimateSegmentation(img,lmeans,gfactor,mask,cycles);
          } while((cycles > 1) && (++n < par.nMax));
          
          windowSize = iround(windowSize*par.windowSizeStep);
          gridSize = max(1,min(windowSize,
                               iround((1-windowSize)*par.windowOverlap +
                                      windowSize)));
        } while (windowSize >= par.lastWindowSize);

        if (level>0) {
          upsampler.apply(mask);
          windowSize  = par.lastWindowSize*2;
          gridSize = max(1,min(windowSize,
                               iround((1-windowSize)*par.windowOverlap +
                                      windowSize)));
          gfactor /= 4.0f;
        }
        level--;
      } while (level >= 0);
      
    }
    
    if (computeMeans) {
      return createMeanImage(lmeans,mask,means);
    } 

    return true;
  }

  bool colorACASegmentation::createMeanImage(const std::vector<image>& lmeans,
                                             const imatrix mask,
                                             image& means) const {
    means.resize(mask.size(),rgbPixel(),false,false);
    
    int y,x;
    for (y=0;y<means.rows();++y) {
      for (x=0;x<means.columns();++x) {
        means.at(y,x) = lmeans[mask.at(y,x)].at(y,x);
      }
    }

    return true;
  }

  bool colorACASegmentation::kMeansInit(const image& src, 
                                              imatrix& mask) const {
    const parameters& par = getParameters();
    kMColorQuantization kMeans(par.kMeansParam);
    palette pal;
    return kMeans.apply(src,mask,pal);
  }

  bool colorACASegmentation::estimateMeans(const image& src,
                                           const imatrix& mask,
                                           std::vector<image>& means,
                                           const int windowSize,
                                           const int gridSize,
                                           const int tMin) const {

    int cx(0),cy(0); // grid points (centers for the windows)
    const int fw(-windowSize/2); // from (window begin)
    const int tw(fw+windowSize-1); // to   (window end)

    int x,y,i;

    vector<frgbPixel> mu(false,means.size()); // the mean value in a window
    ivector num(false,means.size());  // number of pixels of a label in a wnd

    // initialize all the images
    for (i=0;i<mu.size();++i) {
      means[i].resize(src.size(),rgbPixel(0,0,0,0),false,false);
    }

    // for each grid point
    for (cy=0;cy<src.rows();cy+=gridSize) {
      for (cx=0;cx<src.columns();cx+=gridSize) {

        const int firstX = max(0,cx+fw);
        const int firstY = max(0,cy+fw);

        const int lastX = min(src.lastColumn(),cx+tw);
        const int lastY = min(src.lastRow(),cy+tw);

        // initialize the counters
        mu.fill(frgbPixel(0.0f,0.0f,0.0f));
        num.fill(0);

        // for each point in the window
        for (y=firstY;y<=lastY;++y) {
          for (x=firstX;x<=lastX;++x) {
            i=mask.at(y,x);
            num.at(i)++;
            mu.at(i)+=src.at(y,x);
          }
        }

        for (i=0;i<mu.size();++i) {
          const int n = num.at(i);
          if (n>0) {
            mu.at(i).divide(static_cast<float>(n));
          }

          means[i].at(cy,cx) = mu.at(i).getRGBPixel(); // the dummy entry is
                                                       // set to zero!
          if (n<tMin) {
            means[i].at(cy,cx).setAlpha(255);            
          }
        }
      }
    }
   
    bool doBottomRight = false;

    // check if we need to put some irregular grid points at the right and
    // bottom borders
    if (cx-gridSize != src.lastColumn()) {
      // put extra grid values at the right side
      
      // for each grid point
      for (cy=0;cy<src.rows();cy+=gridSize) {
        cx = src.lastColumn();
        
        const int firstX = max(0,cx+fw);
        const int firstY = max(0,cy+fw);
        
        const int lastX = min(src.lastColumn(),cx+tw);
        const int lastY = min(src.lastRow(),cy+tw);
        
        // initialize the counters
        mu.fill(frgbPixel(0.0f,0.0f,0.0f));
        num.fill(0);
        
        // for each point in the window
        for (y=firstY;y<=lastY;++y) {
          for (x=firstX;x<=lastX;++x) {
            i=mask.at(y,x);
            num.at(i)++;
            mu.at(i)+=src.at(y,x);
          }
        }
        
        for (i=0;i<mu.size();++i) {
          const int n = num.at(i);
          if (n>0) {
            mu.at(i).divide(static_cast<float>(n));            
          }

          means[i].at(cy,cx) = mu.at(i).getRGBPixel(); // the dummy entry is
                                                       // set to zero!
          if (n<tMin) {
            means[i].at(cy,cx).setAlpha(255);            
          }        
        }
      }
      
      doBottomRight = (cy-gridSize != src.lastRow());
    }

    // now check if we need to put elements at the bottom
    if (cy-gridSize != src.lastRow()) {
      cy=src.lastRow();
      for (cx=0;cx<src.columns();cx+=gridSize) {
        
        const int firstX = max(0,cx+fw);
        const int firstY = max(0,cy+fw);
        
        const int lastX = min(src.lastColumn(),cx+tw);
        const int lastY = min(src.lastRow(),cy+tw);

        // initialize the counters
        mu.fill(frgbPixel(0.0f,0.0f,0.0f));
        num.fill(0);
        
        // for each point in the window
        for (y=firstY;y<=lastY;++y) {
          for (x=firstX;x<=lastX;++x) {
            i=mask.at(y,x);
            num.at(i)++;
            mu.at(i)+=src.at(y,x);
          }
        }
        
        for (i=0;i<mu.size();++i) {
          const int n = num.at(i);
          if (n>0) {
            mu.at(i).divide(static_cast<float>(n));
          }

          means[i].at(cy,cx) = mu.at(i).getRGBPixel(); // the dummy entry is
                                                       // set to zero!
          if (n<tMin) {
            means[i].at(cy,cx).setAlpha(255);
          }
        }
      }

      doBottomRight = doBottomRight || (cx-gridSize != src.lastColumn());
    }

    // the bottom-right corner
    if (doBottomRight) {
      cy=src.lastRow();
      cx=src.lastColumn();

      const int firstX = max(0,cx+fw);
      const int firstY = max(0,cy+fw);

      const int lastX = min(src.lastColumn(),cx+tw);
      const int lastY = min(src.lastRow(),cy+tw);

      // initialize the counters
      mu.fill(frgbPixel(0.0f,0.0f,0.0f));
      num.fill(0);

      // for each point in the window
      for (y=firstY;y<=lastY;++y) {
        for (x=firstX;x<=lastX;++x) {
          i=mask.at(y,x);
          num.at(i)++;
          mu.at(i)+=src.at(y,x);
        }
      }

      for (i=0;i<mu.size();++i) {
        const int n = num.at(i);
        if (n>0) {
          mu.at(i).divide(static_cast<float>(n));
        }

        means[i].at(cy,cx) = mu.at(i).getRGBPixel(); // the dummy entry is
                                                     // set to zero!
        if (n<tMin) {
          means[i].at(cy,cx).setAlpha(255);
        }
      }
    }

    // at this point we have all label images full with mean values for
    // some support points located at the grid entries.

    // now we need to fill the rest of the images using bilinear interpolation

    for (i=0;i<mu.size();++i) {
      interpolate(means[i],gridSize);
    }

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
    static viewer viewm("Mask1",2);
    viewm.show(mask);
    _lti_debug("Window size: " << windowSize <<
               "\t Grid size: " << gridSize << std::endl);

    static std::vector<viewer> views(mu.size());
    for (i=0;i<mu.size();++i) {
      passiveWait(100000);
      views.at(i).show(means[i]);
    }
#endif

    return true;
  }

  bool colorACASegmentation::interpolate(image& src,
                                         const int gridSize) const {

    int cx,cy;
    int nx(0),ny(0);
    int x,y;

    // first the vertical lines
    const int lastY = src.lastRow();
    frgbPixel p1,p2,p,d;
    rgbPixel px;
    int g = 0;

    for (cx=0;cx<src.lastColumn();cx+=gridSize) {
      cy=0;
      p1 = src.at(cy,cx);
      while(cy<lastY) {
        ny=min(src.lastRow(),cy+gridSize);
        g = ny-cy;
        p2 = src.at(ny,cx);

        if ((src.at(cy,cx).getDummy() != 0) ||
            (src.at(ny,cx).getDummy() != 0)) {
          const bool atcy = (src.at(cy,cx).getDummy() == 0);
          const bool atny = (src.at(ny,cx).getDummy() == 0);

          if (atcy) {
            px = src.at(cy,cx);
            px.setAlpha(255);
          } else if (atny) {
            px = src.at(ny,cx);
            px.setAlpha(255);
          } else {
            px.set(0,0,0,255);
          }

          for (y=cy+1;y<ny;++y) {
            src.at(y,cx)=px;
          }

        } else {
          d.subtract(p1,p2);

          for (y=cy+1;y<ny;++y) {
            p = p1 - (d*static_cast<float>(y-cy))/static_cast<float>(g);
            src.at(y,cx)=p.getRGBPixel();
          }
        }
        cy=ny;
        p1=p2;
      }
    }

    // the last column
    cx=src.lastColumn();
    cy=0;
    p1 = src.at(cy,cx);
    while(cy<lastY) {
      ny=min(src.lastRow(),cy+gridSize);
      g = ny-cy;
      p2 = src.at(ny,cx);

      if ((src.at(cy,cx).getDummy() != 0) ||
          (src.at(ny,cx).getDummy() != 0)) {
        const bool atcy = (src.at(cy,cx).getDummy() == 0);
        const bool atny = (src.at(ny,cx).getDummy() == 0);

        if (atcy) {
          px = src.at(cy,cx);
          px.setAlpha(255);
        } else if (atny) {
          px = src.at(ny,cx);
          px.setAlpha(255);
        } else {
          px.set(0,0,0,255);
        }

        for (y=cy+1;y<ny;++y) {
          src.at(y,cx)=px;
        }
      } else {
        d.subtract(p1,p2);

        for (y=cy+1;y<ny;++y) {
          p = p1 - (d*static_cast<float>(y-cy))/static_cast<float>(g);
          src.at(y,cx)=p.getRGBPixel();
        }
      }
      cy=ny;
      p1=p2;
    }

    // Interpolate between the columns
    for (y=0;y<src.rows();++y) {
      vector<rgbPixel>& line = src.getRow(y);
      cx=0;
      p1 = line.at(cx);

      while(cx<src.lastColumn()) {
        nx=min(src.lastColumn(),cx+gridSize);
        g = nx-cx;
        p2 = line.at(nx);

        if ((line.at(cx).getDummy() != 0) ||
            (line.at(nx).getDummy() != 0)) {
          const bool atcx = (line.at(cx).getDummy() == 0);
          const bool atnx = (line.at(nx).getDummy() == 0);

          if (atcx) {
            px = line.at(cx);
            px.setAlpha(255);
          } else if (atnx) {
            px = line.at(nx);
            px.setAlpha(255);
          } else {
            px.set(0,0,0,255);
          }

          for (x=cx+1;x<nx;++x) {
            line.at(x) = px;
          }
        } else {
          d.subtract(p1,p2);

          for (x=cx+1;x<nx;++x) {
            p = p1 - (d*static_cast<float>(x-cx))/static_cast<float>(g);
            line.at(x)=p.getRGBPixel();
          }
        }

        p1=p2;
        cx=nx;
      }
    }

    return true;
  }

  bool
  colorACASegmentation::estimateSegmentation(const image& src,
                                             const std::vector<image>& means,
                                             const float gfactor,
                                             imatrix& mask,
                                             int& cycles) const {

    const parameters& par = getParameters();
    imatrix nmask;
    int numChanges;
    const int allowedChanges = iround(par.convergenceCriterion*
                                      src.rows()*src.columns());

    _lti_debug("Allowed changes:" << allowedChanges << std::endl);

    cycles = 0;
    do {
      estimateSegmentationStep(src,means,gfactor,mask,nmask,numChanges);
      _lti_debug("  Number of changes: " << numChanges << std::endl);
      nmask.swap(mask);
    } while((numChanges > allowedChanges) && (++cycles < 10));

#if defined(_LTI_DEBUG) && (_LTI_DEBUG > 2)
    _lti_debug("Number of cycles for convergence: " << cycles << std::endl);
    static viewer view("New mask",2);
    view.show(mask);

    _lti_debug("Press ENTER to continue.");
    getchar();
#endif
    return true;
  }

  /*
   * Compute label
   */
  inline int
  colorACASegmentation::computeLabel(const image& src,
                                     const std::vector<image>& means,
                                     const imatrix& mask,
                                     const point& p,
                                     const int from,
                                     const int to,
                                     const float gfactor,
                                     int& numChanges) const {

    static const int deltax[] = {1, 1, 0,-1,-1,-1,0,1, 1, 1, 0,-1};
    static const int deltay[] = {0,-1,-1,-1, 0, 1,1,1, 0,-1,-1,-1};

    float winner = -std::numeric_limits<float>::max();
    int iwinner = -1;
    float pr;
    int i;
    int j;
    float v;
    // MRF
    // for each label
    for (i=0;i<static_cast<int>(means.size());++i) {
      const rgbPixel& px = means[i].at(p);
      if (px.getDummy() == 0) {
        pr=gfactor*distanceSqr(src.at(p),px);
        v = alpha.at(i);
        for (j=from;j<=to;++j) {
          if (i == mask.at(p.y+deltay[j],p.x+deltax[j])) {
            v -= beta;
          } else {
            v += beta;
          }
        }
        pr -= v;
        if (pr>winner) {
          winner=pr;
          iwinner=i;
        }
      }
    }

    if (iwinner < 0) {
      // hmm, noone was detected as winner!
      // just return the previous label as candidate
      return mask.at(p);
    }

    if (iwinner != mask.at(p)) {
      // another label has won! notice the change
      numChanges++;
    }

    return iwinner;
  }


  /*
   * Estimate segmentation given the means
   */
  bool colorACASegmentation
  ::estimateSegmentationStep(const image& src,
                             const std::vector<image>& means,
                             const float gfactor,
                             imatrix& mask,
                             imatrix& nmask,
                             int& numChanges) const {
    numChanges = 0;

    nmask.resize(mask.size(),int(),false,false); // new mask
    point p(0,0);
    // top left pixel
    nmask.at(p) = computeLabel(src,means,mask,p,6,8,gfactor,numChanges);

    // top
    for (p.x=1;p.x<src.lastColumn();++p.x) {
      nmask.at(p) = computeLabel(src,means,mask,p,4,8,gfactor,numChanges);
    }

    // top right
    nmask.at(p) = computeLabel(src,means,mask,p,4,6,gfactor,numChanges);

    for (p.y=1;p.y<src.lastRow();++p.y) {
      // left
      p.x=0;
      nmask.at(p) = computeLabel(src,means,mask,p,6,10,gfactor,numChanges);

      // main block
      for (p.x=1;p.x<src.lastColumn();++p.x) {
        nmask.at(p) = computeLabel(src,means,mask,p,0,7,gfactor,numChanges);
      }

      // right
      nmask.at(p) = computeLabel(src,means,mask,p,2,6,gfactor,numChanges);
    }

    p.x=0;
    // bottom left pixel
    nmask.at(p) = computeLabel(src,means,mask,p,0,2,gfactor,numChanges);

    // bottom
    for (p.x=1;p.x<src.lastColumn();++p.x) {
      nmask.at(p) = computeLabel(src,means,mask,p,0,4,gfactor,numChanges);
    }

    // bottom right
    nmask.at(p) = computeLabel(src,means,mask,p,2,4,gfactor,numChanges);

    return true;
  }
}
