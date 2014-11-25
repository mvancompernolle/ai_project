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


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiEdgeSaliency.cpp
 * authors ....: Stefan Syberichs
 * organization: LTI, RWTH Aachen
 * creation ...: 17.4.2000
 * revisions ..: $Id: ltiEdgeSaliency.cpp,v 1.10 2006/09/05 10:10:21 ltilib Exp $
 */

#include "ltiEdgeSaliency.h"
#include "ltiImage.h"
#include "ltiMatrix.h"
#include "ltiContour.h"
#include "ltiTypes.h"
#include "ltiMath.h"
#include <vector>

namespace lti {
  // --------------------------------------------------
  // edgeSaliency::parameters
  // --------------------------------------------------

  // default constructor
  edgeSaliency::parameters::parameters()
    : saliency::parameters() {

    couplingCenter = 0.0;
    couplingNeighbour = 0.0;
    //sigma = float();
    iterations=20;
    rho = 0.4f;
    gamma=0.5f;
    initialSigma=1.0f;
    binarise=false;
    normalise=true;
    curvationTable = edgeSaliency::parameters::standard;
  }

  // copy constructor
  edgeSaliency::parameters::parameters(const parameters& other)
    : saliency::parameters()  {
    copy(other);
  }

  // destructor
  edgeSaliency::parameters::~parameters() {
  }

  // get type name
  const char* edgeSaliency::parameters::getTypeName() const {
    return "edgeSaliency::parameters";
  }

  // copy member

  edgeSaliency::parameters&
    edgeSaliency::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    saliency::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    modifier::parameters& (modifier::parameters::* p_copy)
      (const modifier::parameters&) =
      modifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    couplingCenter = other.couplingCenter;
    couplingNeighbour = other.couplingNeighbour;
    iterations=other.iterations;
    rho = other.rho;
    gamma = other.gamma;
    initialSigma = other.initialSigma;
    binarise=other.binarise;
    normalise = other.normalise;
    curvationTable = other.curvationTable;

