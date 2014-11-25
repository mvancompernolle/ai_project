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
 * file .......: ltiLTIFunctor.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 23.04.99
 * revisions ..: $Id: ltiLTIFunctor.cpp,v 1.15 2006/09/05 10:35:58 ltilib Exp $
 */

#include "ltiLTIFunctor.h"
#include "ltiUsePalette.h"
#include "ltiMergeRGBToImage.h"
#include "ltiFlateCodec.h"
#include "ltiIdentityCodec.h"
#include "ltiRunLengthCodec.h"

#include <fstream>
#include <vector>
#include <map>

#undef _LTI_DEBUG
//#define _LTI_DEBUG 2
#include "ltiDebug.h"

#ifdef _LTI_DEBUG

#include <iostream>
#include "ltiLispStreamHandler.h"

namespace std {
  ostream& operator<<(ostream& out,const lti::ioObject& obj) {
    static lti::lispStreamHandler lsh;
    lsh.use(out);
    obj.write(lsh);
    return out;
  }
}

#endif

namespace lti {
  // -----------------------------------------------------------------------
  // ioLTI
  // -----------------------------------------------------------------------

  ioLTI::parameters::parameters() : ioFunctor::parameters() {
    compression=ioLTI::None;
  }

  ioLTI::parameters& ioLTI::parameters::copy(const parameters& other) {
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

    compression=other.compression;

    return (*this);
  }

  functor::parameters* ioLTI::parameters::clone() const  {
    return (new parameters(*this));
  }

