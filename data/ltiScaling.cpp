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


/*--------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiScaling.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH-Aachen
 * creation ...: 21.11.2003
 * revisions ..: $Id: ltiScaling.cpp,v 1.12 2006/09/05 10:30:03 ltilib Exp $
 */

#include "ltiScaling.h"
#include "ltiBilinearInterpolator.h"
#include "ltiBiquadraticInterpolator.h"
#include "ltiGenericInterpolator.h"
#include "ltiMath.h"
#include <limits>

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#include "ltiTimer.h"
#endif

namespace lti {

  // ------------------------------------------------------------------------
  // scaling helper
  // ------------------------------------------------------------------------
  //
  // This is a very private helper class, which does the real work.  It is
  // not done as internal class of scaling, because of Visual C++.
  //

  /**
   * Scaling helper class
   */
  template<class T>
  class scalingHelper {
  public:
    /**
     * Construct a helper
     */
    scalingHelper(const eInterpolatorType type);

    /**
     * scale the matrix<T>
     */
    bool apply(const matrix<T>& src, matrix<T>& dst,
               const tpoint<float>& scale) const;

    /**
     * scale the matrix<T>
     */
    bool apply(const matrix<T>& src, matrix<T>& dst,
               const float scale) const;

  protected:
    /**
     * Interpolator type to be used
     */
    eInterpolatorType interpolator;

    /**
     * This functor uses a LUT and to save time it should be allocated
     * only once at construction
     */
    genericInterpolator<T> genericIpl;

    /**
     * Biquadratic interpolation.
     * The dest image must be resized previously
     */
    bool bicubicScale(const matrix<T>& src,matrix<T>& dest) const;

    /**
     * Biquadratic interpolation.
     * The dest image must be resized previously
     */
    bool biquadraticScale(const matrix<T>& src,matrix<T>& dest) const;

    /**
     * Bilinear interpolation. 
     * The dest image must be resized previously
     */
    bool bilinearScale(const matrix<T>& src,matrix<T>& dest) const;

    /**
     * Nearest neighbor interpolation.
     * The dest image must be resized previously
     */
    bool nearestNeighborScale(const matrix<T>& src,matrix<T>& dest) const;

  };



  template<class T>
  bool scalingHelper<T>::bicubicScale(const matrix<T>& src,
                                      matrix<T>& dest) const {

    int x,y,ix,iy;
    float fx,fy,rx,ry;
    const float dx = static_cast<float>(src.lastColumn())/dest.lastColumn();
    const float dy = static_cast<float>(src.lastRow())/dest.lastRow();
    
    // the interpolator used is the member genericIpl.
    
    if ((src.rows() < 4) || (src.columns() < 4)) {
      // the source is too small for a bicubic kernel to pass
      // completely therein, so we need to use the expensive methods!
      for (fy=0.0,y=0;y<dest.lastRow();++y,fy+=dy) {
        for (fx=0.0,x=0;x<dest.lastColumn();++x,fx+=dx) {
          dest.at(y,x) = genericIpl.interpolate(src,fy,fx);
        }
      }

      return true;
    }

    // The interpolation can be computed efficiently only in the middle.
    // The borders require special consideration to compute the boundary
    // conditions.

    // first compute the limits of the valid middle region, where the
    // kernel fits completely

    const int firstCol = iround(ceil(1.0f/dx));
    const int firstRow = iround(ceil(1.0f/dy));

    const int lastCol = static_cast<int>((src.columns() - 2.0f)/dx);
    const int lastRow = static_cast<int>((src.rows() - 2.0f)/dy);

    const T* rows[4];

    // top
    for (fy=0.0f,y=0;y<firstRow;++y,fy+=dy) {
      for (fx=0.0f,x=0;x<dest.columns();++x,fx+=dx) {
        dest.at(y,x) = genericIpl.interpolate(src,fy,fx);
      }
    }

    // vertical central block has three parts: left, center and right 

    for (;y<lastRow;++y,fy+=dy) {

      // left
      for (fx=0.0f,x=0;x<firstCol;++x,fx+=dx) {
        dest.at(y,x) = genericIpl.interpolate(src,fy,fx);
      }

      // center
      
      // initialize the y coordinates for the whole line
      iy = static_cast<int>(fy);
      ry = fy-static_cast<float>(iy);

      // the line itself (without borders)
      for (;x<lastCol;++x,fx+=dx) {
        ix = static_cast<int>(fx);
        rx = fx-static_cast<float>(ix);
        --ix;

        rows[0] = &src.at(iy-1,ix);
        rows[1] = &src.at(iy  ,ix);
        rows[2] = &src.at(iy+1,ix);
        rows[3] = &src.at(iy+2,ix);

        dest.at(y,x) = genericIpl.compute(ry,rx,rows);
      }

      // right
      for (;x<dest.columns();++x,fx+=dx) {
        dest.at(y,x) = genericIpl.interpolate(src,fy,fx);
      }
    }

    // bottom
    for (;y<dest.rows();++y,fy+=dy) {
      for (fx=0.0f,x=0;x<dest.columns();++x,fx+=dx) {
        dest.at(y,x) = genericIpl.interpolate(src,fy,fx);
      }
    }

    return true;
    
  }


