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
 * file .......: ltiQuadTreeSegmentation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 15.12.2003
 * revisions ..: $Id: ltiQuadTreeSegmentation.cpp,v 1.7 2006/09/05 10:27:20 ltilib Exp $
 */

#include "ltiMacroSymbols.h"

// only for compilers different than VC++ 6.0 available
#ifndef _LTI_MSC_6

#include "ltiRGBPixel.h"
#include "ltiRectangle.h"
#include "ltiQuadTreeSegmentation.h"
#include "ltiRegionGraphMeans.h"
#include "ltiComputePalette.h"
#include "ltiUsePalette.h"
#include "ltiTree.h"

namespace lti {
  // --------------------------------------------------
  // quadTreeSegmentation::parameters
  // --------------------------------------------------

  // default constructor
  quadTreeSegmentation::parameters::parameters()
    : segmentation::parameters() {
    
    splitThreshold = 5.0f;
    mergeThreshold = 3.0f;
    minRegionSize  = point(3,3);
  }

  // copy constructor
  quadTreeSegmentation::parameters::parameters(const parameters& other)
    : segmentation::parameters() {
    copy(other);
  }

  // destructor
  quadTreeSegmentation::parameters::~parameters() {
  }

  // get type name
  const char* quadTreeSegmentation::parameters::getTypeName() const {
    return "quadTreeSegmentation::parameters";
  }

  // copy member

  quadTreeSegmentation::parameters&
  quadTreeSegmentation::parameters::copy(const parameters& other) {
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
   
    splitThreshold = other.splitThreshold;
    mergeThreshold = other.mergeThreshold;
    minRegionSize  = other.minRegionSize;

    return *this;
  }

