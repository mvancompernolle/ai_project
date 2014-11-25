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
 * file .......: ltiGeometricFeatureGroup.h
 * authors ....: Claudia Goenner
 * organization: LTI, RWTH Aachen
 * creation ...: 22.07.2003
 * revisions ..: $Id: ltiGeometricFeatureGroup.cpp,v 1.6 2006/02/07 18:58:36 ltilib Exp $
 */

#include "ltiGeometricFeatureGroup.h"
#include <limits>

namespace lti {

  geometricFeatureGroup0::geometricFeatureGroup0()
    : minX(std::numeric_limits<int>::max(),
	   std::numeric_limits<int>::max()),
      maxX(), 
      minY(std::numeric_limits<int>::max(),
	   std::numeric_limits<int>::max()), 
      maxY(), area(0), cog() 
  {}

  geometricFeatureGroup0::geometricFeatureGroup0
  (const point& initMinX, const point& initMaxX,
   const point& initMinY, const point& initMaxY,
   const int initArea, const tpoint<float>& initCog)
    : minX(initMinX), maxX(initMaxX),
      minY(initMinY), maxY(initMaxY), area(initArea), cog(initCog)
  {}

  geometricFeatureGroup0::geometricFeatureGroup0(const geometricFeatureGroup0& other)
  {
    copy(other);
  }

  geometricFeatureGroup0& 
  geometricFeatureGroup0::copy(const geometricFeatureGroup0& other)
  {
    area = other.area;
    minX = other.minX;
    maxX = other.maxX;
    minY = other.minY;
    maxY = other.maxY;
    cog.copy(other.cog);
    
    return *this;
  }

  geometricFeatureGroup0& 
  geometricFeatureGroup0::operator=(const geometricFeatureGroup0& other) {
    return (copy(other));
  }

  bool geometricFeatureGroup0::addObjectPoint(int x, int y) {

    if ( x < minX.x ) {
      minX.x = x;  minX.y = y;
    }
    if ( x > maxX.x ) {
      maxX.x = x;  maxX.y = y;
    } 
    if ( y < minY.y ) {
      minY.x = x;  minY.y = y;
    }
    if ( y > maxY.y ) {
      maxY.x = x;  maxY.y = y;
    }  
 
    area++;
    cog.x += float(x);
    cog.y += float(y);
    return true;
  }

  bool geometricFeatureGroup0::addObjectPoint(point p) {
    return addObjectPoint(p.x, p.y);
  }

  bool geometricFeatureGroup0::completeComputation() {
    
    if (area > 0) {
      cog /= float(area);
      return true;
      
    } else
      return false;
  }

  bool geometricFeatureGroup0
  ::operator+=(const geometricFeatureGroup0& other) {
    
    fpoint cog1 ( cog.x * area,cog.y * area );
    fpoint cog2 ( other.cog.x * other.area, other.cog.y * other.area );
    area += other.area;
    if ( area == 0 )  {
      return true;
    }
    cog = ( cog1 + cog2 ) / area;
    if ( minX.x > other.minX.x ) {
      minX = other.minX;
    }
    if ( maxX.x < other.maxX.x ) {
      maxX = other.maxX;
    }
    if ( minY.y > other.minY.y ) {
      minY = other.minY;
    }
    if ( maxY.y < other.maxY.y ) {
      maxY = other.maxY;
    }
    return true;
  }

  bool geometricFeatureGroup0::operator==(const geometricFeatureGroup0& other) const
  {
    return ( minX == other.minX && maxX == other.maxX
	     && minY == other.minY && maxY == other.maxY
	     && area == other.area && cog == other.cog );
  }

  bool geometricFeatureGroup0::operator!=(const geometricFeatureGroup0& other) const
  {
    return ( minX != other.minX && maxX != other.maxX
	     && minY != other.minY && maxY != other.maxY
	     && area != other.area && cog != other.cog );
  }

  bool 
  geometricFeatureGroup0::isClose(const geometricFeatureGroup0& other,
				    const point& dist) const {
    
    const point br1 ( other.maxX.x + dist.x,
		      other.maxY.y + dist.y );
    const point ul1 ( other.minX.x - dist.x,
		      other.minY.y - dist.y );
    
    if ( maxX.x >= ul1.x ) {
      if ( maxX.x <= br1.x ) {
	if ( maxY.y >= ul1.y ) {
	  if ( maxY.y <= br1.y ) {
	    return true;
	    
	  } else if ( minY.y <= br1.y ) {
	    return true;

	  } else {
	    return false;
	  }

	} else {
	  return false;
	}

      } else if ( minX.x <= br1.x ) {
	if ( minY.y >= ul1.y ) {
	  if ( minY.y <= br1.y ) {
	    return true;

	  } else {
	    return false;
	  }

	} else if ( maxY.y >= ul1.y ) {
	  if ( minY.y <= br1.y ) {
	    return true;

	  } else {
	    return false;
	  }

	} else {
	  return false;
	}
      }
    }
    
    return false;
  }

  bool  geometricFeatureGroup0
  ::read(ioHandler& handler, const bool complete) {

    bool b;

    if (complete) {
      handler.readBegin();
    }

    b = lti::read(handler, minX);

    handler.readDataSeparator();
    b = lti::read(handler, maxX);

    handler.readDataSeparator();
    b = lti::read(handler, minY);

    handler.readDataSeparator();
    b = lti::read(handler, maxY);

    handler.readDataSeparator();
    b = lti::read(handler, area);

    handler.readDataSeparator();
    b = lti::read(handler, cog);

    if (complete) {
      b = handler.readEnd();
    }
    return b;
  }

  bool geometricFeatureGroup0
  ::write(ioHandler& handler, const bool complete) {
    bool b;
    
    if (complete) {
      handler.writeBegin();
    }
    
    b = lti::write(handler, minX);

    handler.writeDataSeparator();
    b = lti::write(handler, maxX);

    handler.writeDataSeparator();
    b = lti::write(handler, minY);

    handler.writeDataSeparator();
    b = lti::write(handler, maxY);

    handler.writeDataSeparator();
    b = lti::write(handler, area);
    handler.writeDataSeparator();
    b = lti::write(handler, cog);

    if (complete) {
      b = handler.writeEnd();
    }

    return b;
  }

}//namespace lti
