/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiPNGLibFunctor.cpp
 * authors ....: Xinghan Yu, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 25.10.99
 * revisions ..: $Id: ltiPNGLibFunctor.cpp,v 1.11 2006/09/22 17:02:28 ltilib Exp $
 */

#include "ltiConfig.h"

/*
 * Implementation for PNG-Lib
 * The libpng functor has a higher priority than the local one. 
 * If HAVE_LIBPNG is defined, then this will be compiled
 */

#if defined(HAVE_LIBPNG)

#include <fstream>
#include <vector>
#include <map>
#include <png.h>
#include "ltiTypes.h"
#include "ltiPNGFunctor.h"
#include "ltiGetStreamFd.h"

#undef _LTI_DEBUG
//#define _LTI_DEBUG
#include "ltiDebug.h"

namespace lti {
  // -----------------------------------------------------------------------
  // ioPNG
  // -----------------------------------------------------------------------

  ioPNG::parameters::parameters() : ioFunctor::parameters(),
         bitsPerPixel(24),quantColors(0),useAlphaChannel(false)  {
  }

  ioPNG::parameters& ioPNG::parameters::copy(const parameters& other) {

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
    bitsPerPixel = other.bitsPerPixel;
    quantColors = other.quantColors;
    useAlphaChannel=other.useAlphaChannel;

    return (*this);
  }

  functor::parameters* ioPNG::parameters::clone() const  {
    return (new parameters(*this));
  }

  const char* ioPNG::parameters::getTypeName() const {
    return "ioPNG::parameters";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool ioPNG::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool ioPNG::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"bitsPerPixel",bitsPerPixel);
      lti::write(handler,"quantColors",quantColors);
      lti::write(handler,"useAlphaChannel",useAlphaChannel);
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
  bool ioPNG::parameters::write(ioHandler& handler,
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
  bool ioPNG::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool ioPNG::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"bitsPerPixel",bitsPerPixel);
      lti::read(handler,"quantColors",quantColors);
      lti::read(handler,"useAlphaChannel",useAlphaChannel);
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
  bool ioPNG::parameters::read(ioHandler& handler,
                               const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // returns the current parameters
  const ioPNG::parameters& ioPNG::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  // ----------------------------------------------------------------------
  // ioPNG
  // ----------------------------------------------------------------------

  // constructor
  ioPNG::ioPNG() {
    parameters param;
    setParameters(param);
  }

  // destructor
  ioPNG::~ioPNG() {
  }

  const char* ioPNG::getTypeName() const {
    return "ioPNG";
  }

  // ----------------------------------------------------------------------
  // loadPNG
  // ----------------------------------------------------------------------

  loadPNG::loadPNG() : ioPNG() {
  }

  loadPNG::~loadPNG() {
  }

  const char* loadPNG::getTypeName() const {
    return "loadPNG";
  }

  functor* loadPNG::clone() const {
    return (new loadPNG(*this));
  }

  //load PNG rgb image using libPNG
  bool loadPNG::apply(image& theImage) {
    // clear image:
    theImage.clear();

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }
    return load(getParameters().filename,theImage);
  }

  bool loadPNG::load(const std::string& filename,image& theImage) {
    // clear image:
    theImage.clear();

    FILE *fp;
    // open the PNG file
    if ((fp = fopen(filename.c_str(), "rb")) == NULL) {
      setStatusString("File not found");
      return false;
    }

    bool res=load(fp,theImage);

    // close the file
    fclose(fp);

    return res;
  }

