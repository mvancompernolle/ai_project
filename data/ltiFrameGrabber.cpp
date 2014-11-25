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
 * file .......: ltiFrameGrabber.cpp
 * authors ....: Peter Mathes
 * organization: LTI, RWTH Aachen
 * creation ...: 13.08.99
 * revisions ..: $Id: ltiFrameGrabber.cpp,v 1.6 2006/09/05 10:35:26 ltilib Exp $
 */

#include "ltiFrameGrabber.h"

namespace lti {

  functor::parameters* frameGrabber::parameters::clone() const {
    return ( new parameters(*this) );
  }

  const char* frameGrabber::parameters::getTypeName() const {
    return "frameGrabber::parameters";
  }

  frameGrabber::parameters&
    frameGrabber::parameters::copy(const parameters& other ) {

# ifndef _LTI_MSC_6
    // for normal  ANSI C++
    functor::parameters::copy(other);
# else
    // this doesn't work with MS-VC++ 6.0 (an once again... another bug)
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters&
      (functor::parameters::* p_copy)(const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif

    turnaround=other.turnaround;

    return ( *this );
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool frameGrabber::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool frameGrabber::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
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

    lti::write(handler,"turnaround",turnaround);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool frameGrabber::parameters::write(ioHandler& handler,
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
  bool frameGrabber::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool frameGrabber::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
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

    lti::read(handler,"turnaround",turnaround);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool frameGrabber::parameters::read(ioHandler& handler,
                                      const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  frameGrabber::frameGrabber( const parameters& theParam ) : functor() {
    setParameters( theParam );
  }

  frameGrabber::frameGrabber(const frameGrabber& other) : functor() {
    copy(other);
  }

  frameGrabber::~frameGrabber() {
  }

  const char* frameGrabber::getTypeName( void ) const {
    return "frameGrabber";
  }

  frameGrabber& frameGrabber::copy(const frameGrabber& other) {
    functor::copy(other);
    return (*this);
  }

  const frameGrabber::parameters& frameGrabber::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if( params==0 )
      throw invalidParametersException(getTypeName());
    return *params;
  }

  // Apply method for channel
  bool frameGrabber::apply(channel& theChannel) {
    channel8 tmp;
    if (apply(tmp)) {
      theChannel.castFrom(tmp);
      return true;
    }
    return false;
  }

  // check if the frame grabber is active
  bool frameGrabber::isActive() const {
    return false;
  }


} // namespace lti
