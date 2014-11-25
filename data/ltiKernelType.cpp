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
 * file .......: ltiKernelType.cpp
 * authors ....: Bodo Hoffmann
 * organization: LTI, RWTH Aachen
 * creation ...: 11.07.2004
 * revisions ..: $Id: ltiKernelType.cpp,v 1.2 2006/02/07 19:08:27 ltilib Exp $
 */

#include "ltiKernelType.h"

namespace lti {

  /**
   * read function for eKernelType.
   *
   * @ingroup gStorable
   */
  bool read(ioHandler& handler,eKernelType& data) {

    std::string str;
    if (handler.read(str)) {
      if (str == "Symmetric") {
        data = Symmetric;
      } else if (str == "Asymmetric") {
        data = Asymmetric;
      } else if (str == "Default") {
        data = Default;
      } else {
        data = Default;
        return false;
      }
      return true;
    }
    return false;
  }

  /**
   * write function for eKernelType.
   *
   * @ingroup gStorable
   */
  bool write(ioHandler& handler,const eKernelType& data) {
    bool b=false;
    switch(data) {
    case Symmetric:
      b=handler.write("Symmetric");
      break;
    case Asymmetric:
      b=handler.write("Asymmetric");
      break;
    case Default:
      b=handler.write("Default");
      break;
    default:
      b=handler.write("Default");
    }

    return b;
  }

}
