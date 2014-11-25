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
 * file .......: ltiBlueScreen.cpp
 * authors ....: Marius Wolf  
 * organization: LTI, RWTH Aachen
 * creation ...: 9.4.2003
 * revisions ..: $Id: ltiBlueScreen.cpp,v 1.10 2006/09/05 10:03:18 ltilib Exp $
 */

#include "ltiBlueScreen.h"
#include <ltiConvolution.h> 
#include <ltiGaussKernels.h> 

namespace lti {
  // --------------------------------------------------
  // blueScreen::parameters
  // --------------------------------------------------

  // default constructor
  blueScreen::parameters::parameters() 
    : modifier::parameters() {        
        
    gaussKernelSize = 4;
    size = 70;
    gaussFilter = true;
    gaussMaskThreshold = 150;
    regionMode = Sphere;
    screenColor = rgbPixel(0,0,0);
  }

  // copy constructor
  blueScreen::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor
  blueScreen::parameters::~parameters() {    
  }

  // get type name
  const char* blueScreen::parameters::getTypeName() const {
    return "blueScreen::parameters";
  }
  
  // copy member

  blueScreen::parameters& 
    blueScreen::parameters::copy(const parameters& other) {
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
      gaussKernelSize = other.gaussKernelSize;      
      size = other.size;
      gaussMaskThreshold = other.gaussMaskThreshold;
      gaussFilter = other.gaussFilter;
      regionMode = other.regionMode;      
      screenColor = other.screenColor;

    return *this;
  }

