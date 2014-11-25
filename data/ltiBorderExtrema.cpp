/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiBorderExtrema.cpp
 * authors ....: Ruediger Weiler
 * organization: LTI, RWTH Aachen
 * creation ...: 11.6.2001
 * revisions ..: $Id: ltiBorderExtrema.cpp,v 1.10 2006/09/05 10:03:25 ltilib Exp $
 */

#include "ltiContour.h"
#include "ltiPolygonPoints.h"
#include "ltiBorderExtrema.h"

namespace lti {
  // --------------------------------------------------
  // borderExtrema::parameters
  // --------------------------------------------------

  // default constructor
  borderExtrema::parameters::parameters()
    : modifier::parameters() {
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    minTolerance  = int(5);
    center = tpoint<double>(0.0,0.0);
  }

  // copy constructor
  borderExtrema::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  borderExtrema::parameters::~parameters() {
  }

  // get type name
  const char* borderExtrema::parameters::getTypeName() const {
    return "borderExtrema::parameters";
  }

  // copy member

  borderExtrema::parameters&
    borderExtrema::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    modifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    modifier::parameters& (modifier::parameters::* p_copy)
      (const modifier::parameters&) =
      modifier::parameters::copy;
    (this->*p_copy)(other);
# endif


      minTolerance  = other.minTolerance ;
      center = other.center;

    return *this;
  }

  // alias for copy member
  borderExtrema::parameters&
    borderExtrema::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* borderExtrema::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool borderExtrema::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool borderExtrema::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"minTolerance ",minTolerance );
      lti::write(handler,"center",center);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,false);
# else
    bool (modifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      modifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool borderExtrema::parameters::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
# endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool borderExtrema::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool borderExtrema::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"minTolerance ",minTolerance );
      lti::read(handler,"center",center);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,false);
