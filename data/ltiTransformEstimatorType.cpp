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
 * file .......: ltiTransformEstimatorType.cpp
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 18.4.2004
 * revisions ..: $Id: ltiTransformEstimatorType.cpp,v 1.3 2006/02/08 11:57:56 ltilib Exp $
 */

#include "ltiTransformEstimatorType.h"

namespace lti {

  /*
   * read function for eTransformEstimatorType.
   *
   * @ingroup gStorable
   */
  bool read(ioHandler& handler,eTransformEstimatorType& data) {

    std::string str;
    if (handler.read(str)) {
   
      if (str.find("omography8DofEstimator") != std::string::npos) {
        data = Homography8DofEstimator;
      } else if (str.find("omography9DofEstimator") != std::string::npos) {
        data = Homography9DofEstimator;
      } else if (str.find("ffineEstimator") != std::string::npos) {
        data = AffineEstimator;
      } else if (str.find("imilarityEstimator") != std::string::npos) {
        data = SimilarityEstimator;
      } else if (str.find("oTransformEstimator") != std::string::npos) {
        data = NoTransformEstimator;
      } else {
        data = NoTransformEstimator;
        return false;
      }

      return true;
    }

    return false;
  }

  /*
   * write function for eTransformEstimatorType.
   *
   * @ingroup gStorable
   */
  bool write(ioHandler& handler,const eTransformEstimatorType& data) {
    bool b=false;
    switch(data) {
      case Homography8DofEstimator:
        b=handler.write("Homography8DofEstimator");
        break;
      case Homography9DofEstimator:
        b=handler.write("Homography9DofEstimator");
        break;
      case AffineEstimator:
        b=handler.write("AffineEstimator");
        break;
      case SimilarityEstimator:
        b=handler.write("SimilarityEstimator");
        break;
      case NoTransformEstimator:
        b=handler.write("NoTransformEstimator");
        break;
      default:
        b=handler.write("Unknown");
        b=false;
	break;
    }

    return b;
  }

}
