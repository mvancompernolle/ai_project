/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiGeometry.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 19.03.02
 * revisions ..: $Id: ltiGeometry.cpp,v 1.4 2006/02/08 12:25:46 ltilib Exp $
 */

#include "ltiGeometry.h"
#include "ltiGeometry_template.h"

namespace lti {

  // explicit instantiations of functions for int, float and double

  template bool intersection<int>(const tpoint<int>& p1,const tpoint<int>& p2,
                                  const tpoint<int>& p3,const tpoint<int>& p4,
                                  tpoint<int>& p);

  template bool intersection<float>(const tpoint<float>& p1,
                                    const tpoint<float>& p2,
                                    const tpoint<float>& p3,
                                    const tpoint<float>& p4,
                                    tpoint<float>& p);

  template bool intersection<double>(const tpoint<double>& p1,
                                     const tpoint<double>& p2,
                                     const tpoint<double>& p3,
                                     const tpoint<double>& p4,
                                     tpoint<double>& p);

  template int minDistanceSqr<int>(const tpoint<int>& p1,
                                  const tpoint<int>& p2,
                                  const tpoint<int>& p3,
                                  tpoint<int>& p);

  template float minDistanceSqr<float>(const tpoint<float>& p1,
                                    const tpoint<float>& p2,
                                    const tpoint<float>& p3,
                                    tpoint<float>& p);

  template double minDistanceSqr<double>(const tpoint<double>& p1,
                                     const tpoint<double>& p2,
                                     const tpoint<double>& p3,
                                     tpoint<double>& p);

  template int minDistanceSqr<int>(const tpoint<int>& p1,
                                 const tpoint<int>& p2,
                                 const tpoint<int>& p3,
                                 const tpoint<int>& p4,
                                 tpoint<int>& pa,
                                 tpoint<int>& pb);

  template float minDistanceSqr<float>(const tpoint<float>& p1,
                                     const tpoint<float>& p2,
                                     const tpoint<float>& p3,
                                     const tpoint<float>& p4,
                                     tpoint<float>& pa,
                                     tpoint<float>& pb);

  template double minDistanceSqr<double>(const tpoint<double>& p1,
                                       const tpoint<double>& p2,
                                       const tpoint<double>& p3,
                                       const tpoint<double>& p4,
                                       tpoint<double>& pa,
                                       tpoint<double>& pb);

  template int clockwiseTurn<int>(const tpoint<int>& p0,
                                  const tpoint<int>& p1,
                                  const tpoint<int>& p2);

  template int clockwiseTurn<float>(const tpoint<float>& p0,
                                    const tpoint<float>& p1,
                                    const tpoint<float>& p2);

  template int clockwiseTurn<double>(const tpoint<double>& p0,
                                     const tpoint<double>& p1,
                                     const tpoint<double>& p2);

}
