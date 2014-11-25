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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiArctanLUT.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 01.06.2003
 * revisions ..: $Id: ltiArctanLUT.cpp,v 1.4 2006/02/08 12:11:33 ltilib Exp $
 */

#include "ltiArctanLUT.h"
#include "ltiConstants.h"

namespace lti {

  // static members
  const arctanLUT::cfloatptr* arctanLUT::atanLUT = 0;

  // constructor
  arctanLUT::arctanLUT() {
    constructArcTanLUT();
  }

  /*
   * construct a atan2 LUT.  It will assume that the difference values
   * of the gradients will be always between -255 and 255.  The 1MB Memory
   * required should not be a problem in modern PCs anymore!
   */
  void arctanLUT::constructArcTanLUT() {
    if (isNull(atanLUT)) {
      static const int side = 511;
      static const int sideH = 255;
      
      // use the singleton approach to avoid memory leak at the end of 
      // the application
      static float lutData[side*side]; // the data
      float* lutLines[side];           // pointers to 0 elements of each line
      static cfloatptr clutLines[side];

      float angle;
      int i,dx,dy;

      for (i=0;i<side;++i) {
        lutLines[i]=&lutData[(side*i)+sideH];
        clutLines[i]=&lutData[(side*i)+sideH];
      }

      float** tmpLUT = &lutLines[sideH];

      for (dy=-255;dy<256;++dy) {
        for (dx=-255;dx<256;++dx) {
          angle = lti::atan2(static_cast<float>(dy),
                             static_cast<float>(dx));
          if (angle < 0) {
            angle += constants<float>::TwoPi();
          }
          tmpLUT[dy][dx] = angle;
        }
      }

      // now make the singleton array "public".  It will be deleted at
      // the end.
      atanLUT = &clutLines[sideH];
    }
  }

}
