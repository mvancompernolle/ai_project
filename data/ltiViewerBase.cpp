/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiViewerBase.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 11.11.2001
 * revisions ..: $Id: ltiViewerBase.cpp,v 1.8 2006/02/08 12:58:03 ltilib Exp $
 */

#include "ltiViewerBase.h"
#include "ltiException.h"
#include "ltiMergeRGBToImage.h"
#include "ltiDraw.h"

#include <string>
#include <cstring>

namespace lti {

  // default empty string for the viewerBase
  const char* const viewerBase::emptyString = "";

  // --------------------------------------------------
  // viewerBase::parameters
  // --------------------------------------------------

  // default constructor
  viewerBase::parameters::parameters()
  : functor::parameters() {
    title = getTypeName();
  }

  // copy constructor
  viewerBase::parameters::parameters(const parameters& other)
  : functor::parameters()  {
    copy(other);
  }

  // destructor
  viewerBase::parameters::~parameters() {
  }

  // get type name
  const char* viewerBase::parameters::getTypeName() const {
    return "viewerBase::parameters";
  }

  // copy member

  viewerBase::parameters&
  viewerBase::parameters::copy(const parameters& other) {
    title = other.title;
    return (*this);
  }

  /*
   * copy data of "other" parameters
   */
  viewerBase::parameters&
  viewerBase::parameters::operator=(const parameters& other) {

    std::string errorMsg;

    errorMsg  = "operator= not implemented for these parameters,";
    errorMsg += "please use copy() member instead or implement the ";
    errorMsg += "operator= properly";

    throw exception(errorMsg.c_str());
    return *this;
  };

  // clone member
  functor::parameters* viewerBase::parameters::clone() const {
    return new parameters(*this);
  }

# ifndef _LTI_MSC_6
  bool viewerBase::parameters::write(ioHandler& handler,
                                     const bool complete) const
# else
  bool viewerBase::parameters::writeMS(ioHandler& handler,
                                     const bool complete) const
# endif
  {

    bool b = true;

    if (complete) {
      b = handler.readBegin();
    }
    b = b && lti::write(handler,"title",title);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  };

# ifdef _LTI_MSC_6
  bool viewerBase::parameters::write(ioHandler& handler,
                                     const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif


# ifndef _LTI_MSC_6
  bool viewerBase::parameters::read(ioHandler& handler,
                                    const bool complete)
# else
  bool viewerBase::parameters::readMS(ioHandler& handler,
                                      const bool complete)
# endif
  {
    bool b = true;

    if (complete) {
      b = handler.readBegin();
    }

    b = b && lti::read(handler,"title",title);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  };

# ifdef _LTI_MSC_6
  bool viewerBase::parameters::read(ioHandler& handler,
                                    const bool complete) {
    return readMS(handler,complete);
  };
# endif

  // ----------------------------
  // exceptions
  // ----------------------------

  const char* viewerBase::invalidParametersException::getTypeName() const {
    return "viewerBase::invalidParametersException";
  }

  // --------------------------------------------------
  // viewerBase
  // --------------------------------------------------

  // constructor
  viewerBase::viewerBase() 
    : object(),params(0),ownParam(true),statusString(0) {
  }

  // copy constructor
  viewerBase::viewerBase(const viewerBase& other)
    : object(),params(0),ownParam(true),statusString(0) {
    copy(other);
  }

  // destructor
  viewerBase::~viewerBase() {
    if (ownParam) {
      delete params;
      params = 0;
    }

    delete[] statusString;
    statusString = 0;
  }

  // returns the name of this type
  const char* viewerBase::getTypeName() const {
    return "viewerBase";
  }

  // copy member
  viewerBase& viewerBase::copy(const viewerBase& other) {

    if (other.validParameters()) {
      setParameters(other.getParameters());
    } else {
      if (ownParam) {
        delete params;
      }
      params = 0;
    }

    return (*this);
  }

  // alias for copy
  viewerBase& viewerBase::operator=(const viewerBase& other) {
    std::string str;
    str = "operator= not defined for class ";
    str += getTypeName();
    str += ". Please use copy member instead.";
    throw exception(str.c_str());
    return *this;
  }

  // return parameters
  const viewerBase::parameters& viewerBase::getParameters() const {
    return *params;
  }

  // return parameters
  viewerBase::parameters& viewerBase::getParameters() {
    return *params;
  }

  bool viewerBase::validParameters() const {
    return notNull(params);
  }

  bool viewerBase::setParameters(const parameters& p) {
    // first copy the data p (which could be the same params instance!)
    parameters* tmp = dynamic_cast<parameters*>(p.clone());
    // delete the actual instance if necessary
    if (ownParam) {
      delete params;
      params=0;
    }
    ownParam=true;
    params = tmp;

    return (notNull(params));
  }

  bool viewerBase::useParameters(parameters& p) {
    if (ownParam) {
      delete params;
      params = 0;
      ownParam=false;
    }

    params = &p;

    return (notNull(params));
  }


  /*
   * return the last message set with setStatusString().  This will
   * never return 0.  If no status-string has been set yet an empty string
   * (pointer to a string with only the char(0)) will be returned.
   */
  const char* viewerBase::getStatusString() const {
    if (isNull(statusString)) {
      return emptyString;
    } else {
      return statusString;
    }
  }

  void viewerBase::setStatusString(const char* msg) const {
    delete[] statusString;
    statusString = 0;

    statusString = new char[strlen(msg)+1];
    strcpy(statusString,msg);
  }

  /*
   * shows a 8-bit channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewerBase::show(const channel8& data) {
    mergeRGBToImage merger;
    image img;
    merger.apply(data,data,data,img);
    return show(img);
  }

  /*
   * shows a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewerBase::show(const channel& data) {
    mergeRGBToImage merger;
    image img;
    merger.apply(data,data,data,img);
    return show(img);
  }

  /*
   * shows a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewerBase::show(const matrix<float>& data) {
    mergeRGBToImage merger;
    image img;
    merger.apply(data,data,data,img);
    return show(img);
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewerBase::show(const vector<double>& data) {
    draw<rgbPixel> drawer;
    image canvas(128,data.size(),Black);

    drawer.use(canvas);
    drawer.setColor(White);;
    drawer.set(data,rgbPixel(192,192,192),true);
    return show(canvas);
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewerBase::show(const vector<int>& data) {
    draw<rgbPixel> drawer;
    image canvas(128,data.size(),Black);

    drawer.use(canvas);
    drawer.setColor(White);;
    drawer.set(data,rgbPixel(192,192,192),true);
    return show(canvas);
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewerBase::show(const vector<float>& data) {
    draw<rgbPixel> drawer;
    image canvas(128,data.size(),Black);

    drawer.use(canvas);
    drawer.setColor(White);;
    drawer.set(data,rgbPixel(192,192,192),true);
    return show(canvas);
  }

  /*
   * shows a matrix of doubles as a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewerBase::show(const matrix<double>& data) {
    channel chnl;
    matrix<float>& mat = chnl;
    mat.castFrom(data);

    return show(chnl);
  }


  /*
   * shows a matrix of integers as a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool viewerBase::show(const matrix<int>& data) {
    channel8 chnl;
    matrix<ubyte>& mat = chnl;
    mat.castFrom(data);

    return show(chnl);
  }


  // write function for symplified use
  bool write(ioHandler& handler,const viewerBase::parameters& p,
             const bool complete) {
    return p.write(handler,complete);
  };

  // read function for symplified use
  bool read(ioHandler& handler,viewerBase::parameters& p,
             const bool complete) {
    return p.read(handler,complete);
  };

}
