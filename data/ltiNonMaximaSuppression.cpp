/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiNonMaximaSuppression.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 23.5.2003
 * revisions ..: $Id: ltiNonMaximaSuppression.cpp,v 1.13 2006/09/05 10:23:18 ltilib Exp $
 */

#include "ltiNonMaximaSuppression.h"
#include "ltiConstants.h"
#include "ltiSort.h"

#undef _LTI_DEBUG
// #define _LTI_DEBUG 2
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#endif


namespace lti {
  // --------------------------------------------------
  // nonMaximaSuppression::parameters
  // --------------------------------------------------

  // default constructor
  nonMaximaSuppression::parameters::parameters()
    : transform::parameters() {
    thresholdMax = float(0.04f);
    indirectThresholdMax = false;
    thresholdMin = float(0.5f);
    indirectThresholdMin = false;
    background=ubyte(0);
    edgeValue=ubyte(255);
    checkAngles = true;
    gradientHistogramSize = 256;
    fillGaps = false;
    endPointValue = ubyte(255);
    gapValue = ubyte(255);
    numGapHints = int(5);
    maxGapLength = int(10);
  }

  // copy constructor
  nonMaximaSuppression::parameters::parameters(const parameters& other)
    : transform::parameters() {
    copy(other);
  }

  // destructor
  nonMaximaSuppression::parameters::~parameters() {
  }

  // get type name
  const char* nonMaximaSuppression::parameters::getTypeName() const {
    return "nonMaximaSuppression::parameters";
  }

  // copy member

  nonMaximaSuppression::parameters&
    nonMaximaSuppression::parameters::copy(const parameters& other) {
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

    thresholdMax          = other.thresholdMax;
    indirectThresholdMax  = other.indirectThresholdMax;
    thresholdMin          = other.thresholdMin;
    indirectThresholdMin  = other.indirectThresholdMin;
    background            = other.background;
    edgeValue             = other.edgeValue;
    checkAngles           = other.checkAngles;
    gradientHistogramSize = other.gradientHistogramSize;

    fillGaps     = other.fillGaps;
    endPointValue= other.endPointValue;
    gapValue     = other.gapValue;
    numGapHints  = other.numGapHints;
    maxGapLength = other.maxGapLength;

    return *this;
  }

