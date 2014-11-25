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
 * file .......: ltiPointList.cpp
 * authors ....: Suat Akyol
 * organization: LTI, RWTH Aachen
 * creation ...: 28.06.00
 * revisions ..: $Id: ltiPointList.cpp,v 1.3 2006/02/08 11:37:42 ltilib Exp $
 */

#include "ltiPointList.h"
#include "ltiPointList_template.h"

namespace lti {
  // explicit instantiations
  template class tpointList<int>;
  template class tpointList<float>;
  template class tpointList<double>;

} // namespace lti

namespace std {
  template ostream& operator<<(ostream& s,const lti::tpointList<int>&);
  template ostream& operator<<(ostream& s,const lti::tpointList<float>&);
  template ostream& operator<<(ostream& s,const lti::tpointList<double>&);
}
