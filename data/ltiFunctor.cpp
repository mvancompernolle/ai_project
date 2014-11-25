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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiFunctor.cpp
 * authors ....: Thomas Rusert
 * organization: LTI, RWTH Aachen
 * creation ...: 14.04.99
 * revisions ..: $Id: ltiFunctor.cpp,v 1.10 2006/02/07 18:08:37 ltilib Exp $
 */

#include "ltiFunctor.h"
#include <cstdio>

namespace lti {

  // default empty string for the functor
	//  const char* const functor::emptyString = "";

  functor::parameters::parameters()
    : ioObject() {
  };

  functor::parameters::parameters(const parameters& other)
    : ioObject() {
    copy(other);
  };


  functor::parameters::~parameters() {
  }

  functor::parameters& functor::parameters::copy(const parameters& other) {
    return (*this);
  }

  functor::parameters& functor::parameters::operator=(const parameters& other){
    std::string errorMsg;

    errorMsg  = "operator= not implemented for these parameters,";
    errorMsg += "please use copy() member instead or implement the ";
    errorMsg += "operator+ properly";

    throw exception(errorMsg.c_str());

    return (*this);
  }

  const char* functor::parameters::getTypeName() const {
    return "functor::parameters";
  }

  bool functor::parameters::write(ioHandler& handler,
                                  const bool complete) const {
    return true;
  }

  bool functor::parameters::read(ioHandler& handler,
                                 const bool complete) {
    return true;
  }

# ifdef _LTI_MSC_6
  bool functor::parameters::readMS(ioHandler& handler,
                                    const bool complete) {
    return true;
  };

  bool functor::parameters::writeMS(ioHandler& handler,
                                    const bool complete) const {
    return true;
  };
# endif


  // ----------------------------
  // exceptions
  // ----------------------------

  const char* functor::invalidParametersException::getTypeName() const {
    return "functor::invalidParametersException";
  }

  const char* functor::invalidMethodException::getTypeName() const {
    return "functor::invalidMethodException";
  }

  // ----------------------------
  // functor
  // ----------------------------

  // constructor
  functor::functor()
		//    : ioObject(),params(0),ownParams(true),statusString(0) {
    : ioObject(),status(),params(0),ownParams(true) {
  }

  // copy constructor
  functor::functor(const functor& other)
    : ioObject(),status(),params(0),ownParams(true) {
    copy(other);
  }

  // destructor
  functor::~functor() {
    if (ownParams) {
      delete params;
      params = 0;
    }
    ownParams=true;
  }

  // copy constructor

  /* sets the functor's parameters.
     The functor keeps its own copy of the given parameters.*/
  bool functor::setParameters(const functor::parameters& theParams) {
    if (ownParams) {
      delete params;
      params = 0;
    }
    params = theParams.clone();
    ownParams = true;

    return updateParameters();
  }

  bool functor::useParameters(functor::parameters& theParams) {
    if (ownParams) {
      delete params;
      params = 0;
    }
    params = &theParams;
    ownParams = false;

    return updateParameters();
  }

  // updateParameters
  bool functor::updateParameters() {
    // return true only if the parameters are valid
    return validParameters();
  }

  /*
   * returns current parameters.
   */
  const functor::parameters& functor::getParameters() const {
    return *params;
  }

  /*
   * returns true if the parameters are valid
   */
  bool functor::validParameters() const {
    return notNull(params);
  }

  /*
   * returns the name of this type
   */
  const char* functor::getTypeName() const {
    return "functor";
  }

  // copy data of "other" functor.
  functor& functor::copy(const functor& other) {
    if(other.validParameters()) {
      setParameters(other.getParameters());
    }
    else {
      delete params;
      params = 0;
      ownParams=true;
    }
    return (*this);
  }

  // alias for copy
  functor& functor::operator=(const functor& other) {
    std::string str;
    str = "operator= not defined for class ";
    str += getTypeName();
    str += ". Please use copy member instead.";
    throw exception(str.c_str());
    return (*this);
  }

//   /*
//    * return the last message set with setStatusString().  This will
//    * never return 0.  If no status-string has been set yet an empty string
//    * (pointer to a string with only the char(0)) will be returned.
//    */
//   const char* functor::getStatusString() const {
//     if (isNull(statusString)) {
//       return emptyString;
//     } else {
//       return statusString;
//     }
//   }


  /*
   * read the functor parameters
   */
  bool functor::read(ioHandler& handler, const bool complete) {
    if (params != 0) {
      return params->read(handler,complete);
    } else {
      setStatusString("Tried to read <null> parameters object");
      return false;
    }
  }

  /*
   * read the functor parameters
   */
  bool functor::write(ioHandler& handler, const bool complete) const {
    if (params != 0) {
      return params->write(handler,complete);
    } else {
      setStatusString("Tried to write <null> parameters object");
      return false;
    }
  }


//   /*
//    * set a status string.
//    *
//    * @param msg the const string to be reported next time by
//    * getStatusString()
//    * This message will be usually set within the apply methods to indicate
//    * an error cause.
//    */
//   void functor::setStatusString(const char* msg) const {
//     delete[] statusString;
//     statusString = 0;

//     statusString = new char[strlen(msg)+1];
//     strcpy(statusString,msg);
//   }

//   void functor::appendStatusString(const char* msg) const {
//     char* oldStatus=statusString;

//     if (isNull(oldStatus)) {
//       setStatusString(msg);
//       return;
//     }

//     statusString = 0;
//     statusString = new char[strlen(oldStatus)+strlen(msg)+1];
//     strcpy(statusString,oldStatus);
//     strcat(statusString,msg);
//     delete[] oldStatus;
//     oldStatus=0;
//   }

//   void functor::appendStatusString(const int& msg) const {
//     char* oldStatus=statusString;
//     char* buf=new char[50];
//     sprintf(buf,"%i",msg);
//     statusString = 0;
//     statusString = new char[strlen(oldStatus)+strlen(buf)+1];
//     strcpy(statusString,oldStatus);
//     strcat(statusString,buf);
//     delete[] oldStatus;
//     oldStatus=0;
//     delete[] buf;
//   }

//   void functor::appendStatusString(const double& msg) const {
//     char* oldStatus=statusString;
//     char* buf=new char[50];
//     sprintf(buf,"%g",msg);
//     statusString = 0;
//     statusString = new char[strlen(oldStatus)+strlen(buf)+1];
//     strcpy(statusString,oldStatus);
//     strcat(statusString,buf);
//     delete[] oldStatus;
//     oldStatus=0;
//     delete[] buf;
//   }

//   void functor::appendStatusString(const functor& other) const {
//     char* oldStatus=statusString;
//     char* buf=new char[1024];
//     sprintf(buf,"\n%s: %s\n",other.getTypeName(),other.getStatusString());
//     statusString = 0;
//     statusString = new char[strlen(oldStatus)+strlen(buf)+1];
//     strcpy(statusString,oldStatus);
//     strcat(statusString,buf);
//     delete[] oldStatus;
//     oldStatus=0;
//     delete[] buf;
//   }

  // write function for symplified use
  bool write(ioHandler& handler,const functor::parameters& p,
             const bool complete) {
    return p.write(handler,complete);
  };

  // read function for symplified use
  bool read(ioHandler& handler,functor::parameters& p,
             const bool complete) {
    return p.read(handler,complete);
  };

  // write function for symplified use
  bool write(ioHandler& handler,const functor& p,
             const bool complete) {
    return p.write(handler,complete);
  };

  // read function for symplified use
  bool read(ioHandler& handler,functor& p,
             const bool complete) {
    return p.read(handler,complete);
  };

} // namespace lti
