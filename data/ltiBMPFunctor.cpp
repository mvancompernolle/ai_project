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
 * file .......: ltiBMPFunctor.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 23.04.99
 * revisions ..: $Id: ltiBMPFunctor.cpp,v 1.13 2008/05/29 02:58:10 alvarado Exp $
 */

#include "ltiRGBPixel.h"
#include "ltiBMPFunctor.h"
#include "ltiUsePalette.h"
#include "ltiMergeRGBToImage.h"

#include <fstream>
#include <vector>
#include <map>

namespace lti {
  // -----------------------------------------------------------------------
  // ioBMP
  // -----------------------------------------------------------------------

  ioBMP::parameters::parameters() : ioFunctor::parameters(),
                                    compression(false),bitsPerPixel(24),quantColors(0)  {
  }

  ioBMP::parameters& ioBMP::parameters::copy(const parameters& other) {
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
    compression = other.compression;
    bitsPerPixel = other.bitsPerPixel;
    quantColors = other.quantColors;

    return (*this);
  }

  functor::parameters* ioBMP::parameters::clone() const  {
    return (new parameters(*this));
  }

  const char* ioBMP::parameters::getTypeName() const {
    return "ioBMP::parameters";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool ioBMP::parameters::write(ioHandler& handler,
                                const bool complete) const
# else
    bool ioBMP::parameters::writeMS(ioHandler& handler,
                                    const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"compression",compression);
      lti::write(handler,"bitsPerPixel",bitsPerPixel);
      lti::write(handler,"quantColors",quantColors);
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
  bool ioBMP::parameters::write(ioHandler& handler,
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
  bool ioBMP::parameters::read(ioHandler& handler,
                               const bool complete)
# else
    bool ioBMP::parameters::readMS(ioHandler& handler,
                                   const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"compression",compression);
      lti::read(handler,"bitsPerPixel",bitsPerPixel);
      lti::read(handler,"quantColors",quantColors);
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
  bool ioBMP::parameters::read(ioHandler& handler,
                               const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // -------------------------------------------------------
  //  ioBMP::header
  // -------------------------------------------------------

  ioBMP::header::header() : type(0x4d42),size(0),
                            reserved1(0),reserved2(0),offsetPixels(0) {
  }

  bool ioBMP::header::read(std::ifstream &in) {
    uint16 tmpType,tmpReserved;
    io.read(in,tmpType);  // io reads with the correct endianness

    if (tmpType != 0x4d42)
      return false;

    io.read(in,size);
    io.read(in,tmpReserved); // just ignore the readed data!
    io.read(in,tmpReserved); // just ignore the readed data!
    io.read(in,offsetPixels);

    return true;
  }

  bool ioBMP::header::write(std::ofstream &out) {
    io.write(out,type);  // io writes with the correct endianness
    io.write(out,size);
    io.write(out,reserved1);
    io.write(out,reserved2);
    io.write(out,offsetPixels);

    return true;
  }

  // -------------------------------------------------------
  //  ioBMP::header
  // -------------------------------------------------------

  ioBMP::infoHeader::infoHeader()
    : size(40), width(0), height(0),
      planes(0), bitCount(0),compression(0),sizeImage(0),xPixPerMeter(0),
      yPixPerMeter(0),colorsUsed(0),colorsImportant(0) {
  }

  bool ioBMP::infoHeader::read(std::ifstream &in) {
    uint32 tmpSize; // counter to check if the size of the header is right

    // read size of this header
    io.read(in,size);
    tmpSize  = sizeof(size);
    if (tmpSize < size) {
      io.read(in,width);
      tmpSize += sizeof(width);
    }
    if (tmpSize < size) {
      io.read(in,height);
      tmpSize += sizeof(height);
    }
    if (tmpSize < size) {
      io.read(in,planes);
      tmpSize += sizeof(planes);
    }
    if (tmpSize < size) {
      io.read(in,bitCount);
      tmpSize += sizeof(bitCount);
    }
    if (tmpSize < size) {
      io.read(in,compression);
      tmpSize += sizeof(compression);
    }
    if (tmpSize < size) {
      io.read(in,sizeImage);
      tmpSize += sizeof(sizeImage);
    }
    if (tmpSize < size) {
      io.read(in,xPixPerMeter);
      tmpSize += sizeof(xPixPerMeter);
    }
    if (tmpSize < size) {
      io.read(in,yPixPerMeter);
      tmpSize += sizeof(yPixPerMeter);
    }
    if (tmpSize < size) {
      io.read(in,colorsUsed);
      tmpSize += sizeof(colorsUsed);
    }
    if (tmpSize < size) {
      io.read(in,colorsImportant);
      tmpSize += sizeof(colorsImportant);
    }

    if (size > tmpSize) {
      // load extra data
      char* buffer;
      buffer = new char[size-tmpSize+2];
      in.read(buffer,size-tmpSize);
      delete[] buffer;
    }

    return true;
  }

  bool ioBMP::infoHeader::write(std::ofstream &out) {
    size = length();
    io.write(out,size);
    io.write(out,width);
    io.write(out,height);
    io.write(out,planes);
    io.write(out,bitCount);
    io.write(out,compression);
    io.write(out,sizeImage);
    io.write(out,xPixPerMeter);
    io.write(out,yPixPerMeter);
    io.write(out,colorsUsed);
    io.write(out,colorsImportant);

    return true;
  }

  // -------------------------------------------------------
  //  ioBMP::palette
  // -------------------------------------------------------
  bool ioBMP::palette::read(std::ifstream &in) {
    int i;
    uint32 v;
    for (i=0;(i<size()) && in.good() && in.is_open() && (!in.eof());i++) {
      io.read(in,v); // read with correct endianness!
      at(i).setValue(v);
    }

    return true;
  }

  bool ioBMP::palette::write(std::ofstream &out) {
    int i;
    for (i=0;i<size();i++) {
      io.write(out,at(i).getValue()); // write with correct endianness!
    }
    return true;
  }

  // returns the current parameters
  const ioBMP::parameters& ioBMP::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  // constructor
  ioBMP::ioBMP() {
  }

  const char* ioBMP::getTypeName() const {
    return "ioBMP";
  }

  // ----------------------------------------------------------------------
  // loadBMP
  // ----------------------------------------------------------------------
  // default constructor
  loadBMP::loadBMP() : ioBMP() {
  }

  // returns name of the file
  const char* loadBMP::getTypeName() const {
    return "loadBMP";
  }

  // clone this object
  functor* loadBMP::clone() const {
    return (new loadBMP(*this));
  }

  // read file
  bool loadBMP::apply(image& theImage) {

    // are the parameters already defined?
    if (!validParameters()) {
      // no parameters -> don't know what to do...

      theImage.clear();

      setStatusString("Invalid parameters");
      return false;
    }

    // open file to read
    std::ifstream in;

    in.open(getParameters().filename.c_str(),std::ios::in |
            std::ios::binary);
    bool success=applyFromStream(in,theImage);
    in.close();
    return success;
  }

  bool loadBMP::applyFromStream(std::ifstream& in, image& theImage) {
    bool success=false;
    if (in.good() && in.is_open()) {
      // read header and infoheader
      if (!theHeader.read(in)) {
        setStatusString("Not a BMP file");
        return false;
      }
      if (!theInfoHeader.read(in)) {
        setStatusString("Wrong info header");
        return false;
      }

      // how long is the color table?
      int colorEntries;
      colorEntries = theHeader.offsetPixels -
        theInfoHeader.size -
        theHeader.length();

      // this is a valid value only if it's divisible by 4!
      if ((colorEntries % 4) != 0) {
        theImage.clear();
        setStatusString("Invalid number of color entries in BMP file");
        return false;
      }

      colorEntries /= 4;

      // read color entries
      thePalette.resize(colorEntries,0,false,false);
      thePalette.read(in);

      theImage.resize(theInfoHeader.height,
                      theInfoHeader.width,0,false,false);

      // read image
      switch (theInfoHeader.bitCount) {
        case 1:
          success = load1bit(in,theImage);
          break;
        case 4:
          success = load4bit(in,theImage);
          break;
        case 8:
          success = load8bit(in,theImage);
          break;
        case 24:
          success = load24bit(in,theImage);
          break;
        default:
          // unknown bit/pixel
          theImage.clear();
          setStatusString("Unknown number of bits per pixel in BMP file");
          return false;
      }
    } // if (good) ...
    else {
      setStatusString("Could not open BMP file (does it exist?)");
      return false;
    }

    return success;
  }

  // read file
  bool loadBMP::apply(channel8& theChannel,lti::palette& colors) {

    // are the parameters already defined?
    if (!validParameters()) {

      theChannel.clear();

      // no parameters -> don't know what to do...
      setStatusString("Invalid parameters");
      return false;
    }

    bool success = false;

    // open file to read
    std::ifstream in;
    in.open(getParameters().filename.c_str(),std::ios::in |
            std::ios::binary);
    if (in.good() && in.is_open()) {
      // read header and infoheader
      if (!theHeader.read(in)) {
        setStatusString("Wrong header. Is this a BMP file?");
        return false;
      }
      if (!theInfoHeader.read(in)) {
        setStatusString("Wrong info header");
        return false;
      }

      // how long is the color table?
      int colorEntries;
      colorEntries = theHeader.offsetPixels -
        theInfoHeader.size -
        theHeader.length();

      // this is a valid value only if it's divisible by 4!
      if ((colorEntries % 4) != 0) {
        in.close();
        theChannel.clear();
        setStatusString("Invalid number of color entries");
        return false;
      }

      colorEntries /= 4;

      // read color entries
      thePalette.resize(colorEntries,0,false,false);
      thePalette.read(in);
      colors.copy(thePalette);

      theChannel.resize(theInfoHeader.height,
                        theInfoHeader.width,0,false,false);

      // read image
      switch (theInfoHeader.bitCount) {
        case 1: {
          success = load1bit(in,theChannel);
        }
          break;
        case 4: {
          success = load4bit(in,theChannel);
        }
          break;
        case 8: {
          success = load8bit(in,theChannel);
        }
          break;
        case 24: {
          theChannel.clear();
          setStatusString("24 bit image cannot be casted to channel");
          success = false;
        }
          break;
        default:
          // unknown bit/pixel
          theChannel.clear();
          setStatusString("Unknown number of bits per pixel in BMP file");
          success = false;
      }
    } // if (good) ...
    else {
      setStatusString("Could not open BMP file (does it exist?)");
      success = false;
    }

    in.close();
    return success;
  }

  // read file
  bool loadBMP::apply(channel& theChannel) {

    image theImage;
    if(apply(theImage)) {
      theChannel.castFrom(theImage);
      return true;
    }
    theChannel.clear();
    return false;
  }

  // read file
  int loadBMP::apply(image& theImage,
                     channel8& theChannel,
                     lti::palette& colors) {

    // are the parameters already defined?
    if (!validParameters()) {

      theChannel.clear();
      theImage.clear();
      colors.clear();

      // no parameters -> don't know what to do...
      setStatusString("Invalid parameters");
      return 0;
    }

    bool success = false;
    // open file to read
    std::ifstream in;
    in.open(getParameters().filename.c_str(),std::ios::in |
            std::ios::binary);
    if (in.good() && in.is_open()) {
      // read header and infoheader
      if (!theHeader.read(in)) {
        setStatusString("Wrong header. Is this a BMP file?");
        return false;
      }
      if (!theInfoHeader.read(in)) {
        setStatusString("Wrong info header");
        return false;
      }

      // how long is the color table?
      int colorEntries;
      colorEntries = theHeader.offsetPixels -
        theInfoHeader.size -
        theHeader.length();

      // this is a valid value only if it's divisible by 4!
      if ((colorEntries % 4) != 0) {
        in.close();

        theChannel.clear();
        theImage.clear();
        colors.clear();

        setStatusString("Invalid number of color entries in BMP file");
        return 0;
      }

      colorEntries /= 4;

      // read color entries
      thePalette.resize(colorEntries,0,false,false);
      thePalette.read(in);
      colors.copy(thePalette);

      if (theInfoHeader.bitCount < 24) {
        theChannel.resize(theInfoHeader.height,
                          theInfoHeader.width,0,false,false);
      }
      else {
        theImage.resize(theInfoHeader.height,
                        theInfoHeader.width,0,false,false);

        colors.clear();
      }

      // read image
      switch (theInfoHeader.bitCount) {
        case 1: {
          success = load1bit(in,theChannel);
        }
          break;
        case 4: {
          success = load4bit(in,theChannel);
        }
          break;
        case 8: {
          success = load8bit(in,theChannel);
        }
          break;
        case 24: {
          success = load24bit(in,theImage);
        }
          break;
        default:
          // unknown bit/pixel
          in.close();

          theChannel.clear();
          theImage.clear();
          colors.clear();

          setStatusString("Invalid number of pixels in BMP file");
          return 0;
      }
    } // if (good) ...
    else {
      in.close();
      setStatusString("Could not open BMP file.  (Does it exist?)");
    }

    in.close();
    return (success ? theInfoHeader.bitCount : 0);
  }

  // checkfile
  bool loadBMP::checkHeader(const std::string& filename,
                            point& imageSize,
                            int& bitsPerPixel,
                            int& colorEntries) {

    bitsPerPixel = 0;
    imageSize = point(0,0);
    colorEntries = 0;
    bool result = true;

    // open file to read
    std::ifstream in;
    in.open(filename.c_str(),std::ios::in | std::ios::binary);
    if (in.good() && in.is_open()) {
      // read header and infoheader
      result = result && theHeader.read(in);
      result = result && theInfoHeader.read(in);

      // how long is the color table?
      colorEntries = theHeader.offsetPixels -
        theInfoHeader.size -
        theHeader.length();

      // this is a valid value only if it's divisible by 4!
      if ((colorEntries % 4) != 0) {
        in.close();
        setStatusString("Invalid number of color entries in BMP file");
        return false;
      }

      colorEntries /= 4;

      imageSize = point(theInfoHeader.width,theInfoHeader.height);
      bitsPerPixel = theInfoHeader.bitCount;
    } // if (good) ...
    else {
      in.close();
      setStatusString("Could not open BMP file: ");
      appendStatusString(filename.c_str());
      return false;
    }

    in.close();
    return result;
  }

  // shortcut for apply (load file with name "filename")
  bool loadBMP::load(const std::string& filename,image& theImage) {

    parameters param;
    if(validParameters())
      param.copy(getParameters());
    param.filename = filename;
    setParameters(param); // a copy of param will be made!
    return apply(theImage);
  }

  // shortcut for apply (load file with name "filename")
  bool loadBMP::load(std::ifstream& file,image& theImage) {
    // file.flags(std::ios::in|std::ios::binary);
    return applyFromStream(file,theImage);
  }

  // shortcut for apply
  bool loadBMP::load(const std::string& filename,
                     channel8& theChannel,
                     lti::palette& colors) {
    parameters param;
    if(validParameters())
      param.copy(getParameters());
    param.filename = filename;
    setParameters(param); // a copy of param will be made!
    return apply(theChannel,colors);
  }

  // shortcut for apply
  bool loadBMP::load(const std::string& filename,
                     channel& theChannel) {
    parameters param;
    if(validParameters())
      param.copy(getParameters());
    param.filename = filename;
    setParameters(param); // a copy of param will be made!
    return apply(theChannel);
  }

  // shortcut for apply
  int loadBMP::load(const std::string& filename,
                    image& theImage,
                    channel8& theChannel,
                    lti::palette& colors) {
    parameters param;
    if(validParameters())
      param.copy(getParameters());
    param.filename = filename;
    setParameters(param); // a copy of param will be made!
    return apply(theImage,theChannel,colors);
  }

  bool loadBMP::load1bit(std::ifstream& in,image& theImage) {
    // don't loose time initializing the channel elements...
    channel8 theChannel;
    // but resize it anyway (this is expected by load8bit)
    theChannel.resize(theInfoHeader.height,theInfoHeader.width,0,
                      false,false);

    if (load1bit(in,theChannel)) {
      usePalette applyPalette; // to apply a palette in a channel
      applyPalette.apply(theChannel,thePalette,theImage);
    } else {
      return false;
    }
    return true;
  }

  bool loadBMP::load1bit(std::ifstream& in,channel8& theChannel) {
    unsigned int x,t;
    int y,mc,b;
    ubyte someBits;

    ubyte* buffer;
    unsigned int bufferSize;

    // every line have 4*i elements (with i integer)
    // the number of rest-bytes 'b' in the file, can be calculated as
    // t = (4 - (x mod 4)) mod 4;
    x = (theInfoHeader.width+7)/8;  // number of bytes required per line
    t = (4-(x%4))%4;

    bufferSize = (x+t)*theInfoHeader.height;

    if ((theInfoHeader.sizeImage>0)&&
        (bufferSize > theInfoHeader.sizeImage)) {
      setStatusString("Invalid header in BMP file");
      return false;
    }

    if (theInfoHeader.sizeImage > bufferSize) {
      bufferSize = theInfoHeader.sizeImage;
    };

    buffer = new ubyte[bufferSize];
    // read the data
    in.read((char*)buffer,bufferSize);


    mc = 0;
    for (y = theInfoHeader.height-1;y>=0;y--) {
      x = 0;
      while (x < theInfoHeader.width) {
        // read 1 byte
        someBits = buffer[mc++];
        // extract bits
        for (b=0;(b<8) && (x<theInfoHeader.width);b++,x++) {
          theChannel.at(y,x) = ((someBits & 0x80) == 0) ? 0 : 1;
          someBits = someBits << 1; // shift to the right!
        }
      }
      mc+=t;
    }
    delete[] buffer;

    return true;
  }

  bool loadBMP::load4bit(std::ifstream& in,image& theImage) {
    // don't loose time initializing the channel elements...
    channel8 theChannel;
    // but resize it anyway (this is expected by load8bit)
    theChannel.resize(theInfoHeader.height,theInfoHeader.width,0,
                      false,false);

    if (load4bit(in,theChannel)) {
      usePalette applyPalette; // to apply a palette in a channel
      applyPalette.apply(theChannel,thePalette,theImage);
    } else {
      return false;
    }
    return true;
  }



  bool loadBMP::load4bit(std::ifstream& in,channel8& theChannel) {
    unsigned int x,t;
    int y,mc;
    ubyte someNybbles;


    switch (theInfoHeader.compression) {
      case 0: {
        ubyte* buffer;
        unsigned int bufferSize;

        // every line must have 4*i bytes (with i integer)
        // the number of rest-bytes 't' in the file, can be calculated as
        // t = (4 - (x mod 4)) mod 4;
        x = (theInfoHeader.width+1)/2; //number of bytes used in a line!
        t = (4 - (x % 4)) % 4;

        bufferSize = (x+t)*theInfoHeader.height;

        if ((theInfoHeader.sizeImage>0)&&
            (bufferSize > theInfoHeader.sizeImage)) {
          setStatusString("Invalid header in BMP file");
          return false;
        }

        if (theInfoHeader.sizeImage > bufferSize) {
          bufferSize = theInfoHeader.sizeImage;
        }

        buffer = new ubyte[bufferSize];
        // read the data
        in.read((char*)buffer,bufferSize);


        mc = 0; // memory counter : position in the buffer
        // no compression
        for (y = theInfoHeader.height-1;y>=0;y--) {
          x = 0;
          while (x < theInfoHeader.width) {
            // read 1 byte
            someNybbles = buffer[mc++];
            // extract bits
            theChannel.at(y,x++) = someNybbles >> 4; // first four bits!
            if (x<theInfoHeader.width) {
              theChannel.at(y,x++) = someNybbles & 0x0F; // shift to the right!
            }
          }
          mc+=t;
        }
        delete[] buffer;
      }
        break;

      case 1: {
        // RLE 8 compression
        setStatusString("RLE 8 not available for 4-bit images");
        return false;
        // not avalilable for 4-bit
      }
        break;

      case 2: { // RLE 4 compression
        ubyte num,c;
        unsigned int x;
        int y,i,mc;

        // first load the whole image file in a memory buffer (faster!)
        ubyte* buffer;
        const int bufferSize = theInfoHeader.sizeImage;

        buffer = new ubyte[bufferSize];
        // read the data
        in.read((char*)buffer,bufferSize);

        x = 0;
        y = theInfoHeader.height-1;
        theChannel.fill(channel8::value_type(0));
        mc = 0; // memory counter: position in the buffer

        while (y>=0) {
          // read 1 byte
          num = buffer[mc++];

          if (num == 0) {
            // read command
            num = buffer[mc++];
            if (num<3) { // is command
              switch (num) {
                case 0: // fill line with 0! (ready!)
                  x = 0;
                  y--;
                  break;
                case 1: // end of graphic
                  x = 0;
                  y = -1; // flag to indicate EOG
                  break;
                case 2:
                  x+=buffer[mc++]; // offset x
                  y+=buffer[mc++]; // offset y
              }
            } else { // no command -> load normal data stream
              for (i=0;i<num;i++) {
                c = buffer[mc++];
                int low,high;
                high = (c >> 4) & 0x0F;
                theChannel.at(y,x++) = high;
                i++;
                if (i<num) {
                  low = c & 0x0F;
                  theChannel.at(y,x++) = low;
                }
              }
              if ((mc & 0x01) != 0) { // odd?
                // read dummy value
                mc++;
              }
            }
          } else { // no special command
            c = buffer[mc++]; // c must be copied 'num'-times
            int high,low;
            low = c & 0x0F;
            high = (c >> 4) & 0x0F;

            for (i=0;i<num;i++) {
              theChannel.at(y,x++) = high;
              i++;
              if (i<num) {
                theChannel.at(y,x++) = low;
              }
            }
          }
        }
        delete[] buffer;
      }
        break;
    }

    return true;
  }

  bool loadBMP::load8bit(std::ifstream& in,image& theImage) {
    // don't loose time initializing the channel elements...
    channel8 theChannel;
    // but resize it anyway (this is expected by load8bit)
    theChannel.resize(theInfoHeader.height,theInfoHeader.width,0,
                      false,false);

    if (load8bit(in,theChannel)) {
      usePalette applyPalette; // to apply a palette in a channel
      applyPalette.apply(theChannel,thePalette,theImage);
    } else {
      return false;
    }

    return true;
  }

  bool loadBMP::load8bit(std::ifstream& in,channel8& theChannel) {
    switch(theInfoHeader.compression) {
      case 0: {
        // no compression
        unsigned int x,t,bufferSize,lineSize;
        int y,i;
        ubyte* buffer;

        // first read all data to a buffer!
        // calculate how big must be the buffer?
        // every line have 4*i byte elements (with i integer)
        // the number of rest-bytes 't' in the file, can be calculated as
        // t = (4 - (columns mod 4)) mod 4;
        x = theInfoHeader.width; // real number of bytes pro line
        t = (4 - (x % 4)) % 4;
        bufferSize = (x+t)*theInfoHeader.height;
        lineSize = t+theInfoHeader.width;

        if ((theInfoHeader.sizeImage>0)&&
            (bufferSize > theInfoHeader.sizeImage)) {
          setStatusString("Invalid header in BMP file");
          return false;
        }

        buffer = new ubyte[bufferSize];

        // read the data
        in.read((char*)buffer,bufferSize);

        // all data readed: now transform to a channel
        i = 0;
        for (y = theInfoHeader.height-1;y>=0;y--) {
          memcpy(&theChannel.at(y,0),&buffer[i], theInfoHeader.width);
          i+=lineSize;
        }

        delete[] buffer;
      }
        break;
      case 1: { // RLE8
        ubyte num,c;
        unsigned int x;
        int y,mc;

        // first load the whole image file in a memory buffer (faster!)
        ubyte* buffer;
        const int bufferSize = theInfoHeader.sizeImage;

        buffer = new ubyte[bufferSize];
        // read the data
        in.read((char*)buffer,bufferSize);

        x = 0;
        y = theInfoHeader.height-1;
        mc = 0; // memory counter: position in the buffer

        theChannel.fill(channel8::value_type(0));

        while (y>=0) {
          // 1 byte lesen
          num = buffer[mc++];

          if (num == 0) {
            // read command
            num = buffer[mc++];
            if (num<3) {
              // is command
              switch (num) {
                case 0: // fill line with 0! (ready!)
                  x = 0;
                  y--;
                  break;
                case 1: // end of graphic
                  x = 0;
                  y = -1; // flag to indicate EOG
                  break;
                case 2:
                  x+=buffer[mc++];// offset X
                  y-=buffer[mc++];
              }
            }
            else { // no command -> normal data
              memcpy(&theChannel.at(y,x),&buffer[mc],num);
              x+=num;
              mc+=num;
              if ((num & 0x01) != 0) { // even number of data must be read!
                mc++;
              }
            }
          }
          else { // no special command
            c = buffer[mc++]; // c must be copied 'num'-times
            memset(&theChannel.at(y,x),c,min(num,
                                             ubyte(theChannel.columns()-x)));
            x+=num;
          }
        }
        delete[] buffer;
      }
        break;
      case 2: {// RLE4
        // not available for 8 bits
        setStatusString("RLE4 not available for 8bit images");
        return false;
      }
        break;
    }

    return true;
  }

  bool loadBMP::load24bit(std::ifstream& in,image& theImage) {

    // first read all data to a buffer!
    ubyte* buffer;

    // calculate how big must be the buffer?
    // every line have 4*i byte elements (with i integer)
    // the number of rest-bytes 't' in the file, can be calculated as
    // t = (4 - (columns mod 4)) mod 4;

    const int x = 3*theInfoHeader.width; // real number of bytes pro line
    const int t = (4 - (x % 4)) % 4;
    const unsigned int bufferSize = (x+t)*theInfoHeader.height;

    int y;

    if ((theInfoHeader.sizeImage>0)&&
        (bufferSize > theInfoHeader.sizeImage)) {
      setStatusString("Invalid header in BMP file");
      return false;
    }

    buffer = new ubyte[bufferSize];

    // read the data
    in.read((char*)buffer,bufferSize);

    ubyte* ptr = buffer;
    vector<rgbPixel>::iterator it,eit;
    
    for (y = theInfoHeader.height-1;y>=0;--y) {
      vector<rgbPixel>& vct = theImage.getRow(y);
      for (it=vct.begin(),eit=vct.end();it!=eit;++it,ptr+=3) {
        (*it).set(ptr[2],ptr[1],*ptr,0);
      }
      ptr+=t;
    }

    delete[] buffer;

    return true;
  }

  // ------------------------------------------------------------------
  // saveBMP
  // ------------------------------------------------------------------
  // default constructor
  saveBMP::saveBMP() : ioBMP(){
  }

  // getTypeName
  const char* saveBMP::getTypeName() const {
    return "saveBMP";
  }

  // clone
  functor* saveBMP::clone() const {
    return (new saveBMP(*this));
  }

  // apply
  bool saveBMP::apply(const image& theImage) {
    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    bool success = false;

    // open file to write
    std::ofstream out;
    out.open(getParameters().filename.c_str(),std::ios::out |
             std::ios::binary);

    if (out.good() && out.is_open()) {
      switch(getParameters().bitsPerPixel) {
        case 1:
          success = save1bit(out,theImage);
          break;

        case 4:
          success = save4bit(out,theImage);
          break;

        case 8:
          success = save8bit(out,theImage);
          break;

        case 24:
          success = save24bit(out,theImage);
          break;
      }
    } else {
      out.close();
      setStatusString("BMP file could not be written. Is the path correct?");
      return false;
    }

    out.close();
    return success;
  }


  // saves float-channel into 24-bit RGB-BMP by assigning
  // each R, G and B the same grey-value...
  bool saveBMP::apply(const channel& theChannel) {

    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    // float -> ubyte
    channel8 ch8;
    ch8.castFrom(theChannel);

    //merge channels
    image anImage;
    mergeRGBToImage merger;
    merger.apply(ch8,ch8,ch8,anImage);

    return apply(anImage);
  }

  /** saves 8-bit channel
      see ioBMP::parameters to the possible effects of this member.
  */
  bool saveBMP::apply(const channel8& theChannel,
                      const lti::palette& colors) {
    if (!validParameters()) {
      setStatusString("Invalid parameters");
      return false;
    }

    if (colors.size() == 0) { // empty palette?
      // create a standard palette
      int i;
      thePalette.resize(256,0,false,false); // new size for the palette
      for (i=0;i<256;i++) {
        thePalette.at(i)=rgbPixel(i,i,i);
      }
    } else {
      thePalette.copy(colors);
    }

    bool success = false;
    // open file to write
    std::ofstream out;
    out.open(getParameters().filename.c_str(),std::ios::out |
             std::ios::binary);

    if (out.good() && out.is_open()) {
      switch(getParameters().bitsPerPixel) {
        case 1: {
          success = save1bit(out,theChannel);
        }
          break;
        case 4: {
          success = save4bit(out,theChannel);
        }
          break;
        case 8: {
          success = save8bit(out,theChannel);
        }
          break;
        case 24: {
          image anImage;
          usePalette applyPalette;
          applyPalette.apply(theChannel,thePalette,anImage);
          success = save24bit(out,anImage);
        }
          break;
      }
    } else {
      out.close();
      setStatusString("BMP file could not be written. Is the path correct?");
      return false;
    }
    out.close();

    return success;
  }

  /* shortcut for save */
  bool saveBMP::save(const std::string& filename,
                     const channel8& theChannel,
                     const lti::palette& colors) {
    parameters param;
    if(validParameters())
      param.copy(getParameters());
    param.filename = filename;
    param.bitsPerPixel = 8;

    setParameters(param); // a copy of param will be made!
    return apply(theChannel,colors);
  }

  /* shortcut for save */
  bool saveBMP::save(const std::string& filename,
                     const channel& theChannel,
                     const lti::palette& colors) {
    // generate a normal channel from the channel8:
    channel8 aChannel;
    aChannel.castFrom(theChannel);

    return save(filename,aChannel,colors);
  }

  /* shortcut for save */
  bool saveBMP::save(const std::string& filename,
                     const image& theImage) {
    parameters param;
    if(validParameters())
      param.copy(getParameters());
    param.filename = filename;
    param.bitsPerPixel = 24;

    setParameters(param); // a copy of param will be made!
    return apply(theImage);
  }


  bool saveBMP::save24bit(std::ofstream& out,const image& theImage) {
    int x,y,mc,t;
    int tmpSize = 0;
    rgbPixel p;

    tmpSize += theHeader.length();
    tmpSize += theInfoHeader.length();

    x = 3*theImage.columns(); // real number of bytes pro line
    t = (4 - (x % 4)) % 4;

    tmpSize += (x+t)*theImage.rows();

    theHeader.size = tmpSize;
    theHeader.offsetPixels = theHeader.length() + theInfoHeader.length();

    theInfoHeader.size = theInfoHeader.length();
    theInfoHeader.width = theImage.columns();
    theInfoHeader.height = theImage.rows();
    theInfoHeader.planes = 1;
    theInfoHeader.bitCount = 24;
    theInfoHeader.sizeImage = (x+t)*theImage.rows();
    theInfoHeader.xPixPerMeter = 2835; // 72 dpi
    theInfoHeader.yPixPerMeter = 2835; // 72 dpi
    theInfoHeader.compression = 0; // no compression?
    theInfoHeader.colorsUsed = 0;
    theInfoHeader.colorsImportant = 0; // All

    ubyte* buffer = new ubyte[theInfoHeader.sizeImage];
    ubyte* bufferRed  = &buffer[2];
    ubyte* bufferGreen= &buffer[1];
    ubyte* bufferBlue = &buffer[0];

    // construct the buffer
    mc = 0; // memory counter: position in the buffer
    for (y=theImage.rows()-1;y>=0;y--) {
      for (x=0;x<theImage.columns();x++) {
        p=theImage.at(y,x);

        bufferRed[mc]   = p.getRed();
        bufferGreen[mc] = p.getGreen();
        bufferBlue[mc]  = p.getBlue();

        mc+=3;
      }

      for (x=1;x<=t;x++) {
        buffer[mc++] = 0;
      }
    }

    // write header and infoheader
    theHeader.write(out);
    theInfoHeader.write(out);
    // write buffer
    out.write((const char*)buffer,theInfoHeader.sizeImage);
    delete[] buffer;

    return true;
  }

  bool saveBMP::save8bit(std::ofstream& out,const image& theImage) {
    setStatusString("24 bit image cannot be saved as 8 bit channel");
    return false;
  }

  bool saveBMP::save8bit(std::ofstream& out,const channel8& theChannel) {
    int x,y,c,t,i;
    uint32 mc;
    int tmpSize;

    ubyte* buffer = 0;

    // calculate header values
    theHeader.offsetPixels = theHeader.length() +
      theInfoHeader.length() +
      thePalette.size()*4;

    tmpSize = 0;
    tmpSize += theHeader.length();
    tmpSize += theInfoHeader.length();
    tmpSize += thePalette.size()*4;

    x = theChannel.columns(); // real number of bytes pro line
    t = (4 - (x % 4)) % 4;

    theInfoHeader.sizeImage = (x+t)*theChannel.rows();

    tmpSize += (x+t)*theChannel.rows();

    theHeader.size = tmpSize;

    theInfoHeader.size = theInfoHeader.length();
    theInfoHeader.width = theChannel.columns();
    theInfoHeader.height = theChannel.rows();
    theInfoHeader.planes = 1;
    theInfoHeader.bitCount = 8;

    theInfoHeader.xPixPerMeter = 2835; // 72 dpi
    theInfoHeader.yPixPerMeter = 2835; // 72 dpi
    // RLE8 compression?
    theInfoHeader.compression = (getParameters().compression) ? 1 : 0;
    theInfoHeader.colorsUsed = 256;
    theInfoHeader.colorsImportant = 0; // All

    // ------
    //  compresion required?

    if (theInfoHeader.compression) {
      // try to compress the data with RLE8
      mc = 0;
      // allocate memory for the buffer, and reserve some space for
      // the case in which the compression won't work!
      buffer = new ubyte[theInfoHeader.sizeImage+theChannel.columns()];
      for (y=theChannel.rows()-1;y>=0;y--) {
        x = 0;
        while (x<theChannel.columns()) {
          // how many pixel repetitions?
          for (c = 1,i = x+1;
               (i<theChannel.columns()) &&
                 (theChannel.at(y,x)==theChannel.at(y,i));
               i++,c++) {
            // c will contain the number of repetition for the pixel
          }
          if (c>1) { // pixel more than once repeated!
            // belong these pixels to the end of the line?
            if ((c+x>=theChannel.columns())&&(theChannel.at(y,x)==0)) {
              // yes!  flag to exit:
              x=theChannel.columns();
            } else {
              buffer[mc++]=c; // code repeat value c times!
              buffer[mc++]=theChannel.at(y,x);
              x+=c;           // new x
            }
          } else {
            // stand alone value: try to code

            // count how many different pixels come...
            for (c=0,i=x+1;
                 (i<theChannel.columns()) &&
                   (theChannel.at(y,i-1) != (theChannel.at(y,i)));
                 c++,i++) {
              // c will contain the number of different pixels
            }

            if (c<3) {
              // just 1 pixel:
              buffer[mc++]=1;
              buffer[mc++]=theChannel.at(y,x);
              x++;
              if (c==2) { // or 2 pixel
                buffer[mc++]=1;
                buffer[mc++]=theChannel.at(y,x);
                x++;
              }
            } else {
              // write the block as is:
              buffer[mc++]=0;
              buffer[mc++]=c;
              memcpy(&buffer[mc],&theChannel.at(y,x),c);
              mc+=c;
              if ((c & 0x01) != 0) { // odd number of bytes?
                buffer[mc++]=0;  // align the buffer to be even!
              }
              x+=c;
            }
          }
        }
        // mark end of line with 0,0!
        buffer[mc++]=0;
        buffer[mc++]=0;

        if (mc>theInfoHeader.sizeImage) {
          // compression didn't work... just save the normal image
          delete[] buffer;
          buffer = 0;
          theInfoHeader.compression = false; // save as no compressed!
          y = -1; // flag to the end of compression!
        }
      }

      // if the compression was ok
      if (theInfoHeader.compression) {
        // mark the end of the graphic with 0,1
        buffer[mc++]=0;
        buffer[mc++]=1;

        theInfoHeader.sizeImage=mc;
        theHeader.size = theHeader.length()+theInfoHeader.length()+
          thePalette.size()*4+mc;
      }
    }

    // if compression is not required or if compressed data is bigger than
    // the original:

    if (!theInfoHeader.compression) {
      // channel data
      buffer = new ubyte[theInfoHeader.sizeImage];

      // construct the buffer
      mc = 0; // memory counter: position in the buffer
      for (y=theChannel.rows()-1;y>=0;y--) {
        memcpy(&buffer[mc],&theChannel.at(y,0),theInfoHeader.width);
        mc += (theInfoHeader.width+t);
        //for (x=1;x<=t;x++) {
        //  buffer[mc++] = 0;
        //}
      }
    }

    // write header and infoheader
    theHeader.write(out);
    theInfoHeader.write(out);
    // write palette
    thePalette.write(out);
    // write buffer
    out.write((const char*)buffer,theInfoHeader.sizeImage);

    delete[] buffer;

    return true;
  }

  bool saveBMP::save4bit(std::ofstream& out,const channel8& theChannel) {
    return save8bit(out,theChannel);
  }

  bool saveBMP::save4bit(std::ofstream& out,const image& theImage) {
    setStatusString("24 bit image cannot be saved as 4 bit channel");
    return false;
  }

  bool saveBMP::save1bit(std::ofstream& out,const channel8& theChannel) {
    int x,y,t,b;
    uint32 mc;
    int tmpSize;
    ubyte theByte;

    ubyte* buffer = 0;

    // calculate header values
    theHeader.offsetPixels = theHeader.length() +
      theInfoHeader.length() +
      thePalette.size()*4;

    tmpSize = 0;
    tmpSize += theHeader.length();
    tmpSize += theInfoHeader.length();
    tmpSize += thePalette.size()*4;

    x = (theChannel.columns()+7)/8; // real number of bytes pro line
    t = (4 - (x % 4)) % 4;          // fill bytes at the end of a line

    theInfoHeader.sizeImage = (x+t)*theChannel.rows();
    tmpSize +=  theInfoHeader.sizeImage;

    theHeader.size = tmpSize;

    theInfoHeader.size = theInfoHeader.length();
    theInfoHeader.width = theChannel.columns();
    theInfoHeader.height = theChannel.rows();
    theInfoHeader.planes = 1;
    theInfoHeader.bitCount = 1;

    theInfoHeader.xPixPerMeter = 2835; // 72 dpi
    theInfoHeader.yPixPerMeter = 2835; // 72 dpi
    theInfoHeader.compression = 0;
    theInfoHeader.colorsUsed = 2;
    theInfoHeader.colorsImportant = 2; // All


    // channel data
    buffer = new ubyte[theInfoHeader.sizeImage];

    // construct the buffer
    mc = 0; // memory counter: position in the buffer
    for (y=theChannel.rows()-1;y>=0;y--) {
      theByte = 0;
      b = 0x80;
      for (x=0;x<theChannel.columns();x++) {
        if (theChannel.at(y,x) != 0) {
          theByte |= b;
        }
        b = b >> 1; // shift to the right
        if (b==0) {             // 8-bit already accumulated...
          buffer[mc++]=theByte; // ...let them out!
          b=0x80;
          theByte = 0;
        }
      }
      if (b<0x80) {// last byte hasn't been stored jet!
        buffer[mc++]=theByte; // ...let them out!
      }

      mc+=t;
    }

    // write header and infoheader
    theHeader.write(out);
    theInfoHeader.write(out);
    // write palette
    thePalette.write(out);
    // write buffer
    out.write((const char*)buffer,theInfoHeader.sizeImage);

    delete[] buffer;

    return true;
  }

  bool saveBMP::save1bit(std::ofstream& out,const image& theImage) {
    setStatusString("24 bit image cannot be saved as 1 bit channel");
    return false;
  }

} // namespace lti
