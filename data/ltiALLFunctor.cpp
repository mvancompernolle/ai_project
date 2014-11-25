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

/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiALLFunctor.cpp
 * authors ....: Axel Berner
 * organization: LTI, RWTH Aachen
 * creation ...: 17.06.2002
 * revisions ..: $Id: ltiALLFunctor.cpp,v 1.12 2008/08/17 22:20:12 alvarado Exp $
 */

#include <fstream>
#include "ltiALLFunctor.h"

namespace lti {

  // -----------------------------------------------------------------------
  // ioImage
  // -----------------------------------------------------------------------

  ioImage::parameters::parameters() {
  }

  ioImage::parameters& ioImage::parameters::copy(const parameters& other) {
#ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    ioFunctor::parameters::copy(other);
#else
    // ...so we have to use this workaround.
    // Conditional on that copy may not be virtual.
    ioFunctor::parameters& (ioFunctor::parameters::* p_copy)
      (const ioFunctor::parameters&) = ioFunctor::parameters::copy;
    (this->*p_copy)(other);
#endif

    paramBMP.copy(other.paramBMP);
#ifdef LTI_HAVE_JPEG
    paramJPEG.copy(other.paramJPEG);
#endif
#ifdef LTI_HAVE_PNG
    paramPNG.copy(other.paramPNG);
#endif
    return (*this);
  }

  functor::parameters* ioImage::parameters::clone() const  {
    return (new parameters(*this));
  }

