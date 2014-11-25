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
 * file .......: ltiHsvHistogramViewer.cpp
 * authors ....: Joerg Zieren
 * organization: LTI, RWTH Aachen
 * creation ...: 19.7.2002
 * revisions ..: $Id: ltiHsvHistogramViewer.cpp,v 1.8 2006/09/05 10:43:30 ltilib Exp $
 */

#include "ltiHsvHistogramViewer.h"

#ifdef HAVE_GTK

#include "ltiImage.h"

namespace lti {
  // --------------------------------------------------
  // hsvHistogramViewer::parameters
  // --------------------------------------------------

  // default constructor
  hsvHistogramViewer::parameters::parameters()
    : functor::parameters() {
    scale = 10;
  }

  // copy constructor
  hsvHistogramViewer::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  hsvHistogramViewer::parameters::~parameters() {
  }

  // get type name
  const char* hsvHistogramViewer::parameters::getTypeName() const {
    return "hsvHistogramViewer::parameters";
  }

  // copy member

  hsvHistogramViewer::parameters&
    hsvHistogramViewer::parameters::copy(const parameters& other) {
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


      scale = other.scale;

    return *this;
  }

  // alias for copy member
  hsvHistogramViewer::parameters&
    hsvHistogramViewer::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* hsvHistogramViewer::parameters::clone() const {
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
  bool hsvHistogramViewer::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool hsvHistogramViewer::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"scale",scale);
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
  bool hsvHistogramViewer::parameters::write(ioHandler& handler,
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
  bool hsvHistogramViewer::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool hsvHistogramViewer::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"scale",scale);
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
  bool hsvHistogramViewer::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // hsvHistogramViewer
  // --------------------------------------------------