  template<class T>
  bool scalingHelper<T>::biquadraticScale(const matrix<T>& src,
                                                matrix<T>& dest) const {

    int x,y,ix,iy,ix1,ix2;
    float fx,fy,rx,ry;
    const float dx = static_cast<float>(src.lastColumn())/dest.lastColumn();
    const float dy = static_cast<float>(src.lastRow())/dest.lastRow();

    // the interpolator
    biquadraticInterpolator<T> biquadIpl(Constant);

    if ((src.columns() < 3) || (dest.columns() < 3)) {
      // the source is too small for a biquadratic kernel to pass
      // completely therein, so we need to use the expensive methods!
      for (fy=0.0,y=0;y<dest.lastRow();++y,fy+=dy) {
        for (fx=0.0,x=0;x<dest.lastColumn();++x,fx+=dx) {
          dest.at(y,x) = biquadIpl.interpolate(src,fy,fx);
        }
      }

      return true;
    }

    const int firstCol = iround(ceil(1.0f/dx));
    const int firstRow = iround(ceil(1.0f/dy));

    const int lastCol = static_cast<int>((src.columns() - 1.5f)/dx);
    const int lastRow = static_cast<int>((src.rows() - 1.5f)/dy);
    
    // top
    for (fy=0.0f,y=0;y<firstRow;++y,fy+=dy) {
      for (fx=0.0f,x=0;x<dest.columns();++x,fx+=dx) {
        dest.at(y,x) = biquadIpl.interpolate(src,fy,fx);
      }
    }

    // the main block

    for (;y<lastRow;++y,fy+=dy) {
      // left
      for (fx=0.0f,x=0;x<firstCol;++x,fx+=dx) {
        dest.at(y,x) = biquadIpl.interpolate(src,fy,fx);
      }
      
      // center

      // initialize the y coordinates for the whole line
      iy = static_cast<int>(fy);
      ry = fy-static_cast<float>(iy);
      if (ry <= 0.5f) {
        --iy;
      } else {
        ry-=1.0f;
      }

      const T *const row0 = &src.at(iy,0);
      const T *const row1 = &src.at(iy+1,0);
      const T *const row2 = &src.at(iy+2,0);

      // the line itself (without borders)
      for (;x<lastCol;++x,fx+=dx) {
        ix = static_cast<int>(fx);
        rx = fx-static_cast<float>(ix);
        if (rx <= 0.5f) {
          --ix;
        } else {
          rx-=1.0f;
        }

        ix1=ix+1;
        ix2=ix+2;

        dest.at(y,x) = biquadIpl.compute(ry,rx,
                                         row0[ix],row0[ix1],row0[ix2],
                                         row1[ix],row1[ix1],row1[ix2],
                                         row2[ix],row2[ix1],row2[ix2]);
      }

      // right
      for (;x<dest.columns();++x,fx+=dx) {
        dest.at(y,x) = biquadIpl.interpolate(src,fy,fx);
      }
    }

    // bottom
    for (;y<dest.rows();++y,fy+=dy) {
      for (fx=0.0f,x=0;x<dest.columns();++x,fx+=dx) {
        dest.at(y,x) = biquadIpl.interpolate(src,fy,fx);
      }
    }

    return true;
  }

