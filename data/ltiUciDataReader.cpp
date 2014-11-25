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
 * file .......: ltiUciDataReader.cpp
 * authors ....: Peter Doerfler, Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 18.3.2002
 * revisions ..: $Id: ltiUciDataReader.cpp,v 1.10 2010/04/10 01:31:05 alvarado Exp $
 */

#include <fstream>
#include <sstream>

#include <cstdio>
#include "ltiUciDataReader.h"

namespace lti {
  // --------------------------------------------------
  // uciDataReader::parameters
  // --------------------------------------------------

  // default constructor
  uciDataReader::parameters::parameters()
    : ioFunctor::parameters() {

    hasIds = true;
    numberOfDimensions = 0;
    hasHeader = false;
  }

  // copy constructor
  uciDataReader::parameters::parameters(const parameters& other)
    : ioFunctor::parameters()  {
    copy(other);
  }

  // destructor
  uciDataReader::parameters::~parameters() {
  }

  // get type name
  const char* uciDataReader::parameters::getTypeName() const {
    return "uciDataReader::parameters";
  }

  // copy member

  uciDataReader::parameters&
  uciDataReader::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    ioFunctor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    ioFunctor::parameters& (ioFunctor::parameters::* p_copy)
      (const ioFunctor::parameters&) =
      ioFunctor::parameters::copy;
    (this->*p_copy)(other);
# endif


    hasIds = other.hasIds;
    numberOfDimensions = other.numberOfDimensions;
    hasHeader = other.hasHeader;

    return *this;
  }

  // alias for copy member
  uciDataReader::parameters&
  uciDataReader::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* uciDataReader::parameters::clone() const {
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
  bool uciDataReader::parameters::write(ioHandler& handler,
                                        const bool complete) const
# else
    bool uciDataReader::parameters::writeMS(ioHandler& handler,
                                            const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"hasIds",hasIds);
      lti::write(handler,"numberOfDimensions",numberOfDimensions);
      lti::write(handler,"hasHeader",hasHeader);
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
  bool uciDataReader::parameters::write(ioHandler& handler,
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
  bool uciDataReader::parameters::read(ioHandler& handler,
                                       const bool complete)
# else
    bool uciDataReader::parameters::readMS(ioHandler& handler,
                                           const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"hasIds",hasIds);
      lti::read(handler,"numberOfDimensions",numberOfDimensions);
      lti::read(handler,"hasHeader",hasHeader);
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
  bool uciDataReader::parameters::read(ioHandler& handler,
                                       const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // uciDataReader
  // --------------------------------------------------

  // default constructor
  uciDataReader::uciDataReader()
    : ioFunctor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  uciDataReader::uciDataReader(std::string theFilename)
    : ioFunctor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    defaultParameters.filename=theFilename;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  uciDataReader::uciDataReader(std::string theFilename, int nbDimensions,
                               bool bIds, bool bHeader)
    : ioFunctor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    defaultParameters.filename=theFilename;
    defaultParameters.numberOfDimensions=nbDimensions;
    defaultParameters.hasIds=bIds;
    defaultParameters.hasHeader=bHeader;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  uciDataReader::uciDataReader(const uciDataReader& other)
    : ioFunctor()  {
    copy(other);
  }

  // destructor
  uciDataReader::~uciDataReader() {
  }

  // returns the name of this type
  const char* uciDataReader::getTypeName() const {
    return "uciDataReader";
  }

  // copy member
  uciDataReader& uciDataReader::copy(const uciDataReader& other) {
    ioFunctor::copy(other);

    return (*this);
  }

  // alias for copy member
  uciDataReader&
  uciDataReader::operator=(const uciDataReader& other) {
    return (copy(other));
  }


  // clone member
  functor* uciDataReader::clone() const {
    return new uciDataReader(*this);
  }

  // return parameters
  const uciDataReader::parameters&
  uciDataReader::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&ioFunctor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool uciDataReader::apply(fmatrix& data, ivector& ids) const {
    dmatrix tmp;
    bool b=apply(tmp, ids);
    data.castFrom(tmp);
    return b;
  };

  bool uciDataReader::apply(fmatrix& data) const {
    dmatrix tmp;
    ivector dummy;
    bool b=apply(tmp, dummy);
    data.castFrom(tmp);
    return b;
  };

  bool uciDataReader::apply(dmatrix& data) const {

    ivector dummy;
    return apply(data, dummy);
  };

  bool uciDataReader::apply(dmatrix& data, ivector& ids) const {

    bool b(true);

    std::vector<dvector> tData;
    std::vector<int> tIds;
    int i,j;

    //      data=dmatrix(0,0);
    //      ids=ivector(0);

    const parameters& param=getParameters();
//     std::cerr << param.filename.c_str() << std::endl;
    std::fstream is(param.filename.c_str(), std::fstream::in);

    if (!is) {
      setStatusString("could not open file");
      return false;
    }

    char c[4096];
    bool ok;
    if (param.hasHeader) {
      int points, dim;
      is >> points >> dim;
      data.resize(points,dim);
      ids.resize(points);
      for (i=0; i<points; i++) {
        for (j=0; j<dim; j++) {
          is >> data.at(i,j);
        }
        if (param.hasIds)
          is >> ids.at(i);
      }
    } else {
      while(!is.eof()) {

        ok=true;
        is.getline(c, 4096);

        if (!is.eof()) {
          //std::string s(c);
          std::istringstream sstr(c);

          dvector tVec(param.numberOfDimensions);
          for (i=0; i<param.numberOfDimensions; i++) {
            sstr >> tVec.at(i);
          }
          if (!sstr.eof()) {
            if (param.hasIds) {
              int tId;
              if (sstr >> tId) {
                tIds.push_back(tId);
              } else {
                b = false;
                ok = false;
                char err[100];
                sprintf(err,"Error in line %i",static_cast<int>(tIds.size()+1));
                setStatusString(err);
              }
            }
            if (ok) {
              tData.push_back(tVec);
            }
          } else {
            b = false;
            char err[100];
            sprintf(err,"Error in line %i",static_cast<int>(tData.size()+1));
            setStatusString(err);        }
        }
      }

      int size=int(tData.size());

      data.resize(size, param.numberOfDimensions);
      for (i=0; i<size; i++) {
        data.setRow(i, tData[i]);
      }

      if (param.hasIds) {
        ids.resize(size);
        for (i=0; i<size; i++) {
          ids.at(i)=tIds[i];
        }
      }
    }
    return b;
  };

}
