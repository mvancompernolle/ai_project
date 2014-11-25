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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiMathObject.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 12.12.00
 * revisions ..: $Id: ltiMathObject.cpp,v 1.5 2006/02/08 12:34:09 ltilib Exp $
 */

#include "ltiMathObject.h"
#include "ltiException.h"

namespace lti {

  /**
   * write the object in the given ioHandler
   */
  bool mathObject::write(ioHandler& handler,
                         const bool complete) const {
    std::string txt;
    txt = "Write not implemented for ";
    txt += getTypeName();
    throw lti::exception(txt.c_str());
    return false;
  }

  /**
   * read the object from the given ioHandler
   */
  bool mathObject::read(ioHandler& handler,const bool complete) {
    std::string txt;
    txt = "Read not implemented for ";
    txt += getTypeName();
    throw lti::exception(txt.c_str());
    return false;
  }

  /**
   * write the functor::parameters in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const mathObject& mo,const bool complete) {
    return mo.write(handler,complete);
  };

  /**
   * read the functor::parameters from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool read(ioHandler& handler,mathObject& mo,const bool complete) {
    return mo.read(handler,complete);
  };
}