  template<class T>
  bool scalingHelper<T>::bilinearScale(const matrix<T>& src,
                                             matrix<T>& dest) const {
    int x,y,ix,iy,ix1;
    float fx,fy,rx,ry;
    const float dx = static_cast<float>(src.lastColumn())/dest.lastColumn();
    const float dy = static_cast<float>(src.lastRow())/dest.lastRow();

    // the interpolator
    bilinearInterpolator<T> bilinIpl;

    if ((src.rows() < 2) || (src.columns() < 2)) {
      // the source is too small for a bilinear kernel to pass
      // completely therein, so we need to use the expensive methods!
      for (fy=0.0,y=0;y<dest.lastRow();++y,fy+=dy) {
        for (fx=0.0,x=0;x<dest.lastColumn();++x,fx+=dx) {
          dest.at(y,x) = bilinIpl.interpolate(src,fy,fx);
        }
      }

      return true;
    }

    for (fy=0.0f,y=0;y<dest.lastRow();++y,fy+=dy) {
      iy = static_cast<int>(fy);
      ry = fy-static_cast<float>(iy);

      const T *const row1 = &src.at(iy,0);
      const T *const row2 = &src.at(iy+1,0);

      for (fx=0.0f,x=0;x<dest.lastColumn();++x,fx+=dx) {
        ix = static_cast<int>(fx);
        ix1 = ix+1;
        rx = fx-static_cast<float>(ix);
        dest.at(y,x) = bilinIpl.compute(ry,rx,row1[ix],row1[ix1],
                                              row2[ix],row2[ix1]);
      }
      ix = src.lastColumn();
      dest.at(y,x) = bilinIpl.compute(ry,row1[ix],row2[ix]);
    }

    // last row
    iy = src.lastRow();
    const T *const row1 = &src.at(iy,0);
    for (fx=0.0f,x=0;x<dest.lastColumn();++x,fx+=dx) {
      ix = static_cast<int>(fx);
      rx = fx-static_cast<float>(ix);
      dest.at(y,x) = bilinIpl.compute(rx,row1[ix],row1[ix+1]);
    }
    
    dest.at(y,x) = row1[src.lastColumn()];

    return true;
  }

  template<class T>
  bool scalingHelper<T>::nearestNeighborScale(const matrix<T>& src,
                                                    matrix<T>& dest) const {
    int x,y;
    float fx,fy;

    const float dx = static_cast<float>(src.lastColumn())/dest.lastColumn();
    const float dy = static_cast<float>(src.lastRow())/dest.lastRow();

    for (fy=0.0f,y=0;y<dest.rows();++y,fy+=dy) {
      const T *const row1 = &src.at(static_cast<int>(fy+0.5f),0);
      for (fx=0.0f,x=0;x<dest.columns();++x,fx+=dx) {
        dest.at(y,x) = row1[static_cast<int>(fx+0.5f)];
      }
    }

    return true;
  }

  // Helper functions.

  template <class T>
  scalingHelper<T>::scalingHelper(const eInterpolatorType type) 
    : interpolator(type) {

    // initialize the generic interpolator used.
    typename genericInterpolator<T>::parameters gipar;
    gipar.kernelType = genericInterpolator<T>::parameters::BicubicKernel;
    gipar.numSamplesPerInterval = 256;
    gipar.boundaryType = Constant;
    genericIpl.setParameters(gipar);
#ifdef _LTI_DEBUG
    static bool alreadyShown = false;
    if (!alreadyShown) {
      alreadyShown=true;
      static viewer viewik("Interpolation Kernel");
      passiveWait(100000);
      viewik.show(genericIpl.getLUT());
    }
#endif
  }

  template <class T>
  bool scalingHelper<T>::apply(const matrix<T>& src,
                                     matrix<T>& dst,
                               const float scale) const {
    return apply(src,dst,tpoint<float>(scale,scale));
  }