  // alias for copy member
  nonMaximaSuppression::parameters&
    nonMaximaSuppression::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* nonMaximaSuppression::parameters::clone() const {
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
  bool nonMaximaSuppression::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool nonMaximaSuppression::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"thresholdMax",thresholdMax);
      lti::write(handler,"indirectThresholdMax",indirectThresholdMax);
      lti::write(handler,"thresholdMin",thresholdMin);
      lti::write(handler,"indirectThresholdMin",indirectThresholdMin);
      lti::write(handler,"background",background);
      lti::write(handler,"edgeValue",edgeValue);
      lti::write(handler,"checkAngles",checkAngles);
      lti::write(handler,"gradientHistogramSize",gradientHistogramSize);
      lti::write(handler,"fillGaps",fillGaps);
      lti::write(handler,"endPointValue",endPointValue);
      lti::write(handler,"gapValue",gapValue);
      lti::write(handler,"numGapHints",numGapHints);
      lti::write(handler,"maxGapLength",maxGapLength);
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
  bool nonMaximaSuppression::parameters::write(ioHandler& handler,
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
  bool nonMaximaSuppression::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool nonMaximaSuppression::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {      
      b = lti::read(handler,"thresholdMax",thresholdMax) && b;
      b = lti::read(handler,"indirectThresholdMax",indirectThresholdMax) && b;
      b = lti::read(handler,"thresholdMin",thresholdMin) && b;
      b = lti::read(handler,"indirectThresholdMin",indirectThresholdMin) && b;
      b = lti::read(handler,"background",background) && b;
      b = lti::read(handler,"edgeValue",edgeValue) && b;
      b = lti::read(handler,"checkAngles",checkAngles) && b;      
      b = lti::read(handler,"gradientHistogramSize",gradientHistogramSize) 
        && b;

      b = lti::read(handler,"fillGaps",fillGaps) && b;
      b = lti::read(handler,"endPointValue",endPointValue) && b;
      b = lti::read(handler,"gapValue",gapValue) && b;
      b = lti::read(handler,"numGapHints",numGapHints) && b;
      b = lti::read(handler,"maxGapLength",maxGapLength) && b;
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
  bool nonMaximaSuppression::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // nonMaximaSuppression
  // --------------------------------------------------

  // default constructor
  nonMaximaSuppression::nonMaximaSuppression()
    : transform(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  nonMaximaSuppression::nonMaximaSuppression(const parameters& par)
    : transform() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  nonMaximaSuppression::nonMaximaSuppression(const nonMaximaSuppression& other)
    : transform() {
    copy(other);
  }

  // destructor
  nonMaximaSuppression::~nonMaximaSuppression() {
  }

  // returns the name of this type
  const char* nonMaximaSuppression::getTypeName() const {
    return "nonMaximaSuppression";
  }

  // copy member
  nonMaximaSuppression&
  nonMaximaSuppression::copy(const nonMaximaSuppression& other) {
    transform::copy(other);

    return (*this);
  }

  // alias for copy member
  nonMaximaSuppression&
    nonMaximaSuppression::operator=(const nonMaximaSuppression& other) {
    return (copy(other));
  }


  // clone member
  functor* nonMaximaSuppression::clone() const {
    return new nonMaximaSuppression(*this);
  }

  // return parameters
  const nonMaximaSuppression::parameters&
    nonMaximaSuppression::getParameters() const {
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
  bool nonMaximaSuppression::apply(const channel& preedges,
                                   const channel& orientation,
                                   channel8& edges,
                                   const float maxPreedge) const {

    if (preedges.empty()) {
      edges.clear();
      return true;
    }
    if (preedges.size() != orientation.size()) {
      setStatusString("preedges and orientation channels must have same size");
      return false;
    } 
    
    channel8 maxima;
    maxima.resize(preedges.size(),ubyte(),false,false);
    edges.resize(preedges.size(),ubyte(),false,false);
    float thresholdMin,thresholdMax;
    thresholdValues(preedges,maxPreedge,thresholdMin,thresholdMax);

    _lti_debug("Non maxima suppression...");
    if (getParameters().checkAngles) {
      channel neworient;
      checkOrientation(orientation,neworient);
      nonMaxSuppression(preedges,neworient,maxima,thresholdMin);
    } else {
      nonMaxSuppression(preedges,orientation,maxima,thresholdMin);
    }
    _lti_debug("done."<<std::endl);

    _lti_debug("Hysteresis...");
    hysteresis(preedges,maxima,thresholdMax,edges);
    _lti_debug("done.");

#ifdef _LTI_DEBUG
    static viewer view1("maxima");
    static viewer view2("edges");
    view1.show(maxima);
    view2.show(edges);
#endif

    if (getParameters().fillGaps) {
      pointList endPtsList;
      tpointList<float> deltas;
      channel8 dest;

      findEndPoints(edges,dest,endPtsList);
      trackPoints(edges,endPtsList,orientation,deltas);
      fillGaps(edges,preedges,endPtsList,deltas,dest);
      dest.detach(edges);
    }

    return true;
  }

  // On place apply for type channel!
  bool nonMaximaSuppression::apply(const channel& preedges,
                                   const channel& orientation,
                                   const channel& relevance,
                                   channel8& edges,
                                   const float maxPreedge) const {

    if (preedges.size() != relevance.size()) {
      setStatusString("preedges and relevance channels must have same sizes");
      return false;
    }

    channel tmp;
    tmp.emultiply(preedges,relevance);

    return apply(tmp,orientation,edges,maxPreedge);
  }


  void nonMaximaSuppression::checkOrientation(const channel& src,
                                              channel& dest) const {
    dest.resize(src.size(),float(),false,false);

    // dest is always connected
    vector<channel::value_type>::const_iterator it,eit;
    channel::iterator dit;
    float tmp;
    int y;
    static const float Pi2=static_cast<float>(2*Pi);

    for (y=0,dit=dest.begin();y<src.rows();++y) {
      const vector<channel::value_type>& vct=src.getRow(y);
      for (it=vct.begin(),eit=vct.end();
           it!=eit;
           ++it,++dit) {
        tmp = (*it);
        while (tmp < 0.0f) {
          tmp+=Pi2;
        }
        while (tmp > Pi2) {
          tmp-=Pi2;
        }
        (*dit)=tmp;
      }
    }
  }

  // y1:      first function value
  // y2:      second function value
  // fOffset: offset (0..1) between
  //          x1 and x2 to interpolate at
  inline float
  nonMaximaSuppression::interpolate(const float y1,
                                    const float y2,
                                    const float fOffset) const {
    return ((y2-y1)*fOffset)+y1;
  };



  // On place apply for type channel!
  void nonMaximaSuppression::nonMaxSuppression(const channel& preedges,
                                               const channel& orientation,
                                               channel8& edges,
                                               const float thresholdMin) const{

    // pointers to the data.
    
    const float* fpGradAbs = &preedges.at(0,0);
    const float* fpGradPhi = &orientation.at(0,0);
    ubyte* ucpNonMaxSupp = &edges.at(0,0);

    const parameters& param = getParameters();
    const ubyte edgeValue=param.edgeValue;
    const ubyte background=param.background;
    
    const int width = preedges.columns();
    const int height = preedges.rows();

    const int w1 = width-1;
    const int h1 = height-1;
    const int w2 = width-2;
    const int h2 = height-2;

    // robustness issue: add 8 elements for "negative" access
    static const int
      iRelNeighbourPosXData[]={1, 1, 0,-1,-1,-1, 0, 1, 
                               1, 1, 0,-1,-1,-1, 0, 1, 
                               1, 1};
    static const int *const iRelNeighbourPosX = &iRelNeighbourPosXData[8];

    // robustness issue: add 8 elements for "negative" access
    static const int
      iRelNeighbourPosYData[]={0, 1, 1, 1, 0,-1,-1,-1,
                               0, 1, 1, 1, 0,-1,-1,-1,
                               0, 1};
    static const int *const iRelNeighbourPosY = &iRelNeighbourPosYData[8];

    // robustness issue: add 8 elements for "negative" access
    const int neighborOffsetData[]={1,1+width,width,w1,-1,-width-1,-width,-w1,
                                    1,1+width,width,w1,-1,-width-1,-width,-w1,
                                    1,1+width};

    const int *const neighborOffset = &neighborOffsetData[8]; 

    int i;
    int yy,xx,x,y,sx,sy;;
    float y1,y2;
    float ftemp;
    float fInterpolTop,fInterpolBottom;
    float fOffset;

    static const float factor = static_cast<float>(4.0/Pi);
    const float* fpPtr;

    // Loop unrolled to speed up!

    // ---------------
    // top left corner
    // ---------------

    _lti_debug("  Top-Left Corner");

    ftemp=factor*fpGradPhi[0];
    i=static_cast<int>(ftemp);
    fOffset=ftemp-i;

    sx=iRelNeighbourPosX[i];
    sy=iRelNeighbourPosY[i];
    if (sx<0) sx=1;
    if (sy<0) sy=1;
    y1=preedges.at(sy,sx);

    sx=iRelNeighbourPosX[i+1];
    sy=iRelNeighbourPosY[i+1];
    if (sx<0) sx=1;
    if (sy<0) sy=1;
    y2=preedges.at(sy,sx);

    fInterpolTop=interpolate(y1,y2,fOffset);

    i=(i+4)%8;
    sx=iRelNeighbourPosX[i];
    sy=iRelNeighbourPosY[i];
    if (sx<0) sx=1;
    if (sy<0) sy=1;
    y1=preedges.at(sy,sx);

    sx=iRelNeighbourPosX[i+1];
    sy=iRelNeighbourPosY[i+1];
    if (sx<0) sx=1;
    if (sy<0) sy=1;
    y2=preedges.at(sy,sx);

    fInterpolBottom=interpolate(y1,y2,fOffset);

    ftemp=fpGradAbs[0];

    if (ftemp>=fInterpolTop && ftemp>fInterpolBottom) {
      //is maximum
      ucpNonMaxSupp[0]=(ftemp>thresholdMin) ? edgeValue : background;
    } else {
      // is not maximum -> suppress
      ucpNonMaxSupp[0]=background;
    }

    // ---------
    // first row
    // ---------

    _lti_debug("- First Row Corner ");

    for (x=1;x<w1;x++) {
      ftemp=factor*fpGradPhi[x];
      i=static_cast<int>(ftemp);
      fOffset=ftemp-i;

      sx=x+iRelNeighbourPosX[i];
      sy=iRelNeighbourPosY[i];
      if (sy<0) sy=1;
      y1=preedges.at(sy,sx);

      sx=x+iRelNeighbourPosX[i+1];
      sy=iRelNeighbourPosY[i+1];
      if (sy<0) sy=1;
      y2=preedges.at(sy,sx);

      fInterpolTop=interpolate(y1,y2,fOffset);

      i=(i+4)%8;

      sx=x+iRelNeighbourPosX[i];
      sy=iRelNeighbourPosY[i];
      if (sy<0) sy=1;
      y1=preedges.at(sy,sx);

      sx=x+iRelNeighbourPosX[i+1];
      sy=iRelNeighbourPosY[i+1];
      if (sy<0) sy=1;
      y2=preedges.at(sy,sx);

      fInterpolBottom=interpolate(y1,y2,fOffset);

      ftemp=fpGradAbs[x];

      if (ftemp>=fInterpolTop && ftemp>fInterpolBottom) {
        //is maximum
        ucpNonMaxSupp[x]=(ftemp>thresholdMin) ? edgeValue : background;
      } else {
        // is not maximum -> suppress
        ucpNonMaxSupp[x]=background;
      }
    }

    // ----------------
    // top right corner
    // ----------------

    _lti_debug("- Top-Right Corner" << std::endl);

    ftemp=factor*fpGradPhi[w1];
    i=static_cast<int>(ftemp);
    fOffset=ftemp-i;

    sx=w1+iRelNeighbourPosX[i];
    sy=iRelNeighbourPosY[i];
    if (sx>w1) sx=w2;
    if (sy<0) sy=1;
    y1=preedges.at(sy,sx);

    sx=w1+iRelNeighbourPosX[i+1];
    sy=iRelNeighbourPosY[i+1];
    if (sx>w1) sx=w2;
    if (sy<0) sy=1;
    y2=preedges.at(sy,sx);

    fInterpolTop=interpolate(y1,y2,fOffset);

    i=(i+4)%8;
    sx=w1+iRelNeighbourPosX[i];
    sy=iRelNeighbourPosY[i];
    if (sx>w1) sx=w2;
    if (sy<0) sy=1;
    y1=preedges.at(sy,sx);

    sx=w1+iRelNeighbourPosX[i+1];
    sy=iRelNeighbourPosY[i+1];
    if (sx>w1) sx=w2;
    if (sy<0) sy=1;
    y2=preedges.at(sy,sx);

    fInterpolBottom=interpolate(y1,y2,fOffset);

    ftemp=fpGradAbs[w1];

    if (ftemp>=fInterpolTop && ftemp>fInterpolBottom) {
      //is maximum
      ucpNonMaxSupp[w1]=(ftemp>thresholdMin) ? edgeValue : background;
    } else {
      // is not maximum -> suppress
      ucpNonMaxSupp[w1]=background;
    }

    // ----------------
    // ----------------
    // |  main loop   |
    // ----------------
    // ----------------

    for (y=1,fpPtr=&fpGradAbs[width],yy=y*width;
         y<h1;
         ++y,yy+=width) {

      _lti_debug("  Line " << y << ":");

      // . . . . . .
      // . left side
      // . . . . . .

      _lti_debug("    Left Side ");

      ftemp=factor*fpGradPhi[yy];
      i=static_cast<int>(ftemp);
      fOffset=ftemp-i;

      sx=iRelNeighbourPosX[i];
      sy=y+iRelNeighbourPosY[i];
      if (sx<0) sx=1;
      y1=preedges.at(sy,sx);

      sx=iRelNeighbourPosX[i+1];
      sy=y+iRelNeighbourPosY[i+1];
      if (sx<0) sx=1;
      y2=preedges.at(sy,sx);

      fInterpolTop=interpolate(y1,y2,fOffset);

      i=(i+4)%8;
      sx=iRelNeighbourPosX[i];
      sy=y+iRelNeighbourPosY[i];
      if (sx<0) sx=1;
      y1=preedges.at(sy,sx);

      sx=iRelNeighbourPosX[i+1];
      sy=y+iRelNeighbourPosY[i+1];
      if (sx<0) sx=1;
      y2=preedges.at(sy,sx);

      fInterpolBottom=interpolate(y1,y2,fOffset);

      ftemp=*fpPtr;
      ++fpPtr;

      if (ftemp>=fInterpolTop && ftemp>fInterpolBottom) {
        //is maximum
        ucpNonMaxSupp[yy]=(ftemp>thresholdMin) ? edgeValue : background;
      } else {
        // is not maximum -> suppress
        ucpNonMaxSupp[yy]=background;
      }

      // . . . . . . .
      //    center
      // . . . . . . .

      _lti_debug(" - Center \n");

      const int endxx=yy+w1;
      for (x=1,xx=yy+1;xx<endxx;++xx,++fpPtr) {

	ftemp=factor*fpGradPhi[xx];
	i=static_cast<int>(ftemp);

        fOffset=ftemp-i;
 	fInterpolTop=interpolate(fpPtr[neighborOffset[i]],
                                 fpPtr[neighborOffset[i+1]],
                                 fOffset);

        i=(i+4)%8;
 	fInterpolBottom=interpolate(fpPtr[neighborOffset[i]],
                                    fpPtr[neighborOffset[i+1]],
                                    fOffset);

	ftemp=(*fpPtr);

	if ((ftemp>=fInterpolTop) && (ftemp>fInterpolBottom)) {
	  //is maximum
          ucpNonMaxSupp[xx]=(ftemp>=thresholdMin) ? edgeValue : background;
	} else {
	  // is not maximum -> suppress
	  ucpNonMaxSupp[xx]=background;
	}
      }

      // . . . . . . .
      //  right side .
      // . . . . . . .

      _lti_debug("- Right Side" << std::endl);

      ftemp=factor*fpGradPhi[xx];
      i=static_cast<int>(ftemp);
      fOffset=ftemp-i;

      sx=x+iRelNeighbourPosX[i];
      sy=y+iRelNeighbourPosY[i];
      if (sx>w1) sx=w2;
      y1=preedges.at(sy,sx);

      sx=x+iRelNeighbourPosX[i+1];
      sy=y+iRelNeighbourPosY[i+1];
      if (sx>w1) sx=w2;
      y2=preedges.at(sy,sx);

      fInterpolTop=interpolate(y1,y2,fOffset);

      i=(i+4)%8;
      sx=x+iRelNeighbourPosX[i];
      sy=y+iRelNeighbourPosY[i];
      if (sx>w1) sx=w2;
      y1=preedges.at(sy,sx);

      sx=x+iRelNeighbourPosX[i+1];
      sy=y+iRelNeighbourPosY[i+1];
      if (sx>w1) sx=w2;
      y2=preedges.at(sy,sx);

      fInterpolBottom=interpolate(y1,y2,fOffset);

      ftemp=*fpPtr;
      ++fpPtr;

      if (ftemp>=fInterpolTop && ftemp>fInterpolBottom) {
        //is maximum
        ucpNonMaxSupp[endxx]=(ftemp>thresholdMin) ?
          edgeValue : background;
      } else {
        // is not maximum -> suppress
        ucpNonMaxSupp[endxx]=background;
      }
    }

    // ------------------
    // bottom left corner
    // ------------------

    _lti_debug("  Bottom Left Corner");

    yy=h1*width;

    ftemp=factor*fpGradPhi[yy];
    i=static_cast<int>(ftemp);
    fOffset=ftemp-i;

    sx=iRelNeighbourPosX[i];
    sy=h1+iRelNeighbourPosY[i];
    if (sx<0) sx=1;
    if (sy>h1) sy=h2;
    y1=preedges.at(sy,sx);

    sx=iRelNeighbourPosX[i+1];
    sy=h1+iRelNeighbourPosY[i+1];
    if (sx<0) sx=1;
    if (sy>h1) sy=h2;
    y2=preedges.at(sy,sx);

    fInterpolTop=interpolate(y1,y2,fOffset);

    i=(i+4)%8;
    sx=iRelNeighbourPosX[i];
    sy=h1+iRelNeighbourPosY[i];
    if (sx<0) sx=1;
    if (sy>h1) sy=h2;
    y1=preedges.at(sy,sx);

    sx=iRelNeighbourPosX[i+1];
    sy=h1+iRelNeighbourPosY[i+1];
    if (sx<0) sx=1;
    if (sy>h1) sy=h2;
    y2=preedges.at(sy,sx);

    fInterpolBottom=interpolate(y1,y2,fOffset);

    ftemp=fpGradAbs[yy];

    if (ftemp>=fInterpolTop && ftemp>fInterpolBottom) {
      //is maximum
      ucpNonMaxSupp[yy]=(ftemp>thresholdMin) ? edgeValue : background;
    } else {
      // is not maximum -> suppress
      ucpNonMaxSupp[yy]=background;
    }

    // ---------
    // last row
    // ---------

    _lti_debug("- Last Row");

    for (x=1,xx=yy+x;x<w1;++x,++xx) {
      ftemp=factor*fpGradPhi[xx];
      i=static_cast<int>(ftemp);
      fOffset=ftemp-i;

      sx=x +iRelNeighbourPosX[i];
      sy=h1+iRelNeighbourPosY[i];
      if (sy>h1) sy=h2;
      y1=preedges.at(sy,sx);

      sx=x +iRelNeighbourPosX[i+1];
      sy=h1+iRelNeighbourPosY[i+1];
      if (sy>h1) sy=h2;
      y2=preedges.at(sy,sx);

      fInterpolTop=interpolate(y1,y2,fOffset);

      i=(i+4)%8;

      sx=x+iRelNeighbourPosX[i];
      sy=h1+iRelNeighbourPosY[i];
      if (sy>h1) sy=h2;
      y1=preedges.at(sy,sx);

      sx=x+iRelNeighbourPosX[i+1];
      sy=h1+iRelNeighbourPosY[i+1];
      if (sy>h1) sy=h2;
      y2=preedges.at(sy,sx);

      fInterpolBottom=interpolate(y1,y2,fOffset);

      ftemp=fpGradAbs[xx];

      if (ftemp>=fInterpolTop && ftemp>fInterpolBottom) {
        //is maximum
        ucpNonMaxSupp[xx]=(ftemp>thresholdMin) ? edgeValue : background;
      } else {
        // is not maximum -> suppress
        ucpNonMaxSupp[xx]=background;
      }
    }

    // -------------------
    // bottom right corner
    // -------------------

    _lti_debug("- Bottom Right Corner" << std::endl);

    ftemp=factor*fpGradPhi[xx];
    i=static_cast<int>(ftemp);
    fOffset=ftemp-i;

    sx=w1+iRelNeighbourPosX[i];
    sy=h1+iRelNeighbourPosY[i];
    if (sx>w1) sx=w2;
    if (sy>h1) sy=h2;
    y1=preedges.at(sy,sx);

    sx=w1+iRelNeighbourPosX[i+1];
    sy=h1+iRelNeighbourPosY[i+1];
    if (sx>w1) sx=w2;
    if (sy>h1) sy=h2;
    y2=preedges.at(sy,sx);

    fInterpolTop=interpolate(y1,y2,fOffset);

    i=(i+4)%8;
    sx=w1+iRelNeighbourPosX[i];
    sy=h1+iRelNeighbourPosY[i];
    if (sx>w1) sx=w2;
    if (sy>h1) sy=h2;
    y1=preedges.at(sy,sx);

    sx=w1+iRelNeighbourPosX[i+1];
    sy=h1+iRelNeighbourPosY[i+1];
    if (sx>w1) sx=w2;
    if (sy>h1) sy=h2;
    y2=preedges.at(sy,sx);

    fInterpolBottom=interpolate(y1,y2,fOffset);

    ftemp=fpGradAbs[xx];

    if (ftemp>=fInterpolTop && ftemp>fInterpolBottom) {
      //is maximum
      ucpNonMaxSupp[xx]=(ftemp>thresholdMin) ? edgeValue : background;
    } else {
      // is not maximum -> suppress
      ucpNonMaxSupp[xx]=background;
    }

  }

  void nonMaximaSuppression::hysteresis(const channel& preedge,
                                        const channel8& maxima,
                                        const float thresholdMax,
                                        channel8& dest) const {

    const parameters& param = getParameters();
    const ubyte edgeValue=param.edgeValue;
    const ubyte background=param.background;

    const int width    = preedge.columns();
    const int height   = preedge.rows();
    const int w1       = width-1;
    const int offset[] = {-w1,-width,-width-1,-1,w1,width,width+1,1};

    // ensure proper size of destination channel
    dest.resize(preedge.size(),ubyte(),false,false);

    const float* fpGradAbs     = &preedge.at(0,0);
    const ubyte* ucpNonMaxSupp = &maxima.at(0,0);
    ubyte* ucpDest             = &dest.at(0,0);

    int sp=0; // stack pointer
    int i,j,ii,jj;
    const int endi = width*height;
    ivector stack;
    stack.resize(endi,int(),false,false);

    for (i=0;i<endi;++i) {
      if ((ucpNonMaxSupp[i]==edgeValue) &&
          (fpGradAbs[i]>=thresholdMax)) {
        ucpDest[i]=edgeValue;
        stack.at(sp++) = i;
      } else {
        ucpDest[i]=background;
      }
    }

    // if an edge, try to find other edges around
    while (sp>0) {
      ii=stack.at(--sp); // pop point from stack
      
      for (j=0;j<4;++j) {
        jj=ii+offset[j];
        if ((jj>=0) &&
            (ucpDest[jj] != edgeValue) &&
            (ucpNonMaxSupp[jj] == edgeValue)) {
          // sacrifice a little bit precision ignoring the x-boundaries
          // to gain some speed.  The worst thing that can happen is that
          // an "almost" edge value on the other side will be considered
          // edge, which is most times anyway an edge.  In
          // noised images, you can consider that the "noise" force some
          // pixel to be edge.
          ucpDest[jj]=edgeValue;
          stack.at(sp++)=jj;
        }
      }

      for (j=4;j<8;++j) {
        jj=ii+offset[j];
        if ((jj<endi) &&
            (ucpDest[jj] != edgeValue) &&
            (ucpNonMaxSupp[jj] == edgeValue)) {
          // sacrifice a little bit precision ignoring the x-boundaries
          // to gain some speed.  The worst thing that can happen is that
          // an "almost" edge value on the other side will be considered
          // edge, which is most times anyway an edge. In
          // noised images, you can consider that the "noise" force some
          // pixel to be edge.
          ucpDest[jj]=edgeValue;
          stack.at(sp++)=jj;
        }
      }

    } // while !stack.empty()
  };


  /*
   * compute the real thresholMin and threshodMax values to be used
   */
  void nonMaximaSuppression::thresholdValues(const channel& grad,
                                             const float maxGrad,
                                             float& thresholdMin,
                                             float& thresholdMax) const {

    const parameters& param = getParameters();
    thresholdMax = param.thresholdMax*maxGrad;
    thresholdMin = param.thresholdMin*thresholdMax;
    
    if (param.indirectThresholdMin || param.indirectThresholdMax) {
      // the histogram has to be computed for any min or max threshold or both.
      vector<float> histo(param.gradientHistogramSize);
      int y,x;
      const float factor = histo.lastIdx()/maxGrad;
      // compute the histogram
      for (y=0;y<grad.rows();++y) {
        for (x=0;x<grad.columns();++x) {
          histo.at(iround(factor*grad.at(y,x)))++;
        }
      }

      const int n = grad.rows()*grad.columns();
      histo.at(histo.lastIdx())/=n;
      // compute the accumulative histogram:
      for (x=histo.size()-2;x>=0;--x) {
        histo.at(x)= (histo.at(x)/n) + histo.at(x+1);
      }
     
      x=histo.lastIdx();
      if (param.indirectThresholdMax) {
        while ((x>=0) && (histo.at(x) < param.thresholdMax)) {
          --x;
        };
        thresholdMax = maxGrad*x/histo.lastIdx();
      }
      
      if (param.indirectThresholdMin) {
        while ((x>=0) && (histo.at(x) < param.thresholdMin)) {
          --x;
        };
        thresholdMin = maxGrad*x/histo.lastIdx();
      } else {
        thresholdMin = param.thresholdMin*thresholdMax;        
      }
    }

    if (thresholdMin > thresholdMax) {
      thresholdMin = thresholdMax;
    }

  }

  /*
   * find end points of edges
   */
  int nonMaximaSuppression::findEndPoints(const channel8& src,
                                    channel8& endPts,
                                    pointList& endPtsList) const {

    const parameters& par = getParameters();
    const ubyte background = par.background;

    endPtsList.clear();
    endPts.resize(src.size(),background,false,true);

    if (src.empty()) {
      return 0;
    }

    // delta factors
    static const int dx[] = { 1, 1, 0,-1,-1,-1, 0, 1}; 
    static const int dy[] = { 0,-1,-1,-1, 0, 1, 1, 1}; 

    int x,y,i,n(0);
    int count,lastEdge(0);
    bool isEdge[8];
    bool* boolPtr;

    // the main part, (boundaries will be checked later)
    for (y=1;y<src.lastRow();++y) {
      for (x=1;x<src.lastColumn();++x) {

        if (src.at(y,x) != background) {
          // edge pixel found, check if end point
          count=0;
          boolPtr = &isEdge[0];
          for (i=0;i<8;++i,++boolPtr) {
            // count first how many pixels around this pixel are also edges
            if (src.at(y+dy[i],x+dx[i]) != background) {
              ++count;
              *boolPtr=true;
              lastEdge=i;
            } else {
              *boolPtr=false;
            }
          }

          // if less than two, the pixel is an end point
          if (count < 2) {
            endPts.at(y,x)=par.endPointValue;
            endPtsList.push_back(point(x,y));
            ++n;
          } else if (count == 2) {
            // if only two, is end-point only if both pixels are adjacent.
            if (isEdge[(lastEdge+7)%8] || isEdge[(lastEdge+1)%8]) {
              // yes! End-Point too
              endPts.at(y,x)=par.endPointValue;
              endPtsList.push_back(point(x,y));
              ++n;
            }
          }
        }
      }
    }

    return n;
  }

  inline const point& nonMaximaSuppression::add(const point& a,
                                                const point& b,
                                                const point& size,
                                                point& q) const {
    q.add(a,b);
    if (q.x<0) {
      q.x=0;
    } else if (q.x>=size.x) {
      q.x=size.x-1;
    }
    if (q.y<0) {
      q.y=0;
    } else if (q.y>=size.y) {
      q.y=size.y-1;
    }

    return q;
  }

  /*
   * Track points back the given number of pixels
   *
   * This method is used to track back the end points to get enough
   * information about the possible extrapolations.
   *
   */
  bool nonMaximaSuppression::trackPoints(const channel8& edges,
                                         const pointList& endPtsList,
                                         const channel& orientation,
                                         tpointList<float>& deltas
                                         ) const {

    const parameters& par = getParameters();
    deltas.clear();
    imatrix tracked(edges.size(),int(0));

    // robustness issue: add 8 elements for "negative" access
    static const point ld[]={point( 0, 1),
                             point(-1, 1),
                             point(-1, 0),
                             point(-1,-1),
                             point( 0,-1),
                             point( 1,-1),
                             point( 1, 0),
                             point( 1, 1),

                             point( 0, 1),
                             point(-1, 1),
                             point(-1, 0),
                             point(-1,-1),
                             point( 0,-1),
                             point( 1,-1),
                             point( 1, 0),
                             point( 1, 1),

                             point( 0, 1),
                             point(-1, 1)};

    static const point *const d = &ld[8];
    static const float factor = static_cast<float>(4.0/Pi);

    int i;
    int label(1);
    point p,q;
    const point size(edges.size());
    float angle(0),tmp;
    int idx,sc,n;
    pointList::const_iterator it,eit;
    tpoint<double> sum(0.0,0.0),sum2(0.0,0.0);
    double sumxy(0.0),a;
    float cosa,sina,rho1,rho2,cosb(0),sinb,beta;
    double e1,e2;
    bool consider;
    
    for (it=endPtsList.begin(),eit=endPtsList.end();
         it!=eit;
         ++it) {
      
      p = (*it);
      tracked.at(p)=label;

      sum.castFrom(p);
      sum2.set(p.x*p.x,p.y*p.y);
      sumxy=p.x*p.y;
      n=1;
      consider=true;

      _lti_debug2("--------------------------\nEnd Point: " << p << std::endl);

      for (i=1;i<par.numGapHints;++i) {

        angle = orientation.at(p);
        tmp = angle*factor;
        idx = iround(tmp);
        if (static_cast<float>(idx) > tmp) {
          sc=7;
        } else {
          sc=1;
        }

        if ((edges.at(add(p,d[idx],size,q)) == par.edgeValue) &&
            (tracked.at(q) != label)) {
          tracked.at(q) = label;
        } else if ((edges.at(add(p,d[((idx+4)%8)],size,q)) ==
                    par.edgeValue) &&
                   (tracked.at(q) != label)) {
          tracked.at(q) = label;
        } else if ((edges.at(add(p,d[(idx+sc)%8],size,q)) ==
                    par.edgeValue) &&
                   (tracked.at(q) != label)) {
          tracked.at(q) = label;
        } else if ((edges.at(add(p,d[(idx+sc+4)%8],size,q)) == 
                    par.edgeValue)&&
                   (tracked.at(q) != label)) {
          tracked.at(q) = label;
        } else if ((edges.at(add(p,d[(idx+8-sc)%8],size,q)) ==
                    par.edgeValue)&&
                   (tracked.at(q) != label)) {
          tracked.at(q) = label;
        } else if ((edges.at(add(p,d[(idx+12-sc)%8],size,q)) ==
                    par.edgeValue) &&
                   (tracked.at(q) != label)) {
          tracked.at(q) = label;
        } else {
          // seems there is no follower any more (single point?)
          i=par.numGapHints; // flag to indicate end!
          consider=false;
        }
        p=q;

        _lti_debug2(q << " ");

        if (consider) {
          sum.x+=p.x;
          sum.y+=p.y;
          sum2.x+=(p.x*p.x);
          sum2.y+=(p.y*p.y);
          sumxy+=(p.x*p.y);
          ++n;
        }
      }

      _lti_debug2(std::endl);

      if (n>1) {
        // compute statistics:
        a = 2.0*(sum.x*sum.y - n*sumxy) / 
          (n*(sum2.y-sum2.x) - (sum.y*sum.y - sum.x*sum.x));
        angle = static_cast<float>(0.5*atan(a));
      } 

      // the line parameters:
      sincos(angle, sina, cosa);
      
      // first try: angle
      rho1 = static_cast<float>((sum.x*cosa + sum.y*sina)/n);
      e1 = cosa*cosa*(sum2.x-sum2.y) -n*rho1*rho1 +2*sina*cosa*sumxy +sum2.y;

      // second try: angle + Pi/2
      rho2 = static_cast<float>((-sum.x*sina + sum.y*cosa)/n);
      e2 = sina*sina*(sum2.x-sum2.y) -n*rho2*rho2 -2*sina*cosa*sumxy +sum2.y;
      
      if (e2 < e1) {
        angle += constants<float>::HalfPi();
        tmp  = sina;
        sina = cosa;
        cosa = -tmp;
      } 

      // now decide in which of both directions should it go?
      // to do it, check if the proyection on the unit vector (cosb,sinb) 
      // of the mean value is positive, in which case we are on the wrong side.
      beta = constants<float>::HalfPi() + angle;
      sinb = cosa;
      cosb = -sina;

      if ((((sum.x/n)-(*it).x)*cosb + ((sum.y/n)-(*it).y)*sinb) > 0.0f) {
        sinb = -sinb;
        cosb = -cosb;
      }

      // normalize
      tmp = max(abs(sinb),abs(cosb));
      // remember the left-handed coordinate system! (beta -> -beta)
      deltas.push_back(tpoint<float>(cosb/tmp,sinb/tmp));

      _lti_debug2("    -> delta=(" << cosb/tmp << "," << sinb/tmp << ")" <<
                  std::endl);

      ++label;
    }

    return true;
  }

  bool nonMaximaSuppression::fillGaps(const channel8& edges,
                                      const channel& gradMag,
                                      const pointList& endPtsList,
                                      const tpointList<float>& deltas,
                                      channel8& dest) const {
    // the first coding means:
    // 0. background
    // 1. edge
    const parameters& par = getParameters();
    int x,y;
    float maxGrad(0.0f);
    dest.resize(edges.size(),0,false,false);
    for (y=0;y<dest.rows();++y) {
      for (x=0;x<dest.columns();++x) {
        dest.at(y,x) = (edges.at(y,x) == par.background) ? 0 : 1; 
        maxGrad = max(maxGrad,gradMag.at(y,x));
      }
    }
    
    // we need the end points sorted from max gradient to min gradient, because
    // most probably, the gradient magnitudes were not continued due to a
    // shadowing of a neighbor edge.
    const int size = endPtsList.size();
    vector<point> endPts;
    vector< tpoint<float> > dvct;
    ivector idx;
    vector<float> keys;

    endPts.resize(size,point(),false,false);
    dvct.resize(size,tpoint<float>(),false,false);
    idx.resize(size,0,false,false);
    keys.resize(size,0.0f,false,false);

    int i,j,ix,k;
    pointList::const_iterator it,eit;
    tpointList<float>::const_iterator fit;

    for (it=endPtsList.begin(),eit=endPtsList.end(),fit=deltas.begin(),i=0;
         it!=eit;
         ++it,++fit,++i) {
      endPts.at(i)=(*it);
      dvct.at(i)=(*fit);
      
      idx.at(i)=i;
      keys.at(i)=gradMag.at(*it);
    }

    //
    sort2<float,int> sorter(true);
    sorter.apply(keys,idx);

    point p,q;
    tpoint<float> pf;
    const float thresh = maxGrad*par.thresholdMax*par.thresholdMin;
//     const float thresh = maxGrad*par.thresholdMax;
//     const float thresh = maxGrad*par.thresholdMax*(1.0f+par.thresholdMin)*0.5f;

    for (i=0;i<size;++i) {
      ix = idx.at(i);
      p=endPts.at(ix);
      pf.castFrom(p);
      const tpoint<float>& delta = dvct.at(ix);
      j=1;

      while(j<par.maxGapLength) {
        pf.add(delta);
        q.set(iround(pf.x),iround(pf.y));
        if ( (static_cast<unsigned int>(q.x) < 
              static_cast<unsigned int>(dest.columns())) &&
             (static_cast<unsigned int>(q.y) < 
              static_cast<unsigned int>(dest.rows())) &&
             (gradMag.at(q) > thresh) ) {

          if (dest.at(q) < 255) {
            k = (++dest.at(q));
            if (k>1) {
              j=par.maxGapLength;
            }
          }
          
          ++j;
        } else {
          j=par.maxGapLength;
        }
      }
    }

    // use the final labels;
    for (y=0;y<dest.rows();++y) {
      for (x=0;x<dest.columns();++x) {
        ubyte& px = dest.at(y,x);
        if (px == 0) {
          px = par.background;
        } else if (edges.at(y,x) != par.background) {
          px = par.edgeValue;
        } else {
          px = par.gapValue;
        }
      }
    }
    
    for (i=0;i<size;++i) {
      dest.at(endPts.at(i)) = par.endPointValue;
    }

    return true;
  }
  
}