    return *this;
  }

  // clone member
  functor::parameters* edgeSaliency::parameters::clone() const {
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
  bool edgeSaliency::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool edgeSaliency::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"couplingCenter",couplingCenter);
      lti::write(handler,"couplingNeighbour",couplingNeighbour);
      lti::write(handler,"initialSigma",initialSigma);
      lti::write(handler,"iterations",iterations);
      lti::write(handler,"rho",rho);
      lti::write(handler,"gamma",gamma);
      lti::write(handler,"normalise",normalise);
      lti::write(handler,"binarise",binarise);
      switch (curvationTable) {
          case biased:
            lti::write(handler,"curvationTable","biased");
            break;
          default:
            lti::write(handler,"curvationTable","standard");
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && saliency::parameters::write(handler,false);
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
  bool edgeSaliency::parameters::write(ioHandler& handler,
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
  bool edgeSaliency::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool edgeSaliency::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"couplingCenter",couplingCenter);
      lti::read(handler,"couplingNeighbour",couplingNeighbour);
      lti::read(handler,"initialSigma",initialSigma);
      lti::read(handler,"iterations",iterations);
      lti::read(handler,"rho",rho);
      lti::read(handler,"gamma",gamma);
      lti::read(handler,"normalise",normalise);
      lti::read(handler,"binarise",binarise);
      std::string str;
      lti::read(handler,"curvationTable",str);
      if (str == "biased") {
        curvationTable = biased;
      } else {
        curvationTable = standard;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && saliency::parameters::read(handler,false);
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
  bool edgeSaliency::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // edgeSaliency
  // --------------------------------------------------


  float edgeSaliency::curvationLookupTable1[8][8]=                                                // i:
  {{ 1.0f     , 0.58332f , 0.0432f  , 0.00008f , 0.0f     , 0.00008f , 0.0432f  , 0.58332f  },   // E
   { 0.58332f , 1.0f     , 0.58332f , 0.10845f , 0.00008f , 0.0f     , 0.00008f , 0.10845f  },   // NE
   { 0.0432f  , 0.58332f , 1.0f     , 0.58332f , 0.0432f  , 0.00008f , 0.0f     , 0.00008f  },   // N
   { 0.00008f , 0.10845f , 0.58332f , 1.0f     , 0.58332f , 0.10845f , 0.00008f , 0.0f      },   // NW
   { 0.0f     , 0.00008f , 0.0432f  , 0.58332f , 1.0f     , 0.58332f , 0.0432f  , 0.00008f  },   // W
   { 0.00008f , 0.0f     , 0.00008f , 0.10845f , 0.58332f , 1.0f     , 0.58332f , 0.10845f  },   // SW
   { 0.0432f  , 0.00008f , 0.0f     , 0.00008f , 0.0432f  , 0.58332f , 1.0f     , 0.58332f  },   // S
   { 0.58332f , 0.10845f , 0.00008f , 0.0f     , 0.00008f , 0.10845f , 0.58332f , 1.0f      }};  // SE
  //j:  E           NE          N        NW         W          SW          S         SE



  float edgeSaliency::curvationLookupTable2[8][8]=                                                // i:
  {{ 1.0f     , 0.72229f , 0.20788f , 0.00338f , 0.0f     , 0.00338f , 0.20788f , 0.72229f   },   // E
   { 0.72229f , 1.0f     , 0.72229f , 0.20788f , 0.00338f , 0.0f     , 0.00338f , 0.20788f   },   // NE
   { 0.20788f , 0.72229f , 1.0f     , 0.72229f , 0.20788f , 0.00338f , 0.0f     , 0.00338f   },   // N
   { 0.00338f , 0.20788f , 0.72229f , 1.0f     , 0.72229f , 0.20788f , 0.00338f , 0.0f       },   // NW
   { 0.0f     , 0.00338f , 0.20788f , 0.72229f , 1.0f     , 0.72229f , 0.20788f , 0.00338f   },   // W
   { 0.00338f , 0.0f     , 0.00338f , 0.20788f , 0.72229f , 1.0f     , 0.72229f , 0.20788f   },   // SW
   { 0.20788f , 0.00338f , 0.0f     , 0.00338f , 0.20788f , 0.72229f , 1.0f     , 0.72229f   },   // S
   { 0.72229f , 0.20788f , 0.00338f , 0.0f     , 0.00338f , 0.20788f , 0.72229f , 1.0f       }};  // SE
  //j:  E           NE          N        NW         W          SW          S         SE



  float edgeSaliency::radAngles16[16][16] =
  {{ 0.0000f, 0.4636f, 0.7854f, 1.1072f, 1.5708f, 1.8926f, 2.3562f, 2.8179f, 3.1416f, 2.6780f, 2.3562f, 2.0343f, 1.5708f, 1.1072f, 0.7854f, 0.4636f },
   { 0.4636f, 0.0000f, 0.3218f, 0.6436f, 1.1072f, 1.5708f, 1.8926f, 2.4979f, 2.6780f, 3.1416f, 2.8197f, 2.4979f, 2.0343f, 1.5708f, 1.2489f, 0.9271f },
   { 0.7854f, 0.3218f, 0.0000f, 0.3218f, 0.7854f, 1.1072f, 1.5708f, 2.0343f, 2.3562f, 2.6780f, 3.1416f, 2.8197f, 2.3562f, 1.8926f, 1.5708f, 1.2489f },
   { 1.1072f, 0.6436f, 0.3218f, 0.0000f, 0.4636f, 0.6434f, 1.2489f, 1.5708f, 2.0343f, 2.2145f, 2.8197f, 3.1416f, 2.6780f, 2.2145f, 1.8926f, 1.5708f },
   { 1.5708f, 1.1072f, 0.7854f, 0.4636f, 0.0000f, 0.4636f, 0.7854f, 1.1072f, 1.5708f, 1.8926f, 2.3562f, 2.8179f, 3.1416f, 2.6780f, 2.3562f, 2.0343f },
   { 2.0343f, 1.5708f, 1.2489f, 0.9271f, 0.4636f, 0.0000f, 0.3218f, 0.6436f, 1.1072f, 1.5708f, 1.8926f, 2.4979f, 2.6780f, 3.1416f, 2.8197f, 2.4979f },
   { 2.3562f, 1.8926f, 1.5708f, 1.2489f, 0.7854f, 0.3218f, 0.0000f, 0.3218f, 0.7854f, 1.1072f, 1.5708f, 2.0343f, 2.3562f, 2.6780f, 3.1416f, 2.8197f },
   { 2.6780f, 2.2145f, 1.8926f, 1.5708f, 1.1072f, 0.6436f, 0.3218f, 0.0000f, 0.4636f, 0.6434f, 1.2489f, 1.5708f, 2.0343f, 2.2145f, 2.8197f, 3.1416f },
   { 3.1416f, 2.6780f, 2.3562f, 2.0343f, 1.5708f, 1.1072f, 0.7854f, 0.4636f, 0.0000f, 0.4636f, 0.7854f, 1.1072f, 1.5708f, 1.8926f, 2.3562f, 2.8179f },
   { 2.6780f, 3.1416f, 2.8197f, 2.4979f, 2.0343f, 1.5708f, 1.2489f, 0.9271f, 0.4636f, 0.0000f, 0.3218f, 0.6436f, 1.1072f, 1.5708f, 1.8926f, 2.4979f },
   { 2.3562f, 2.6780f, 3.1416f, 2.8197f, 2.3562f, 1.8926f, 1.5708f, 1.2489f, 0.7854f, 0.3218f, 0.0000f, 0.3218f, 0.7854f, 1.1072f, 1.5708f, 2.0343f },
   { 2.0343f, 2.2145f, 2.8197f, 3.1416f, 2.6780f, 2.2145f, 1.8926f, 1.5708f, 1.1072f, 0.6436f, 0.3218f, 0.0000f, 0.4636f, 0.6434f, 1.2489f, 1.5708f },
   { 1.5708f, 1.8926f, 2.3562f, 2.8179f, 3.1416f, 2.6780f, 2.3562f, 2.0343f, 1.5708f, 1.1072f, 0.7854f, 0.4636f, 0.0000f, 0.4636f, 0.7854f, 1.1072f },
   { 1.1072f, 1.5708f, 1.8926f, 2.4979f, 2.6780f, 3.1416f, 2.8197f, 2.4979f, 2.0343f, 1.5708f, 1.2489f, 0.9271f, 0.4636f, 0.0000f, 0.3218f, 0.6436f },
   { 0.7854f, 1.1072f, 1.5708f, 2.0343f, 2.3562f, 2.6780f, 3.1416f, 2.8197f, 2.3562f, 1.8926f, 1.5708f, 1.2489f, 0.7854f, 0.3218f, 0.0000f, 0.3218f },
   { 0.4636f, 0.6434f, 1.2489f, 1.5708f, 2.0343f, 2.2145f, 2.8197f, 3.1416f, 2.6780f, 2.2145f, 1.8926f, 1.5708f, 1.1072f, 0.6436f, 0.3218f, 0.0000f }};

    // i -->


  float edgeSaliency::geometricLength[16] = {1.0f, 2.2361f, 1.4142f, 2.2361f, 1.0f, 2.2361f, 1.4142f, 2.2361f,
                                             1.0f, 2.2361f, 1.4142f, 2.2361f, 1.0f, 2.2361f, 1.4142f, 2.2361f};


  // default constructor
  edgeSaliency::edgeSaliency()
    : saliency(){
  }

  // copy constructor
  edgeSaliency::edgeSaliency(const edgeSaliency& other)
    : saliency()  {
    copy(other);
  }

  // destructor
  edgeSaliency::~edgeSaliency() {
  }

  // returns the name of this type
  const char* edgeSaliency::getTypeName() const {
    return "edgeSaliency";
  }

  // copy member
  edgeSaliency&
    edgeSaliency::copy(const edgeSaliency& other) {
      saliency::copy(other);
    return (*this);
  }

  // clone member
  functor* edgeSaliency::clone() const {
    return (new edgeSaliency(*this));
  }

  // return parameters
  const edgeSaliency::parameters&
    edgeSaliency::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------


  // On place apply for type channel8!
  bool edgeSaliency::apply(channel8& srcdest) const {

    channel8 dest;

    if (apply(srcdest, dest)) {
      srcdest.copy(dest);
      return true;
    }

    return false;
  };

  // On place apply for type channel!
  bool edgeSaliency::apply(channel& srcdest) const {

    channel dest;

    if (apply(srcdest, dest)) {
      srcdest.copy(dest);
      return true;
    }

    return false;
  };

  // On copy apply for type channel8!
  bool edgeSaliency::apply(const channel8& src,channel8& dest) const {

    channel floatresult;

    // workaround:
    // use channel-type buffer as argument for updateSaliency()
    floatresult.resize(src.rows(), src.columns());

    updateSaliency(src, floatresult);

    // cast from channel to channel8
    dest.castFrom(floatresult);

    return true;
  };

  // On copy apply for type channel!
  bool edgeSaliency::apply(const channel& src,channel& dest) const {

    dest.resize(src.rows(), src.columns());

    updateSaliency(src, dest);

    return true;
  };

  // the fastets method !
  bool edgeSaliency::apply(const channel8& src, channel& dest) const {

    dest.resize(src.rows(), src.columns());

    updateSaliency(src, dest);

    return true;

  }

  /*
  // OnCopy apply for type image, output data is channel8
  channel8& edgeSaliency::apply(const image& src, channel8& dest) const{


    // throw an exception

    return dest;
  }

 // OnCopy apply for type image, output data is channel
  channel& edgeSaliency::apply(const image& src, channel& dest) const{

    // throw an exception

    return dest;
  }
  */

  // --------------------------------------------
  //  private members
  // --------------------------------------------

  // update the saliency of the edges with a
  // connected network
  // channel8 version
  void edgeSaliency::updateSaliency(const channel8& src, channel& map) const{

    int col, row, run, i, j, otherMap;
    int xOffset, yOffset;
    float maximum, overallMaximum, temp;
    std::vector< std::vector< eVector > > sigmaMap;
    std::vector< std::vector< eVector > > activityMaps[2];

    point centerpoint;
    point otherpoint;
    point neighbour;
    int orientation;

    //chainCode here;
    sigmaMap.resize(src.rows());

    activityMaps[0].resize(src.rows());
    activityMaps[1].resize(src.rows());

    for (i=0;i<src.rows();++i) {
      sigmaMap[i].resize(src.columns());
      activityMaps[0][i].resize(src.columns());
      activityMaps[1][i].resize(src.columns());
    }

    map.resize(src.rows(), src.columns());

    // initialize all sigmas in every sigmaMap
    // and all activities in the activityMap  to zero
    for(row = 0; row < src.rows(); row++) {
      for(col=0; col < src.columns(); col++) {
	      sigmaMap[row][col].resize(16, 0.0f);
     	  activityMaps[0][row][col].resize(16,0.0f);
	      activityMaps[1][row][col].resize(16,0.0f);
      }
    }

    // initialisation of sigmas
    for(row = 3; row < src.rows()-3; row++){
      for(col=3; col < src.columns()-3; col++){

	/*                           edgecodes:
             NW  N  NE ^
                \|/    |        x --> i : sigma
              W -x- E  0        =============================
                /|\             0     0    0
             SW  S  SE          0     1    coupling Neighbour
                                1     0    coupling Center
                                1     1    initial sigma (1.0)

      	    The first 8 fields of the activity vector in the sigma Map
	          denote the outgoing orientation elements, (0..7), the second
	          8 fields denote the incoming orientation elements

            According to the tables above, the activity of an edge is
            zero, if the central pixel x and the indicated
            direction-pixel are both zero (== no edge points), and 1,
            if both points are edge points. if only one point marks an
            edge, the activity is set to 0.5 (the presence of an edge
            in the src channel is actually indicated by the value
            255...)

        */

	// outgoing connections and incoming connections

	// calculate activity for all directions


	centerpoint.copy(point(col,row));


	for(yOffset = -1; yOffset <=1; yOffset++){
	  for(xOffset = -1; xOffset <= 1; xOffset++) {

	    // except centerpixel, both offsets are zero
	    if(xOffset!=0 || yOffset!=0) {

	      otherpoint.copy(point(col+xOffset, row+yOffset));
              //orientationElement.copy(chainCode(centerpoint, otherpoint));
              orientation = getOrientation16(centerpoint, otherpoint);

              //cout <<"orientation : " << orientation <<"\n";

	      if (src.at(otherpoint) > 0 && src.at(centerpoint) > 0) {
		      sigmaMap[row][col].at(orientation) = getParameters().initialSigma;
		      activityMaps[0][row][col].at(orientation) = getParameters().initialSigma;
          //cout <<"\nsigma = 1.0 at (r,c), i " << row << "," << col << ", " << orientation;

	      }
	      else if(src.at(centerpoint)>0 && src.at(otherpoint)==0) {
	        sigmaMap[row][col].at(orientation) = getParameters().couplingCenter;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingCenter;

	      }
	      else if(src.at(centerpoint)==0 && src.at(otherpoint)>0) {
          sigmaMap[row][col].at(orientation) = getParameters().couplingNeighbour;
          activityMaps[0][row][col].at(orientation) = getParameters().couplingNeighbour;
	      }//else
	    }// if...
	  }// for (xOffset...)
	}// for (yOffset...)

        // "outer" orientations

        // direction 1
        yOffset = -1;
        xOffset = 2;
        otherpoint.copy(point(col+xOffset, row+yOffset));
        orientation = getOrientation16(centerpoint, otherpoint);  //cout <<"orientation : " << orientation <<"\n";
        if (src.at(otherpoint) > 0 && src.at(centerpoint) > 0) {
          sigmaMap[row][col].at(orientation) = getParameters().initialSigma;
          activityMaps[0][row][col].at(orientation) = getParameters().initialSigma;
        }
        else if(src.at(centerpoint)>0 && src.at(otherpoint)==0) {
	        sigmaMap[row][col].at(orientation) = getParameters().couplingCenter;
          activityMaps[0][row][col].at(orientation) = getParameters().couplingCenter;
        }
	 else if(src.at(centerpoint)==0 && src.at(otherpoint)>0) {
		sigmaMap[row][col].at(orientation) = getParameters().couplingNeighbour;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingNeighbour;
	 }//else


         //direction 3
         yOffset = -2;
         xOffset = 1;
         otherpoint.copy(point(col+xOffset, row+yOffset));
         orientation = getOrientation16(centerpoint, otherpoint);  //cout <<"orientation : " << orientation <<"\n";
         if (src.at(otherpoint) > 0 && src.at(centerpoint) > 0) {
		sigmaMap[row][col].at(orientation) = getParameters().initialSigma;
		activityMaps[0][row][col].at(orientation) = getParameters().initialSigma;

         }
	 else if(src.at(centerpoint)>0 && src.at(otherpoint)==0) {
	        sigmaMap[row][col].at(orientation) = getParameters().couplingCenter;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingCenter;

	 }
	 else if(src.at(centerpoint)==0 && src.at(otherpoint)>0) {
		sigmaMap[row][col].at(orientation) = getParameters().couplingNeighbour;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingNeighbour;
	 }//else

         //direction 5
         yOffset = -2;
         xOffset = -1;
         otherpoint.copy(point(col+xOffset, row+yOffset));
         orientation = getOrientation16(centerpoint, otherpoint);  //cout <<"orientation : " << orientation <<"\n";
         if (src.at(otherpoint) > 0 && src.at(centerpoint) > 0) {
		sigmaMap[row][col].at(orientation) = getParameters().initialSigma;
		activityMaps[0][row][col].at(orientation) = getParameters().initialSigma;

         }
	 else if(src.at(centerpoint)>0 && src.at(otherpoint)==0) {
	        sigmaMap[row][col].at(orientation) = getParameters().couplingCenter;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingCenter;

	 }
	 else if(src.at(centerpoint)==0 && src.at(otherpoint)>0) {
		sigmaMap[row][col].at(orientation) = getParameters().couplingNeighbour;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingNeighbour;
	 }//else


         //direction 7
         yOffset = -1;
         xOffset = -2;
         otherpoint.copy(point(col+xOffset, row+yOffset));
         orientation = getOrientation16(centerpoint, otherpoint);  //cout <<"orientation : " << orientation <<"\n";
         if (src.at(otherpoint) > 0 && src.at(centerpoint) > 0) {
		sigmaMap[row][col].at(orientation) = getParameters().initialSigma;
		activityMaps[0][row][col].at(orientation) = getParameters().initialSigma;

         }
	 else if(src.at(centerpoint)>0 && src.at(otherpoint)==0) {
	        sigmaMap[row][col].at(orientation) = getParameters().couplingCenter;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingCenter;

	 }
	 else if(src.at(centerpoint)==0 && src.at(otherpoint)>0) {
		sigmaMap[row][col].at(orientation) = getParameters().couplingNeighbour;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingNeighbour;
	 }//else


         //direction 9
         yOffset = 1;
         xOffset = -2;
         otherpoint.copy(point(col+xOffset, row+yOffset));
         orientation = getOrientation16(centerpoint, otherpoint); // cout <<"orientation : " << orientation <<"\n";
         if (src.at(otherpoint) > 0 && src.at(centerpoint) > 0) {
		sigmaMap[row][col].at(orientation) = getParameters().initialSigma;
		activityMaps[0][row][col].at(orientation) = getParameters().initialSigma;

         }
	 else if(src.at(centerpoint)>0 && src.at(otherpoint)==0) {
	        sigmaMap[row][col].at(orientation) = getParameters().couplingCenter;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingCenter;

	 }
	 else if(src.at(centerpoint)==0 && src.at(otherpoint)>0) {
		sigmaMap[row][col].at(orientation) = getParameters().couplingNeighbour;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingNeighbour;
	 }//else



         // direction 11
         yOffset = 2;
         xOffset = -1;
         otherpoint.copy(point(col+xOffset, row+yOffset));
         orientation = getOrientation16(centerpoint, otherpoint); // cout <<"orientation : " << orientation <<"\n";
         if (src.at(otherpoint) > 0 && src.at(centerpoint) > 0) {
		sigmaMap[row][col].at(orientation) = getParameters().initialSigma;
		activityMaps[0][row][col].at(orientation) = getParameters().initialSigma;

         }
	 else if(src.at(centerpoint)>0 && src.at(otherpoint)==0) {
	        sigmaMap[row][col].at(orientation) = getParameters().couplingCenter;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingCenter;

	 }
	 else if(src.at(centerpoint)==0 && src.at(otherpoint)>0) {
		sigmaMap[row][col].at(orientation) = getParameters().couplingNeighbour;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingNeighbour;
	 }//else


         // direction 13
         yOffset = 2;
         xOffset = 1;
         otherpoint.copy(point(col+xOffset, row+yOffset));
         orientation = getOrientation16(centerpoint, otherpoint); // cout <<"orientation : " << orientation <<"\n";
         if (src.at(otherpoint) > 0 && src.at(centerpoint) > 0) {
		sigmaMap[row][col].at(orientation) = getParameters().initialSigma;
		activityMaps[0][row][col].at(orientation) = getParameters().initialSigma;

         }
	 else if(src.at(centerpoint)>0 && src.at(otherpoint)==0) {
	        sigmaMap[row][col].at(orientation) = getParameters().couplingCenter;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingCenter;

	 }
	 else if(src.at(centerpoint)==0 && src.at(otherpoint)>0) {
		sigmaMap[row][col].at(orientation) = getParameters().couplingNeighbour;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingNeighbour;
	 }//else

         // direction 15
         yOffset = 1;
         xOffset = 2;
         otherpoint.copy(point(col+xOffset, row+yOffset));
         orientation = getOrientation16(centerpoint, otherpoint);  //cout <<"orientation : " << orientation <<"\n";
         if (src.at(otherpoint) > 0 && src.at(centerpoint) > 0) {
		sigmaMap[row][col].at(orientation) = getParameters().initialSigma;
		activityMaps[0][row][col].at(orientation) = getParameters().initialSigma;

         }
	 else if(src.at(centerpoint)>0 && src.at(otherpoint)==0) {
	        sigmaMap[row][col].at(orientation) = getParameters().couplingCenter;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingCenter;

	 }
	 else if(src.at(centerpoint)==0 && src.at(otherpoint)>0) {
		sigmaMap[row][col].at(orientation) = getParameters().couplingNeighbour;
          	activityMaps[0][row][col].at(orientation) = getParameters().couplingNeighbour;
	 }//else


      }// for (cols...)
    } // for (rows...)




    // sigmas initialized, step 0 is done

    //cout <<"\noverallMaximum in run 0 1";

    overallMaximum=0;
    // next N runs...
    //cout << "\nrunning " << getParameters().iterations << "\n";
    for(run = 1; run <= getParameters().iterations; run++) {

     // we allways switch between map 0 and map 1 in every run to buffer the data from the last iteration
      if(run%2==1)
        otherMap = 0;
      else
        otherMap = 1;

      // run%2 now the index of the actual map, othermap the index of the last iteration's map

      for(row = 3; row < src.rows()-3; row++){
        for(col=3; col < src.columns()-3; col++){

	  // look at all 16 orientation elements of pixel .at(row,col):
	  for(i = 0; i< 16; i++) {

	    //get (x,y) coordinates of neighbour in direction i

            //here.copy(i); // direction i; E = 0, NE=1, N=2 etc.

	    // neighbour is the point in direction i from (row, col)
	    neighbour = getNeighbour(point(col, row), i);

            //cout <<"\nneighbour of (col,row) : "<<col<<","<<row<<" in direction" <<i<<" =  "<<neighbour.x <<"," << neighbour.y;

            // calculate maximum of E_j * curvation(i,j) for all j

	    maximum=0;
            // look at all 16 orientation elements of the neighbour of i:
	    for(j = 0; j < 16; j++) {

	      temp = activityMaps[otherMap][neighbour.y][neighbour.x].at(j)*curvation(i,j);

	      if(temp > maximum) {
                maximum = temp;
	      }
	    } // for all j


            //cout <<"\nmaximum : "<< maximum << ", sigma(i) = " << sigmaMap[row][col].at(i);


            activityMaps[run%2][row][col].at(i) = sigmaMap[row][col].at(i)
	                                       + rho(sigmaMap[row][col].at(i))*maximum;

	    // update overallMaximum
	    if(activityMaps[run%2][row][col].at(i) > overallMaximum)
	       overallMaximum = activityMaps[run%2][row][col].at(i);


	  } // for all i
        } // for (col...)
      }// for(row...)

      //cout <<"\noverallMaximum in run "<< run << " " << overallMaximum;

    } // for all runs


    //cout <<"\noverallMaximum nach allen runs: " << overallMaximum;


    // retrieve maximum of all 16 activities in every map position
    for(row = 3; row < src.rows()-3; row++) {
      for(col=3; col < src.columns()-3; col++) {

        maximum = 0;

	for(i = 0; i < 16; i++) {
	  if (activityMaps[run%2][row][col].at(i) > maximum) {
	    maximum = activityMaps[run%2][row][col].at(i);
	  }
	}


	//normalise, if needed
	if(getParameters().normalise == true) {
	  map.at(row,col) = maximum/overallMaximum;


	  // apply gamma
	  if (map.at(row,col)<getParameters().gamma && !getParameters().binarise)
	    map.at(row,col)=0.0;

	  // binarize, if needed
	  if (getParameters().binarise){
	    if(map.at(row,col)>getParameters().gamma)
	      map.at(row,col)=1.0;
	    else
	      map.at(row,col)=0.0;
	  }
	}
	else {
	  // no normalisation, binarise and gamma do not have any effect
	  map.at(row,col) = maximum;
       }
      }
    }
  } // updateSaliency()


  // update the saliency of the edges with a
  // connected network
  // channel version
  void edgeSaliency::updateSaliency(const channel& src, channel& map) const{

    channel floatbuffer;
    channel8 bytebuffer;

    // the channel version cast a channel8 from the src data
    // and applies the channel8-update method above.
    // Hence, this version is slower

    bytebuffer.castFrom(src);

    updateSaliency(bytebuffer, map);


  }


  float edgeSaliency::rho(const float sigma) const {


    // rhoZero is supposed to be:  0 <= rhoZero <= 1

    if (sigma>0) // ?
      return 1.0f*sigma;
    else
      return getParameters().rho;

  }


  float edgeSaliency::curvation(const int pi, const int pj) const {

  //    if (getParameters().curvationTable==edgeSaliency::parameters::biased)
//        return curvationLookupTable2[pi][pj];
//      else
//        return curvationLookupTable1[pi][pj];


    float c=0;
    // float ds = 1.0f;
    float alpha = radAngles16[pj][pi];

    //cout <<"\nalpha = " << alpha;
    //cout <<"\ntan alpha = " << tan(alpha/2.0f);

    c = exp(-(2.0f*alpha*tan(alpha/2.0f))/(orientationSquareMean(pi, pj)));

    //cout <<"\nc = " << c;

     if(c > 2)
      return 0.0;
     else
       return c;


  }


  int edgeSaliency::getOrientation16(const point here, const point there) const {

    // point(x,y) = (col, row)

    if(there.y == here.y   && there.x == here.x+1)
      return 0;

    if(there.y == here.y-1 && there.x == here.x+2)
      return 1;

    if(there.y == here.y-1 && there.x == here.x+1)
      return 2;

    if(there.y == here.y-2 && there.x == here.x+1)
      return 3;

    if(there.y == here.y-1 && there.x == here.x)
      return 4;

    if(there.y == here.y-2 && there.x == here.x-1)
      return 5;

    if(there.y == here.y-1 && there.x == here.x-1)
      return 6;

    if(there.y == here.y-1 && there.x == here.x-2)
      return 7;

    if(there.y == here.y   && there.x == here.x-1)
      return 8;

    if(there.y == here.y+1 && there.x == here.x-2)
      return 9;

    if(there.y == here.y+1 && there.x == here.x-1)
      return 10;

    if(there.y == here.y+2 && there.x == here.x-1)
      return 11;

    if(there.y == here.y+1 && there.x == here.x)
      return 12;

    if(there.y == here.y+2 && there.x == here.x+1)
      return 13;

    if(there.y == here.y+1 && there.x == here.x+1)
      return 14;

    if(there.y == here.y+1 && there.x == here.x+2)
      return 15;

    //else
    return -1;

  }

  // get the neighbour (x,y) position in direction DIRECTION from HERE
  point edgeSaliency::getNeighbour(const point here, const int direction) const {

    point neighbour;


    switch(direction) {
    case 0 : neighbour.x = here.x+1;
             neighbour.y = here.y;
             break;

    case 1 : neighbour.x = here.x+2;
             neighbour.y = here.y-1;
             break;

    case 2 : neighbour.x = here.x+1;
             neighbour.y = here.y-1;
             break;

    case 3 : neighbour.x = here.x+1;
             neighbour.y = here.y-2;
             break;

    case 4 : neighbour.x = here.x;
             neighbour.y = here.y-1;
             break;

    case 5 : neighbour.x = here.x-1;
             neighbour.y = here.y-2;
             break;

    case 6 : neighbour.x = here.x-1;
             neighbour.y = here.y-1;
             break;

    case 7 : neighbour.x = here.x-2;
             neighbour.y = here.y -1;
             break;

    case 8 : neighbour.x = here.x-1;
             neighbour.y = here.y;
             break;

    case 9 : neighbour.x = here.x-2;
             neighbour.y = here.y+1;
             break;

    case 10: neighbour.x = here.x-1;
             neighbour.y = here.y+1;
             break;

    case 11: neighbour.x = here.x-1;
             neighbour.y = here.y+2;
             break;

    case 12: neighbour.x = here.x;
             neighbour.y = here.y+1;
             break;

    case 13: neighbour.x = here.x+1;
             neighbour.y = here.y+2;
             break;

    case 14: neighbour.x = here.x+1;
             neighbour.y = here.y+1;
             break;

    case 15: neighbour.x = here.x+2;
             neighbour.y = here.y+1;
             break;

    default: neighbour.x = -1;
             neighbour.y = -1;
             break;


    }

    return neighbour;


  }


  float edgeSaliency::orientationSquareMean(const int i, const int j)const {



    return sqrt(geometricLength[i]*geometricLength[j]);



  }
}