  // Scale image or channel
  template <class T>
  bool scalingHelper<T>::apply(const matrix<T>& src,
                                     matrix<T>& dst,
                               const tpoint<float>& scale) const {

    if (scale == tpoint<float>(1.0f,1.0f)) {
      dst.copy(src);
      return true;
    }

    if (src.empty() || (scale.x == 0.0f) || (scale.y == 0.0f)) {
      dst.clear();
      return true;
    }

    int x,y;

    // avoid empty images
    x = max(1,iround(src.columns()*scale.x));
    y = max(1,iround(src.rows()*scale.y));

    dst.resize(y,x,T(),false,false);

    switch(interpolator) {
      case NearestNeighborInterpolator:
        return nearestNeighborScale(src,dst);
      case BilinearInterpolator:
        return bilinearScale(src,dst);
      case BiquadraticInterpolator:
        return biquadraticScale(src,dst);
      case BicubicInterpolator:
        return bicubicScale(src,dst);
      default:
        return false;
    }
    return true;
  }

  // --------------------------------------------------
  // scalingWorker
  // --------------------------------------------------
  
  class scalingWorker {
  public:
    scalingWorker(const eInterpolatorType type) 
      : fhelper(type),bhelper(type),chelper(type), ihelper(type) {};
    
    // Three instances required for one scaling functor
    scalingHelper<float>    fhelper;
    scalingHelper<ubyte>    bhelper;
    scalingHelper<rgbPixel> chelper;
    scalingHelper<int>      ihelper;
  };


  // --------------------------------------------------
  // scaling::parameters
  // --------------------------------------------------

  // default constructor
  scaling::parameters::parameters()
    : modifier::parameters() {
    scale = tpoint<float>(sqrt(2.0f),sqrt(2.0f));
    interpolatorType = BilinearInterpolator;
  }

  // copy constructor
  scaling::parameters::parameters(const parameters& other)
    : modifier::parameters() {
    copy(other);
  }

  // destructor
  scaling::parameters::~parameters() {
  }

  // get type name
  const char* scaling::parameters::getTypeName() const {
    return "scaling::parameters";
  }

  // copy member

  scaling::parameters&
  scaling::parameters::copy(const parameters& other) {
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
    
    scale = other.scale;
    interpolatorType = other.interpolatorType;
    return *this;
  }

  // alias for copy member
  scaling::parameters&
    scaling::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* scaling::parameters::clone() const {
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
  bool scaling::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool scaling::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"scale",scale);
      lti::write(handler,"interpolatorType",interpolatorType);
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
  bool scaling::parameters::write(ioHandler& handler,
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
  bool scaling::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool scaling::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"scale",scale);
      lti::read(handler,"interpolatorType",interpolatorType);
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
  bool scaling::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // scaling
  // --------------------------------------------------