  // alias for copy member
  quadTreeSegmentation::parameters&
  quadTreeSegmentation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* quadTreeSegmentation::parameters::clone() const {
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
  bool quadTreeSegmentation::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool quadTreeSegmentation::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {      
      lti::write(handler,"splitThreshold",splitThreshold);
      lti::write(handler,"mergeThreshold",mergeThreshold);
      lti::write(handler,"minRegionSize",minRegionSize);
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
  bool quadTreeSegmentation::parameters::write(ioHandler& handler,
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
  bool quadTreeSegmentation::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool quadTreeSegmentation::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"splitThreshold",splitThreshold);
      lti::read(handler,"mergeThreshold",mergeThreshold);
      lti::read(handler,"minRegionSize",minRegionSize);
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
  bool quadTreeSegmentation::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // quadTreeSegmentation
  // --------------------------------------------------

  // default constructor
  quadTreeSegmentation::quadTreeSegmentation()
    : segmentation(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  quadTreeSegmentation::quadTreeSegmentation(const parameters& par)
    : segmentation() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  quadTreeSegmentation::quadTreeSegmentation(const quadTreeSegmentation& other)
    : segmentation() {
    copy(other);
  }

  // destructor
  quadTreeSegmentation::~quadTreeSegmentation() {
  }

  // returns the name of this type
  const char* quadTreeSegmentation::getTypeName() const {
    return "quadTreeSegmentation";
  }

  // copy member
  quadTreeSegmentation&
  quadTreeSegmentation::copy(const quadTreeSegmentation& other) {
    segmentation::copy(other);
    return (*this);
  }

  // alias for copy member
  quadTreeSegmentation&
    quadTreeSegmentation::operator=(const quadTreeSegmentation& other) {
    return (copy(other));
  }


  // clone member
  functor* quadTreeSegmentation::clone() const {
    return new quadTreeSegmentation(*this);
  }

  // return parameters
  const quadTreeSegmentation::parameters&
    quadTreeSegmentation::getParameters() const {
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

  
  // On place apply for type image!
  bool quadTreeSegmentation::apply(const image& src,image& dest) const {
    imatrix mask;
    palette pal;

    apply(src,mask,pal);
    usePalette paletter;
    paletter.apply(mask,pal,dest);

    return true;
  };

  // On copy apply for type image!
  bool quadTreeSegmentation::apply(const image& src,
                                   imatrix& mask,
                                   palette& pal) const {
    
    split(src,mask);
    regionGraphColorMeans::parameters rgcmp;
    rgcmp.mergeMode = regionGraphColorMeans::parameters::Optimal;
    rgcmp.mergeThreshold = getParameters().mergeThreshold/255.0f;
    regionGraphColorMeans rgcm(rgcmp);
    regionGraphColorMeans::graph_type graph;

    // generate the graph
    rgcm.apply(mask,src,0,graph);
    
    // merge 
    ivector equivs;
    rgcm.apply(graph,equivs);
    
    rgcm.reassignLabels(equivs,mask,true);

    computePalette paletter;
    paletter.apply(src,mask,pal);

    return true;
  };

  bool quadTreeSegmentation::split(const image& img,imatrix& mask) const {
    // degree of the tree is four (_quad_ tree)
    tree<rectangle> quadTree(4);
    
    // set the root as the whole image
    quadTree.pushRoot(rectangle(point(0,0),
                                point(img.lastColumn(),img.lastRow())));
    
    const parameters& par = getParameters();

    // generate the quad-tree
    split(img,par.splitThreshold,par.minRegionSize,quadTree.root());

    // fill the mask with the corresponding labels
    int label = 1;

    mask.resize(img.size(),0,false,true);

    tree<rectangle>::const_iterator it;
    for (it=quadTree.begin();it!=quadTree.end();++it) {
      if ((*it).numberOfChildren() == 0) {
        mask.fill(label,(*it).getData());
        label++;
      }
    }
    
    return true;
  }
  
  bool quadTreeSegmentation::split(const image& img,
                                   const float& threshold,
                                   const point& minSize,
                                   tree<rectangle>::node& qtree) const {
    trgbPixel<float> stat;
    const point size = qtree.getData().getDimensions();
    if ((size.x     >= minSize.x) &&
        (size.y     >= minSize.y)) {

      stats(img,qtree.getData().ul,qtree.getData().br,stat);

      if ((stat.red   > threshold) ||
          (stat.green > threshold) ||
          (stat.blue  > threshold)) {
        const rectangle& r = qtree.getData();
        const point c((r.ul + r.br)/2);
        
        // recursively split the sub-images
        split(img,threshold,minSize,qtree.insertChild(rectangle(r.ul,c)));
        split(img,threshold,minSize,qtree.insertChild(rectangle(c.x+1,r.ul.y,
                                                                r.br.x,c.y)));
        split(img,threshold,minSize,qtree.insertChild(rectangle(r.ul.x,c.y+1,
                                                                c.x,r.br.y)));
        split(img,threshold,minSize,qtree.insertChild(rectangle(c.x+1,c.y+1,
                                                                r.br.x,
                                                                r.br.y)));
        return true;
      }
    }

    return false; // no split
  }
  

  bool quadTreeSegmentation::stats(const image& img,
                                   const point& from,
                                   const point& to,
                                   trgbPixel<float>& stats) const {
    trgbPixel<float> sum(0.0f,0.0f,0.0f);
    trgbPixel<float> sum2(0.0f,0.0f,0.0f);

    int y,x,n;
    n=0;
    for (y=from.y;y<=to.y;++y) {
      for (x=from.x;x<to.x;++x) {
        trgbPixel<float> px(img.at(y,x));
        sum+=px;
        sum2+=px*px;
        n++;
      }
    }
    
    if (n==0) {
      stats.set(0.0f,0.0f,0.0f);
    } else {
      sum2.divide(static_cast<float>(n));
      sum.divide(static_cast<float>(n));
      stats = sum2 - (sum*sum);
      stats.apply(sqrt);
    }
    return true;
  }
}

#endif
