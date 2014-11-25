/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiViewerFunctor.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 31.12.2002
 * revisions ..: $Id: ltiViewerFunctor.cpp,v 1.11 2006/02/08 12:58:41 ltilib Exp $
 */

#include "ltiViewerFunctor.h"
#include "ltiMergeHSIToImage.h"
#include "ltiSplitImageToRGB.h"

#include "ltiSerialStatsExtFunctor.h"

#include "ltiLabelAdjacencyMap.h"
#include "ltiFilledUpsampling.h"
#include "ltiDecimation.h"

#include <limits>

namespace lti {

  // --------------------------------------------------
  // viewerFunctor
  // --------------------------------------------------

  // static members

  palette viewerFunctor::grayPalette;
  palette viewerFunctor::redPalette;
  palette viewerFunctor::greenPalette;
  palette viewerFunctor::bluePalette;
  palette viewerFunctor::huePalette;

  void viewerFunctor::computePalettes() {
    if (grayPalette.size() != 256) {
      grayPalette.resize(256,0,false,false);
      redPalette.resize(256,0,false,false);
      greenPalette.resize(256,0,false,false);
      bluePalette.resize(256,0,false,false);
      huePalette.resize(256,0,false,false);

      mergeHSIToImage hsiSpace;
      int i;
      for (i=0;i<256;++i) {
        grayPalette.at(i)=rgbPixel(i,i,i);
        redPalette.at(i)=rgbPixel(i,0,0);
        greenPalette.at(i)=rgbPixel(0,i,0);
        bluePalette.at(i)=rgbPixel(0,0,i);
        hsiSpace.apply(static_cast<ubyte>(i),128,192,huePalette.at(i));
      }

    }
  }

