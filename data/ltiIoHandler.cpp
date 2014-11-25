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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiIoHandler.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 7.12.2000
 * revisions ..: $Id: ltiIoHandler.cpp,v 1.10 2006/02/08 12:09:18 ltilib Exp $
 */

#include "ltiIoHandler.h"
#include <cstring>
#include <cstdio> // required for sprintf in append status string

namespace lti {

  // default empty string for the functor
  const char* const ioHandler::emptyString = "";

  // --------------------------------------------------
  // ioHandler
  // --------------------------------------------------

  // default constructor
  ioHandler::ioHandler()
    : object(),level(0),statusString(0) {
  }

  // copy constructor
  ioHandler::ioHandler(const ioHandler& other)
    : object(),statusString(0) {
    copy(other);
  }

  // destructor
  ioHandler::~ioHandler() {
    delete[] statusString;
    statusString=0;
  }

  // returns the name of this type
  const char* ioHandler::getTypeName() const {
    return "ioHandler";
  }

  // copy member
  ioHandler&
  ioHandler::copy(const ioHandler& other) {
    level = other.level;
    return (*this);
  }

  const int& ioHandler::getLevel() const {
    return level;
  }

  void ioHandler::resetLevel(const int& theLevel) {
    level = theLevel;
  }

  bool ioHandler::writeBegin() {
    level++;
    return true;
  }

  bool ioHandler::writeEnd() {
    level--;
    return (level>=0);
  }

  bool ioHandler::readBegin() {
    level++;
    return true;
  }
  bool ioHandler::readEnd() {
    level--;
    return (level>=0);
  }

  /** @name write members for symbol data pairs
   */
  //@{
  bool ioHandler::write(const std::string& name,const std::string& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const char* data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const double& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const float& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const int& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const unsigned int& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const char& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const byte& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const ubyte& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const bool& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const long& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const unsigned long& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const short& data) {
    return lti::write(*this,name,data);
  };
  bool ioHandler::write(const std::string& name,const unsigned short& data) {
    return lti::write(*this,name,data);
  };
  //@}

  /* @name write member for standard types
   */
  //@{
  bool write(ioHandler& handler,const std::string& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const char* data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const double& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const float& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const int& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const unsigned int& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const char& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const byte& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const ubyte& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const bool& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const long& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const unsigned long& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const short& data) {
    return handler.write(data);
  }
  bool write(ioHandler& handler,const unsigned short& data) {
    return handler.write(data);
  }
  //@}


  /* @name read members for symbol data pairs
   */
  //@{
  bool ioHandler::read(const std::string& name,std::string& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,double& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,float& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,int& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,unsigned int& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,char& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,byte& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,ubyte& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,bool& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,long& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,unsigned long& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,short& data) {
    return lti::read(*this,name,data);
  };
  bool ioHandler::read(const std::string& name,unsigned short& data) {
    return lti::read(*this,name,data);
  };
  //@}

  /* @name read member for standard types
   */
  //@{
  bool read(ioHandler& handler,std::string& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,double& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,float& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,int& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,unsigned int& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,char& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,byte& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,ubyte& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,bool& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,long& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,unsigned long& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,short& data) {
    return handler.read(data);
  }
  bool read(ioHandler& handler,unsigned short& data) {
    return handler.read(data);
  }
  //@}

  /*
   * return the last message set with setStatusString().  This will
   * never return 0.  If no status-string has been set yet an empty string
   * (pointer to a string with only the char(0)) will be returned.
   */
  const char* ioHandler::getStatusString() const {
    if (isNull(statusString)) {
      return emptyString;
    } else {
      return statusString;
    }
  }

  /*
   * set a status string.
   *
   * @param msg the const string to be reported next time by
   * getStatusString()
   * This message will be usually set within the apply methods to indicate
   * an error cause.
   */
  void ioHandler::setStatusString(const char* msg) const {
    delete[] statusString;
    statusString = 0;

    statusString = new char[strlen(msg)+1];
    strcpy(statusString,msg);
  }

  void ioHandler::setStatusString(const std::string& msg) const {
    setStatusString(msg.c_str());
  }

  void ioHandler::appendStatusString(const char* msg) const {
    char* oldStatus=statusString;

    if (isNull(oldStatus)) {
      setStatusString(msg);
      return;
    }

    statusString = 0;
    statusString = new char[strlen(oldStatus)+strlen(msg)+1];
    strcpy(statusString,oldStatus);
    strcat(statusString,msg);
    delete[] oldStatus;
    oldStatus=0;
  }

  void ioHandler::appendStatusString(const std::string& msg) const {
    appendStatusString(msg.c_str());
  }

  void ioHandler::appendStatusString(const int& msg) const {
    char* oldStatus=statusString;
    char* buf=new char[50];
    sprintf(buf,"%i",msg);
    statusString = 0;
    statusString = new char[strlen(oldStatus)+strlen(buf)+1];
    strcpy(statusString,oldStatus);
    strcat(statusString,buf);
    delete[] oldStatus;
    oldStatus=0;
    delete[] buf;
  }

  void ioHandler::appendStatusString(const double& msg) const {
    char* oldStatus=statusString;
    char* buf=new char[50];
    sprintf(buf,"%g",msg);
    statusString = 0;
    statusString = new char[strlen(oldStatus)+strlen(buf)+1];
    strcpy(statusString,oldStatus);
    strcat(statusString,buf);
    delete[] oldStatus;
    oldStatus=0;
    delete[] buf;
  }

  void ioHandler::appendContextStatus() const {
  }

}
