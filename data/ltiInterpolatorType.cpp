/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiInterpolatorType.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 21.11.2003
 * revisions ..: $Id: ltiInterpolatorType.cpp,v 1.3 2006/02/08 11:18:53 ltilib Exp $
 */

#include "ltiInterpolatorType.h"

namespace lti {

  /*
   * read function for eInterpolatorType.
   *
   * @ingroup gStorable
   */
  bool read(ioHandler& handler,eInterpolatorType& data) {

    std::string str;
    if (handler.read(str)) {
   
      if (str.find("earest") != std::string::npos) {
        data = NearestNeighborInterpolator;
      } else if (str.find("ilinear") != std::string::npos) {
        data = BilinearInterpolator;
      } else if (str.find("iquadratic") != std::string::npos) {
        data = BiquadraticInterpolator;
      } else if (str.find("icubic") != std::string::npos) {
        data = BicubicInterpolator;
      } else {
        data = NearestNeighborInterpolator;
        return false;
      }

      return true;
    }

    return false;
  }

  /*
   * write function for eInterpolatorType.
   *
   * @ingroup gStorable
   */
  bool write(ioHandler& handler,const eInterpolatorType& data) {
    bool b=false;
    switch(data) {
      case NearestNeighborInterpolator:
        b=handler.write("NearestNeighborInterpolator");
        break;
      case BilinearInterpolator:
        b=handler.write("BilinearInterpolator");
        break;
      case BiquadraticInterpolator:
        b=handler.write("BiquadraticInterpolator");
        break;
      case BicubicInterpolator:
        b=handler.write("BicubicInterpolator");
        break;
      default:
        b=handler.write("Unknown");
        b=false;
    }

    return b;
  }

}
