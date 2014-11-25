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
 * file .......: ltiIOFunctor.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 23.04.99
 * revisions ..: $Id: ltiIOFunctor.cpp,v 1.8 2006/09/05 10:35:36 ltilib Exp $
 */

#include "ltiIOFunctor.h"

namespace lti {

  functor::parameters* ioFunctor::parameters::clone() const {
    return (new parameters(*this));
  }

  const char* ioFunctor::parameters::getTypeName() const {
    return "ioFunctor::parameters";
  }

  ioFunctor::parameters& ioFunctor::parameters::copy(const parameters& other) {
#ifndef _LTI_MSC_6
    // for normal  ANSI C++
    functor::parameters::copy(other);
#else
    // this doesn't work with MS-VC++ 6.0 (an once again... another bug)
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters&
      (functor::parameters::* p_copy)(const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
#endif
    filename = other.filename;

    return (*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool ioFunctor::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool ioFunctor::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"filename",filename);
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
  bool ioFunctor::parameters::write(ioHandler& handler,
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
  bool ioFunctor::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool ioFunctor::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"filename",filename);
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
  bool ioFunctor::parameters::read(ioHandler& handler,
                                    const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  // construction
  ioFunctor::ioFunctor(const parameters& theParam) {
    setParameters(theParam);
  }

  ioFunctor::ioFunctor(const std::string& theFilename) {
    parameters tmpParam;
    tmpParam.filename = theFilename;
    setParameters(tmpParam);
  }

  // destructor
  ioFunctor::~ioFunctor() {
  }

  const char* ioFunctor::getTypeName() const {
    return "ioFunctor";
  }

  // returns the current parameters
  const ioFunctor::parameters& ioFunctor::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0)
      throw invalidParametersException(getTypeName());
    return *params;
  }

  // ------------------
  // class endianness
  // ------------------

  // constructor
  endianness::endianness(const eEndianness fileEndianness) {
    // Test for endianness:
    uint32 dword;
    char*  ptrDword;
    ptrDword = (char*)(&dword);

    dword = 0x04030201;

    if ((ptrDword[0] == 0x01) &&
	(ptrDword[1] == 0x02) &&
	(ptrDword[2] == 0x03) &&
	(ptrDword[3] == 0x04)) {
      // system uses little Endian
      if (fileEndianness == LittleEndian)
	theEndian = new defaultEndian;
      else
	theEndian = new oppositeEndian;
    }
    else if ((ptrDword[0] == 0x04) &&
	     (ptrDword[1] == 0x03) &&
	     (ptrDword[2] == 0x02) &&
	     (ptrDword[3] == 0x01)) {
      // system uses big Endian
      if (fileEndianness == BigEndian)
	theEndian = new defaultEndian;
      else
	theEndian = new oppositeEndian;
    }
    else {
      // unknown Endian! try with the default!!
      theEndian = new defaultEndian;
    }
  }

  // destructor
  endianness::~endianness() {
    delete theEndian;
    theEndian = 0;
  }

  // return the type name ("endianness")
  const char* endianness::getTypeName() const {
    return "endianness";
  }

  // read a byte
  byte& endianness::read(std::istream& in,byte& data) const {
    theEndian->read(in,data);
    return data;
  }

  /// read a ubyte
  ubyte& endianness::read(std::istream& in,ubyte& data) const {
    theEndian->read(in,data);
    return data;
  }

  /// read a int16 (2 bytes)
  int16& endianness::read(std::istream& in,int16& data) const {
    theEndian->read(in,data);
    return data;
  }
  /// read a uint16 (2 bytes)
  uint16& endianness::read(std::istream& in,uint16& data) const {
    theEndian->read(in,data);
    return data;
  }
  /// read a int32
  int32& endianness::read(std::istream& in,int32& data) const {
    theEndian->read(in,data);
    return data;
  }
  /// read a uint32
  uint32& endianness::read(std::istream& in,uint32& data) const {
    theEndian->read(in,data);
    return data;
  }

  /// write a byte
  const byte& endianness::write(std::ostream& out,const byte& data) const {
    theEndian->write(out,data);
    return data;
  }
  /// write a ubyte
  const ubyte& endianness::write(std::ostream& out,const ubyte& data) const {
    theEndian->write(out,data);
    return data;
  }
  /// write a int16 (2 bytes)
  const int16& endianness::write(std::ostream& out,const int16& data) const {
    theEndian->write(out,data);
    return data;
  }
  /// write a uint16 (2 bytes)
  const uint16& endianness::write(std::ostream& out,const uint16& data) const {
    theEndian->write(out,data);
    return data;
  }
  /// write a int32
  const int32& endianness::write(std::ostream& out,const int32& data) const {
    theEndian->write(out,data);
    return data;
  }
  /// write a uint32
  const uint32& endianness::write(std::ostream& out,const uint32& data) const {
    theEndian->write(out,data);
    return data;
  }

  // ------------------
  // class defaultEndian
  // ------------------

  const char* endianness::defaultEndian::getTypeName() const {
    return "defaultEndian";
  }

  // read a byte
  byte& endianness::defaultEndian::read(std::istream& in,byte& data) const {
    in.read(reinterpret_cast<char*>(&data),1);
    return data;
  }
  /// read a ubyte
  ubyte& endianness::defaultEndian::read(std::istream& in,ubyte& data) const {
    in.read(reinterpret_cast<char*>(&data),1);
    return data;
  }
  /// read a int16 (2 bytes)
  int16& endianness::defaultEndian::read(std::istream& in,int16& data) const {
    in.read(reinterpret_cast<char*>(&data),2);
    return data;
  }
  /// read a uint16 (2 bytes)
  uint16& endianness::defaultEndian::read(std::istream& in,uint16& data) const {
    in.read(reinterpret_cast<char*>(&data),2);
    return data;
  }
  /// read a int32
  int32& endianness::defaultEndian::read(std::istream& in,int32& data) const {
    in.read(reinterpret_cast<char*>(&data),4);
    return data;
  }
  /// read a uint32
  uint32& endianness::defaultEndian::read(std::istream& in,uint32& data) const {
    in.read(reinterpret_cast<char*>(&data),4);
    return data;
  }

  /// write a byte
  const byte& endianness::defaultEndian::write(std::ostream& out,
					       const byte& data) const {
    out.write(reinterpret_cast<const char*>(&data),1);
    return data;
  }

  /// write a ubyte
  const ubyte& endianness::defaultEndian::write(std::ostream& out,
						const ubyte& data) const {
    out.write(reinterpret_cast<const char*>(&data),1);
    return data;
  }

  /// write a int16 (2 bytes)
  const int16& endianness::defaultEndian::write(std::ostream& out,
						const int16& data) const {
    out.write(reinterpret_cast<const char*>(&data),2);
    return data;
  }

  /// write a uint16 (2 bytes)
  const uint16& endianness::defaultEndian::write(std::ostream& out,
						 const uint16& data) const {
    out.write(reinterpret_cast<const char*>(&data),2);
    return data;
  }

  /// write a int32
  const int32& endianness::defaultEndian::write(std::ostream& out,
						const int32& data) const {
    out.write(reinterpret_cast<const char*>(&data),4);
    return data;
  }

  /// write a uint32
  const uint32& endianness::defaultEndian::write(std::ostream& out,
						 const uint32& data) const {
    out.write(reinterpret_cast<const char*>(&data),4);
    return data;
  }

  // ------------------
  // class oppositeEndian
  // ------------------

  const char* endianness::oppositeEndian::getTypeName() const {
    return "oppositeEndian";
  }

  /// read a int16 (2 bytes)
  int16& endianness::oppositeEndian::read(std::istream& in,int16& data) const {
    char buffer[2];
    char* alias = reinterpret_cast<char*>(&data);
    in.read(buffer,2);
    alias[0]=buffer[1];
    alias[1]=buffer[0];
    return data;
  }
  /// read a uint16 (2 bytes)
  uint16& endianness::oppositeEndian::read(std::istream& in,uint16& data) const {
    char buffer[2];
    char* alias = reinterpret_cast<char*>(&data);
    in.read(buffer,2);
    alias[0]=buffer[1];
    alias[1]=buffer[0];
    return data;
  }
  /// read a int32
  int32& endianness::oppositeEndian::read(std::istream& in,int32& data) const {
    char buffer[4];
    char* alias = reinterpret_cast<char*>(&data);
    in.read(buffer,4);
    alias[0]=buffer[3];
    alias[1]=buffer[2];
    alias[2]=buffer[1];
    alias[3]=buffer[0];
    return data;
  }
  /// read a uint32
  uint32& endianness::oppositeEndian::read(std::istream& in,
                                           uint32& data) const {
    char buffer[4];
    char* alias = reinterpret_cast<char*>(&data);
    in.read(buffer,4);
    alias[0]=buffer[3];
    alias[1]=buffer[2];
    alias[2]=buffer[1];
    alias[3]=buffer[0];
    return data;
  }
  
  /// write a int16 (2 bytes)
  const int16& endianness::oppositeEndian::write(std::ostream& out,
                                                 const int16& data) const {
    
    char buffer[2];
    const char* alias = reinterpret_cast<const char*>(&data);
    buffer[0]=alias[1];
    buffer[1]=alias[0];
    
    out.write(buffer,2);
    return data;
  }
  /// write a uint16 (2 bytes)
  const uint16& endianness::oppositeEndian::write(std::ostream& out,
                                                  const uint16& data) const {
    char buffer[2];
    const char* alias = reinterpret_cast<const char*>(&data);
    buffer[0]=alias[1];
    buffer[1]=alias[0];
    
    out.write(buffer,2);
    return data;
  }
  /// write a int32
  const int32& endianness::oppositeEndian::write(std::ostream& out,
                                                 const int32& data) const {
    char buffer[4];
    const char* alias = reinterpret_cast<const char*>(&data);
    buffer[0]=alias[3];
    buffer[1]=alias[2];
    buffer[2]=alias[1];
    buffer[3]=alias[0];
    
    out.write(buffer,4);
    return data;
  }
  /// write a uint32
  const uint32& endianness::oppositeEndian::write(std::ostream& out,
                                                  const uint32& data) const {
    char buffer[4];
    const char* alias = reinterpret_cast<const char*>(&data);
    buffer[0]=alias[3];
    buffer[1]=alias[2];
    buffer[2]=alias[1];
    buffer[3]=alias[0];
    
    out.write(buffer,4);
    return data;
  }
} // namespace lti
