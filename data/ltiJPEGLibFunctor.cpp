/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiJPEGFunctor.cpp
 * authors ....: Xinghan Yu, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 27.10.00
 * revisions ..: $Id: ltiJPEGLibFunctor.cpp,v 1.15 2007/04/08 22:34:28 alvarado Exp $
 */

#include "ltiJPEGFunctor.h"

// the libjpeg functor has a higher priority than the local one. 
// If HAVE_LIBJPEG is defined, then this will be compiled

#if defined(HAVE_LIBJPEG) 

#include <cstdio>
#include <setjmp.h>
#include "ltiGetStreamFd.h"

extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}

namespace lti {

  // Error handler used in loading jpeg image
  struct my_error_mgr {
    struct jpeg_error_mgr pub;  // "public" fields
    jmp_buf setjmp_buffer;      // for return to caller
    char error_msg[256];
  };

  typedef my_error_mgr *my_error_ptr;

  // Here is the routine that will replace the standard error_exit method
  static void my_error_exit(j_common_ptr cinfo) {
    // cinfo-err really points to a myErrorMgr struct, so coerce pointer
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    // Always display the message
    // We could postpone this until after returning, if we chose
    (*cinfo->err->format_message) (cinfo,myerr->error_msg);
    // Return control to the setjmp point
    longjmp(myerr->setjmp_buffer, 1);
  }

  // -----------------------------------------------------------------------
  // ioJPEG
  // -----------------------------------------------------------------------


  ioJPEG::parameters::parameters()
    : ioFunctor::parameters(), quality(75),progressive(false),
      comment(""), rowsPerRestart(0) {
  }


  ioJPEG::parameters&
  ioJPEG::parameters::copy(const ioJPEG::parameters& other) {

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
    // copy additional information
    quality = other.quality;
    progressive = other.progressive;
    comment = other.comment;
    rowsPerRestart = other.rowsPerRestart;

    return (*this);
  }


  functor::parameters* ioJPEG::parameters::clone() const  {
    return (new parameters(*this));
  }


