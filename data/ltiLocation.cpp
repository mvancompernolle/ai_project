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


/*--------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiLocation.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 7.1.2002
 * revisions ..: $Id: ltiLocation.cpp,v 1.7 2008/08/17 22:20:12 alvarado Exp $
 */


#include "ltiMath.h"
#include "ltiGeometry.h"
#include "ltiLocation.h"

namespace lti {
  /*
   * default constructor
   */
  location::location() : position(0,0),angle(0),radius(0) {
  };

  /*
   * Constructor
   * @param pos position of the middle point of the location
   * @param ang angle of the location (in radians)
   * @param radi radius or size of the location
   */
  location::location(const point& pos, const float& ang,const float& rad)
    : position((float)pos.x,(float)pos.y),angle(ang),radius(rad) {
  }

  /*
   * Constructor
   * @param pos position of the middle point of the location
   * @param ang angle of the location (in radians)
   * @param radi radius or size of the location
   */
  location::location(const tpoint<float>& pos,
                     const float& ang,
                     const float& rad)
    : position(pos),angle(ang),radius(rad) {
  };

  /*
   * copy constructor
   */
  location::location(const location& other) {
    copy(other);
  }

  /*
   * copy operator
   */
  location& location::copy(const location& other) {
    position = other.position;
    angle = other.angle;
    radius = other.radius;
    return *this;
  }

  /*
   * alias for copy operator
   */
  location& location::operator=(const location& other) {
    return copy(other);
  }

  /*
   * operator ==
   */
  bool location::operator==(const location& other) const {
    return ((position == other.position) &&
            (angle == other.angle) &&
            (radius == other.radius));
  }

  /*
   * operator !=
   */
  bool location::operator!=(const location& other) const {
    return ((position != other.position) ||
            (angle != other.angle) ||
            (radius != other.radius));
  }

  /*
   * Comparition operator.
   *
   * A location is smaller than another one if its radius is smaller, or
   * in case of equal radii, if the position is smaller, i.e. if it has
   * smaller y-coordinate, or in case of equal y-position, if it has
   * smaller x-coordinate.
   */
  bool location::operator<(const location& other) {
    if (radius < other.radius) {
      return true;
    } else if (radius == other.radius) {
      if (position < other.position) {
        return true;
      } else if (position == other.position) {
        return (angle < other.angle);
      }
    }

    return false;
  }

  /*
   * Comparition operator.
   *
   * A location is greater than another one if its radius is greater, or
   * in case of equal radii, if the position is greater, i.e. if it has
   * greater y-coordinate, or in case of equal y-position, if it has
   * greater x-coordinate.
   */
  bool location::operator>(const location& other) {
    if (radius > other.radius) {
      return true;
    } else if (radius == other.radius) {
      if (position < other.position) {
        return false;
      } else if (position == other.position) {
        return (angle > other.angle);
      } else {
        return true;
      }
    }

    return false;
  }

  /*
   * Multiply the radius with the given factor
   */
  location& location::scale(const float& factor) {
    radius *= factor;
    position.multiply(factor);
    return *this;
  };

  /*
   * Multiply the other location with the given factor and leave the
   * result here.
   */
  location& location::scale(const location& other,const float& factor) {
    position = other.position*factor;
    angle = other.angle;
    radius = other.radius*factor;
    return *this;
  };

  /*
   * Shift the location by the given point
   */
  location& location::shift(const point& shft) {
    position.x += shft.x;
    position.y += shft.y;
    return *this;
  }

  /*
   * Shift the location by the given point
   */
  location& location::shift(const tpoint<float>& shft) {
    position.add(shft);
    return *this;
  }

  /*
   * Shift the other location by the given point and leave the result here
   */
  location& location::shift(const location& other,const point& shft) {
    position.x = other.position.x+shft.x;
    position.y = other.position.y+shft.y;
    angle = other.angle;
    radius = other.radius;
    return *this;
  }

