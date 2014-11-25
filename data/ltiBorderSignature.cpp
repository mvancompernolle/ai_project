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
 * file .......: ltiborderSignature.cpp
 * authors ....: Ruediger Weiler
 * organization: LTI, RWTH Aachen
 * creation ...: 7.6.2001
 * revisions ..: $Id: ltiBorderSignature.cpp,v 1.11 2006/09/05 10:03:36 ltilib Exp $
 */

// include files
#include "ltiMath.h"
#include "ltiContour.h"
#include "ltiBorderSignature.h"

namespace lti {
  // --------------------------------------------------
  // borderSignature::parameters
  // --------------------------------------------------

  // default constructor
  borderSignature::parameters::parameters()
    : featureExtractor::parameters() {
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    center = tpoint<double>(0.0,0.0);
    distanceType = eDistanceType(average);
    numberOfSegments = int(32);
    orientation = double(0.0);
    method = eMethod(distance);
  }

  // copy constructor
  borderSignature::parameters::parameters(const parameters& other)
    : featureExtractor::parameters()  {
    copy(other);
  }

  // destructor
  borderSignature::parameters::~parameters() {
  }

  // get type name
  const char* borderSignature::parameters::getTypeName() const {
    return "borderSignature::parameters";
  }

  // copy member

  borderSignature::parameters&
    borderSignature::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    featureExtractor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    featureExtractor::parameters& (featureExtractor::parameters::* p_copy)
      (const featureExtractor::parameters&) =
      featureExtractor::parameters::copy;
    (this->*p_copy)(other);
# endif


      center = other.center;
      distanceType = other.distanceType;
      numberOfSegments = other.numberOfSegments;
      orientation = other.orientation;
      method = other.method;