# else
    bool (modifier::parameters::* p_readMS)(ioHandler&,const bool) =
      modifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool borderExtrema::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // borderExtrema
  // --------------------------------------------------

  // default constructor
  borderExtrema::borderExtrema()
    : modifier(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  borderExtrema::borderExtrema(const borderExtrema& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  borderExtrema::~borderExtrema() {
  }

  // returns the name of this type
  const char* borderExtrema::getTypeName() const {
    return "borderExtrema";
  }

  // copy member
  borderExtrema&
    borderExtrema::copy(const borderExtrema& other) {
      modifier::copy(other);
    return (*this);
  }

  // clone member
  functor* borderExtrema::clone() const {
    return new borderExtrema(*this);
  }

  // return parameters
  const borderExtrema::parameters&
    borderExtrema::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool borderExtrema::updateParameters() {
    //check if the parameters are ok
    const parameters& par = getParameters();

    if( par.minTolerance  >= 1 ) {
      return true;
    }

    setStatusString("minTolerance must be greater or equal to 1");
    return false;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // apply for borderPoints with two extrema lists
  bool borderExtrema::apply(const borderPoints& src,
                            polygonPoints& min,
                            polygonPoints& max) const {
    // check if we have a border
    if( src.size()<=0 ) return false;		  // no object no extrema
    //get the parameters
    const parameters pa = getParameters();
    //clear the polygonPoint lists
    min.clear();
    max.clear();
    // iter pointer to the border
    borderPoints::const_iterator iter;
    iter = src.begin();
    // initialize the variables
    int x = (*iter).x;          // the actual point.x
    int y = (*iter).y;          // the actual point.y
    float dist=0, lastdist=0, lastExtremaDistance=0;  //some distances
    int countDown = 0, countUp = 0;     // for counting the minima and maxima
    int maxx= -1, maxy= -1, minx= -1, miny= -1;
    // don't terminate the search at the at the end of the list
    borderPoints::const_iterator enditer; // but at the first extremum
    enditer = src.begin();                // therefore set an end-indicator
    for (int i=0; i<=pa.minTolerance +1; i++) enditer++;
    // calculate the distance square.
    // need only the suare of the distance for comparison
    lastExtremaDistance = static_cast<float>(((x-pa.center.x)*(x-pa.center.x)+
                           (y-pa.center.y)*(y-pa.center.y)));
    dist = lastExtremaDistance;
    // While loop: search and save minima and maxima
    int turn = 0;		// variable for identifying the second run
    while ( (iter != enditer) || (turn == 0) ) {
      // calculate the square distance
      x = (*iter).x;
      y = (*iter).y;
      lastdist = dist;
      dist = static_cast<float>(((x-pa.center.x)*(x-pa.center.x)+
                                 (y-pa.center.y)*(y-pa.center.y)));
      //count the extrema
      if (dist<=lastdist) {// count falling edges
        countDown++;
        if (countDown >= pa.minTolerance  && dist <= lastExtremaDistance) {
          minx = x;miny = y;
          lastExtremaDistance = dist;
          countUp=0;
        }
      }
      else {// count rising edges
        countUp++;
        if (countUp >= pa.minTolerance  && dist >= lastExtremaDistance) {
          maxx = x;maxy = y;
          lastExtremaDistance = dist;
          countDown=0;
        }
      }
      // if minimum found
      if (countUp == pa.minTolerance  && minx > -1) {
        // save it
        min.push_back(tpoint<int>(minx,miny));
        minx = -1;
      }
      // if maximum found
      if (countDown == pa.minTolerance  && maxx > -1) {
        // save it too
        max.push_back(tpoint<int>(maxx,maxy));
        maxx = -1;
      }
      iter++;
      if ( iter == src.end() && turn == 0) {
        iter = src.begin();
        turn = 1;
      }
    }
    return true;
  };

  // apply for borderPoints with one list for all extrema
  bool borderExtrema::apply(const borderPoints& src,
                            polygonPoints& minMax) const {
    // check if we have a border
    if( src.size()<=0 ) return false;		// no object no extrema
    // get the parameters
    const parameters pa = getParameters();
    // want to know type of last extremum, because minimum has to be first
    bool lastElementIsMax = false;
    // erase the polygonPoints
    minMax.clear();
    // iter pointer to the border
    borderPoints::const_iterator iter;
    iter = src.begin();
    // initialize the variables
    int x = (*iter).x;					// the actual point.x
    int y = (*iter).y;					// the actual point.y
    float dist, lastdist, lastExtremaDistance;
    int countDown = 0, countUp = 0;
    int maxx= -1, maxy= -1, minx= -1, miny= -1;
    // don't terminate the search at the at the end of the list
    borderPoints::const_iterator enditer; // but at the first extremum
    enditer = src.begin();                // therefore set an end-indicator
    for (int i=0; i<=pa.minTolerance +1; i++) enditer++;
    // calculate the distance square.
    // need only the suare of the distance for comparison
    dist = static_cast<float>(((x-pa.center.x)*(x-pa.center.x)+
                               (y-pa.center.y)*(y-pa.center.y)));
    lastExtremaDistance = dist;

    // While loop: search and save the extrema
    int turn = 0;		// variable for identifying the second run
    while ( (iter != enditer) || (turn == 0) ) {
      // calculate the square distance
      x = (*iter).x;
      y = (*iter).y;
      lastdist = dist;
      dist = static_cast<float>(((x-pa.center.x)*(x-pa.center.x)+
                                 (y-pa.center.y)*(y-pa.center.y)));

      //count the extrema
      if (dist<=lastdist) {// count falling edges
        countDown++;
        if (countDown >= pa.minTolerance  && dist <= lastExtremaDistance) {
          minx = x;miny = y;
          lastExtremaDistance = dist;
          countUp=0;
        }
      }
      else {// count rising edges
        countUp++;
        if (countUp >= pa.minTolerance  && dist >= lastExtremaDistance) {
          maxx = x;maxy = y;
          lastExtremaDistance = dist;
          countDown=0;
        }
      }
      // found a minimum
      if (countUp == pa.minTolerance  && minx > -1) {
        // save it
        minMax.push_back(tpoint<int>(minx,miny));
        lastElementIsMax = false;
        minx = -1;
      }
      // found a maximum
      if (countDown == pa.minTolerance  && maxx > -1) {
        // save it to the same lsit
        minMax.push_back(tpoint<int>(maxx,maxy));
        lastElementIsMax = true;
        maxx = -1;
      }

      iter++;
      if ( iter == src.end() && turn == 0) {
        iter = src.begin();
        turn = 1;
      }
    }
    //the first element should always be a minimum
    if( !lastElementIsMax ) {
      // if the last element is a minimum, then the first element is a maximum.
      // Therefore move the first element ,the maxima, to the end.
      point x = (*minMax.begin());
      minMax.pop_front();
      minMax.push_back(x);
    }

    return true;
  };

}
