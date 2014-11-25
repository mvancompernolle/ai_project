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
 * file .......: ltiRegionsPolygonizer.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 1.10.2003
 * revisions ..: $Id: ltiRegionsPolygonizer.cpp,v 1.8 2006/09/05 10:28:59 ltilib Exp $
 */

#include "ltiRegionsPolygonizer.h"
#include <vector>

#undef _LTI_DEBUG 
// #define _LTI_DEBUG 4
#include "ltiDebug.h"


#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#include "ltiDraw.h"
#endif

namespace lti {
  // --------------------------------------------------
  // regionsPolygonizer::parameters
  // --------------------------------------------------

  // default constructor
  regionsPolygonizer::parameters::parameters()
    : functor::parameters() {
    
    maxPixelDistance = 1;
    detectNeighbors = true;
  }

  // copy constructor
  regionsPolygonizer::parameters::parameters(const parameters& other)
    : functor::parameters() {
    copy(other);
  }

  // destructor
  regionsPolygonizer::parameters::~parameters() {
  }

  // get type name
  const char* regionsPolygonizer::parameters::getTypeName() const {
    return "regionsPolygonizer::parameters";
  }

  // copy member

  regionsPolygonizer::parameters&
    regionsPolygonizer::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif

    
    maxPixelDistance = other.maxPixelDistance;
    detectNeighbors = other.detectNeighbors;