  //load PNG rgb image using libPNG
  bool loadPNG::load(FILE* fp,image& theImage) {

    // clear image:
    _lti_debug("clearing image\n");
    theImage.clear();

    static const unsigned int pngByteToCheck = 4;
    png_structp pngPtr;
    png_infop infoPtr;
    png_uint_32 width, height, row;
    int bitDepth, colorType;
    png_byte buf[pngByteToCheck];
    unsigned int sigRead = pngByteToCheck;

    _lti_debug("reading signature\n");

    // read in some of the signature bytes
    if (fread(buf, 1, pngByteToCheck, fp) != pngByteToCheck) {
      setStatusString("Problems reading PNG file header");
      return false;
    }

    _lti_debug("Bytes to check: " << pngByteToCheck << "\n");
    _lti_debug("To Check: " << (int)buf[0] << " " << (int)buf[1] << " " 
               << (int)buf[2] << " " << (int)buf[3] << "\n");

    // compare the first pngByteToCheck bytes of the signature
    if (png_sig_cmp(buf, (png_size_t)0, pngByteToCheck)) {
      setStatusString("Not a PNG file");
      return false;
    }

    _lti_debug("Creating PNGstruct\n");
    // create and initialize the png_struct with the desired error
    // handler functions.
    pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (pngPtr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&pngPtr, (png_infopp)NULL, (png_infopp)NULL);
      setStatusString("Error creating PNG structures.  File too large?");
      return false;
    }