  // default constructor
  hsvHistogramViewer::hsvHistogramViewer()
    : functor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // constructor with window title
  hsvHistogramViewer::hsvHistogramViewer(const std::string& windowTitle)
    : functor(), m_viewer(lti::viewer(windowTitle.c_str())) {
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  hsvHistogramViewer::hsvHistogramViewer(const hsvHistogramViewer& other)
    : functor()  {
    copy(other);
  }

  // destructor
  hsvHistogramViewer::~hsvHistogramViewer() {
  }

  // returns the name of this type
  const char* hsvHistogramViewer::getTypeName() const {
    return "hsvHistogramViewer";
  }

  // copy member
  hsvHistogramViewer&
    hsvHistogramViewer::copy(const hsvHistogramViewer& other) {
      functor::copy(other);
    return (*this);
  }

  // alias for copy member
  hsvHistogramViewer&
    hsvHistogramViewer::operator=(const hsvHistogramViewer& other) {
    return (copy(other));
  }


  // clone member
  functor* hsvHistogramViewer::clone() const {
    return new hsvHistogramViewer(*this);
  }

  // return parameters
  const hsvHistogramViewer::parameters&
    hsvHistogramViewer::getParameters() const {
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


  // On place apply for type thistogram<double>!
  bool hsvHistogramViewer::apply(thistogram<double>& hist) const {
    if (hist.dimensions() != 3)
      return false;
    // const parameters& params = getParameters();
    const int hCells = hist.cellsInDimension(0);
    const int sCells = hist.cellsInDimension(1);
    const int vCells = hist.cellsInDimension(2);
    // map 3D histogram to 2D
    histogram2D hist2D(sCells,hCells);
    ivector hsv(3);
    for (hsv[0] = 0; hsv[0] < hCells; ++hsv[0]) {
      for (hsv[1] = 0; hsv[1] < sCells; ++hsv[1]) {
        // sum up all "value" values
        for (hsv[2] = 0; hsv[2] < vCells; ++hsv[2]) {
          // beware parameter order: put(y,x,...)
          hist2D.put(hsv[1],hsv[0],hist.at(hsv));
        }
      }
    }
    return apply(hist2D);
  };

  // On place apply for type histogram2D!
  bool hsvHistogramViewer::apply(histogram2D& hist) const {
    const parameters& params = getParameters();
    const double max = hist.maximum();
    const int hCells = hist.getLastCell().x+1;
    const int sCells = hist.getLastCell().y+1;
    image histView(hCells*params.scale,sCells*params.scale,lti::Black);

    int h, s;
    for (h = 0; h < hCells; ++h) {
      for (s = 0; s < sCells; ++s) {
        fillCell(
          histView,
          getColor(h,hCells,s,sCells),
          h*params.scale,
          s*params.scale,
          params.scale,
          hist.at(s,h)/max);
      }
    }

    m_viewer.show(histView);
    return true;
  };


  void hsvHistogramViewer::fillCell(
    image& img,
    const rgbPixel& color,
    const int& fromRow,
    const int& fromCol,
    const int& sideLength,
    float percent) const {
    // define rectangle to be filled
    rectangle rect(fromCol,fromRow,fromCol+sideLength-1,fromRow+sideLength-1);
    // get center
    point c = rect.getCenter();
    // limit percent
    percent = min(1.0f,percent);
    // compute number of pixels to fill
    int pixelsToFill = int(rect.getArea()*percent + 0.5f);
    // anything to do?
    if (pixelsToFill == 0)
      return;

    if (sideLength % 2) { // odd side length makes a special case: 1st pixel
      img.at(c) = color;
      pixelsToFill--;
    }

    // create 4 rays
    ray* r[4];
    if (sideLength % 2) { // odd side length
      r[0] = new ray(point(c.x-1,c.y),point(+1,-1),point(-1,0),1);
      r[1] = new ray(point(c.x,c.y-1),point(+1,+1),point(0,-1),1);
      r[2] = new ray(point(c.x+1,c.y),point(-1,+1),point(1,0),1);
      r[3] = new ray(point(c.x,c.y+1),point(-1,-1),point(0,1),1);
    } else { // even side length
      r[0] = new ray(point(c.x,c.y),    point(+1,-1),point(-1,0),1);
      r[1] = new ray(point(c.x+1,c.y),  point(+1,+1),point(0,-1),1);
      r[2] = new ray(point(c.x+1,c.y+1),point(-1,+1),point(1,0),1);
      r[3] = new ray(point(c.x,c.y+1),  point(-1,-1),point(0,1),1);
    }
    // index for ray to increment (cycles from 0-3)
    int pi = 0;

    point p;
    for (; pixelsToFill > 0; --pixelsToFill) {
      do {
        r[pi]->getNextPoint(p);
      } while (!rect.isInside(p)); // proceed until inside rect
      // (this loop is never infinite, because each ray fills the same percentage
      // of the square, and each one is drawn in turn.)
      img.at(p) = color;
      pi = (pi+1)%4;
    }

    // destroy rays
    for (int i = 0; i < 4; ++i)
      delete r[i];
  }

  rgbPixel hsvHistogramViewer::getColor(
    const int&h, const int& hValues,
    const int&s, const int& sValues) const {
    rgbPixel p;
    m_hsvToRgb.apply(
      (float(h)+0.5f)/float(hValues),
      (float(s)+0.5f)/float(sValues),
      1.0f,
      p);
    return p;
  }


  hsvHistogramViewer::ray::ray(
    const point& start,
    const point& direction,
    const point& nextStartOffset,
    const int& steps) {
    m_start = start;
    m_cursor = start;
    m_direction = direction;
    m_nextStartOffset = nextStartOffset;
    m_steps = steps;
    m_stepsLeft = steps;
  }

  void hsvHistogramViewer::ray::getNextPoint(point& p) {
    if (m_stepsLeft > 0) {
      p = m_cursor;
      m_cursor += m_direction;
      --m_stepsLeft;
    } else {
      m_start += m_nextStartOffset;
      m_cursor = m_start;
      p = m_cursor;
      m_cursor += m_direction;
      ++m_steps; // one more pixel each time
      m_stepsLeft = m_steps-1;
    }
  }
}

#endif
