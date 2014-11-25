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
 * file .......: ltiUciDataWriter.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 21.5.2002
 * revisions ..: $Id: ltiUciDataWriter.cpp,v 1.9 2006/09/05 10:38:18 ltilib Exp $
 */

#include <fstream>
#include <sstream>

#include <cstdio>

#include "ltiUciDataWriter.h"

using std::endl;
using std::fstream;
using std::ostream;

namespace lti {
  // --------------------------------------------------
  // uciDataWriter::parameters
  // --------------------------------------------------

  // default constructor
  uciDataWriter::parameters::parameters()
    : ioFunctor::parameters() {

    append = true;
    hasHeader = false;
  }

  // copy constructor
  uciDataWriter::parameters::parameters(const parameters& other)
    : ioFunctor::parameters()  {
    copy(other);
  }

  // destructor
  uciDataWriter::parameters::~parameters() {
  }

  // get type name
  const char* uciDataWriter::parameters::getTypeName() const {
    return "uciDataWriter::parameters";
  }

  // copy member

  uciDataWriter::parameters&
  uciDataWriter::parameters::copy(const parameters& other) {
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

    append = other.append;
    hasHeader = other.hasHeader;

    return *this;
  }

  // alias for copy member
  uciDataWriter::parameters&
  uciDataWriter::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* uciDataWriter::parameters::clone() const {
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
  bool uciDataWriter::parameters::write(ioHandler& handler,
                                        const bool complete) const
# else
    bool uciDataWriter::parameters::writeMS(ioHandler& handler,
                                            const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"hasHeader",hasHeader);
      lti::write(handler,"append",append);
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
  bool uciDataWriter::parameters::write(ioHandler& handler,
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
  bool uciDataWriter::parameters::read(ioHandler& handler,
                                       const bool complete)
# else
    bool uciDataWriter::parameters::readMS(ioHandler& handler,
                                           const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"hasHeader",hasHeader);
      lti::read(handler,"filename",filename);
      lti::read(handler,"append",append);
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
  bool uciDataWriter::parameters::read(ioHandler& handler,
                                       const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // uciDataWriter
  // --------------------------------------------------

  // default constructor
  uciDataWriter::uciDataWriter()
    : ioFunctor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  uciDataWriter::uciDataWriter(std::string theFilename)
    : ioFunctor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    defaultParameters.filename=theFilename;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  uciDataWriter::uciDataWriter(std::string theFilename,
                               bool bHeader)
    : ioFunctor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    defaultParameters.filename=theFilename;
    defaultParameters.hasHeader=bHeader;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  uciDataWriter::uciDataWriter(const uciDataWriter& other)
    : ioFunctor()  {
    copy(other);
  }

  // destructor
  uciDataWriter::~uciDataWriter() {
  }

  // returns the name of this type
  const char* uciDataWriter::getTypeName() const {
    return "uciDataWriter";
  }

  // copy member
  uciDataWriter& uciDataWriter::copy(const uciDataWriter& other) {
    ioFunctor::copy(other);

    return (*this);
  }

  // alias for copy member
  uciDataWriter&
  uciDataWriter::operator=(const uciDataWriter& other) {
    return (copy(other));
  }


  // clone member
  functor* uciDataWriter::clone() const {
    return new uciDataWriter(*this);
  }

  // return parameters
  const uciDataWriter::parameters&
  uciDataWriter::getParameters() const {
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

  bool uciDataWriter::apply(fmatrix& data) const {
    dmatrix tmp;
    tmp.castFrom(data);
    return apply(tmp);
  }

  bool uciDataWriter::apply(fmatrix& data, ivector& ids) const {
    dmatrix tmp;
    tmp.castFrom(data);
    return apply(tmp,ids);
  }

  bool uciDataWriter::apply(dmatrix& data) const {

    bool ok=true;
    int i,j;

    const parameters& param=getParameters();
    std::fstream os;
    if (param.append) {
      dmatrix tmp;
      if (param.hasHeader) {
        os.open(param.filename.c_str(), fstream::in ); //open file for reading
        double oldRows=0.0;
        double dim=0.0;
        if ( os.is_open() ) { // file exists
          os >> oldRows >> dim; // read old header
          tmp.resize(iround(oldRows),iround(dim));
          for (i=0; i<oldRows; i++) { // read old data
            for (j=0; j<dim; j++) {
              os >> tmp.at(i,j);
            }
          }
          os.close();
        }
        os.open(param.filename.c_str(), std::ostream::out);
        os << oldRows+data.rows() << endl  // write new header
           << data.columns() << endl;
        for (i=0; i<oldRows; i++) {    // write old data into file, because
          for (j=0; j<dim; j++) {      // a new file was created, when
            os << tmp.at(i,j) << " ";  // overwriting the old header
          }
          os << endl;
        }
        os.close();
      }
      // open file in append mode
      os.open(param.filename.c_str(), fstream::out|fstream::app);

    } else {
      // create a new file
      os.open(param.filename.c_str(), fstream::out);
      if ( param.hasHeader ) {
        os << data.rows() << endl  // write header
           << data.columns() << endl;
      }

    }
    if (!os) {
      setStatusString("unable to open output file");
      return false;
    }
    for (i=0; i<data.rows(); i++) {
      for (j=0; j<data.columns(); j++) {
        os << data.at(i,j) << " ";
      }
      os << endl;
    }
    return ok;
  }


  bool uciDataWriter::apply(dmatrix& data, ivector& ids) const {

    bool ok(true);
    int i,j;
    const parameters& param=getParameters();
    std::fstream os;

    if (param.append) {
      if (param.hasHeader) {
        dmatrix tmpMat;
        dvector tmpVec;
        os.open(param.filename.c_str(), fstream::in ); //open file for reading
        double oldRows=0.0;
        double dim=0.0;
        if ( os.is_open() ) { // file exists
          os >> oldRows >> dim; // read old header
          tmpMat.resize(iround(oldRows),iround(dim));
          tmpVec.resize(iround(oldRows));
          for (i=0; i<oldRows; i++) { // read old data
            for (j=0; j<dim; j++) {
              os >> tmpMat.at(i,j);
            }
            os >> tmpVec.at(i);
          }
          os.close();
        }
        os.open(param.filename.c_str(), ostream::out); // open file for output
        os << oldRows+data.rows() << endl  // write new header
           << data.columns() << endl;
        for (i=0; i<oldRows; i++) {    // write old data into file, because
          for (j=0; j<dim; j++) {      // a new file was created, when
            os << tmpMat.at(i,j) << " ";  // overwriting the old header
          }
          os << tmpVec.at(i) << endl;
        }
        os.close();
      }
      // open file in append mode
      os.open(param.filename.c_str(), fstream::out|fstream::app);

    } else {
      // create a new file
      os.open(param.filename.c_str(), ostream::out);
      if ( param.hasHeader ) { // write header
        os << data.rows() << endl
           << data.columns() << endl;
      }

    }
    if (!os) { // if opened failed
      setStatusString("unable to open output file");
      return false;
    }

    if (data.rows()!=ids.size()) {
      ok=false;
    }
    for (i=0; i<data.rows(); i++) {
      for (j=0; j<data.columns(); j++) {
        os << data.at(i,j) << " ";
      }
      os << ids.at(i) << endl;
    }
    return ok;
  }

  bool uciDataWriter::apply(std::list<dmatrix>& data) const {
    int i,j,nbPoints;
    std::list<dmatrix>::iterator it;
    const parameters& param=getParameters();
    std::fstream os;

    nbPoints=0;
    for (it=data.begin(); it!=data.end(); it++) {
      nbPoints+=(*it).rows();
    }
    double dim=(*data.begin()).columns();

    /* Either open file in append mode or create every time a new file,
     * corresponding to the parameter append
     */
    if (param.append) {
      dmatrix tmp;
      if (param.hasHeader) {
        os.open(param.filename.c_str(), fstream::in ); //open file for reading
        double oldRows=0.0;
        if ( os.is_open() ) { // file exists
          os >> oldRows >> dim; // read old header
          tmp.resize(iround(oldRows),iround(dim));
          for (i=0; i<oldRows; i++) { // read old data
            for (j=0; j<dim; j++) {
              os >> tmp.at(i,j);
            }
          }
          os.close();
        }
        os.open(param.filename.c_str(), ostream::out);
        os << oldRows+nbPoints << endl  // write new header
           << dim << endl;
        for (i=0; i<oldRows; i++) {    // write old data into file, because
          for (j=0; j<dim; j++) {      // a new file was created, when
            os << tmp.at(i,j) << " ";  // overwriting the old header
          }
          os << endl;
        }
        os.close();
      }
      // open file in append mode
      os.open(param.filename.c_str(), fstream::out|fstream::app);

    } else {
      // create a new file
      os.open(param.filename.c_str(), ostream::out);
      if ( param.hasHeader ) { // write header
        os << nbPoints << endl
           << dim << endl;
      }
    }

    if (!os) {  // open file failed?
      setStatusString("unable to open output file");
      return false;
    }
    for (it=data.begin(); it!=data.end(); it++) {
      for (i=0; i<(*it).rows(); i++) {
        for (j=0; j<(*it).columns(); j++) {
          os << (*it).at(i,j) << " ";
        }
        os << endl;
      }
    }
    return true;
  }

  bool uciDataWriter::apply(std::list<dmatrix>& data,ivector& ids) const {

    int i,j,nbPoints;
    std::list<dmatrix>::iterator it;
    const parameters& param=getParameters();
    std::fstream os;

    nbPoints=0;
    for (it=data.begin(); it!=data.end(); it++) {
      nbPoints+=(*it).rows();
    }
    double dim=(*data.begin()).columns();

    if (param.append) {
      if (param.hasHeader) {
        dmatrix tmpMat;
        dvector tmpVec;
        os.open(param.filename.c_str(), fstream::in ); //open file for reading
        double oldRows=0.0;
        if ( os.is_open() ) { // file exists
          os >> oldRows >> dim; // read old header
          tmpMat.resize(iround(oldRows),iround(dim));
          tmpVec.resize(iround(oldRows));
          for (i=0; i<oldRows; i++) { // read old data
            for (j=0; j<dim; j++) {
              os >> tmpMat.at(i,j);
            }
            os >> tmpVec.at(i);
          }
          os.close();
        }
        os.open(param.filename.c_str(), ostream::out); // open file for output
        os << oldRows+nbPoints << endl  // write new header
           << dim << endl;
        for (i=0; i<oldRows; i++) {    // write old data into file, because
          for (j=0; j<dim; j++) {      // a new file was created, when
            os << tmpMat.at(i,j) << " ";  // overwriting the old header
          }
          os << tmpVec.at(i) << endl;
        }
        os.close();
      }
      // open file in append mode
      os.open(param.filename.c_str(), fstream::out|fstream::app);

    } else {
      // create a new file
      os.open(param.filename.c_str(), ostream::out);
      if ( param.hasHeader ) {
        os << nbPoints << endl  // write header
           << dim << endl;
      }
    }
    if (!os) {
      setStatusString("unable to open output file");
      return false;
    }
    int counter=0;
    for (it=data.begin(); it!=data.end(); it++) {
      for (i=0; i<(*it).rows(); i++) {
        for (j=0; j<(*it).columns(); j++) {
          os << (*it).at(i,j) << " ";
        }
        os << ids.at(counter) << endl;
      }
      counter++;
    }
    return true;
  }


} // namespace lti