  const char* ioImage::parameters::getTypeName() const {
    return "ioImage::parameters";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool ioImage::parameters::write(ioHandler& handler,
				  const bool complete) const
# else
  bool ioImage::parameters::writeMS(ioHandler& handler,
				    const bool complete) const
# endif
  {

   bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    b = b && paramBMP.write(handler,false);
#ifdef LTI_HAVE_JPEG
    b = b && paramJPEG.write(handler,false);
#endif
#ifdef LTI_HAVE_PNG
    b = b && paramPNG.write(handler,false);
#endif

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioFunctor::parameters::write(handler,false);
# else
    bool (ioFunctor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      ioFunctor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool ioImage::parameters::write(ioHandler& handler,
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
  bool ioImage::parameters::read(ioHandler& handler,
				   const bool complete)
# else
  bool ioImage::parameters::readMS(ioHandler& handler,
				   const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    b = b && paramBMP.read(handler,false);
#ifdef LTI_HAVE_JPEG
    b = b && paramJPEG.read(handler,false);
#endif
#ifdef LTI_HAVE_PNG
    b = b && paramPNG.read(handler,false);
#endif

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioFunctor::parameters::read(handler,false);
# else
    bool (ioFunctor::parameters::* p_readMS)(ioHandler&,const bool) =
      ioFunctor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool ioImage::parameters::read(ioHandler& handler,
				 const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  bool ioImage::setParameters(const functor::parameters& theParam ){
    return functor::setParameters(theParam);
  };

  bool ioImage::setParameters(const ioBMP::parameters& theParam ){

    parameters param;
    if(validParameters())
      param.copy(getParameters());
    param.paramBMP.copy(*dynamic_cast<const ioBMP::parameters*>(&theParam));
    if (setParameters(param)) {
      type =  ioImage::BMP;
      return true;
    }
    return false;
  };

#ifdef LTI_HAVE_JPEG
  bool ioImage::setParameters(const ioJPEG::parameters& theParam ){

    parameters param;
    if(validParameters())
      param.copy(getParameters());
    param.paramJPEG.copy(*dynamic_cast<const ioJPEG::parameters*>(&theParam));
    if(setParameters(param)) {
      type = ioImage::JPEG;
      return true;
    }
    return false;
  };
#endif

#ifdef LTI_HAVE_PNG
  bool ioImage::setParameters(const ioPNG::parameters& theParam ){

    parameters param;
    if(validParameters())
      param.copy(getParameters());
    param.paramPNG.copy(*dynamic_cast<const ioPNG::parameters*>(&theParam));
    if(setParameters(param)) {
      type = ioImage::PNG;
      return true;
    }
    return false;
  };
#endif

 // returns the current parameters
  const ioImage::parameters& ioImage::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  };

  const ioFunctor::parameters& ioImage::getCurrParameters() const{

    if (!validParameters()) {
      throw invalidParametersException(getTypeName());
    }
    
    switch(type) {
      case ioImage::BMP: {
        return getParameters().paramBMP;
        break;
      }
#ifdef LTI_HAVE_JPEG
      case ioImage::JPEG: {
        return getParameters().paramJPEG;
        break;
      }
#endif
#ifdef LTI_HAVE_PNG
      case ioImage::PNG: {
        return getParameters().paramPNG;
        break;
      }
#endif
      case ioImage::UNKNOWN:
      default: {
        return getParameters();
        break;
      }
    }
    
    return getParameters();        
  };

  // constructor
  ioImage::ioImage() {
    type = ioImage::UNKNOWN;
  };

  const char* ioImage::getTypeName() const {
    return "ioImage";
  };

  bool ioImage::setType(const eImageFileType& theType) {

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    type = theType;
    return true;
  };

  ioImage::eImageFileType 
  ioImage::getExtensionType(const std::string& filename) const {
    //find extension in filename
    const unsigned int pointPos = filename.rfind('.',filename.size());
    std::string ext = filename.substr(pointPos+1,filename.size());

    //formate extension name
    std::string::iterator it;
    for(it=ext.begin();it!=ext.end();++it) {
	*it = tolower(*it);
    }

    //decision of file type
    eImageFileType tType = ioImage::UNKNOWN;
    if(ext == "bmp")
      tType = ioImage::BMP;
    else if(ext == "jpeg")
      tType = ioImage::JPEG;
    else if(ext == "jpg")
      tType = ioImage::JPEG;
    else if(ext == "png")
      tType = ioImage::PNG;

    return tType;
  };

  bool ioImage::setFilenameInParam(const std::string& filename,
                                   const eImageFileType& etype) {
    parameters param;
    if (validParameters())
      param.copy(getParameters());

      switch(etype) {
      case ioImage::BMP: {
        param.paramBMP.filename = filename;
        break;
      }
#ifdef LTI_HAVE_JPEG
      case ioImage::JPEG: {
        param.paramJPEG.filename = filename;
        break;
      }
#endif
#ifdef LTI_HAVE_PNG
      case ioImage::PNG: {
        param.paramPNG.filename = filename;
        break;
      }
#endif
      case ioImage::UNKNOWN:
      default: {
        return false;
        break;
      }
    }

    setParameters(param);
    return true;
  };

  // ----------------------------------------------------------------------
  // loadImage
  // ----------------------------------------------------------------------
  // default constructor
  loadImage::loadImage() :ioImage() {
  };

  const char* loadImage::getTypeName() const {
    return "loadImage";
  };

  functor* loadImage::clone() const {
    return (new loadImage(*this));
  };

  // read file
  bool loadImage::apply(image& theImage) {

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    bool success = false;
    switch(type) {
      case ioImage::BMP: {
        loaderBMP.setParameters(ioImage::getParameters().paramBMP);
        if (!(success = loaderBMP.apply(theImage))) {
          setStatusString(loaderBMP.getStatusString());
        };
        break;
      }
#ifdef LTI_HAVE_JPEG
      case ioImage::JPEG: {
        loaderJPEG.setParameters(ioImage::getParameters().paramJPEG);
        if (!(success = loaderJPEG.apply(theImage))) {
          setStatusString(loaderJPEG.getStatusString());
        }
        break;
      }
#endif
#ifdef LTI_HAVE_PNG
      case ioImage::PNG: {
        loaderPNG.setParameters(ioImage::getParameters().paramPNG);
        if (!(success = loaderPNG.apply(theImage))) {
          setStatusString(loaderPNG.getStatusString());
        }
        break;
      }
#endif
      case ioImage::UNKNOWN:
      default: {
        // no parameters -> don't know what to do...
        theImage.clear();
        setStatusString("Invalid fileformat");
        break;
      }
    }

    return success;
  };

  // read file
  bool loadImage::apply(channel8& theChannel,lti::palette& colors) {

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    bool success = false;
    switch(type) {
      case ioImage::BMP: {
        loaderBMP.setParameters(ioImage::getParameters().paramBMP);
        if (!(success = loaderBMP.apply(theChannel,colors))) {
          setStatusString(loaderBMP.getStatusString());
        }
        break;
      }
#ifdef LTI_HAVE_JPEG
      case ioImage::JPEG: {
        image theImage;
        bool isGrey = false;
        loaderJPEG.setParameters(ioImage::getParameters().paramJPEG);
        if( (success = loaderJPEG.apply(theImage,isGrey)) ) {
	        if(isGrey) {
	          //get one channel as the grey channel
	          rgbSplitter.getGreen(theImage,theChannel);
	        }
	        else {
	          //convert color channels to the intensity channel
	          hsiSplitter.getIntensity(theImage,theChannel);
	        }
	        colors.resize(256,0,false,false);
	        for (int i=0;i<256;++i) {
	          colors.at(i)=rgbPixel(i,i,i);
	        }
        } else {
          setStatusString(loaderJPEG.getStatusString());
        }
        break;
      }
#endif
#ifdef LTI_HAVE_PNG
      case ioImage::PNG: {
        loaderPNG.setParameters(ioImage::getParameters().paramPNG);
        if (!(success = loaderPNG.apply(theChannel,colors))) {
          setStatusString(loaderPNG.getStatusString());
        }
        break;
      }
#endif
      case ioImage::UNKNOWN:
      default: {
        // no parameters -> don't know what to do...
        theChannel.clear();
        colors.clear();
        setStatusString("Invalid fileformat");
        break;
      }
    }
    return success;
  };

  // read file
  bool loadImage::apply(channel& theChannel) {

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    bool success = false;
    switch(type) {
      case ioImage::BMP: {
        loaderBMP.setParameters(ioImage::getParameters().paramBMP);
        if (!(success = loaderBMP.apply(theChannel))) {
          setStatusString(loaderBMP.getStatusString());
        }
	      break;
      }
#ifdef LTI_HAVE_JPEG
      case ioImage::JPEG: {
        channel8 ch8;
        palette pal;
        success = apply(ch8,pal);
        theChannel.castFrom(ch8);
        break;
      }
#endif
#ifdef LTI_HAVE_PNG
      case ioImage::PNG: {
        image theImage;
        loaderPNG.setParameters(ioImage::getParameters().paramPNG);
        success = loaderPNG.apply(theImage);
        hsiSplitter.getIntensity(theImage,theChannel);
        if (!success) {
          setStatusString(loaderPNG.getStatusString());
        }
        break;
      }
#endif
      case ioImage::UNKNOWN:
      default: {
        // no parameters -> don't know what to do...
        theChannel.clear();
        setStatusString("Invalid fileformat");
        break;
      }
    }
    return success;
  };

  // checkfile
  bool loadImage::checkHeader(const std::string& filename,
			      point& imageSize) {
    bool trueColor;
    return checkHeader(filename,imageSize,trueColor);
  }

  // checkfile
  bool loadImage::checkHeader(const std::string& filename,
			      point& imageSize,
                              bool& trueColor) {
    bool success = false;
    type = getExtensionType(filename);

    switch(type) {
      case ioImage::BMP: {
        int bitsPerPixel;
        int colorEntries;
        if (!(success = loaderBMP.checkHeader(filename, imageSize, 
                                              bitsPerPixel, colorEntries))) {
          setStatusString(loaderBMP.getStatusString());
        }
        trueColor=(bitsPerPixel>8);
        break;
      }
#ifdef LTI_HAVE_JPEG
      case ioImage::JPEG: {
        std::string comment;
        bool isGray;
        if (!(success = loaderJPEG.checkHeader(filename, imageSize,
                                                     comment, isGray))) {
          setStatusString(loaderJPEG.getStatusString());
        }
        trueColor=!isGray;
        break;
      }
#endif
#ifdef LTI_HAVE_PNG
      case ioImage::PNG: {
        int bitsPerPixel;
        int imageType;
        if (!(success = loaderPNG.checkHeader(filename, imageSize, 
                                                    bitsPerPixel, imageType))) {
          setStatusString(loaderPNG.getStatusString());
        }
        trueColor=((imageType & 1) == 0); // 1 == PNG_COLOR_MASK_PALETTE
        break;
      }
#endif
      case ioImage::UNKNOWN:
      default: {
        // no parameters -> don't know what to do...
        imageSize = point(0,0);
        trueColor = false;
        setStatusString("Invalid fileformat");
        break;
      }
    }
    return success;
  };

  // shortcut for apply (load file with name "filename")
  bool loadImage::load(const std::string& filename,image& theImage) {

    type = getExtensionType(filename);
    setFilenameInParam(filename,type);
    return apply(theImage);
  };

  // shortcut for apply
  bool loadImage::load(const std::string& filename,
                       channel8& theChannel,
                       lti::palette& colors) {

    type = getExtensionType(filename);
    setFilenameInParam(filename,type);
    return apply(theChannel,colors);
  };

  // shortcut for apply
  bool loadImage::load(const std::string& filename,
                       channel& theChannel) {

    type = getExtensionType(filename);
    setFilenameInParam(filename,type);
    return apply(theChannel);
  };

  // ------------------------------------------------------------------
  // saveBMP
  // ------------------------------------------------------------------

  // constructor
  saveImage::saveImage() :ioImage() {
  };

  const char* saveImage::getTypeName() const {
    return "saveImage";
  };

  functor* saveImage::clone() const {
    return (new saveImage(*this));
  };

  // apply
  bool saveImage::apply(const image& theImage) {

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    bool success = false;
    switch(type) {
      case ioImage::BMP: {
        saverBMP.setParameters(ioImage::getParameters().paramBMP);
        success = saverBMP.apply(theImage);
        if (!success) {
          setStatusString(saverBMP.getStatusString());
        }
        break;
      }
#ifdef LTI_HAVE_JPEG
      case ioImage::JPEG: {
        saverJPEG.setParameters(ioImage::getParameters().paramJPEG);
        success = saverJPEG.apply(theImage);
        if (!success) {
          setStatusString(saverJPEG.getStatusString());
        }
        break;
      }
#endif
#ifdef LTI_HAVE_PNG
      case ioImage::PNG: {
        saverPNG.setParameters(ioImage::getParameters().paramPNG);
        success = saverPNG.apply(theImage);
        if (!success) {
          setStatusString(saverPNG.getStatusString());
        }
        break;
      }
#endif
      case ioImage::UNKNOWN:
      default: {
        setStatusString("Invalid parameters");
        break;
      }
    }

    return success;
  }


  // saves float-channel into 24-bit RGB by assigning
  // each R, G and B the same grey-value...
  bool saveImage::apply(const channel& theChannel) {

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    bool success = false;
    switch(type) {
      case ioImage::BMP: {
        saverBMP.setParameters(ioImage::getParameters().paramBMP);
        success = saverBMP.apply(theChannel);
        if (!success) {
          setStatusString(saverBMP.getStatusString());
        }
        break;
      }
#ifdef LTI_HAVE_JPEG
      case ioImage::JPEG: {
        saverJPEG.setParameters(ioImage::getParameters().paramJPEG);
        success = saverJPEG.apply(theChannel);
        if (!success) {
          setStatusString(saverJPEG.getStatusString());
        }
        break;
      }
#endif
#ifdef LTI_HAVE_PNG
      case ioImage::PNG: {
        saverPNG.setParameters(ioImage::getParameters().paramPNG);
        success = saverPNG.apply(theChannel);
        if (!success) {
          setStatusString(saverPNG.getStatusString());
        }
        break;
      }
#endif
      case ioImage::UNKNOWN:
      default: {
        setStatusString("Invalid parameters");
        break;
      }
    }

    return success;
  };

  /** saves 8-bit channel
      see ioImage::parameters to the possible effects of this member.
  */
  bool saveImage::apply(const channel8& theChannel,
			const lti::palette& colors) {

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    bool success = false;
    switch(type) {
      case ioImage::BMP: {
        saverBMP.setParameters(ioImage::getParameters().paramBMP);
        success = saverBMP.apply(theChannel, colors);
        if (!success) {
          setStatusString(saverBMP.getStatusString());
        }
        break;
      }
#ifdef LTI_HAVE_JPEG
      case ioImage::JPEG: {
        setStatusString("jpeg doesn't support palettes");
        break;
      }
#endif
#ifdef LTI_HAVE_PNG
      case ioImage::PNG: {
        saverPNG.setParameters(ioImage::getParameters().paramPNG);
        success = saverPNG.apply(theChannel, colors);
        if (!success) {
          setStatusString(saverPNG.getStatusString());
        }
        break;
      }
#endif
      case ioImage::UNKNOWN:
      default: {
        setStatusString("Invalid parameters");
        break;
      }
    }

    return success;
  };
  /* shortcut for save */
  bool saveImage::save(const std::string& filename,
                     const image& theImage) {

    type = getExtensionType(filename);
    setFilenameInParam(filename,type);
    return apply(theImage);
  };

  /* shortcut for save */
  bool saveImage::save(const std::string& filename,
                     const channel8& theChannel,
                     const lti::palette& colors) {

    type = getExtensionType(filename);
    setFilenameInParam(filename,type);
    return apply(theChannel,colors);
  };

  /* shortcut for save */
  bool saveImage::save(const std::string& filename,
                     const channel& theChannel) {

    type = getExtensionType(filename);
    setFilenameInParam(filename,type);
    return apply(theChannel);
  }

} // namespace lti