  /*
   * Shift the other location by the given point and leave the result here
   */
  location& location::shift(const location& other,const tpoint<float>& shft) {
    position.x = other.position.x+shft.x;
    position.y = other.position.y+shft.y;
    angle = other.angle;
    radius = other.radius;
    return *this;
  }

  /*
   * Add the given angle in radius to the actual angle
   */
  location& location::rotate(const float& factor) {
    angle+=factor;
    return *this;
  };

  /*
   * Add the given angle in radius to the angle of the other location and
   * leave the result here.
   */
  location& location::rotate(const location& other,const float& factor) {
    position = other.position;
    angle += other.angle;
    radius = other.radius;
    return *this;
  }

  /*
   * returns the square of the distance between the borders of two
   * locations or zero if they overlap or if one of the locations
   * lies inside the other one.
   */
  float location::distance(const location& other) {

    float d;
    d = position.distanceTo(other.position) - (other.radius+radius);

    if (d<0) d = 0;

    return d;
  }


  /*
   * Check if the given point can be considered within the location, i.e.
   * if ||p-position|| < radius
   */
  bool location::contains(const point& p) const {
    float dx,dy;
    dx = (p.x - position.x);
    dy = (p.y - position.y);

    return ((dx*dx+dy*dy) < radius*radius);
  }

  /*
   * Check if the given point can be considered within the location, i.e.
   * if ||p-position|| < radius
   */
  bool location::contains(const tpoint<float>& p) const {
    float dx,dy;
    dx = (p.x - position.x);
    dy = (p.y - position.y);

    return ((dx*dx+dy*dy) < radius*radius);
  };