    // allocate/initialize the memory for the image information
    infoPtr = png_create_info_struct(pngPtr);
    if (infoPtr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&pngPtr, (png_infopp)NULL, (png_infopp)NULL);
      setStatusString("Could not get information chunk from PNG file");
      return false;
    }

    // set up the input control
    png_init_io(pngPtr, fp);

    // let libpng know that there are some bytes missing from the
    // start of the file
    png_set_sig_bytes(pngPtr, sigRead);

    // get all information of the png image
    png_read_info(pngPtr, infoPtr);
    png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType,
     NULL, NULL, NULL);

   // strip 16 bit files to 8 bit
    if (bitDepth == 16) {
      png_set_strip_16(pngPtr);
    }
    else if (bitDepth > 16) {
      setStatusString("Invalid bit depth (bigger than 16)");
      return false;
    }

    // convert the image from palette to rgb format
    if (colorType == PNG_COLOR_TYPE_PALETTE) {
      png_set_palette_to_rgb(pngPtr);
    }

    // convert the image from grayscale to rgb format
    else if (colorType == PNG_COLOR_TYPE_GRAY ||
             colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
      // convert 1, 2, 4 bit to 8 bit
      if (bitDepth < 8) {
        png_set_gray_1_2_4_to_8(pngPtr);
      }
      else if (!(bitDepth == 8 || bitDepth == 16)) {
        setStatusString("Invalid bit depth in grayscale image");
        return false;
      }
      png_set_gray_to_rgb(pngPtr);
     }

    // read the update information into the infoPtr
    png_read_update_info(pngPtr, infoPtr);

    // resize theImage matrix
    theImage.resize(height, width, 0);

    // allocate the memory to hold the image
    png_bytep* rowPointers = new png_bytep[height];
    int rbytes=png_get_rowbytes(pngPtr, infoPtr);
    for (row = 0; row < height; row++) {
      rowPointers[row] = (png_bytep)png_malloc(pngPtr,rbytes);
    }

    // read the entire image in one go
    png_read_image(pngPtr, rowPointers);

    // read rest of the file, get additional chunks in infoPtr
    png_read_end(pngPtr, infoPtr);

    // we need to re-get the color type because we need to know
    // if there are 4 bytes per pixel, even if the image
    // was stored as an indexed (paletted) image.
    colorType=png_get_color_type(pngPtr, infoPtr);

    // store the image to the pixel matrix row by row
    unsigned int ii;
    ubyte* ptr;
    palette::iterator it,eit;
    const bool useAlpha = (colorType == PNG_COLOR_TYPE_RGB_ALPHA ||
                           colorType == PNG_COLOR_TYPE_GRAY_ALPHA);

    if (useAlpha) {
      for (ii=0; ii < height; ++ii) {
        ptr = (ubyte*)rowPointers[ii];
        it=theImage.getRow(ii).begin();
        eit=theImage.getRow(ii).end();
        for (; it != eit; ++it, ptr+=4) {
          (*it).set(ptr[0], ptr[1], ptr[2], ptr[3]);
//           (*it).setRed(*ptr);
//           ++ptr;
//           (*it).setGreen(*ptr);
//           ++ptr;
//           (*it).setBlue(*ptr);
//           ++ptr;
//           (*it).setAlpha(*ptr);
//           ++ptr;
        }
      }
    } else {
      for (ii=0; ii < height; ++ii) {
        ptr = (ubyte*)rowPointers[ii];
        it=theImage.getRow(ii).begin();
        eit=theImage.getRow(ii).end();
        for (; it != eit; ++it, ptr+=3) {
          (*it).set(ptr[0], ptr[1], ptr[2],0);
//           (*it).setRed(*ptr);
//           ++ptr;
//           (*it).setGreen(*ptr);
//           ++ptr;
//           (*it).setBlue(*ptr);
//           ++ptr;
        }
      }
    }

    for (row = 0; row < height; row++) {
      png_free(pngPtr,rowPointers[row]);
    }

    // clean up after the read, get free any memory allocated
    png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)NULL);

    delete[] rowPointers;
    rowPointers=0;

    return true;
  }



  // load channel and palette using libPNG
  bool loadPNG::apply(channel8& theChannel, lti::palette& colors) {

    // clear image:
    theChannel.clear();
    colors.clear();

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    const parameters& param = getParameters();
    static const unsigned int pngByteToCheck = 4;
    png_structp pngPtr;
    png_infop infoPtr;
    png_uint_32 width, height, row;
    int bitDepth, colorType, numPalette;
    FILE *fp;
    png_byte buf[pngByteToCheck];
    unsigned int sigRead = pngByteToCheck;
    png_colorp pngPalette;
    bool error = true;

    const char *fileName = param.filename.c_str();

    // open the PNG file
    if ((fp = fopen(fileName, "rb")) == NULL) {
      setStatusString("File not found");
      return false;
    }

    // read in some of the signiture bytes
    if (fread(buf, 1, pngByteToCheck, fp) != pngByteToCheck) {
      setStatusString("Problems reading PNG file header");
      return false;
    }

    // compare the first pngByteToCheck bytes of the signature
    if (png_sig_cmp(buf, (png_size_t)0, pngByteToCheck)) {
      setStatusString("Not a PNG file");
      return false;
    }

    // create and initialize the png_struct with the desired error
    // handler functions
    pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (pngPtr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&pngPtr, (png_infopp)NULL, (png_infopp)NULL);
      setStatusString("Error creating PNG structures.  File too large?");
      return false;
    }

    // allocate/initialize the memory for the image information
    infoPtr = png_create_info_struct(pngPtr);
    if (infoPtr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&pngPtr, (png_infopp)NULL, (png_infopp)NULL);
      setStatusString("Could not get information chunk from PNG file");
      return false;
    }

    // set up the input control
    png_init_io(pngPtr, fp);

    // let libpng know that there are some bytes missing from the
    // start of the file
    png_set_sig_bytes(pngPtr, sigRead);

    // get all information of the png image
    png_read_info(pngPtr, infoPtr);
    png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType,
                 NULL, NULL, NULL);

    // get the palette of the image from the infoPtr directly
    numPalette = infoPtr->num_palette;
    pngPalette = infoPtr->palette;

    // convert images and handle palette in 2 different cases below,
    // grayscale or palette data grayscale image convertion
    if (colorType == PNG_COLOR_TYPE_GRAY ||
        colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
      if (colorType == PNG_COLOR_TYPE_GRAY) {

        if (bitDepth < 8) {
          // convert 1, 2, 4 bit grayscale to 8 bit
          png_set_gray_1_2_4_to_8(pngPtr);
        }
        else if (bitDepth == 16) {
          // strip 16 bit grayscale to 8 bit
          png_set_strip_16(pngPtr);
        }
        else if (bitDepth != 8) {
          setStatusString("Grayscale PNG file with wrong bit depth");
          return false;
        }
      }

      // grayscale image with alpha channel convertion
      else if (colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
        // strip the alpha channel
        png_set_strip_alpha(pngPtr);
        if (bitDepth == 16) {
          // strip 16 bit to 8 bit
          png_set_strip_16(pngPtr);
        }
        else if (bitDepth != 8) {
          setStatusString("Grayscale PNG file with wrong bit depth");
          return false;
        }
      }

      // save the updated information into the infoPtr
      png_read_update_info(pngPtr, infoPtr);

      // Handle the palette
      numPalette = 1<<bitDepth; // 2^bitDepth
      // numPalette = 256;
      colors.resize(numPalette);
      float scalar = 255.0/(numPalette-1);
      for (int i=0;i<colors.size();++i) {
        colors.at(i)=rgbPixel(static_cast<ubyte>(i*scalar),
                              static_cast<ubyte>(i*scalar),
                              static_cast<ubyte>(i*scalar));
      }
      error = false;
    }
    else if (colorType == PNG_COLOR_TYPE_PALETTE) {
      png_set_packing(pngPtr);
      // save the updated information into the infoPtr
      png_read_update_info(pngPtr, infoPtr);
      // resize the lti palette
      colors.resize(numPalette, rgbPixel(), false, false);

      // store the PNG palette to the lti palette
      for (int ii=0; ii < numPalette; ii++) {
        colors.at(ii).setRed(ubyte(pngPalette[ii].red));
        colors.at(ii).setGreen(ubyte(pngPalette[ii].green));
        colors.at(ii).setBlue(ubyte(pngPalette[ii].blue));
      }
      error = false;
    } 


    if (error) {
      setStatusString("Wrong color type, not a gray or indexed image");

      // clean up after the read, and free any memory allocated
      png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)NULL);
      
      // close the file
      fclose(fp);
      
      return false;
    }

    // read in the image, store it into the channel8 matrix
    // resize theChannel matrix
    theChannel.resize(height, width, 0);

    // allocate the memory to hold the image
    png_bytep* rowPointers = new png_bytep[height];
    for (row = 0; row < height; row++) {
      rowPointers[row] = (png_bytep)png_malloc(pngPtr,
                                               png_get_rowbytes(pngPtr,
                                                                infoPtr));
    }

    // read the entire image in one go
    png_read_image(pngPtr, rowPointers);

    // read the rest of the file, get additional chunks in infoPtr
    png_read_end(pngPtr, infoPtr);

    unsigned int jj;
    ubyte* ptr;
    vector<channel8::value_type>::iterator it,eit;

    // store the image to theChannel matrix row by row
    for (jj = 0; jj < height; jj++) {
      it=theChannel.getRow(jj).begin();
      eit=theChannel.getRow(jj).end();
      ptr = (ubyte*)rowPointers[jj];
      for (;it!=eit;++it,++ptr) {
        (*it)=*ptr;
      }
    }

    // free the palette memory
    pngPalette = 0;

    for (row = 0; row < height; row++) {
      png_free(pngPtr,rowPointers[row]);
    }

    // clean up after the read, and free any memory allocated
    png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)NULL);

    // close the file
    fclose(fp);

    delete[] rowPointers;
    return true;
  }

  // (load file from stream)
  bool loadPNG::load(std::ifstream& file,image& theImage) {
    getStreamFd fdGetter;
    fdGetter.setDuplicate();

    FILE* f=0;

    bool success=fdGetter.apply(file,f);
    if (success) {
      success=load(f,theImage);
      fclose(f);
    } else {
      setStatusString("Cannot open a file on descriptor");
    }
    return success;
  }

  // shortcut for apply (load file with name "filename")
  bool loadPNG::load(const std::string& filename,
         channel8& theChannel,
         lti::palette& colors) {
    parameters param(getParameters());
    param.filename = filename;
    setParameters(param); // a copy of param will be made!
    return apply(theChannel,colors);
  }

  bool loadPNG::checkHeader(const std::string& filename,
                            point& imageSize,
                            int& bitsPerPixel,
                            int& imageType) {

    static const unsigned int pngByteToCheck = 4;
    png_structp pngPtr;
    png_infop infoPtr;
    unsigned int sigRead = pngByteToCheck;
    png_uint_32 width, height;
    int bitDepth, colorType;
    png_byte buf[pngByteToCheck];
    FILE *fp;

    const char *fileName = filename.c_str();

    // open the PNG file
    if ((fp = fopen(fileName, "rb")) == NULL) {
      setStatusString("File could not be found: ");
      appendStatusString(fileName);
      return false;
    }

    // read in some of the signature bytes
    if (fread(buf, 1, pngByteToCheck, fp) != pngByteToCheck) {
      setStatusString("Problems in reading header of the PNG file");
      return false;
    }

    // compare the first pngByteToCheck bytes of the signature
    if (png_sig_cmp(buf, (png_size_t)0, pngByteToCheck)) {
      setStatusString("Not a PNG file");
      return false;
    }

    // create and initialize the png_struct with the desired error
    // handler functions.
    pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (pngPtr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&pngPtr, (png_infopp)NULL, (png_infopp)NULL);
      setStatusString("The pngPtr of the PNG file does not exist");
      return false;
    }

    // allocate/initialize the memory for the image information
    infoPtr = png_create_info_struct(pngPtr);
    if (infoPtr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&pngPtr, (png_infopp)NULL, (png_infopp)NULL);
      setStatusString("The information chunk of the PNG file does not exist");
      return false;
    }

    // set up the input control
    png_init_io(pngPtr, fp);

    // let libpng know that there are some bytes missing from the
    // start of the file
    png_set_sig_bytes(pngPtr, sigRead);

    // get all information of the png image
    png_read_info(pngPtr, infoPtr);
    png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType,
     NULL, NULL, NULL);

    // return the values to the parameters
    imageSize.x = width;
    imageSize.y = height;
    bitsPerPixel = bitDepth;
    imageType = colorType;

    // clean up after the read, and free any memory allocated
    png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)NULL);

    // close the file
    fclose(fp);
    return true;
  }


  // ------------------------------------------------------------------
  // savePNG
  // ------------------------------------------------------------------

  savePNG::savePNG() : ioPNG() {
  }

  savePNG::~savePNG() {
  }

  const char* savePNG::getTypeName() const {
    return "savePNG";
  }

  functor* savePNG::clone() const {
    return (new savePNG(*this));
  }


  bool savePNG::apply(const image& theImage) {

    png_structp pngPtr;
    png_infop infoPtr;
    png_uint_32  width, height, row;
    int bitDepth, colorType;
    FILE *fp;

    // open the file
    const parameters& param = getParameters();
    const char *fileName = param.filename.c_str();

    if ((fp = fopen(fileName, "wb")) == NULL) {
      setStatusString("File could not be opened: ");
      appendStatusString(fileName);
      return false;
    }

    // create and initialize the png_struct with the error handler functions
    pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (pngPtr == NULL) {
      fclose(fp);
      png_destroy_write_struct(&pngPtr, (png_infopp)NULL);
      setStatusString("Write struct could not be created");
      return false;
    }

    // allocate / initialize the image information data
    infoPtr = png_create_info_struct(pngPtr);
    if (infoPtr == NULL) {
      fclose(fp);
      png_destroy_write_struct(&pngPtr, (png_infopp)NULL);
      setStatusString("Info struct could not be created");
      return false;
    }

    // set up the output control
    png_init_io(pngPtr, fp);

    // set/write the image information into infoPtr
    width = (png_uint_32)theImage.columns();
    height = (png_uint_32)theImage.rows();
    bitDepth = 8;
    if (param.useAlphaChannel) {
      colorType = PNG_COLOR_TYPE_RGB_ALPHA;
    } else {
      colorType = PNG_COLOR_TYPE_RGB;
    }
    png_set_IHDR(pngPtr, infoPtr, width, height, bitDepth, colorType,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_write_info(pngPtr, infoPtr);

    // allocation of rowPointers
    png_bytep* rowPointers = new png_bytep[height];
    for (row = 0; row < height; row++) {
      rowPointers[row] = (png_bytep)png_malloc(pngPtr,
                                               png_get_rowbytes(pngPtr,
                                                                infoPtr));
    }

    palette::const_iterator it,eit;
    png_bytep ptr;
    const bool alpha = param.useAlphaChannel;
    // store the image to rowPointers row by row
    for (unsigned int ii=0; ii < height; ++ii) {
      ptr = rowPointers[ii];
      it = theImage.getRow(ii).begin();
      eit = theImage.getRow(ii).end();
      for (;it!=eit;++it) {
        *ptr = (png_byte)(*it).getRed();
        ++ptr;
        *ptr = (png_byte)(*it).getGreen();
        ++ptr;
        *ptr = (png_byte)(*it).getBlue();
        ++ptr;
        if (alpha) {
          *ptr = (png_byte)(*it).getDummy();
          ++ptr;
        }
      } // column
    } // row

    // write out the entire image in one go
    png_write_image(pngPtr, rowPointers);

    // finish writing the rest of the file
    png_write_end(pngPtr, infoPtr);

    for (row = 0; row < height; row++) {
      png_free(pngPtr,rowPointers[row]);
    }

    // clean up after the write, free any memory allocated
    png_destroy_write_struct(&pngPtr, &infoPtr);

    // close the file
    fclose(fp);

    delete[] rowPointers;
    return true;
  }

  // saves float-channel as an 8-bit gray image
  bool savePNG::apply(const channel& theChannel) {

    channel8 chnl;

    chnl.castFrom(theChannel);
    return apply(chnl);
  }

  // save 8-bit channel using libPNG
  bool savePNG::apply(const channel8& theChannel,
          const lti::palette& colors) {

    // check if the given palette is ok
    int bitDepth, colorType, paletteSize;
    lti::palette thePalette;
    if (colors.size() == 0) {
      thePalette.resize(256,0,false,false);
      // generate a gray-valued palette
      int i;
      for (i=0;i<256;i++) {
        thePalette.at(i)=rgbPixel(i,i,i);
      }
      bitDepth = 8;
      paletteSize = 256;
    } else {
      channel8::value_type maxIdx = theChannel.maximum();
      if (maxIdx < 2) {
        bitDepth = 1;
      } else if (maxIdx < 4) {
        bitDepth = 2;
      } else if (maxIdx < 16) {
        bitDepth = 4;
      } else {
        bitDepth = 8;
      }
      paletteSize = 1 << bitDepth;
      thePalette.resize(paletteSize,0,false,false);
      thePalette.fill(colors);
      int i;
      for (i=colors.size();i<thePalette.size();++i) {
        thePalette.at(i)=rgbPixel(i,i,i);
      }
    }

    png_structp pngPtr;
    png_infop infoPtr;
    png_uint_32 width, height, row, numPalette;
    png_colorp pngPalette;
    FILE *fp;

    // open the file
    const char *fileName = (getParameters().filename).c_str();
    if ((fp = fopen(fileName, "wb")) == NULL) {
      setStatusString("File does not exist");
      return false;
    }

    // create and initialize the png_struct with the error handler functions
    pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (pngPtr == NULL) {
      fclose(fp);
      png_destroy_write_struct(&pngPtr, (png_infopp)NULL);
      setStatusString("Write struct could not be created");
      return false;
    }

    // allocate / initialize the image information data
    infoPtr = png_create_info_struct(pngPtr);
    if (infoPtr == NULL) {
      fclose(fp);
      png_destroy_write_struct(&pngPtr, (png_infopp)NULL);
      setStatusString("Info struct could not be created");
      return false;
    }

    // set up the output control
    png_init_io(pngPtr, fp);

    // set/write the image information into infoPtr
    width = (png_uint_32)theChannel.columns();
    height = (png_uint_32)theChannel.rows();
    colorType = PNG_COLOR_TYPE_PALETTE;

    png_set_IHDR(pngPtr, infoPtr, width, height, bitDepth, colorType,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_write_info_before_PLTE(pngPtr, infoPtr);

    if (bitDepth < 8) {
      png_set_packing(pngPtr);
    }

    // store the lti palette into the PNG palette
    numPalette = (png_uint_32)thePalette.size();

    // allocate the pngPalette
    pngPalette = (png_colorp)png_malloc(pngPtr, numPalette*sizeof(png_color));
    for (unsigned int jj=0; jj < numPalette; ++jj) {
      pngPalette[jj].red   = thePalette.at(jj).getRed();
      pngPalette[jj].green = thePalette.at(jj).getGreen();
      pngPalette[jj].blue  = thePalette.at(jj).getBlue();
    }

    png_set_PLTE(pngPtr, infoPtr, pngPalette, numPalette);
    png_write_info(pngPtr, infoPtr);

    // write the image into the file
    // allocation of rowPointers
    png_bytep* rowPointers = new png_bytep[height];
    //png_uint_32 rbytes=png_get_rowbytes(pngPtr,infoPtr);
    for (row = 0; row < height; row++) {
      rowPointers[row] = (png_bytep)png_malloc(pngPtr, width);
    }

    // store the image from theChannel to rowPointers
    vector<channel8::value_type>::const_iterator it,eit;
    png_bytep ptr;

    bool result=true;

    // we just need to copy the bytes
    for (unsigned int ii = 0; ii < height; ii++) {
      ptr = rowPointers[ii];
      it=theChannel.getRow(ii).begin();
      eit=theChannel.getRow(ii).end();
      for (;it!=eit;++it,++ptr) {
        *ptr = (png_byte)(*it);;
      }
    }

    png_write_image(pngPtr, rowPointers);
    png_write_end(pngPtr, infoPtr);

    for (row = 0; row < height; row++) {
      png_free(pngPtr,rowPointers[row]);
    }
    png_free(pngPtr,pngPalette);  // should we do this?

    // free the allocated memory
    png_destroy_write_struct(&pngPtr, &infoPtr);

    // close the file
    fclose(fp);

    delete[] rowPointers;

    return result;
  }


  /*
   * shortcut for save
   */
  bool savePNG::save(const std::string& filename,
         const image& theImage) {
    parameters param;
    if (validParameters()) {
      param.copy(getParameters());
    }

    param.filename = filename;
    setParameters(param); // a copy of param will be made!
    return apply(theImage);
  }

  /*
   * shortcut for save
   */
  bool savePNG::save(const std::string& filename,
         const channel8& theChannel,
         const lti::palette& colors) {
    parameters param;
    if (validParameters()) {
      param.copy(getParameters());
    }

    param.filename = filename;
    setParameters(param); // a copy of param will be made!
    return apply(theChannel,colors);
  }

  /*
   * shortcut for save
   */
  bool savePNG::save(const std::string& filename,
         const channel& theChannel) {

    parameters param;
    if (validParameters()) {
      param.copy(getParameters());
    }

    param.filename = filename;
    setParameters(param);             // a copy of param will be made!
    return apply(theChannel);
  }

}

#else

#ifndef HAVE_LOCALPNG
#  ifdef _LTI_MSC_VER
#    pragma message("WARNING: No PNG support found.")
#  else
#    warning "WARNING: No PNG support found."
#  endif
#endif

#endif