  const char* ioLTI::parameters::getTypeName() const {
    return "ioLTI::parameters";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool ioLTI::parameters::write(ioHandler& handler,
                                const bool complete) const
# else
    bool ioLTI::parameters::writeMS(ioHandler& handler,
                                    const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
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

    std::string tmp;
    switch (compression) {
      case ioLTI::None:
        tmp="None";
        break;
      case ioLTI::Flate:
        tmp="Flate";
        break;
      case ioLTI::RunLength: 
        tmp="RunLength"; 
        break;
      default:  
        // error
        tmp="None";
        break;
    }
    lti::write(handler, "compression", tmp);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool ioLTI::parameters::write(ioHandler& handler,
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
  bool ioLTI::parameters::read(ioHandler& handler,
                               const bool complete)
# else
    bool ioLTI::parameters::readMS(ioHandler& handler,
                                   const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
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

    std::string tmp;
    lti::read(handler, "compression", tmp);
    if (tmp == "None") {
      compression=None;
    } else if (tmp == "RunLength") {
      compression=RunLength;
    } else if (tmp == "Flate") {
      compression=Flate;
    } else {
      // ERROR
      compression=None;
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool ioLTI::parameters::read(ioHandler& handler,
                               const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // -------------------------------------------------------
  //  ioLTI::header
  // -------------------------------------------------------

  ioLTI::header::header() : type(0x544c),contents(0),size(0),
                            reserved1(0),reserved2(0),
                            rows(0),columns(0) {
  }

  bool ioLTI::header::read(std::ifstream &in) {
    uint16 tmpType;
    uint32 tmpReserved;
    io.read(in,tmpType);  // io reads with the correct endianness

    if (tmpType != type) {
      return false;
    }

    io.read(in,contents);
    io.read(in,size);
    io.read(in,tmpReserved); // just ignore the read data!
    io.read(in,tmpReserved); // just ignore the read data!
    io.read(in,rows);
    io.read(in,columns);

    return true;
  }

  bool ioLTI::header::write(std::ofstream &out) {
    io.write(out,type);  // io writes with the correct endianness
    io.write(out,contents);
    io.write(out,size);
    io.write(out,reserved1);
    io.write(out,reserved2);
    io.write(out,rows);
    io.write(out,columns);

    return true;
  }

  // -------------------------------------------------------
  //  ioLTI
  // -------------------------------------------------------

  // returns the current parameters
  const ioLTI::parameters& ioLTI::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  // constructor
  ioLTI::ioLTI()
  : codec(0) {
  }

  // destructor
  ioLTI::~ioLTI() {
    delete codec;
    codec=0;
  }

  const char* ioLTI::getTypeName() const {
    return "ioLTI";
  }

  bool ioLTI::updateParameters() {
    bool t=true;
    const parameters& theParam = getParameters();

    delete codec;
    codec=0;
    //======================================================================
    //NOTE!!!!
    //if you change or append this do the same in ioLTIworker::loadMatrix!!!
    //======================================================================
    switch (theParam.compression) {
      case None: 
        codec=new identityCodec();
        break;
#if HAVE_LIBZ
      case Flate:
        codec=new flateCodec();
        break;
#endif
      case RunLength:
        codec=new runLengthCodec();
        break;
      default: 
        codec=new identityCodec();
        t=false;
        break;
    }
    return t;
  }

  // ---------------------------------------------------------------------
  // internal template class for doing all the work, loadLTI and
  // saveLTI are just facades.
  // ---------------------------------------------------------------------

  /**
   * Helper class to do all the work.
   */
  template <class T>
  class ioLTIworker: public ioLTI {
  public:
    /**
     * Loads a matrix from the given stream. The data are written
     * into theChannel, the meta-data are written into theHeader
     * These methods ignore all the parameters in this objects.
     * The only reason they are not static is that they modify
     * the status string.
     */
    bool loadMatrix(const std::string& filename, matrix<T>& theChannel,
                    header& theHeader) {
      theChannel.clear();
      
      //open stream
      std::ifstream in;
      in.open(filename.c_str(),std::ios::in|std::ios::binary);

      //open failed?
      if (!(in.good() && in.is_open())) {
        std::string str = "Could not open file " + filename;
        setStatusString(str.c_str());
        return false;
      }

      // read header
      if (!theHeader.read(in)) {
        setStatusString("Wrong header. Is this an LTI file?");
        return false;
      }

     
      //create the right codec for this file
      dataCodec* codec=0;

      switch (theHeader.encoding.compression) {
        case None: 
          codec=new identityCodec();
          break;
#if HAVE_LIBZ
        case Flate:
          codec=new flateCodec();
          break;
#endif
        case RunLength:
          codec=new runLengthCodec();
          break;
        default: 
          delete codec;
          setStatusString("Unknown codec of libz not installed");
          return false;
      }
      
      
      _lti_debug2("type: " << theHeader.type << std::endl);
      _lti_debug2("encoding.contents: " << theHeader.encoding.contents << std::endl);
      _lti_debug2("encoding.compression: " << theHeader.encoding.compression << std::endl);
      _lti_debug2("size: " << theHeader.size << std::endl);
      _lti_debug2("rows: " << theHeader.rows << std::endl);
      _lti_debug2("columns: " << theHeader.columns << std::endl);
      
      if (theHeader.encoding.compression == None && theHeader.size != 
          (sizeof(T)*theHeader.rows*theHeader.columns)) {
        setStatusString("Inconsistent header.  Wrong file size");
        delete codec;
        return false;
      }
      
      if (theHeader.encoding.contents != getTypeCode(T())) {
        bool flag=true;
        std::string str="Inconsistent file type. ";
        str+=theChannel.getTypeName();
        str+" expected but ";
        // should we really try to recover from type errors???
        // the following compiles, but might not get you what you expect
        switch(theHeader.encoding.contents) {
          case 'b':
            {
              str+=" lti::matrix<ubyte> found.";
              channel8 tmp;
              ioLTIworker<channel8::value_type> worker;
              if (!(flag=worker.loadBody(in,tmp,theHeader,codec))) {
                setStatusString(worker.getStatusString());
              }
              theChannel.castFrom(tmp);
              break;
            }
          case 'i':
            {
              str+=" lti::imatrix found.";
              imatrix tmp;
              ioLTIworker<imatrix::value_type> worker;
              if (!(flag=worker.loadBody(in,tmp,theHeader,codec))) {
                setStatusString(worker.getStatusString());
              }
              theChannel.castFrom(tmp);
              break;
            }
          case 'f':
            {
              str+=" lti::channel found.";
              flag=false;
              //           channel tmp;
              //           flag=loadBody(in,tmp);
              //           // this generates a warning and will probabily generate funny 
              //           // images or channels
              //           theChannel.castFrom(tmp);
              break;
            }
          case 'c':
            {
              // no chance: Forcing an image to something else is task-dependent
              str+=" lti::image found.";
              flag=false;
              //           image tmp;
              //           flag=loadBody(in,tmp);
              //           theChannel.castFrom(tmp);
              break;
            }
          default:
            str+=" unknown type found.";
            flag=false;
        }
        setStatusString(str.c_str());
        in.close();
        delete codec;
        return flag;
      } else {
        bool flag=loadBody(in,theChannel,theHeader,codec);
        in.close();
        delete codec;
        return flag;
      }
      
    }

    /**
     * Saves the given matrix to the given stream. The header is modified
     * with appropriate data derived from the matrix structure.
     */
    bool saveMatrix(const std::string& filename, 
                    const matrix<T>& theChannel,
                    header& theHeader, 
                    const eCompressionType compr,
                    dataCodec* codec) {

      //open stream
      std::ofstream out;
      out.open(filename.c_str(),std::ios::out|std::ios::binary);

      //stream ok?
      if (!(out.good() && out.is_open())) {
        std::string str = "Could not open file " + filename + " for writing.";
        setStatusString(str.c_str());
        out.close();
        return false;
      }

      theHeader.encoding.contents=getTypeCode(T());
      theHeader.encoding.compression=compr;
      
      int tmpsize=sizeof(T)*theChannel.rows()*theChannel.columns();
      int encsize = codec->estimateEncodedSize(tmpsize);
      
      // create the temporary buffer
      dataCodec::buffer tmp(encsize);
      
      // encode the data
      codec->encode((const ubyte*)&theChannel.at(0,0),tmpsize,&tmp.at(0),encsize);
      
      theHeader.size = encsize;
      theHeader.rows = theChannel.rows();
      theHeader.columns = theChannel.columns();
    
      // write the header
      if (!theHeader.write(out)) {
        setStatusString("Could not write header.");
        out.close();
        return false;
      }

      // write the data
      out.write((const char*)(&tmp.at(0)),theHeader.size);
      out.close();
      
      return true;
    }

    /**
     * Loads the matrix data from the given stream. The meta-data
     * must be passed in the given header.
     */
    bool loadBody(std::ifstream& in, matrix<T>& theChannel,
                  const header& theHeader,
                  dataCodec* codec) const {

      
      dataCodec::buffer tmp(theHeader.size);
      in.read((char*)(&tmp.at(0)),theHeader.size);
      
      int dummy=theHeader.rows*theHeader.columns*sizeof(T);
      
      theChannel.resize(theHeader.rows,
                        theHeader.columns,T(),false,false);
      
      codec->decode(&tmp.at(0),theHeader.size,(ubyte*)(&theChannel.at(0,0)),dummy);
      
      return true;

    }

    lti::functor* clone() const {
      return 0;
    }

    /**
     * Methods used to return the code type
     */
    //@{
    inline char getTypeCode(const ubyte& dummy) const    {return 'b';};
    inline char getTypeCode(const rgbPixel& dummy) const {return 'c';};
    inline char getTypeCode(const int32& dummy) const    {return 'i';};
    inline char getTypeCode(const float& dummy) const    {return 'f';};
    inline char getTypeCode(const double& dummy) const   {return 'd';};
    //@}
  };


  // ----------------------------------------------------------------------
  // loadLTI
  // ----------------------------------------------------------------------
  // default constructor
  loadLTI::loadLTI() : ioLTI() {
    parameters par;
    setParameters(par);
  }

  // returns name of the file
  const char* loadLTI::getTypeName() const {
    return "loadLTI";
  }

  // clone this object
  functor* loadLTI::clone() const {
    return (new loadLTI(*this));
  }


  bool loadLTI::load(const std::string& filename,
                     matrix<rgbPixel>& data) { 
    ioLTIworker<matrix<rgbPixel>::value_type> worker;
    if (worker.loadMatrix(filename, data, theHeader)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }

  bool loadLTI::load(const std::string& filename,
                     imatrix& data) { 
    ioLTIworker<imatrix::value_type> worker;
    if (worker.loadMatrix(filename, data, theHeader)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }

  bool loadLTI::load(const std::string& filename,
                     dmatrix& data) { 
    ioLTIworker<dmatrix::value_type> worker;
    if (worker.loadMatrix(filename, data, theHeader)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }

  bool loadLTI::load(const std::string& filename,
                     fmatrix& data) { 
    ioLTIworker<fmatrix::value_type> worker;
    if (worker.loadMatrix(filename, data, theHeader)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }

  bool loadLTI::load(const std::string& filename,
                     matrix<ubyte>& data) { 
    ioLTIworker<matrix<ubyte>::value_type> worker;
    if (worker.loadMatrix(filename, data, theHeader)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }


  bool loadLTI::apply(matrix<rgbPixel>& theChannel) {
    return load(getParameters().filename,theChannel);
  }

  bool loadLTI::apply(imatrix& theChannel) {
    return load(getParameters().filename,theChannel);
  }

  bool loadLTI::apply(dmatrix& theChannel) {
    return load(getParameters().filename,theChannel);
  }

  bool loadLTI::apply(fmatrix& theChannel) {
    return load(getParameters().filename,theChannel);
  }

  bool loadLTI::apply(matrix<ubyte>& theChannel) {
    return load(getParameters().filename,theChannel);
  }


  // checkfile
  bool loadLTI::checkHeader(const std::string& filename,
                            point& imageSize,
                            char& imageType,
                            eCompressionType& compr) {

    std::ifstream in;
    in.open(filename.c_str(),std::ios::in|std::ios::binary);
    if (in.good() && in.is_open()) {
      if (!theHeader.read(in)) {
        setStatusString("Wrong header. Is this an LTI file?");
        return false;
      }
      imageSize.set(theHeader.columns,theHeader.rows);
      imageType = static_cast<char>(theHeader.encoding.contents);
      compr = eCompressionType(theHeader.encoding.compression);      
      return true;
    } 

    std::string str = "Could not open file " + filename;
    setStatusString(str.c_str());
    return false;
  }

  // ------------------------------------------------------------------
  // saveLTI
  // ------------------------------------------------------------------
  // default constructor
  saveLTI::saveLTI() : ioLTI(){
    parameters par;
    setParameters(par);
  }

  // getTypeName
  const char* saveLTI::getTypeName() const {
    return "saveLTI";
  }

  // clone
  functor* saveLTI::clone() const {
    return (new saveLTI(*this));
  }

  // apply
  bool saveLTI::apply(const matrix<rgbPixel>& theChannel) {
    return save(getParameters().filename,theChannel);
  }

  bool saveLTI::apply(const imatrix& theChannel) {
    return save(getParameters().filename,theChannel);
  }

  bool saveLTI::apply(const dmatrix& theChannel) {
    return save(getParameters().filename,theChannel);
  }

  bool saveLTI::apply(const fmatrix& theChannel) {
    return save(getParameters().filename,theChannel);
  }

  bool saveLTI::apply(const matrix<ubyte>& theChannel) {
    return save(getParameters().filename,theChannel);
  }

  //shortcut to save

  bool saveLTI::save(const std::string& filename,
                     const matrix<rgbPixel>& data) { 
    ioLTIworker<matrix<rgbPixel>::value_type> worker;
    if (worker.saveMatrix(filename, data, theHeader, 
                          getParameters().compression,codec)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }

  bool saveLTI::save(const std::string& filename,
                     const imatrix& data) { 
    ioLTIworker<imatrix::value_type> worker;
    if (worker.saveMatrix(filename, data, theHeader, 
                          getParameters().compression,codec)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }

  bool saveLTI::save(const std::string& filename,
                     const dmatrix& data) { 
    ioLTIworker<dmatrix::value_type> worker;
    if (worker.saveMatrix(filename, data, theHeader, 
                          getParameters().compression,codec)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }

  bool saveLTI::save(const std::string& filename,
                     const fmatrix& data) { 
    ioLTIworker<fmatrix::value_type> worker;
    if (worker.saveMatrix(filename, data, theHeader, 
                          getParameters().compression,codec)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }

  bool saveLTI::save(const std::string& filename,
                     const matrix<ubyte>& data) { 
    ioLTIworker<matrix<ubyte>::value_type> worker;
    if (worker.saveMatrix(filename, data, theHeader, 
                          getParameters().compression,codec)) {
      return true;
    } else {
      setStatusString(worker.getStatusString()); 
      return false; 
    }
  }


} // namespace lti


#include "ltiUndebug.h"