  /*
   * read the vector from the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be readed
   */
  bool read(ioHandler& handler,location& loc,const bool complete) {
    bool b(true);

    if (complete) {
      b = handler.readBegin();
    }

    b = b && read(handler,loc.position);
    b = b && handler.readDataSeparator();
    b = b && read(handler,loc.angle);
    b = b && handler.readDataSeparator();
    b = b && read(handler,loc.radius);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

  /*
   * write the vector in the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,
             const location& loc,
             const bool complete) {

    bool b(true);

    if (complete) {
      b = handler.writeBegin();
    }

    b = b && write(handler,loc.position);
    b = b && handler.writeDataSeparator();
    b = b && write(handler,loc.angle);
    b = b && handler.writeDataSeparator();
    b = b && write(handler,loc.radius);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }


  //----------------------------------------------------------------------
  // rectLocation
  //----------------------------------------------------------------------

  /*
   * default constructor
   */
  rectLocation::rectLocation()
    : position(0,0),angle(0),maxLength(0),minLength(0) {
  };

  /*
   * Constructor
   * @param pos position of the middle point of the rectLocation
   * @param ang angle of the rectLocation (in radians)
   * @param radi radius or size of the rectLocation
   */
  rectLocation::rectLocation(const point& pos,
                             const float& ang,
                             const float& maxL,
                             const float& minL)
    : position((float)pos.x,(float)pos.y),angle(ang),
      maxLength(maxL),minLength(minL) {
  }

  /*
   * Constructor
   * @param pos position of the middle point of the rectLocation
   * @param ang angle of the rectLocation (in radians)
   * @param radi radius or size of the rectLocation
   */
  rectLocation::rectLocation(const tpoint<float>& pos,
                             const float& ang,
                             const float& maxL,
                             const float& minL)
    : position(pos),angle(ang),maxLength(maxL),minLength(minL) {
  };

  /*
   * copy constructor
   */
  rectLocation::rectLocation(const location& other) {
    castFrom(other);
  }


  /*
   * copy constructor
   */
  rectLocation::rectLocation(const rectLocation& other) {
    copy(other);
  }

  /*
   * copy operator
   */
  rectLocation& rectLocation::copy(const rectLocation& other) {
    position = other.position;
    angle = other.angle;
    maxLength = other.maxLength;
    minLength = other.minLength;
    return *this;
  }

  /*
   * copy operator
   */
  rectLocation& rectLocation::castFrom(const location& other) {
    position = other.position;
    angle = other.angle;
    maxLength = other.radius;
    minLength = other.radius;
    return *this;
  }

  /*
   * alias for copy operator
   */
  rectLocation& rectLocation::operator=(const rectLocation& other) {
    return copy(other);
  }

  /*
   * operator ==
   */
  bool rectLocation::operator==(const rectLocation& other) const {
    return ((position == other.position) &&
            (angle == other.angle) &&
            (maxLength == other.maxLength) &&
            (minLength == other.minLength));
  }

  /*
   * operator !=
   */
  bool rectLocation::operator!=(const rectLocation& other) const {
    return ((position != other.position) ||
            (angle != other.angle) ||
            (maxLength != other.maxLength) ||
            (minLength != other.minLength));
  }

  float rectLocation::getArea() const {
    return maxLength*minLength;
  }

  /*
   * Comparition operator.
   *
   * A rectLocation is smaller than another one if its radius is smaller, or
   * in case of equal radii, if the position is smaller, i.e. if it has
   * smaller y-coordinate, or in case of equal y-position, if it has
   * smaller x-coordinate.
   */
  bool rectLocation::operator<(const rectLocation& other) {
    float athis = getArea();
    float o = other.getArea();
    if (athis < o) {
      return true;
    } else if (athis == o) {
      if (position < other.position) {
        return true;
      } else if (position == other.position) {
        return (angle < other.angle);
      }
    }

    return false;
  }

  /*
   * Comparition operator.
   *
   * A rectLocation is greater than another one if its radius is greater, or
   * in case of equal radii, if the position is greater, i.e. if it has
   * greater y-coordinate, or in case of equal y-position, if it has
   * greater x-coordinate.
   */
  bool rectLocation::operator>(const rectLocation& other) {
    float athis = getArea();
    float o = other.getArea();
    if (athis > o) {
      return true;
    } else if (athis == o) {
      if (position < other.position) {
        return false;
      } else if (position == other.position) {
        return (angle > other.angle);
      } else {
        return true;
      }
    }

    return false;
  }

  /*
   * Multiply the radius with the given factor
   */
  rectLocation& rectLocation::scale(const float& factor) {
    maxLength *= factor;
    minLength *= factor;
    position.multiply(factor);
    return *this;
  };

  /*
   * Multiply the other rectLocation with the given factor and leave the
   * result here.
   */
  rectLocation& rectLocation::scale(const rectLocation& other,
                                    const float& factor) {
    position = other.position*factor;
    angle = other.angle;
    maxLength = other.maxLength*factor;
    minLength = other.minLength*factor;
    return *this;
  };

  /*
   * Shift the rectLocation by the given point
   */
  rectLocation& rectLocation::shift(const point& shft) {
    position.x += shft.x;
    position.y += shft.y;
    return *this;
  }

  /*
   * Shift the rectLocation by the given point
   */
  rectLocation& rectLocation::shift(const tpoint<float>& shft) {
    position.add(shft);
    return *this;
  }

  /*
   * Shift the other rectLocation by the given point and leave the result here
   */
  rectLocation& rectLocation::shift(const rectLocation& other,
                                    const point& shft) {
    position.x = other.position.x+shft.x;
    position.y = other.position.y+shft.y;
    angle = other.angle;
    maxLength = other.maxLength;
    minLength = other.minLength;
    return *this;
  }

  /*
   * Shift the other rectLocation by the given point and leave the result here
   */
  rectLocation& rectLocation::shift(const rectLocation& other,
                                    const tpoint<float>& shft) {

    position.x = other.position.x+shft.x;
    position.y = other.position.y+shft.y;
    angle = other.angle;
    maxLength = other.maxLength;
    minLength = other.minLength;
    return *this;
  }

  /*
   * Add the given angle in radius to the actual angle
   */
  rectLocation& rectLocation::rotate(const float& factor) {
    angle+=factor;
    return *this;
  };

  /*
   * Add the given angle in radius to the angle of the other rectLocation and
   * leave the result here.
   */
  rectLocation& rectLocation::rotate(const rectLocation& other,
                                     const float& factor) {
    position = other.position;
    angle += other.angle;
    maxLength = other.maxLength;
    minLength = other.minLength;
    return *this;
  }

  /*
   * returns the distance between the borders of two locations or zero
   * if they overlap or if one of the locations lies inside the other one.
   */
  float rectLocation::distanceSqr(const rectLocation& other) {
    tpoint<float> pt,po;
    return distanceSqr(other,pt,po);
  }

  /*
   * returns the distance between the borders of two locations or zero
   * if they overlap or if one of the locations lies inside the other one.
   */
  float rectLocation::distanceSqr(const rectLocation& other,
                                tpoint<float>& pt,
                                tpoint<float>& po) {

    static const int I   = 0;
    static const int II  = 1;
    static const int III = 2;
    static const int IV  = 3;

    tpoint<float> d1,p1[4],d2,p2[4];
    tpoint<float> co1,co2;
    int a1,b1,c1,a2,b2,c2;

    float stmp, ctmp;
    sincos(angle, stmp, ctmp);
    const float cosa1(ctmp), sina1(-stmp);
    sincos(other.angle, stmp, ctmp);
    const float cosa2(ctmp), sina2(-stmp);

    const float m(maxLength/2.0f);
    const float l(minLength/2.0f);
    const float mo(other.maxLength/2.0f);
    const float lo(other.minLength/2.0f);

    // select the two lines of the first location that could be the
    // nearest to the next one.
    // rotation matrix:
    //
    // | cos(a)  sin(a) | | m |
    // |-sin(a)  cos(a) | | l |

    d1.subtract(other.position,position);
    d2.subtract(position,other.position);

    co1.x = cosa1*d1.x - sina1*d1.y;
    co1.y = sina1*d1.x + cosa1*d1.y;

    co2.x = cosa2*d2.x - sina2*d2.y;
    co2.y = sina2*d2.x + cosa2*d2.y;

    // the corners of this location:
    const float lsina1 = l*sina1;
    const float lcosa1 = l*cosa1;
    const float msina1 = m*sina1;
    const float mcosa1 = m*cosa1;

    p1[I].x   =  mcosa1 + lsina1 + position.x; // I
    p1[I].y   = -msina1 + lcosa1 + position.y;

    p1[II].x  = -mcosa1 + lsina1 + position.x; // II
    p1[II].y  =  msina1 + lcosa1 + position.y;

    p1[III].x = -mcosa1 - lsina1 + position.x; // III
    p1[III].y =  msina1 - lcosa1 + position.y;

    p1[IV].x  =  mcosa1 - lsina1 + position.x; // IV
    p1[IV].y  = -msina1 - lcosa1 + position.y;

    // the corners of the other location:
    const float losina2 = lo*sina2;
    const float locosa2 = lo*cosa2;
    const float mosina2 = mo*sina2;
    const float mocosa2 = mo*cosa2;

    p2[I].x   =  mocosa2 + losina2 + other.position.x; // I
    p2[I].y   = -mosina2 + locosa2 + other.position.y;

    p2[II].x  = -mocosa2 + losina2 + other.position.x; // II
    p2[II].y  =  mosina2 + locosa2 + other.position.y;

    p2[III].x = -mocosa2 - losina2 + other.position.x; // III
    p2[III].y =  mosina2 - locosa2 + other.position.y;

    p2[IV].x  =  mocosa2 - losina2 + other.position.x; // IV
    p2[IV].y  = -mosina2 - locosa2 + other.position.y;

    if (co1.x >= 0) {
      if (co1.y >= 0) {
        // 1st quadrant
        a1 = I; b1 = IV; c1 = II;
      } else {
        // 4th quadrant
        a1 = IV; b1 = III; c1 = I;
      }
    } else {
      if (co1.y >= 0) {
        // 2nd quadrant
        a1 = II; b1 = I; c1 = III;
      } else {
        // 3rd quadrant
        a1 = III; b1 = II; c1 = IV;
      }
    }

    // select the two lines of the first location that could be the
    // nearest to the next one.
    if (co2.x >= 0) {
      if (co2.y >= 0) {
        // 1st quadrant
        a2 = I; b2 = IV; c2 = II;
      } else {
        // 4th quadrant
        a2 = IV; b2 = III; c2 = I;
      }
    } else {
      if (co2.y >= 0) {
        // 2nd quadrant
        a2 = II; b2 = I; c2 = III;
      } else {
        // 3rd quadrant
        a2 = III; b2 = II; c2 = IV;
      }
    }

    tpoint<float> dpa[4];
    tpoint<float> dpb[4];
    float d[4];

    d[0] = minDistanceSqr(p1[a1],p1[b1],p2[a2],p2[b2],dpa[0],dpb[0]);
    d[1] = minDistanceSqr(p1[a1],p1[b1],p2[a2],p2[c2],dpa[1],dpb[1]);
    d[2] = minDistanceSqr(p1[a1],p1[c1],p2[a2],p2[b2],dpa[2],dpb[2]);
    d[3] = minDistanceSqr(p1[a1],p1[c1],p2[a2],p2[c2],dpa[3],dpb[3]);

    int idx(0);
    for (int i=1;i<4;++i) {
      if (d[i] < d[idx]) {
        idx = i;
      }
    }

    pt = dpa[idx];
    po = dpb[idx];

    return d[idx];
  }

  /*
   * Check if the given point can be considered within the rectLocation, i.e.
   * if ||p-position|| < radius
   */
  bool rectLocation::contains(const point& p) const {
    float sina, cosa;
    sincos(angle, sina, cosa);
    return (((2*(abs(cosa*(p.x-position.x)+sina*(p.y-position.y))))
             <= maxLength) &&
             ((2*(abs(cosa*(p.y-position.y)-sina*(p.x-position.x))))
             <= minLength));
  }

  /*
   * Check if the given point can be considered within the rectLocation, i.e.
   * if ||p-position|| < radius
   */
  bool rectLocation::contains(const tpoint<float>& p) const {
    float sina, cosa;
    sincos(angle, sina, cosa);
    return (((2*(abs(cosa*(p.x-position.x)+sina*(p.y-position.y))))
             <= maxLength) &&
             ((2*(abs(cosa*(p.y-position.y)-sina*(p.x-position.x))))
             <= minLength));
  };

  /*
   * read the vector from the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be readed
   */
  bool read(ioHandler& handler,rectLocation& loc,const bool complete) {
    bool b(true);

    if (complete) {
      b = handler.readBegin();
    }

    b = b && read(handler,loc.position);
    b = b && handler.readDataSeparator();
    b = b && read(handler,loc.angle);
    b = b && handler.readDataSeparator();
    b = b && read(handler,loc.maxLength);
    b = b && handler.readDataSeparator();
    b = b && read(handler,loc.minLength);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

  /*
   * write the vector in the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,
             const rectLocation& loc,
             const bool complete) {

    bool b(true);

    if (complete) {
      b = handler.writeBegin();
    }

    b = b && write(handler,loc.position);
    b = b && handler.writeDataSeparator();
    b = b && write(handler,loc.angle);
    b = b && handler.writeDataSeparator();
    b = b && write(handler,loc.maxLength);
    b = b && handler.writeDataSeparator();
    b = b && write(handler,loc.minLength);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }



}

namespace std {

  ostream& operator<<(ostream& s,const lti::location& loc) {
    s << "(" << loc.position << ","
      << loc.angle  << ","
      << loc.radius << ")";
    return s;
  };


  ostream& operator<<(ostream& s,const lti::rectLocation& loc) {
    s << "(" << loc.position << ","
      << loc.angle  << ","
      << loc.maxLength << ","
      << loc.minLength << ")";
    return s;
  };
}
