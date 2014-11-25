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
 * file .......: ltiPointSetNormalizationType.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 18.4.2004
 * revisions ..: $Id: ltiPointSetNormalizationType.cpp,v 1.3 2006/02/08 11:38:29 ltilib Exp $
 */

#include "ltiPointSetNormalizationType.h"

namespace lti {

  /*
   * read function for ePointSetNormalizationType.
   *
   * @ingroup gStorable
   */
  bool read(ioHandler& handler,ePointSetNormalizationType& data) {

    std::string str;
    if (handler.read(str)) {
   
      if (str.find("sotropicNormalization") != std::string::npos) {
        data = IsotropicNormalization;
      } else if (str.find("oPointSetNormalization") != std::string::npos) {
        data = NoPointSetNormalization;
      } else {
        data = NoPointSetNormalization;
        return false;
      } 

      return true;
    }

    return false;
  }

  /*
   * write function for ePointSetNormalizationType.
   *
   * @ingroup gStorable
   */
  bool write(ioHandler& handler,const ePointSetNormalizationType& data) {
    bool b=false;
    switch(data) {
      case IsotropicNormalization:
        b=handler.write("IsotropicNormalization");
        break;
      case NoPointSetNormalization:
        b=handler.write("NoPointSetNormalization");
	break;
      default:
        b=handler.write("Unkown");
        b=false;
	break;
    }

    return b;
  }

}
