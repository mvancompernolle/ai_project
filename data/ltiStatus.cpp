/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiStatus.cpp
 * authors ....:
 * organization: LTI, RWTH Aachen
 * creation ...: 01.07.04
 * revisions ..: $Id: ltiStatus.cpp,v 1.2 2006/02/07 18:10:34 ltilib Exp $
 */

#include "ltiStatus.h"
#include "ltiClassName.h"
#include "ltiObject.h"
#include <cstring>
#include <cstdio>
#include <typeinfo>

namespace lti {

  // default empty string for the functor
  const char* const status::emptyString = "";

  status::status()
  : statusString(0) {
  }

  status::~status() {
    delete[] statusString;
    statusString=0;
  }

    /*
   * return the last message set with setStatusString().  This will
   * never return 0.  If no status-string has been set yet an empty string
   * (pointer to a string with only the char(0)) will be returned.
   */
  const char* status::getStatusString() const {
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
  void status::setStatusString(const char* msg) const {
    delete[] statusString;

    statusString = new char[strlen(msg)+1];
    strcpy(statusString,msg);
  }

  void status::appendStatusString(const char* msg) const {
    char* oldStatus=statusString;

    if (isNull(oldStatus)) {
      setStatusString(msg);
      return;
    }

    statusString = new char[strlen(oldStatus)+strlen(msg)+1];
    strcpy(statusString,oldStatus);
    strcat(statusString,msg);
    delete[] oldStatus;
    oldStatus=0;
  }

  void status::appendStatusString(const int& msg) const {
    char* oldStatus=statusString;
    char* buf=new char[50];
    sprintf(buf,"%i",msg);
    statusString = new char[strlen(oldStatus)+strlen(buf)+1];
    strcpy(statusString,oldStatus);
    strcat(statusString,buf);
    delete[] oldStatus;
    oldStatus=0;
    delete[] buf;
  }

  void status::appendStatusString(const double& msg) const {
    char* oldStatus=statusString;
    char* buf=new char[50];
    sprintf(buf,"%g",msg);
    statusString = new char[strlen(oldStatus)+strlen(buf)+1];
    strcpy(statusString,oldStatus);
    strcat(statusString,buf);
    delete[] oldStatus;
    oldStatus=0;
    delete[] buf;
  }

  void status::appendStatusString(const status& other) const {
    char* oldStatus=statusString;
    char* buf=new char[1024];
		className cname;
		sprintf(buf,"%s: %s\n",cname.decode(typeid(other).name()).c_str(),other.getStatusString());
    statusString = new char[strlen(oldStatus)+strlen(buf)+1];
    strcpy(statusString,oldStatus);
    strcat(statusString,buf);
    delete[] oldStatus;
    oldStatus=0;
    delete[] buf;
  }


}