    return *this;
  }

  // alias for copy member
  regionsPolygonizer::parameters&
  regionsPolygonizer::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* regionsPolygonizer::parameters::clone() const {
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
  bool regionsPolygonizer::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool regionsPolygonizer::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {      
      lti::write(handler,"maxPixelDistance",maxPixelDistance);
      lti::write(handler,"detectNeighbors",detectNeighbors);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool regionsPolygonizer::parameters::write(ioHandler& handler,
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
  bool regionsPolygonizer::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool regionsPolygonizer::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      
      lti::read(handler,"maxPixelDistance",maxPixelDistance);
      lti::read(handler,"detectNeighbors",detectNeighbors);      
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool regionsPolygonizer::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // regionsPolygonizer
  // --------------------------------------------------

  // default constructor
  regionsPolygonizer::regionsPolygonizer()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  regionsPolygonizer::regionsPolygonizer(const parameters& par)
    : functor() {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  regionsPolygonizer::regionsPolygonizer(const regionsPolygonizer& other)
    : functor() {
    copy(other);
  }

  // destructor
  regionsPolygonizer::~regionsPolygonizer() {
  }

  // returns the name of this type
  const char* regionsPolygonizer::getTypeName() const {
    return "regionsPolygonizer";
  }

  // copy member
  regionsPolygonizer&
  regionsPolygonizer::copy(const regionsPolygonizer& other) {

    functor::copy(other);
    return (*this);
  }

  // alias for copy member
  regionsPolygonizer&
  regionsPolygonizer::operator=(const regionsPolygonizer& other) {
    return (copy(other));
  }


  // clone member
  functor* regionsPolygonizer::clone() const {
    return new regionsPolygonizer(*this);
  }

  // return parameters
  const regionsPolygonizer::parameters&
  regionsPolygonizer::getParameters() const {
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

  bool regionsPolygonizer::apply(const matrix<int>& src,
                                 const int maxLabel,
                                 std::vector<polygonPoints>& polygons) const {

    std::vector<borderPoints> borders;
    imatrix neighbors;
    return apply(src,maxLabel,polygons,borders,neighbors);
  }
  
  // On place apply for type matrix<int>!
  bool regionsPolygonizer::apply(const matrix<int>& src,
                                 const int maxLabel,
                                 std::vector<polygonPoints>& polygons,
                                 std::vector<borderPoints>& borders,
                                 matrix<int>& neighbors) const {

    if (src.empty()) {
      polygons.clear();
      borders.clear();
      neighbors.clear();
      return true;
    }

#   if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 4)
    image canvas(src.size(),Black);
    draw<rgbPixel> drawer;
    drawer.use(canvas);
    static lti::viewer view("polygonizer");
#   endif
    
    const parameters& param = getParameters();
    const int size = maxLabel+1;
    std::vector<areaPoints> areaPts(size);
    neighbors.copy(src);

    // compute area points
    point p;
    int i;
    for (p.y=0;p.y<src.rows();++p.y) {
      for (p.x=0;p.x<src.columns();++p.x) {
        i = src.at(p);
        assert(i<size); // at least in debug should note an error!
        areaPts[i].push_back(p);
      }
    }

    // transform all area points in border points
    borderPoints::const_iterator bit,lit;
    pointList forcedVertices;
    polygons.resize(size);
    borders.resize(size);

    // transform each border point in a polygon point
    for (i=0;i<size;++i) {

      forcedVertices.clear();
      polygons[i].clear();
      borders[i].castFrom(areaPts[i]);

      borderPoints& borderPts = borders[i];

      // try to get the first and second border points
      lit=borderPts.begin();
      if (lit == borderPts.end()) {
        // emtpy border points => empty polygon
        continue;
      }

      bit=borderPts.begin();
      ++bit;

      if (bit == borderPts.end()) {
        // only one border point => same thing as polygon
        polygons[i].push_back(*lit);
        continue;
      }

      // check if border is clockwise or counter-clockwise
      const int regionLabel = i;
      int currentLabel(-1),tmpLabel;

      chainCode dir(*lit,*bit); // get "from here to there"'s chain code
      chainCode tmpcc;
      chainCode turn(chainCode::N); // default turn: right

      point p;
      tmpcc = dir + chainCode::N;
      p=tmpcc.getNext(*lit);
      if ((static_cast<uint32>(p.x) < static_cast<uint32>(src.columns()))&&
          (static_cast<uint32>(p.y) < static_cast<uint32>(src.rows())) &&
          (src.at(p) == regionLabel)) {
        turn = chainCode::S;
      }

      // get current label, i.e. the label of the neighbor region
      p = (dir+turn).getNext(*lit);
      if ((static_cast<uint32>(p.x) < static_cast<uint32>(src.columns()))&&
          (static_cast<uint32>(p.y) < static_cast<uint32>(src.rows()))) {
        currentLabel = src.at(p);
      } else {
        currentLabel = regionLabel;
      }

      // now follow the border
      while (bit != borderPts.end()) {
        dir = chainCode(*lit,*bit);
        p = (dir+turn).getNext(*lit);
        if ((static_cast<uint32>(p.x)<static_cast<uint32>(src.columns()))&&
            (static_cast<uint32>(p.y)<static_cast<uint32>(src.rows()))) {
          tmpLabel = src.at(p);
          if ((tmpLabel != currentLabel) &&
              (tmpLabel != regionLabel)) {
            // region has changed!  Push the border point
            forcedVertices.push_back(*bit);
            forcedVertices.push_back(*lit);
            neighbors.at(*lit)=currentLabel;
            neighbors.at(*bit)=tmpLabel;
            currentLabel = tmpLabel;
          } else {
            neighbors.at(*bit)=currentLabel;            
          }
        }
        
        ++lit;
        ++bit;
      }

      if (param.detectNeighbors) {
        polygons[i].approximate(borderPts,forcedVertices,
                                -1,param.maxPixelDistance);
      }
      else {
        polygons[i].approximate(borderPts, 
                                -1,param.maxPixelDistance);
      }

#     if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 4)
      drawer.setColor(rgbPixel(92,92,92));
      drawer.set(areaPts[i]);
      
      drawer.setColor(rgbPixel(92,128,255));
      drawer.set(borderPts);
      
      drawer.setColor(rgbPixel(255,255,0));
      drawer.set(polygons[i]);

      view.show(canvas);
      view.waitKey();
#     endif

    }
    
    return true;
  }             
}