  const char* ioJPEG::parameters::getTypeName() const {
    return "ioJPEG::parameters";
  }


  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool ioJPEG::parameters::write(ioHandler& handler,
                                 const bool complete) const
# else
    bool ioJPEG::parameters::writeMS(ioHandler& handler,
                                     const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"quality",quality);
      lti::write(handler,"progressive",progressive);
      lti::write(handler,"comment",comment);
      lti::write(handler,"rowsPerRestart",rowsPerRestart);
    }

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
  bool ioJPEG::parameters::write(ioHandler& handler,
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
  bool ioJPEG::parameters::read(ioHandler& handler,
                                const bool complete)
# else
    bool ioJPEG::parameters::readMS(ioHandler& handler,
                                    const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"quality",quality);
      lti::read(handler,"progressive",progressive);
      lti::read(handler,"comment",comment);
      lti::read(handler,"rowsPerRestart",rowsPerRestart);
    }

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
  bool ioJPEG::parameters::read(ioHandler& handler,
                                const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // ----------------------------------------------------------------------
  // ioJPEG
  // ----------------------------------------------------------------------

  // constructor
  ioJPEG::ioJPEG() {
    parameters defPar;
    setParameters(defPar);
  }

  // destructor
  ioJPEG::~ioJPEG() {
  }

  // returns the current parameters
  const ioJPEG::parameters& ioJPEG::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  const char* ioJPEG::getTypeName() const {
    return "ioJPEG";
  }

  // ----------------------------------------------------------------------
  // loadJPEG
  // ----------------------------------------------------------------------

  // constructor
  loadJPEG::loadJPEG() : ioJPEG() {
  }

  // destructor
  loadJPEG::~loadJPEG() {
  }

  const char* loadJPEG::getTypeName() const {
    return "loadJPEG";
  }

  functor* loadJPEG::clone() const {
    return (new loadJPEG(*this));
  }


  // shortcut for apply (load file named "filename")
  bool loadJPEG::load(const std::string& filename,
                      image& theImage,
                      bool& isGray) {

    FILE* fp;

    // open the file
    if ((fp = fopen(filename.c_str(), "rb")) == NULL) {
      setStatusString("File could not be found: ");
      appendStatusString(filename.c_str());
      return false;
    }

    bool res = load(fp,theImage,isGray);

    fclose(fp);

    return res;
  }

  // shortcut for apply (load file named "filename")
  bool loadJPEG::load(const std::string& filename,image& theImage) {
    bool isGray;
    return load(filename,theImage,isGray);
  }

  // load the picture defined in parameters, determine whether a gray
  // scale image was loaded, while using libJPEG
  bool loadJPEG::load(FILE* fp, image& theImage, bool& isGray) {
    // clear image:
    theImage.clear();

    // contains JPEG decompression parameters and pointers to working place
    struct jpeg_decompress_struct compressInfo;
    // error handler
    struct my_error_mgr jpegErrorHandler;
    //output row buffer
    JSAMPARRAY outputBuffer;
    // physical row width in output buffer
    int rowStride, height, width, rowCounter;

    // set up the normal JPEG error routines
    compressInfo.err = jpeg_std_error(&jpegErrorHandler.pub);
    jpegErrorHandler.pub.error_exit = my_error_exit;

    if (setjmp(jpegErrorHandler.setjmp_buffer)) {
      jpeg_destroy_decompress(&compressInfo);
      setStatusString(jpegErrorHandler.error_msg);
      return false;
    }

    // Initialize the JPEG decompression object
    jpeg_create_decompress(&compressInfo);

    // specify data source
    jpeg_stdio_src(&compressInfo, fp);

    // read file parameters
    jpeg_read_header(&compressInfo, TRUE);

    // Start decompressor
    jpeg_start_decompress(&compressInfo);

    // JSAMPLEs per row in output buffer
    rowStride = compressInfo.output_width * compressInfo.output_components;
    rowCounter = 0;
    height = compressInfo.output_height;
    width = compressInfo.output_width;
    theImage.resize(height, width, Black, false,false);

    // Make a one-row-high sample array that will go away when done with image
    outputBuffer = (*compressInfo.mem->alloc_sarray)
      ((j_common_ptr) &compressInfo,JPOOL_IMAGE, rowStride, 1);

    // Use the library's state variable compressInfo.output_scanline
    // as the loop counter
    image::iterator it = theImage.begin();
    JSAMPLE *ptr,*eptr;

    if (compressInfo.output_components == 1) {
      isGray = true;
      while (compressInfo.output_scanline < compressInfo.output_height) {
        jpeg_read_scanlines(&compressInfo, outputBuffer, 1);
        ptr = outputBuffer[0];
        eptr = ptr+compressInfo.output_width;
        ubyte val;
        // in case of grayscale image
        for (;ptr != eptr;++ptr,++it) {
          val = *ptr;
          (*it).set(val,val,val,0);
        }
      }
    } else if (compressInfo.output_components == 3) {
      isGray = false;
      while (compressInfo.output_scanline < compressInfo.output_height) {
        jpeg_read_scanlines(&compressInfo, outputBuffer, 1);
        ptr = outputBuffer[0];
        eptr = ptr+(3*compressInfo.output_width);
        // in case of color image
        for (;ptr != eptr;++it, ptr+=3) {
          (*it).set(ptr[0], ptr[1], ptr[2],0);
//           (*it).setRed(*ptr);
//           ++ptr;
//           (*it).setGreen(*ptr);
//           ++ptr;
//           (*it).setBlue(*ptr);
        }
      }
    } else {
      setStatusString("Unknown image format!");
      return false;
    }

    // Finish decompression
    jpeg_finish_decompress(&compressInfo);

    // Release JPEG decompression object, close the file
    jpeg_destroy_decompress(&compressInfo);

    return true;
  }

  // load the picture defined in parameters
  bool loadJPEG::apply(image& theImage) {
    bool gray;
    return apply(theImage, gray);
  }

  bool loadJPEG::apply(image& theImage, bool& isGray) {
    return load(getParameters().filename,theImage,isGray);
  }

  // (load file from stream)
  bool loadJPEG::load(std::ifstream& file,image& theImage) {
    getStreamFd fdGetter;
    fdGetter.setDuplicate();

    FILE* f=0;

    bool success=fdGetter.apply(file,f);
    if (success) {
      bool isGray;
      success=load(f,theImage,isGray);
      fclose(f);
    } else {
      setStatusString("Cannot open a file on descriptor");
    }
    return success;
  }

  //scans all chunks before the first SoS-chunk(!) for image information
  bool loadJPEG::checkHeader(const std::string& filename,
                             point &imageSize,
                             std::string &comment,
                             bool &isGray) {

    // contains JPEG decompression parameters and pointers to working place
    jpeg_decompress_struct compressInfo;

    // error handler
    my_error_mgr jpegErrorHandler;

    // file to open
    FILE *fp;

    // open the file
    if ((fp = fopen(filename.c_str(), "rb")) == NULL) {
      setStatusString("File could not be found: ");
      appendStatusString(filename.c_str());
      return false;
    }

    // set up the normal JPEG error routines
    compressInfo.err = jpeg_std_error(&jpegErrorHandler.pub);
    jpegErrorHandler.pub.error_exit = my_error_exit;

    if (setjmp(jpegErrorHandler.setjmp_buffer)) {
      jpeg_destroy_decompress(&compressInfo);
      fclose(fp);
      setStatusString(jpegErrorHandler.error_msg);
      return false;
    }

    // Initialize the JPEG decompression object
    jpeg_create_decompress(&compressInfo);

    // specify data source
    jpeg_stdio_src(&compressInfo, fp);

    // read in the different comments marker
    jpeg_save_markers(&compressInfo, JPEG_COM, 0xFFFF);

    // read file parameters
    jpeg_read_header(&compressInfo, TRUE);

    // return the value to the parameters
    imageSize.x = compressInfo.image_width;
    imageSize.y = compressInfo.image_height;

    // gray valued image?
    isGray = (compressInfo.out_color_space == JCS_GRAYSCALE);
    
    // get the comment marker
    if (notNull(compressInfo.marker_list)) {
      comment = (char*)compressInfo.marker_list->data;
    } else {
      comment = "";
    }

    // Release JPEG decompression object, close the file
    jpeg_destroy_decompress(&compressInfo);
    fclose(fp);

    return true;
  }

  // ------------------------------------------------------------------
  // saveJPEG
  // ------------------------------------------------------------------

  saveJPEG::saveJPEG() : ioJPEG() {
  }

  saveJPEG::~saveJPEG() {
  }

  const char* saveJPEG::getTypeName() const {
    return "saveJPEG";
  }

  functor* saveJPEG::clone() const {
    return (new saveJPEG(*this));
  }



  // save JPEG images using libjpeg
  bool saveJPEG::save(const std::string& filename,const image& theImage) {
    FILE *fp;

    // check the file
    if ((fp = fopen(filename.c_str(), "wb")) == NULL) {
      setStatusString("File does not exist: ");
      appendStatusString(filename.c_str());
      return false;
    }
    bool result=save(fp,theImage);
    fclose(fp);

    return result;
  }

  // (load file from stream)
  bool saveJPEG::save(std::ofstream& file,const image& theImage) {
    getStreamFd fdGetter;
    //fdGetter.setDuplicate();

    FILE* f=0;

    bool success=fdGetter.apply(file,f);
    if (success) {
      success=save(f,theImage);
    } else {
      setStatusString("Cannot open a file on descriptor");
    }
    return success;
  }


  // save JPEG images using libjpeg
  bool saveJPEG::save(FILE* fp, const image& theImage) {

    const parameters& param = getParameters();
    struct jpeg_compress_struct compressInfo;
    jpeg_error_mgr jerr;
    JSAMPARRAY inputBuffer;
    int rowStride, rowCounter;

    if (isNull(fp)) {
      setStatusString("Internal Error: file was null");
      return false;
    }

    // allocate and initialize JPEG compression object
    // set up the error handler at first
    compressInfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&compressInfo);

    // specify data destination
    jpeg_stdio_dest(&compressInfo, fp);

    // set parameters for compression
    compressInfo.image_width = theImage.columns();
    compressInfo.image_height = theImage.rows();
    // # of color components per pixel
    compressInfo.input_components = 3;
    // colorspace of input image
    compressInfo.in_color_space = JCS_RGB;

    // set other parameters as defaults
    jpeg_set_defaults(&compressInfo);
    jpeg_set_quality(&compressInfo, param.quality, TRUE);

    // Start compressor
    jpeg_start_compress(&compressInfo, TRUE);

    //  write the image, allocate the memory needed
    rowStride = 3 * theImage.columns();
    rowCounter = 0;   // set the row index in the theI2mage matrix
    inputBuffer =
      (*compressInfo.mem->alloc_sarray)((j_common_ptr) &compressInfo,
                                        JPOOL_IMAGE, rowStride, 1);

    // copy theImage matrix to the inputBuffer one line every time,
    // and write into the file
    palette::const_iterator it,eit;
    JSAMPLE* ptr;
    while (compressInfo.next_scanline < compressInfo.image_height) {
      ptr = inputBuffer[0];
      it = theImage.getRow(rowCounter).begin();
      eit = theImage.getRow(rowCounter).end();
      for (;it != eit;++it) {
        (*ptr) = (*it).getRed();
        ++ptr;
        (*ptr) = (*it).getGreen();
        ++ptr;
        (*ptr) = (*it).getBlue();
        ++ptr;
      }
      rowCounter++;  // increment of the row index in theImage matrix
      jpeg_write_scanlines(&compressInfo, inputBuffer, 1);
    }

    // Finish compression, close the file
    jpeg_finish_compress(&compressInfo);
    // release the allocated memory
    jpeg_destroy_compress(&compressInfo);

    return true;
  }

  // save JPEG images using libjpeg
  bool saveJPEG::save(const std::string& filename,const channel8& theImage) {

    const parameters& param = getParameters();
    struct jpeg_compress_struct compressInfo;
    jpeg_error_mgr jerr;
    JSAMPARRAY inputBuffer;
    int rowStride, rowCounter;
    FILE *fp;

    // check the file
    if ((fp = fopen(filename.c_str(), "wb")) == NULL) {
      setStatusString("File does not exist: ");
      appendStatusString(filename.c_str());
      return false;
    }

    // allocate and initialize JPEG compression object
    // set up the error handler at first
    compressInfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&compressInfo);

    // specify data destination
    jpeg_stdio_dest(&compressInfo, fp);

    // set parameters for compression
    compressInfo.image_width = theImage.columns();
    compressInfo.image_height = theImage.rows();
    // # of color components per pixel
    compressInfo.input_components = 1;
    // colorspace of input image
    compressInfo.in_color_space = JCS_GRAYSCALE;

    // set other parameters as defaults
    jpeg_set_defaults(&compressInfo);
    jpeg_set_quality(&compressInfo, param.quality, TRUE);

    // Start compressor
    jpeg_start_compress(&compressInfo, TRUE);

    //  write the image, allocate the memory needed
    rowStride = theImage.columns();
    rowCounter = 0;   // set the row index in the theI2mage matrix
    inputBuffer =
      (*compressInfo.mem->alloc_sarray)((j_common_ptr) &compressInfo,
                                        JPOOL_IMAGE, rowStride, 1);

    // copy theImage matrix to the inputBuffer one line every time,
    // and write into the file
    vector<channel8::value_type>::const_iterator it,eit;
    JSAMPLE* ptr;
    while (compressInfo.next_scanline < compressInfo.image_height) {
      ptr = inputBuffer[0];
      it = theImage.getRow(rowCounter).begin();
      eit = theImage.getRow(rowCounter).end();
      for (;it != eit;++it,++ptr) {
        (*ptr) = (JSAMPLE)(*it);
      }
      rowCounter++;  // increment of the row index in theImage matrix
      jpeg_write_scanlines(&compressInfo, inputBuffer, 1);
    }

    // Finish compression, close the file
    jpeg_finish_compress(&compressInfo);
    // release the allocated memory
    jpeg_destroy_compress(&compressInfo);

    fclose(fp);

    return true;
  }

  bool saveJPEG::save(const std::string& filename,const channel& theImage) {
    channel8 chnl;
    chnl.castFrom(theImage);
    return save(filename,chnl);
  }

  bool saveJPEG::apply(const image& theImage) {
    return save(getParameters().filename,theImage);
  }

  bool saveJPEG::apply(const channel& theImage) {
    return save(getParameters().filename,theImage);
  }

  bool saveJPEG::apply(const channel8& theImage) {
    return save(getParameters().filename,theImage);
  }

}

#else

#ifndef HAVE_LOCALJPEG
#  ifdef _LTI_MSC_VER
#    pragma message("WARNING: No JPEG support found.")
#  else
#    warning "WARNING: No JPEG support found."
#  endif
#endif

#endif
