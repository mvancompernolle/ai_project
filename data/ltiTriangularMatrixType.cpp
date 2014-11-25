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


/*--------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiTriangularMatrixType.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 7.7.2003
 * revisions ..: $Id: ltiTriangularMatrixType.cpp,v 1.4 2006/02/08 12:49:27 ltilib Exp $
 */

#include "ltiTriangularMatrixType.h"

namespace lti {

  triangularMatrixType& 
  triangularMatrixType::copy(const triangularMatrixType& other){
    triangularType=other.triangularType;
    return *this;
  }
  
  triangularMatrixType& 
  triangularMatrixType::operator=(const triangularMatrixType& other) {
    return copy(other);
  }

  bool triangularMatrixType::write(ioHandler& handler,
                                   const bool complete) const {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }
    
    if (b) {
      
      switch(triangularType) {
        case Lower:
          lti::write(handler,"triangularType","Lower");
          break;
        case Upper:
          lti::write(handler,"triangularType","Upper");
          break;
        default:
          b=false;
          handler.setStatusString("triangularType undefined");
          lti::write(handler,"triangularType","Lower");
      }              
    }
    
    
    if (complete) {
      b = b && handler.writeEnd();
    }
    
    return b;
  }
  
  
  bool triangularMatrixType::read(ioHandler& handler,
                                  const bool complete) {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }
    
    if (b) {
      
      std::string tmp;
      lti::read(handler,"triangularType",tmp);
      if (tmp=="Lower") {
        triangularType=Lower;
      } else if (tmp=="Upper") {
        triangularType=Upper;
      } else {
        b=false;
        handler.setStatusString("undefined triangularType");
        triangularType=Lower;
      }            
    }

    if (complete) {
      b = b && handler.readEnd();
    }
    
    return b;
  }

}