  // alias for copy member
  blueScreen::parameters& 
    blueScreen::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* blueScreen::parameters::clone() const {
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
  bool blueScreen::parameters::write(ioHandler& handler,
                                         const bool complete) const 
# else
  bool blueScreen::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      
      lti::write(handler,"gaussKernelSize",gaussKernelSize);
      lti::write(handler,"gaussMaskThreshold",gaussMaskThreshold);      
      lti::write(handler,"size",size);
      lti::write(handler,"gaussFilter",gaussFilter);

      switch(regionMode) {
          case Cube:
            handler.write("regionMode","Cube");
            break;
          case Sphere:
            handler.write("regionMode","Sphere");
            break;
          default:
            handler.write("regionMode","unknown");
      }
      
      lti::write(handler,"screenColor",screenColor);
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
  bool blueScreen::parameters::write(ioHandler& handler,
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
  bool blueScreen::parameters::read(ioHandler& handler,
                                        const bool complete) 
# else
  bool blueScreen::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
            

      std::string str;
      lti::read(handler,"gaussKernelSize",gaussKernelSize);
      lti::read(handler,"gaussMaskThreashold",gaussMaskThreshold);      
      lti::read(handler,"size",size);
      lti::read(handler,"gaussFilter",gaussFilter);
      //lti::read(handler,"regionMode",regionMode);
      
      lti::read(handler,"regionMode",str);
      if (str == "Sphere") {
        regionMode = Sphere;
      } else if (str == "Cube") {
        regionMode = Cube;
      } else {
        regionMode = Sphere;
      }

      lti::read(handler,"screenColor",screenColor);
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
  bool blueScreen::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // blueScreen
  // --------------------------------------------------

  // default constructor
  blueScreen::blueScreen()
    : modifier(){
    
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  blueScreen::blueScreen(const parameters& par)
    : modifier() {
    

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  blueScreen::blueScreen(const blueScreen& other) {
    copy(other);
  }

  // destructor
  blueScreen::~blueScreen() {
  }

  // returns the name of this type
  const char* blueScreen::getTypeName() const {
    return "blueScreen";
  }

  // copy member
  blueScreen&
    blueScreen::copy(const blueScreen& other) {
      modifier::copy(other);
    

    return (*this);
  }

  // alias for copy member
  blueScreen&
    blueScreen::operator=(const blueScreen& other) {
    return (copy(other));
  }


  // clone member
  functor* blueScreen::clone() const {
    return new blueScreen(*this);
  }

  // return parameters
  const blueScreen::parameters&
    blueScreen::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }
  
  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  
  // On place apply for type image!
  bool blueScreen::apply(image& srcdest,image& background) const {
    return apply(srcdest,srcdest,background);
  };

  bool blueScreen::apply(image& srcdest,rgbPixel& background) const {      
    return apply(srcdest,srcdest,background);
  };

  
  //On copy apply for type image!
  bool blueScreen::apply(const image& src,
                         image& dest,image& background) const {
    const parameters& param = getParameters();    

    if (src.empty()) {
      setStatusString("Source image is empty");
      return false;
    }

    if (background.empty()) {
      setStatusString("Background image is empty");
      return false;
    }

    dest.resize(src.size());
    point backgroundSize=background.size();

    if (param.gaussFilter) {
      
      channel8 mask(src.size());
      static convolution              filter; 
      static convolution::parameters  filterParams;
      
      //Create mask
      //Use a spherical color region
      if(param.regionMode==parameters::Sphere) {
        int dist;
        int sqrThreshold=param.size*param.size;        
        for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {
            dist=src.at(i,j).distanceSqr(param.screenColor);
            mask.at(i,j)=(dist>sqrThreshold) ? 0 : 255;
          }
        }
      }

      // Use a cubic color region
      else {
        for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {         
            mask.at(i,j)=(inCube(src.at(i,j)))? 255 : 0;
          }
        }
      }

      //Apply Gauss filter on mask
      filterParams.boundaryType=modifier::parameters::Constant;
      filterParams.setKernel(gaussKernel2D<float>(param.gaussKernelSize));
      filter.setParameters(filterParams);
      filter.apply(mask);

      //Replace old background according to the mask
      for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {
            if (mask.at(i,j)>=param.gaussMaskThreshold) {
              dest.at(i,j)=background.at(i%backgroundSize.y,
                                         j%backgroundSize.x);
            }
            else {
              dest.at(i,j)=src.at(i,j);
            }
          }
      }
    }

    // Do not use Gauss filter
    else {
      //Use spherical color region
      if (param.regionMode==parameters::Sphere) {
        int sqrThreshold;
        int dist;
        sqrThreshold=param.size*param.size;
        for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {
            dist=src.at(i,j).distanceSqr(param.screenColor);          
            if (dist>sqrThreshold) {
              dest.at(i,j)=src.at(i,j);
            }
            else {
              dest.at(i,j)=background.at(i%backgroundSize.y,
                                         j%backgroundSize.x);
            }
          }
        }
      }
      //Use cubic color region
      else {
        for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {                   
            if (inCube(src.at(i,j))) {
              dest.at(i,j)=background.at(i%backgroundSize.y,
                                         j%backgroundSize.x);
            }
            else {
              dest.at(i,j)=src.at(i,j);              
            }
          }
        }   
      }
    }

    return true;
  };

  bool blueScreen::apply(const image& src,
                         image& dest,
                         rgbPixel& background) const {
    const parameters& param = getParameters(); 
    if (src.empty()) {
      setStatusString("Source image is empty");
      return false;
    }

    dest.resize(src.size(),rgbPixel(),false,false);
    if (param.gaussFilter) {
      
      channel8 mask(src.size());
      static convolution              filter; 
      static convolution::parameters  filterParams;
      
      //Create mask
      //Use a spherical color region
      if(param.regionMode==parameters::Sphere) {
        int dist;
        int sqrThreshold;
        sqrThreshold=param.size*param.size;
        for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {
            dist=src.at(i,j).distanceSqr(param.screenColor);          
            mask.at(i,j) = (dist>sqrThreshold) ? 0 : 255;
          }
        }
      }
      //Use a cubic color region
      else {
        for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {         
            mask.at(i,j) = (inCube(src.at(i,j))) ? 255 : 0;
          }
        }
      }
       //Apply Gauss filter on mask
      filterParams.boundaryType=modifier::parameters::Constant;
      filterParams.setKernel(gaussKernel2D<float>(param.gaussKernelSize));
      filter.setParameters(filterParams);
      filter.apply(mask);
      //Replace old background according to the mask
      for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {
            if (mask.at(i,j)>=param.gaussMaskThreshold) {
              dest.at(i,j)=background;
            }
            else {
              dest.at(i,j)=src.at(i,j);
            }
          }
        }
    }
    //Do not use Gauss filter
    else {
      //Use spherical color region
      if (param.regionMode==parameters::Sphere) {
        int sqrThreshold;
        int dist;
        sqrThreshold=param.size*param.size;
        for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {
            dist=src.at(i,j).distanceSqr(param.screenColor);          
            if (dist>sqrThreshold) {
              dest.at(i,j)=src.at(i,j);
            }
            else {
              dest.at(i,j)=background;
            }
          }
        }
      }
      //Use cubic color region
      else {
        for (int i=0;i<src.rows();++i) {
          for (int j=0;j<src.columns();++j) {
                   
            if (!inCube(src.at(i,j))) {
              dest.at(i,j)=src.at(i,j);
            }
            else {
              dest.at(i,j)=background;
            }
          }
        }   
      }
    }

    return true;
  }

  bool blueScreen::inCube(rgbPixel check) const {
    const parameters& param = getParameters(); 
    //Check if given pixel is in the right red interval
    if ( (check.getRed()>=param.screenColor.getRed()+param.size) ||
         (check.getRed()<=param.screenColor.getRed()-param.size) ) {
      return false;
    }

    //Check if given pixel is in the right green interval
    if ( (check.getGreen()>=param.screenColor.getGreen()+param.size) ||
         (check.getGreen()<=param.screenColor.getGreen()-param.size) ) {
      return false;
    }

    //Check if given pixel is in the right blue interval
    if ( (check.getBlue()>=param.screenColor.getBlue()+param.size) || 
         (check.getBlue()<=param.screenColor.getBlue()-param.size) ) {
      return false;
    }

    return true;

  };



}