    return *this;
  }

  // alias for copy member
  borderSignature::parameters&
    borderSignature::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* borderSignature::parameters::clone() const {
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
  bool borderSignature::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool borderSignature::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"center",center);
      lti::write(handler,"numberOfSegments",numberOfSegments);
      lti::write(handler,"orientation",orientation);
      switch(method) {
          case area:
            handler.write("method","area");
            break;
          case distance:
            handler.write("method","distance");
            break;
          default:
            handler.write("method","unknown");
      }
      switch(distanceType) {
          case min:
            handler.write("distanceType","min");
            break;
          case max:
            handler.write("distanceType","max");
            break;
          case average:
            handler.write("distanceType","average");
            break;
          default:
            handler.write("distanceType","unknown");
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && featureExtractor::parameters::write(handler,false);
# else
    bool (featureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      featureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool borderSignature::parameters::write(ioHandler& handler,
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
  bool borderSignature::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool borderSignature::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"center",center);
      lti::read(handler,"numberOfSegments",numberOfSegments);
      lti::read(handler,"orientation",orientation);

      std::string str;
      lti::read(handler,"method",str);
      if (str == "area") {
        method = area;
      } else if (str == "distance") {
        method = distance;
      } else {
        method = distance;
      }

      lti::read(handler,"distanceType",str);
      if (str == "min") {
        distanceType = min;
      } else if (str == "max") {
        distanceType = max;
      } else if (str == "average") {
        distanceType = average;
      } else {
        distanceType = average;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && featureExtractor::parameters::read(handler,false);
# else
    bool (featureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      featureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool borderSignature::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // borderSignature
  // --------------------------------------------------

  // default constructor
  borderSignature::borderSignature()
    : shapeFeatureExtractor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  borderSignature::borderSignature(const borderSignature& other)
    : shapeFeatureExtractor()  {
    copy(other);
  }

  // destructor
  borderSignature::~borderSignature() {
  }

  // returns the name of this type
  const char* borderSignature::getTypeName() const {
    return "borderSignature";
  }

  // copy member
  borderSignature&
    borderSignature::copy(const borderSignature& other) {
      shapeFeatureExtractor::copy(other);
    return (*this);
  }

  // clone member
  functor* borderSignature::clone() const {
    return new borderSignature(*this);
  }

  // return parameters
  const borderSignature::parameters&
    borderSignature::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool borderSignature::updateParameters() {
    
    const parameters& par = getParameters();
    //chech if the parameters are ok
    if (par.numberOfSegments >= 1) {
      if (abs(par.orientation) <= 2*Pi) {
        return true;
      }
      else {
        setStatusString("The orientation must be between -2*Pi and 2*Pi.");
      }
    }
    else {
      setStatusString("At least one segment is required.");
    }
      
    return false;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  // On copy apply for type borderPoints!
  bool borderSignature::apply(const pointList& src, dvector& segmentation) const {

    //cast the pointlist to borderPoints
    const borderPoints& bp = *dynamic_cast<const borderPoints*>(&src);

    const parameters& pa = getParameters();

    // check requirements
    if ( bp.size() <= 0 ) return false;   // No borderPoints -> no Signature
    borderPoints::const_iterator iter;
    double segmsize;   // angular size of segment
    int segmnr;       // ordinal number of segment
    int x1, y1;       // one point
    double temporient, w1;

    segmsize = (2*Pi)/pa.numberOfSegments;
    segmentation.resize(pa.numberOfSegments,0,false,true);

    if( pa.method == parameters::area ) { //do the area method ----------------
      double w0;
      double deltaA;      // area change within segment
      int signum;         // sign of area change (+ = increment; - = decrement)
      double area = 0;    // total area within borderPoints
      int   x0, y0;       //

      // Initialization of variables
      iter = bp.begin();    // set Iterator to begin of list
      x1 = (*iter).x;       // set first point
      y1 = (*iter).y;
      x0 = x1;
      y0 = y1;
      w0 = atan2(y0-pa.center.y, x0-pa.center.x);
      if(w0 < 0) w0 = 2*Pi+w0; // transform range -Pi to Pi into 0 to 2*Pi

      // process list
      for(iter=bp.begin();iter != bp.end();iter++) {
        // Given: Vertices p0,p1,center of a polygon
        // Searched: Area of polygon, sign of area and assigned segment
        x1 = (*iter).x;
        y1 = (*iter).y;
        // 1.) compute angle
        w1 = atan2(y1-pa.center.y, x1-pa.center.x);
        if(w1 < 0) w1 = 2*Pi+w1; // transform range -Pi to Pi into 0 to 2*Pi

        temporient = w1 - pa.orientation;
        if (temporient <= -Pi) temporient += 2*Pi;
        if (temporient > +Pi)  temporient -= 2*Pi;

        // 2.) compute ordinal number of segment
        segmnr = static_cast<int>(floor( (temporient-segmsize/100) /segmsize));
        if (segmnr < 0) segmnr = pa.numberOfSegments + segmnr;

        // 3.) compute sign of polygon area
        if (fabs(w1-w0)<=Pi)    (w1>=w0) ? signum = +1: signum = -1;
        else                    (w1>=w0) ? signum = -1: signum = +1;

        // 4.) Area of a triangle by scalar-product of 2-D vectors
        // |vectA x vectB| = |vectA|*|vectB|*sin( Winkel(vectA,vectB) )  scalar-prod.
        // |vectA x vectB| = |Ax*By - Ay*Bx| in 2-D
        // triangle area formula: F = 0.5 * a*b*sin( angle(a,b) ) = 0.5*|vectA x vectB|
        deltaA = 0.5*fabs( fabs((x1-pa.center.x)*(y0-pa.center.y))
                         - fabs((x0-pa.center.x)*(y1-pa.center.y)) );

        // 5.) Assign area to segment
        segmentation[segmnr] += deltaA*signum;
        area += deltaA*signum;

        // 6.) get next coord.0 (coord.1 is computed with next iteration)
        x0 = x1;
        y0 = y1;
        w0 = w1;
      }// End of for

      // Normalization of segment entries with total area
      if(area <= 0) area = 1;
      segmentation.divide(area);
    }
    //-------------------------------------------------------------------------
    else { //do the distance method
      // local variables
      double r1;
      double meanDistance = 0;
      dvector segmentPixelCount(pa.numberOfSegments,double(0));

      // Initialize
      iter  = bp.begin();   // Set iterator to begin of list
      x1 = (*iter).x;       // start coordinates
      y1 = (*iter).y;

      // process list
      for(iter=bp.begin();iter != bp.end();iter++) {
        // calculate radius, angle and segment for each pixel
        x1 = (*iter).x;
        y1 = (*iter).y;
        // 1.) calculate angle and distance
        w1 = atan2(y1-pa.center.y, x1-pa.center.x);
        if(w1 < 0) w1 = 2*Pi+w1; // transform range -Pi to Pi into 0 to 2*Pi

        temporient = w1 - pa.orientation;
        if (temporient <= -Pi)  temporient += 2*Pi;
        if (temporient > +Pi)   temporient -= 2*Pi;

        r1 = sqrt( (y1-pa.center.y)*(y1-pa.center.y) + (x1-pa.center.x)*(x1-pa.center.x) );
        // 2.) determine segment no.
        segmnr = static_cast<int>(floor( (temporient-segmsize/100) /segmsize ));
        if (segmnr < 0) segmnr = pa.numberOfSegments + segmnr;
        // 3.) update segment entry
        switch (pa.method){
          case parameters::max:
            // save the maxima of each segment
            if( r1 > segmentation[segmnr] ) segmentation[segmnr] = r1;
            break;
          case parameters::min:
            // save the minima of each segment
            if( segmentation[segmnr] == 0 ) segmentation[segmnr] = r1;
            else if( r1 < segmentation[segmnr] ) segmentation[segmnr] = r1;
            break;
          default: // average
            //save the average radius of each segment
            segmentation[segmnr] += r1;
            segmentPixelCount[segmnr]++;
            break;
        }
        meanDistance += r1;
      }// End of for
      // calculate the mean distance
      meanDistance = meanDistance/bp.size();
      if( meanDistance <= 0 ) meanDistance = 1.0;

      int i = 0;
      switch (pa.method){
        case parameters::max:
          //norm the distances on the average distance
            segmentation.divide(meanDistance);
          break;
        case parameters::min:
          //norm the distances on the average distance
            segmentation.divide(meanDistance);
          break;
        default: //average
          //calculate the average distance for each segment
          //and norm the distances on the average distance
          for (i=0; i<pa.numberOfSegments; i++)
            segmentation[i] = (segmentation[i]/segmentPixelCount[i])/meanDistance;
          break;
      }
    }//------------------------------------------------------------------------

    return true;
  };

}