  // default constructor
  scaling::scaling()
    : modifier(), worker(0) {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  scaling::scaling(const parameters& par)
    : modifier(), worker(0)  {

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  scaling::scaling(const scaling& other)
    : modifier(), worker(0)  {
    copy(other);
  }

  // destructor
  scaling::~scaling() {
    delete worker;
    worker = 0;
  }

  // returns the name of this type
  const char* scaling::getTypeName() const {
    return "scaling";
  }

  // copy member
  scaling& scaling::copy(const scaling& other) {
    modifier::copy(other);

    return (*this);
  }

  // alias for copy member
  scaling& scaling::operator=(const scaling& other) {
    return (copy(other));
  }


  // clone member
  functor* scaling::clone() const {
    return new scaling(*this);
  }

  // return parameters
  const scaling::parameters& scaling::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool scaling::updateParameters() {
    const parameters& par = getParameters();
    delete worker;
    worker = new scalingWorker(par.interpolatorType);
    return notNull(worker);
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  
  // On place apply for type image!
  bool scaling::apply(image& srcdest) const {
    image tmpImg;
    if (apply(srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type image!
  bool scaling::apply(const image& src,image& dest) const {
    const parameters& par = getParameters();
    return worker->chelper.apply(src,dest,par.scale);
  }

  // On place apply for type matrix<ubyte>!
  bool scaling::apply(matrix<ubyte>& srcdest) const {
    matrix<ubyte> tmpImg;
    if (apply(srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type matrix<ubyte>!
  bool scaling::apply(const matrix<ubyte>& src,matrix<ubyte>& dest) const {
    
    const parameters& par = getParameters();
    return worker->bhelper.apply(src,dest,par.scale);

  }

  // On place apply for type image!
  bool scaling::apply(matrix<float>& srcdest) const {
    matrix<float> tmpImg;
    if (apply(srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }

  // On copy apply for type matrix<float>!
  bool scaling::apply(const matrix<float>& src,matrix<float>& dest) const {
    const parameters& par = getParameters();
    return worker->fhelper.apply(src,dest,par.scale);
  }

  // On place apply for type matrix<int>!
  bool scaling::apply(matrix<int>& srcdest) const {
    matrix<int> tmpImg;
    if (apply(srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }

  // On copy apply for type matrix<int>!
  bool scaling::apply(const matrix<int>& src,matrix<int>& dest) const {
    const parameters& par = getParameters();
    return worker->ihelper.apply(src,dest,par.scale);
  }

  // --------------------
  // shortcuts with scale
  // --------------------

  // On place apply for type image!
  bool scaling::apply(const tpoint<float>& scale, image& srcdest) const {
    image tmpImg;
    if (apply(scale,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }

  // On copy apply for type image!
  bool scaling::apply(const tpoint<float>& scale,
                      const image& src,image& dest) const {    
    return worker->chelper.apply(src,dest,scale);
  }

  // On place apply for type matrix<ubyte>!
  bool scaling::apply(const tpoint<float>& scale,
                            matrix<ubyte>& srcdest) const {
    matrix<ubyte> tmpImg;
    if (apply(scale,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }

  // On copy apply for type matrix<ubyte>!
  bool scaling::apply(const tpoint<float>& scale,
                      const matrix<ubyte>& src,
                      matrix<ubyte>& dest) const {
    
    return worker->bhelper.apply(src,dest,scale);
  }

  // On place apply for type image!
  bool scaling::apply(const tpoint<float>& scale,
                            matrix<float>& srcdest) const {
    matrix<float> tmpImg;
    if (apply(scale,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type matrix<float>!
  bool scaling::apply(const tpoint<float>& scale,
                      const matrix<float>& src,matrix<float>& dest) const {
    
    return worker->fhelper.apply(src,dest,scale);
  }

  // On place apply for type matrix<int>!
  bool scaling::apply(const tpoint<float>& scale,
		      matrix<int>& srcdest) const {
    matrix<int> tmpImg;
    if (apply(scale,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }
  
  // On copy apply for type matrix<int>!
  bool scaling::apply(const tpoint<float>& scale,
                      const matrix<int>& src,matrix<int>& dest) const {
    
    return worker->ihelper.apply(src,dest,scale);
  }

  // --------------------
  // shortcuts with scale
  // --------------------

  // On place apply for type image!
  bool scaling::apply(const float scale, image& srcdest) const {
    image tmpImg;
    if (apply(scale,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type image!
  bool scaling::apply(const float scale,
                      const image& src,image& dest) const {
    return worker->chelper.apply(src,dest,scale);

  }

  // On place apply for type matrix<ubyte>!
  bool scaling::apply(const float scale,
                            matrix<ubyte>& srcdest) const {
    matrix<ubyte> tmpImg;
    if (apply(scale,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type matrix<ubyte>!
  bool scaling::apply(const float scale,
                      const matrix<ubyte>& src,
                      matrix<ubyte>& dest) const {
    return worker->bhelper.apply(src,dest,scale);

  }

  // On place apply for type image!
  bool scaling::apply(const float scale,
                            matrix<float>& srcdest) const {
    matrix<float> tmpImg;
    if (apply(scale,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }


  // On copy apply for type matrix<float>!
  bool scaling::apply(const float scale,
                      const matrix<float>& src,matrix<float>& dest) const {
    return worker->fhelper.apply(src,dest,scale);
  }

  // On place apply for type image!
  bool scaling::apply(const float scale,
		      matrix<int>& srcdest) const {
    matrix<int> tmpImg;
    if (apply(scale,srcdest,tmpImg)) {
      tmpImg.detach(srcdest);
      return true;
    }
    return false;
  }
  
  
  // On copy apply for type matrix<float>!
  bool scaling::apply(const float scale,
                      const matrix<int>& src,matrix<int>& dest) const {
    return worker->ihelper.apply(src,dest,scale);
  }
}