  // default constructor
  viewerFunctor::viewerFunctor()
    : transform(){

    computePalettes();

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  viewerFunctor::viewerFunctor(const parameters& par)
    : transform() {

    computePalettes();

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  viewerFunctor::viewerFunctor(const viewerFunctor& other)
    : transform() {
    copy(other);
  }

  // destructor
  viewerFunctor::~viewerFunctor() {
  }

  // returns the name of this type
  const char* viewerFunctor::getTypeName() const {
    return "viewerFunctor";
  }

  // copy member
  viewerFunctor&
    viewerFunctor::copy(const viewerFunctor& other) {
      transform::copy(other);

    return (*this);
  }

  // alias for copy member
  viewerFunctor&
    viewerFunctor::operator=(const viewerFunctor& other) {
    return (copy(other));
  }


  // clone member
  functor* viewerFunctor::clone() const {
    return new viewerFunctor(*this);
  }

  // return parameters
  const viewerFunctor::parameters& viewerFunctor::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // return parameters
  viewerFunctor::parameters&  viewerFunctor::getParameters() {
    parameters* par =
      dynamic_cast<parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const vector<double>& src,image& dest,
                            const bool doStat)  {
    dest.clear();

    if (src.empty()) {
      return true;
    }

    return scalarToImage(src,dest,doStat);
  };

  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const histogram1D& src,image& dest,
                            const bool doStat)  {
    dest.clear();
    const int size = src.getLastCell() - src.getFirstCell() + 1;

    if (size == 0) {
      return true;
    }

    dvector tmp;
    tmp.useExternData(size,const_cast<double*>(&src.at(src.getFirstCell())));

    return scalarToImage(tmp,dest,doStat);
  };

  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const histogram2D& src,image& dest,
                            const bool doStat)  {
    dest.clear();
    const point size = (src.getLastCell() - src.getFirstCell() + point(1,1));

    if (size.x*size.y == 0) {
      return true;
    }

    dmatrix tmp;
    tmp.useExternData(size.y,size.x,
                      const_cast<double*>(&src.at(src.getFirstCell())));

    return scalarToImage(tmp,dest,doStat);
  };


  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const vector<float>& src,image& dest,
                            const bool doStat)  {
    dest.clear();

    if (src.empty()) {
      return true;
    }

    dvector tmp;
    tmp.castFrom(src);

    return scalarToImage(tmp,dest,doStat);
  };

  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const vector<int>& src,image& dest,
                            const bool doStat)  {
    dest.clear();

    if (src.empty()) {
      return true;
    }

    dvector tmp;
    tmp.castFrom(src);

    return scalarToImage(tmp,dest,doStat);
  };

  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const vector<ubyte>& src,image& dest,
                            const bool doStat)  {
    dest.clear();

    if (src.empty()) {
      return true;
    }

    dvector tmp;
    tmp.castFrom(src);

    return scalarToImage(tmp,dest,doStat);
  };

  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const matrix<ubyte>& src,image& dest,
                            const bool doStat)  {
    dest.clear();

    if (src.empty()) {
      return true;
    }

    const parameters& par = getParameters();

    if (par.labelAdjacencyMap) {
      labelsToImage(src,dest,doStat);
    } else {
      scalarToImage(src,dest,doStat);
    }

    return zoom(dest);

  };

  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const matrix<int>& src,image& dest,
                            const bool doStat)  {
    dest.clear();

    if (src.empty()) {
      return true;
    }

    const parameters& par = getParameters();

    if (par.labelAdjacencyMap) {
      if (!labelsToImage(src,dest,doStat)) {
        // some error occured! maybe negative numbers in labeled mask.
        scalarToImage(src,dest,doStat);
      };
    } else {
      scalarToImage(src,dest,doStat);
    }

    return zoom(dest);

  };

  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const matrix<float>& src,image& dest,
                            const bool doStat)  {
    dest.clear();

    if (src.empty()) {
      return true;
    }

    scalarToImage(src,dest,doStat);
    return zoom(dest);
  };


  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const matrix<double>& src,image& dest,
                            const bool doStat)  {
    dest.clear();

    if (src.empty()) {
      return true;
    }

    scalarToImage(src,dest,doStat);
    return zoom(dest);
  };

  // On copy apply for type matrix<ubyte>!
  bool viewerFunctor::apply(const image& src,image& dest,
                            const bool doStat)  {
    dest.clear();

    if (src.empty()) {
      return true;
    }

    imageToImage(src,dest,doStat);
    return zoom(dest);
  };


  /*
   * Zoom
   */
  bool viewerFunctor::zoom(image& img) {
    const parameters& par = getParameters();

    if (par.zoomFactor > 0) {
      int s = iround(pow(2.0,par.zoomFactor));
      filledUpsampling up(s);
      return up.apply(img);
    } else if (par.zoomFactor<0) {
      int s = iround(pow(2.0,-par.zoomFactor));
      decimation down(point(s,s));
      return down.apply(img);
    }

    return false;
  }

  /**
   * The input channel is a scalar matrix
   */
  bool viewerFunctor::scalarToImage(const matrix<ubyte>& chnl,image& img,
                                    const bool doStat) {
    // this draws on theImage the in theData given matrix<int> or matrix<ubyte>
    const viewerFunctor::parameters& par = getParameters();

    // select the palette to be used
    palette* pal;
    palette userPalette;

    switch(par.whichPalette) {
      case viewerFunctor::parameters::Gray:
        pal = &grayPalette;
        break;
      case viewerFunctor::parameters::Hue:
        pal = &huePalette;
        break;
      case viewerFunctor::parameters::Red:
        pal = &redPalette;
        break;
      case viewerFunctor::parameters::Green:
        pal = &greenPalette;
        break;
      case viewerFunctor::parameters::Blue:
        pal = &bluePalette;
        break;
      case viewerFunctor::parameters::UserDefined: {
        userPalette.resize(256,rgbPixel(),false,false);
        for (int i=0;i<256;++i) {
          userPalette.at(i)=par.colors.at(i%par.colors.size());
        }
        pal = &userPalette;
      }
      break;
      default:
        pal = &grayPalette;
    }

    // check if we need to update the parameters (the informational part)
    // (the part we are modifying here does not belong to the user's
    // parameters, but to the informational components of the parameters,
    // which play a different role in the viewers.
    if (doStat) {
      statistics(chnl,getParameters());
    }

    float m,b;
    computeLinearTrans(255,m,b);

    // prepare the LUT
    rgbPixel lut[256];
    int i,val;
    for (i=0;i<256;++i) {
      val = iround(m*i + b);
      if (val < 0) {
        lut[i]=par.underflowColor;
      } else if (val > 255) {
        lut[i]=par.overflowColor;
      } else {
        lut[i]=pal->at(val);
      }
    }

    img.resize(chnl.size(),rgbPixel(),false,false);
    image::iterator it;
    vector<ubyte>::const_iterator cit,eit;
    int y;
    it = img.begin();
    for (y=0;y<chnl.rows();++y) {
      const vector<ubyte>& vct = chnl.getRow(y);
      for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {
        (*it) = lut[*cit];
      }
    }

    return true;
  }

  /**
   * The input channel is a scalar matrix
   */
  bool viewerFunctor::scalarToImage(const matrix<int>& chnl,image& img,
                                    const bool doStat) {
    // this draws on theImage the in theData given matrix<int> or matrix<ubyte>
    const viewerFunctor::parameters& par = getParameters();

    // select the palette to be used
    palette* pal;
    palette userPalette;

    switch(par.whichPalette) {
      case viewerFunctor::parameters::Gray:
        pal = &grayPalette;
        break;
      case viewerFunctor::parameters::Hue:
        pal = &huePalette;
        break;
      case viewerFunctor::parameters::Red:
        pal = &redPalette;
        break;
      case viewerFunctor::parameters::Green:
        pal = &greenPalette;
        break;
      case viewerFunctor::parameters::Blue:
        pal = &bluePalette;
        break;
      case viewerFunctor::parameters::UserDefined: {
        userPalette.resize(256,rgbPixel(),false,false);
        for (int i=0;i<256;++i) {
          userPalette.at(i)=par.colors.at(i%par.colors.size());
        }
        pal = &userPalette;
      }
      break;
      default:
        pal = &grayPalette;
    }

    // check if we need to update the parameters (the informational part)
    // (the part we are modifying here does not belong to the user's
    // parameters, but to the informational components of the parameters,
    // which play a different role in the viewers.
    if (doStat) {
      statistics(chnl,getParameters());
    }

    float m,b;
    computeLinearTrans(255,m,b);

    // two ways: with lookup-table or without, depending on
    // the number of used values in chnl32.
    static const int lutThresh = 4192; // just a magic value,
    const int imin = iround(par.minI);
    const int imax = iround(par.maxI);

    const int lutSize = 1+imax-imin;

    img.resize(chnl.size(),rgbPixel(),false,false);
    image::iterator it;
    vector<int>::const_iterator cit,eit;
    int y;
    int i;
    int val;

    if ((lutSize < lutThresh) &&
        (lutSize < (img.rows() * img.columns()/2))) {
      // use a LUT

      // prepare the LUT
      rgbPixel* lut0 = new rgbPixel[lutSize];
      // add the correct offset to the lut0!
      rgbPixel* lut = lut0 - imin;

      for (i=imin;i<=imax;++i) {
        val = iround(m*i + b);
        if (val < 0) {
          lut[i]=par.underflowColor;
        } else if (val > 255) {
          lut[i]=par.overflowColor;
        } else {
          lut[i]=pal->at(val);
        }
      }

      it = img.begin();
      for (y=0;y<chnl.rows();++y) {
        const vector<int>& vct = chnl.getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {
          (*it) = lut[*cit];
        }
      }

      delete[] lut0;
    } else {
      // transform each pixel
      it = img.begin();
      for (y=0;y<chnl.rows();++y) {
        const vector<int>& vct = chnl.getRow(y);
        for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {

          val = iround(m*(*cit) + b);
          if (val < 0) {
            (*it)=par.underflowColor;
          } else if (val > 255) {
            (*it)=par.overflowColor;
          } else {
            (*it)=pal->at(val);
          }
        }
      }
    }

    return true;
  }

  /**
   * The input channel is a scalar matrix
   */
  bool viewerFunctor::scalarToImage(const matrix<float>& chnl,image& img,
                                    const bool doStat) {
    // this draws on theImage the in theData given matrix<int> or matrix<ubyte>
    const viewerFunctor::parameters& par = getParameters();

    // select the palette to be used
    palette* pal;
    palette userPalette;

    switch(par.whichPalette) {
      case viewerFunctor::parameters::Gray:
        pal = &grayPalette;
        break;
      case viewerFunctor::parameters::Hue:
        pal = &huePalette;
        break;
      case viewerFunctor::parameters::Red:
        pal = &redPalette;
        break;
      case viewerFunctor::parameters::Green:
        pal = &greenPalette;
        break;
      case viewerFunctor::parameters::Blue:
        pal = &bluePalette;
        break;
      case viewerFunctor::parameters::UserDefined: {
        userPalette.resize(256,rgbPixel(),false,false);
        for (int i=0;i<256;++i) {
          userPalette.at(i)=par.colors.at(i%par.colors.size());
        }
        pal = &userPalette;
      }
      break;
      default:
        pal = &grayPalette;
    }

    // check if we need to update the parameters (the informational part)
    // (the part we are modifying here does not belong to the user's
    // parameters, but to the informational components of the parameters,
    // which play a different role in the viewers.
    if (doStat) {
      statistics(chnl,getParameters());
    }

    float m,b;
    computeLinearTrans(1.0,m,b);
    m*=255.0f;
    b*=255.0f;

    img.resize(chnl.size(),rgbPixel(),false,false);
    image::iterator it;
    vector<float>::const_iterator cit,eit;
    int y;
    int val;

    // transform each pixel
    it = img.begin();
    for (y=0;y<chnl.rows();++y) {
      const vector<float>& vct = chnl.getRow(y);
      for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {

        val = iround(m*(*cit) + b);
        if (val < 0) {
          (*it)=par.underflowColor;
        } else if (val > 255) {
          (*it)=par.overflowColor;
        } else {
          (*it)=pal->at(val);
        }
      }
    }

    return true;
  }


  /**
   * The input channel is a scalar matrix
   */
  bool viewerFunctor::scalarToImage(const matrix<double>& chnl,image& img,
                                    const bool doStat) {
    // this draws on theImage the in theData given matrix<int> or matrix<ubyte>
    const viewerFunctor::parameters& par = getParameters();

    // select the palette to be used
    palette* pal;
    palette userPalette;

    switch(par.whichPalette) {
      case viewerFunctor::parameters::Gray:
        pal = &grayPalette;
        break;
      case viewerFunctor::parameters::Hue:
        pal = &huePalette;
        break;
      case viewerFunctor::parameters::Red:
        pal = &redPalette;
        break;
      case viewerFunctor::parameters::Green:
        pal = &greenPalette;
        break;
      case viewerFunctor::parameters::Blue:
        pal = &bluePalette;
        break;
      case viewerFunctor::parameters::UserDefined: {
        userPalette.resize(256,rgbPixel(),false,false);
        for (int i=0;i<256;++i) {
          userPalette.at(i)=par.colors.at(i%par.colors.size());
        }
        pal = &userPalette;
      }
      break;
      default:
        pal = &grayPalette;
    }

    // check if we need to update the parameters (the informational part)
    // (the part we are modifying here does not belong to the user's
    // parameters, but to the informational components of the parameters,
    // which play a different role in the viewers.
    if (doStat) {
      statistics(chnl,getParameters());
    }

    float m,b;
    computeLinearTrans(1.0,m,b);
    m*=255.0f;
    b*=255.0f;

    img.resize(chnl.size(),rgbPixel(),false,false);
    image::iterator it;
    vector<double>::const_iterator cit,eit;
    int y;
    int val;

    // transform each pixel
    it = img.begin();
    for (y=0;y<chnl.rows();++y) {
      const vector<double>& vct = chnl.getRow(y);
      for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {

        val = iround(m*(*cit) + b);
        if (val < 0) {
          (*it)=par.underflowColor;
        } else if (val > 255) {
          (*it)=par.overflowColor;
        } else {
          (*it)=pal->at(val);
        }
      }
    }

    return true;
  }

  /**
   * The input channel is a scalar matrix
   */
  bool viewerFunctor::scalarToImage(const dvector& vct,image& img,
                                    const bool doStat) {
    // this draws on theImage the in vct given dvector
    const parameters& par = getParameters();

    // check if we need to update the parameters (the informational part)
    // (the part we are modifying here does not belong to the user's
    // parameters, but to the informational components of the parameters,
    // which play a different role in the viewers.
    if (doStat) {
      statistics(vct,getParameters());
    }

    const double s = pow(2.0,par.zoomFactor)*par.pixelsPerElement;

    // clear with the background color
    img.resize(par.vectorHeight,
               int(ceil(s*vct.size())),
               par.backgroundColor,false,true);

    // linear mapping between vector values and image coordinates
    double tmp = double(par.maxI-par.minI);
    if (abs(tmp)>std::numeric_limits<double>::epsilon()) {
      tmp = (1.0-par.vectorHeight)/tmp;
    } else {
      tmp = 0.0;
    }
    const double m = tmp;
    const double b = -m*par.maxI;
    int i,j,k;

    // draw zero level
    if (par.drawHorizontalAxis) {
      i = iround(b);
      if ((i>=0) && (i<img.rows())) {
        img.getRow(i).fill(par.underflowColor);
      }
    }

    const int zero = min(max(0,iround(b)),img.rows()-1);
    double indexMean(0.0),sumVal(0.0),sumAbsVal(0.0);
    int step,val,lastPos(-1),ipos,lastVal(zero);
    double pos(0.0);
    

    // transform each pixel
    for (i=0;i<vct.size();++i) {
      // compute the y-value for the image
      tmp = vct.at(i);  // read vector once
      val = iround(m*tmp + b);

      // computations towards mean values
      sumVal+=tmp;
      tmp = abs(tmp);
      sumAbsVal+=tmp;
      indexMean+=(i*tmp);

      // actual image position
      ipos = static_cast<int>(pos);

      // always draw the point
      img.at(val,ipos) = par.lineColor;

      if (par.useLines && (ipos > lastPos)) {
        lastPos = ipos;
        step = (val>zero) ? +1 : -1;
        if (par.useBoxes) {
          j = (step>0) ? min(zero,lastVal) : max(zero,lastVal);
          k = (step>0) ? max(val,lastVal)  : min(val,lastVal);
        } else {
          j = zero;
          k = val;
        }
        while(j!=k) {
          img.at(j,ipos)=par.lineColor;
          j+=step;
        }
        lastVal=val;
      }

      if (s>1.01) {
        if (par.useBoxes) {
          const int tmp = iround(pos+s);
          for (++pos;pos<tmp;++pos) {
            img.at(val,iround(pos)) = par.lineColor;
          }
        } else {
          pos+=iround(s);
        }
      } else {
        pos+=s;
      }
    }

    // draw the mean values if necessary
    if (par.drawMeanValues) {
      if (sumAbsVal != 0.0) {
        indexMean/=sumAbsVal;
      } 
      sumVal/=vct.size();

      val = iround(m*sumVal+b);
      if ((val>=0) && (val<img.rows())) {
        img.getRow(val).fill(par.overflowColor);
      }

      val = iround(indexMean*s);
      if ((val>=0) && (val<img.columns())) {
        for (i=0;i<img.rows();++i) {
          img.at(i,val)=par.overflowColor;
        }
      }
    }

    return true;
  }

  /**
   * The input channel is a scalar matrix
   */
  bool viewerFunctor::imageToImage(const image& data,image& img,
                                   const bool doStat) {
    // check if we need to update the parameters (the informational part)
    // (the part we are modifying here does not belong to the user's
    // parameters, but to the informational components of the parameters,
    // which play a different role in the viewers.
    if (doStat) {
      statistics(data,getParameters());
    }

    float m,b;
    computeLinearTrans(255,m,b);

    // prepare the LUT
    ubyte lut[256];
    int i,val;
    for (i=0;i<256;++i) {
      val = iround(m*i + b);
      if (val < 0) {
        lut[i]=0;
      } else if (val > 255) {
        lut[i]=255;
      } else {
        lut[i]=static_cast<ubyte>(val);
      }
    }

    img.resize(data.size(),rgbPixel(),false,false);
    image::iterator it;
    vector<rgbPixel>::const_iterator cit,eit;
    int y;
    it = img.begin();
    for (y=0;y<data.rows();++y) {
      const vector<rgbPixel>& vct = data.getRow(y);
      for (cit=vct.begin(),eit=vct.end();cit!=eit;++cit,++it) {
        (*it) = rgbPixel(lut[(*cit).getRed()],
                         lut[(*cit).getGreen()],
                         lut[(*cit).getBlue()]);
      }
    }

    return true;
  }


  /**
   * The input channel is a label map
   */
  bool viewerFunctor::labelsToImage(const matrix<ubyte>& chnl,image& img,
                                    const bool doStat) {
    const parameters& par = getParameters();

    labelAdjacencyMap lam;
    labelAdjacencyMap::parameters lamPar;
    lamPar.thePalette.copy(par.colors);
    lamPar.neighborhood = par.neighborhood8 ? 8 : 4;
    lamPar.minColors = par.useFewColors;
    lam.setParameters(lamPar);

    // check if we need to update the parameters (the informational part)
    // (the part we are modifying here does not belong to the user's
    // parameters, but to the informational components of the parameters,
    // which play a different role in the viewers.
    if (doStat) {
      statistics(chnl,getParameters());
    }

    return lam.apply(chnl,img);
  }

  /**
   * The input channel is a label map
   */
  bool viewerFunctor::labelsToImage(const matrix<int>& chnl,image& img,
                                    const bool doStat) {
    const parameters& par = getParameters();

    labelAdjacencyMap lam;
    labelAdjacencyMap::parameters lamPar;
    lamPar.thePalette.copy(par.colors);
    lamPar.neighborhood = par.neighborhood8 ? 8 : 4;
    lamPar.minColors = par.useFewColors;
    lam.setParameters(lamPar);

    // check if we need to update the parameters (the informational part)
    // (the part we are modifying here does not belong to the user's
    // parameters, but to the informational components of the parameters,
    // which play a different role in the viewers.
    if (doStat) {
      statistics(chnl,getParameters());
    }

    return lam.apply(chnl,img);
  }


  /*
   * statistics
   */
  bool viewerFunctor::statistics(const matrix<ubyte>& chnl,
                                 parameters& par) const {

    par.size = chnl.size();

    // functor to create statistics
    serialStatsExtFunctor<ubyte,float> stats;
    stats.considerElements(chnl);

    stats.apply(par.averageI,par.stdDeviation,par.minI,par.maxI);
    par.stdDeviation = sqrt(par.stdDeviation); // fix the variance -> stdDev

    return true;
  }

  /*
   * statistics
   */
  bool viewerFunctor::statistics(const matrix<int>& chnl,
                                 parameters& par) const {
    par.size = chnl.size();

    // functor to create statistics
    serialStatsExtFunctor<int,float> stats;
    stats.considerElements(chnl);

    stats.apply(par.averageI,par.stdDeviation,par.minI,par.maxI);
    par.stdDeviation = sqrt(par.stdDeviation); // fix the variance -> stdDev

    return true;
  }

  /*
   * statistics
   */
  bool viewerFunctor::statistics(const matrix<float>& chnl,
                                 parameters& par) const {
    par.size = chnl.size();

    // functor to create statistics
    serialStatsExtFunctor<float> stats;
    stats.considerElements(chnl);

    stats.apply(par.averageI,par.stdDeviation,par.minI,par.maxI);
    par.stdDeviation = sqrt(par.stdDeviation); // fix the variance -> stdDev

    return true;
  }

  /*
   * statistics
   */
  bool viewerFunctor::statistics(const matrix<double>& chnl,
                                 parameters& par) const {
    par.size = chnl.size();

    // functor to create statistics
    serialStatsExtFunctor<double> stats;
    stats.considerElements(chnl);

    double avrg,vari,mini,maxi;

    stats.apply(avrg,vari,mini,maxi);

    par.averageI = static_cast<float>(avrg);
    par.stdDeviation=static_cast<float>(sqrt(vari));
    par.minI=static_cast<float>(mini);
    par.maxI=static_cast<float>(maxi);

    return true;
  }

  /*
   * statistics
   */
  bool viewerFunctor::statistics(const image& img,
                                 parameters& par) const {
    par.size = img.size();

    channel8 red,green,blue;
    splitImageToRGB splitter;
    splitter.apply(img,red,green,blue);

    // functor to create statistics
    serialStatsExtFunctor<ubyte,float> stats;

    float tmin,tmax;
    // red
    stats.considerElements(red);
    stats.apply(par.averageRGB.red,par.diagCovariance.red,
                tmin,tmax);
    par.minRGB.setRed(static_cast<ubyte>(tmin));
    par.maxRGB.setRed(static_cast<ubyte>(tmax));

    // green
    stats.considerElements(green);
    stats.apply(par.averageRGB.green,par.diagCovariance.green,
                tmin,tmax);
    par.minRGB.setGreen(static_cast<ubyte>(tmin));
    par.maxRGB.setGreen(static_cast<ubyte>(tmax));

    // blue
    stats.considerElements(blue);
    stats.apply(par.averageRGB.blue,par.diagCovariance.blue,
                tmin,tmax);
    par.minRGB.setBlue(static_cast<ubyte>(tmin));
    par.maxRGB.setBlue(static_cast<ubyte>(tmax));

    // compute the intensity values
    par.minI = min(par.minRGB.getRed(),
                   par.minRGB.getGreen(),
                   par.minRGB.getBlue());

    par.maxI = max(par.maxRGB.getRed(),
                   par.maxRGB.getGreen(),
                   par.maxRGB.getBlue());

    par.averageI = (par.averageRGB.blue +
                    par.averageRGB.green +
                    par.averageRGB.red)/3.0f;

    par.stdDeviation = sqrt((par.diagCovariance.blue +
                             par.diagCovariance.green +
                             par.diagCovariance.red)/3.0f);

    // fix variance to stdDev
    par.diagCovariance.apply(sqrt);

    return true;
  }

  /*
   * statistics
   */
  bool viewerFunctor::statistics(const dvector& vct,
                                       parameters& par) const {
    par.size = point(1,vct.size());

    // functor to create statistics
    serialStatsExtFunctor<double> stats;
    stats.considerElements(vct);

    double avrg,vari,mini,maxi;

    stats.apply(avrg,vari,mini,maxi);

    par.averageI = static_cast<float>(avrg);
    par.stdDeviation=static_cast<float>(sqrt(vari));
    par.minI=static_cast<float>(mini);
    par.maxI=static_cast<float>(maxi);

    return true;
  }

  void viewerFunctor::computeContrastExtrema(const float norm,
                                             const float c,
                                             float& minp,
                                             float& maxp) const {

    const parameters& par = getParameters();

    const float mi = par.minI/norm;
    const float ma = par.maxI/norm;

    float mx1,mx2,bx1,bx2;
    float mn1,mn2,bn1,bn2;

    mx1 = (ma-mi)/2.0f;
    mn1 = -mx1;
    bx1 = bn1 = (mi+ma)/2.0f;

    mx2 = (1.0f-ma);
    mn2 = -mi;
    bx2 = 1.0f-mx2*2.0f;
    bn2 = -mn2*2.0f;

    // the minp and maxp a function from the contrast are defined here as
    // a piecewise function with five intervals
    if (c < -2) {
      float m,b;
      m=-min(mn1,mn2);
      b=m*2.0f;
      maxp=m*c+b;
      m=-max(mx1,mx2);
      b=1.0f+m*2.0f;
      minp=m*c+b;
    } else if (c < -1) {
      maxp=-mn2*c+bn2;
      minp=-mx2*c+bx2;
    } else if (c <  1) {
      maxp=mx1*c+bx1;
      minp=mn1*c+bn1;
    } else if (c <  2) {
      maxp=mx2*c+bx2;
      minp=mn2*c+bn2;
    } else {
      float m,b;
      m=max(mx1,mx2);
      b=1.0f-m*2.0f;
      maxp=m*c+b;
      m=min(mn1,mn2);
      b=-m*2.0f;
      minp=m*c+b;
    }
  }

  void viewerFunctor::computeLinearTrans(const float norm,
                                         float& m,
                                         float& b) const {
    float minp,maxp;
    const parameters& par = getParameters();

    if ( (par.contrast == 1.0f) && (par.brightness == 0.0f)) {
      // in case the channel has wrong information, like NaN or Inf, we
      // still want to get all valid pixels.  So avoid the computation of
      // m and b for the standard case using the statistics:
      m=1.0f;
      b=0.0f;
      return;
    }

    const float mi = par.minI/norm;
    const float ma = par.maxI/norm;

    computeContrastExtrema(norm,par.contrast,minp,maxp);

    if (abs(ma-mi)>std::numeric_limits<float>::epsilon()) {
      m = (maxp-minp)/(ma-mi);
    } else {
      m = 0.0f;
    }
    b = norm*(minp - m*mi +  par.brightness);
  }

  void viewerFunctor::computeContrastBrightness(const float minp,
                                                const float maxp,
                                                const float norm,
                                                float& c,
                                                float& b) const {
    const parameters& par = getParameters();
    const float mi = par.minI/norm;
    const float ma = par.maxI/norm;

    const float mnp = minp/norm;
    const float mxp = maxp/norm;

    // first compute the delta
    const float delta = mxp-mnp;

    if (delta<0) {
      // error max<min!
      c=1;
      b=0;
      return;
    }

    // now compute the deltas at the boundaries of the slices of the function
    float tmp1,tmp2;
    computeContrastExtrema(norm,1,tmp1,tmp2);
    const float delta1=tmp2-tmp1;
    computeContrastExtrema(norm,2,tmp1,tmp2);
    const float delta2=tmp2-tmp1;

    float mx1;
    mx1 = (ma-mi)/2.0f;

    float mx2,bx2,mn2,bn2;

    mx2 = (1.0f-ma);
    mn2 = -mi;
    bx2 = 1.0f-mx2*2.0f;
    bn2 = -mn2*2.0f;

    // find in which interval must be the contrast and compute it
    if (delta<delta1) {
      c = delta/(2.0f*mx1);
    } else if (delta<delta2) {
      c = (delta - (bx2-bn2))/(mx2-mn2);
    } else {
      float mx,mn,bx,bn;
      mx = max(mx1,mx2);
      mn = min(-mx1,mn2);
      bx = 1.0f - mx*2.0f;
      bn = -2.0f*mn;
      c = (delta - (bx-bn))/(mx-mn);
    }

    // now compute with the known contrast the brightness:
    computeContrastExtrema(norm,c,tmp1,tmp2);
    b = mnp-tmp1;

  }

}
