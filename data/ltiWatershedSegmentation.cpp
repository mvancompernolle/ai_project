
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
 * file .......: ltiWatershedSegmentation.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 11.1.2001
 * revisions ..: $Id: ltiWatershedSegmentation.cpp,v 1.11 2006/09/05 10:33:34 ltilib Exp $
 */

#include "ltiWatershedSegmentation.h"

#undef _LTI_DEBUG
// #define _LTI_DEBUG 2
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiViewer.h"
#include "ltiTimer.h"
#endif

namespace lti {
  // --------------------------------------------------
  // watershedSegmentation::parameters
  // --------------------------------------------------

  // default constructor
  watershedSegmentation::parameters::parameters()
    : segmentation::parameters() {

    neighborhood8 = false;

    watershedValue = 255;
    basinValue = 0;
    rainfall = true;
    threshold = 0;
  }

  // copy constructor
  watershedSegmentation::parameters::parameters(const parameters& other)
    : segmentation::parameters()  {
    copy(other);
  }

  // destructor
  watershedSegmentation::parameters::~parameters() {
  }

  // get type name
  const char* watershedSegmentation::parameters::getTypeName() const {
    return "watershedSegmentation::parameters";
  }

  // copy member

  watershedSegmentation::parameters&
  watershedSegmentation::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    segmentation::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    segmentation::parameters& (segmentation::parameters::* p_copy)
      (const segmentation::parameters&) =
      segmentation::parameters::copy;
    (this->*p_copy)(other);
# endif


    neighborhood8 = other.neighborhood8;
    watershedValue = other.watershedValue;
    basinValue = other.basinValue;
    rainfall = other.rainfall;
    threshold = other.threshold;
    return *this;
  }

  // alias for copy member
  watershedSegmentation::parameters&
  watershedSegmentation::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* watershedSegmentation::parameters::clone() const {
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
  bool watershedSegmentation::parameters::write(ioHandler& handler,
                                                const bool complete) const
# else
    bool watershedSegmentation::parameters::writeMS(ioHandler& handler,
                                                    const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"neighborhood8",neighborhood8);
      lti::write(handler,"watershedValue",watershedValue);
      lti::write(handler,"basinValue",basinValue);
      lti::write(handler,"rainfall",rainfall);
      lti::write(handler,"threshold",threshold);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::write(handler,false);
# else
    bool (segmentation::parameters::* p_writeMS)(ioHandler&,const bool) const =
      segmentation::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool watershedSegmentation::parameters::write(ioHandler& handler,
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
  bool watershedSegmentation::parameters::read(ioHandler& handler,
                                               const bool complete)
# else
    bool watershedSegmentation::parameters::readMS(ioHandler& handler,
                                                   const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"neighborhood8",neighborhood8);
      lti::read(handler,"watershedValue",watershedValue);
      lti::read(handler,"basinValue",basinValue);
      lti::read(handler,"rainfall",rainfall);
      lti::read(handler,"threshold",threshold);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::read(handler,false);
# else
    bool (segmentation::parameters::* p_readMS)(ioHandler&,const bool) =
      segmentation::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool watershedSegmentation::parameters::read(ioHandler& handler,
                                               const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // watershedSegmentation
  // --------------------------------------------------

  // default constructor
  watershedSegmentation::watershedSegmentation()
    : segmentation(){
    parameters param;
    setParameters(param);
  }

  // default constructor
  watershedSegmentation::watershedSegmentation(const parameters& par)
    : segmentation() {
    setParameters(par);
  }

  // copy constructor
  watershedSegmentation::watershedSegmentation(const watershedSegmentation& other)
    : segmentation()  {
    copy(other);
  }

  // destructor
  watershedSegmentation::~watershedSegmentation() {
  }

  // returns the name of this type
  const char* watershedSegmentation::getTypeName() const {
    return "watershedSegmentation";
  }

  // copy member
  watershedSegmentation&
  watershedSegmentation::copy(const watershedSegmentation& other) {
    segmentation::copy(other);
    return (*this);
  }

  // clone member
  functor* watershedSegmentation::clone() const {
    return new watershedSegmentation(*this);
  }

  // return parameters
  const watershedSegmentation::parameters&
  watershedSegmentation::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The private methods!
  // -------------------------------------------------------------------

  /**
   * fill neighborhood pointlist
   */
  void watershedSegmentation::createNeighborhood(const int colms, 
                                                 const bool neigh8) {
    if (!neigh8) {
      neigh.resize(4,0,false,false);
    } else {
      neigh.resize(8,0,false,false);
    }

    neigh[0]= +1;
    neigh[1]= -colms;
    neigh[2]= -1;
    neigh[3]= +colms;

    if( neigh8 ) {
      neigh[4]= -colms+1;
      neigh[5]= -colms-1;
      neigh[6]= +colms-1;
      neigh[7]= +colms+1;
    }
  }

  /**
   * Initialize a border LUT to save time detecting if a pixel is in
   * the border.
   */
  void watershedSegmentation::initBorderLUT(const point& size,
                                            channel8& borderLUT) const {
    borderLUT.resize(size,0,false,true);
    borderLUT.getRow(0).fill(255);
    borderLUT.getRow(borderLUT.lastRow()).fill(255);

    const int lastRow = size.y-1;
    for (int y=0;y<lastRow;++y) {
      borderLUT.at(y,0)=borderLUT.at(y,borderLUT.lastColumn())=255;
    }
  }

  /**
   * create "histogram" with pixelPositions
   */
  void 
  watershedSegmentation::sortPixels(const channel8& src,
                                  std::vector<list_type>& sortedPoints) const {
    // sortedPoints is assumed empty.
    sortedPoints.resize(256);
    int i, threshold = getParameters().threshold;
    if( threshold != 0 ) { // raise water to threshold (to eleminate noise)
      for(i=0;i<imgSize;i++) {
	if (src.at(i) < threshold) {
	  sortedPoints[threshold].push_back(i);
        }
	else {
	  sortedPoints[src.at(i)].push_back(i);
        }
      }
    }
    else {
      for (i=0; i<imgSize; i++) {
	sortedPoints[src.at(i)].push_back(i);
      }
    }
  }
  
  // scan all points of specific gray value
  // "result" is filled with -1
  void watershedSegmentation::maskCurrLevelPoints(
                                        const list_type& currentPointList,
                                        vector<int>& distance,
                                        std::queue<int>& fifoQueue,
                                        matrix<int>& result) const {
    static const int WSHED =  0;
    static const int MASK  = -2;

    int currentNeighbor;
    list_type::const_iterator pointIterator = currentPointList.begin();
    while (pointIterator != currentPointList.end()) {

      const int currentPoint = *pointIterator;
      pointIterator++;

      result.at(currentPoint) = MASK;
      int n;
      for(n=0;n<neigh.size();n++) { // scan neighborhood
	// calculate absolute neighbor point
	currentNeighbor = currentPoint + neigh[n];
	// skip invalid neighbors (pixel outside of image)
	if (invalidNeighbor(currentPoint,currentNeighbor)) continue;
	// already processed and not MASKed?
	if (result.at(currentNeighbor) >= WSHED) {
	  distance.at(currentPoint) = 1;
	  fifoQueue.push(currentPoint);
	  // point is supposed to be pushed only once, so leave
	  // neighborhood process
	  break;
	}
      }
    }
  }
    
  void
  watershedSegmentation::assignCurrLevelPoints(vector<int>& distance,
                                               std::queue<int>& fifoQueue,
                                               matrix<int>& result) const {
    static const int WSHED =  0;
    static const int MASK  = -2;
    static const int fictitiousPixel = -1; // used as a queue marker
    int currentDistance = 1;
    int currentNeighbor,currentPoint;

    fifoQueue.push(fictitiousPixel);

    // process fifo queue
    while (true) {
      currentPoint = fifoQueue.front(); // get first pixel
      fifoQueue.pop(); // remove it

      if (currentPoint == fictitiousPixel) { // marker found

	// all points have been processed and MASKed or declared as
	// watersheds
	if (fifoQueue.empty()) // leave while-loop
	  break;

	// else ...

	fifoQueue.push(fictitiousPixel);
	currentDistance++;
	currentPoint = fifoQueue.front();// get next pixel
	fifoQueue.pop(); //remove it
      }

      int n;
      for(n=0;n<neigh.size();n++) {
	currentNeighbor = currentPoint + neigh[n];

	// skip invalid neighbors (pixel outside of image)
	if (invalidNeighbor(currentPoint,currentNeighbor)) continue;

	// neighbor belongs to an already labeled basin or to the watersheds
	if ((distance.at(currentNeighbor) < currentDistance) &&
	    (result.at(currentNeighbor) >= WSHED)) {

	  if (result.at(currentNeighbor) > 0) { // neighbor already labeled
	    if ((result.at(currentPoint) == MASK) ||
		(result.at(currentPoint) == WSHED)) {
	      result.at(currentPoint) = result.at(currentNeighbor);
	    }
	    else if (result.at(currentPoint) != result.at(currentNeighbor)) {
              // different basin in the neighborhood => watershed found
              result.at(currentPoint) = WSHED;
	    }
	  }
	  else if (result.at(currentPoint) == MASK)
	    result.at(currentPoint) = WSHED;
	}
	else if ((result.at(currentNeighbor) == MASK) &&
		 (distance.at(currentNeighbor) == 0)) {
	  distance.at(currentNeighbor) = currentDistance + 1;
	  fifoQueue.push(currentNeighbor);
	}
      }//for
    }//while
  }

  // check if new minima have been discovered
  // process all pixels of the current gray value again
  // to avoid alloc of a new queue: fifoQueue is taken from parent-methode
  void 
  watershedSegmentation::checkForMins(const list_type& currentPointList,
                                      vector<int>& distance,
                                      std::queue<int>& fifoQueue,
                                      matrix<int>& result,
                                      int& currentLabel) const {
    static const int MASK = -2;
    list_type::const_iterator pointIterator = currentPointList.begin();
    while (pointIterator != currentPointList.end()) {
      const int currentPoint = *pointIterator;
      pointIterator++;

      // distance is reset to 0
      distance.at(currentPoint) = 0;

      // new basin found
      if (result.at(currentPoint) == MASK) {
	currentLabel++;
	fifoQueue.push(currentPoint);
	result.at(currentPoint) = currentLabel;

	while (!fifoQueue.empty()) {
	  // get next pixel
	  const int nextPoint = fifoQueue.front();
	  fifoQueue.pop();

	  // scan neighborhood
          int n;
	  for(n=0;n<neigh.size();n++) {
	    // calculate absolute neighbor point
	    const int currentNeighbor = nextPoint + neigh[n];
	    // skip invalid neighbors (pixel outside of image)
	    if (invalidNeighbor(nextPoint,currentNeighbor)) continue;
	    // expand basin
	    if (result.at(currentNeighbor) == MASK) {
	      fifoQueue.push(currentNeighbor);
	      result.at(currentNeighbor) = currentLabel;
	    }
	  }
	}
      }
    }
  }

  void watershedSegmentation::raiseWaterLevel(
                                    const std::vector<list_type>& sortedPoints,
                                    matrix<int>& result) const {
    std::queue<int> fifoQueue;
    vector<int> distance(imgSize, 0);

    int currentLabel = 0;
    int currentLevel = 0;

    // check for minimum value used
    currentLabel = 0;
    while (currentLabel<256 && (sortedPoints[currentLabel].empty())) {
      ++currentLabel;
    }
    const int hmin = currentLabel;

    // check for maximum value used
    currentLabel = 255;
    while (currentLabel>hmin && (sortedPoints[currentLabel].empty())) {
      --currentLabel;
    }
    const int hmax = currentLabel;

    // raise water level
    for (currentLevel = hmin; currentLevel <= hmax; currentLevel++) {

      maskCurrLevelPoints  (sortedPoints[currentLevel], distance,
                            fifoQueue, result);
      assignCurrLevelPoints(                            distance, 
                                                        fifoQueue, result);
      checkForMins         (sortedPoints[currentLevel], distance,
                            fifoQueue, result, currentLabel);

#    if defined(_LTI_DEBUG) && (_LTI_DEBUG >= 2)
      static viewer view("Flooding",2,0,1,false);
      view.show(result);
#     if (_LTI_DEBUG >= 3)
      view.waitKey();
#     endif
#    endif
    }
  }

  /** convert the result matrix to appropriate channel8  */
  void watershedSegmentation::copyMatrixToChannel8(const matrix<int>& src,
                                                   channel8& dest) {
    int WSHED =  0; // watersheds have value 0

    // get colors for resulting channel8
    int watershedColor = getParameters().watershedValue;
    int basinColor = getParameters().basinValue;

    // 4-neighborhood is sufficient here, so the watersheds won't get too thick
    // only for visualisation (calculation by user-parameter)
    createNeighborhood(src.columns(),false);

    dest.resize(src.size(),0,false,false);

    int currentPoint;
    for (currentPoint = 0; currentPoint < imgSize; currentPoint++) {
      int currentValue = src.at(currentPoint);

      if (currentValue == WSHED) // watershed
	dest.at(currentPoint) = watershedColor;
      else {                     // point is labeled
	// assume point is in basin
	dest.at(currentPoint) = basinColor;
	// check for adjacent basins
        int n;
	for(n=0;n<neigh.size();n++) {
	  int currentNeighbor = currentPoint + neigh[n];
	  // skip invalid neighbors (pixel outside of image)
	  if (invalidNeighbor(currentPoint,currentNeighbor)) continue;
	  if (currentValue != src.at(currentNeighbor)) {
	    // different basin bordering => declare point as watershed
	    dest.at(currentPoint) = watershedColor;
	    break; // next currentPoint
	  }
	}
      }
    }
  }

  //----------------------------------------------------------------
  //                       RAINFALLING-WATERSHED
  //----------------------------------------------------------------

  // create regions (numbers by "counter") which are lokal minima(s)
  // rainfalling-method
  void watershedSegmentation::markMinimas(const matrix<int>& downPos,
                                          const channel8& src,
                                          matrix<int>& result) {
    // smelt points of same level and give it a number(counter)
    int* tempRegion = new int[imgSize];
    const int MASK = -2;
    int countF, countB ;
    int counter = 1; // number of region
    int tempi,tempiNeigh;
    int n,i;
    for(i=0;i<imgSize;i++) {
      if (result.at(i)==-1 && downPos.at(i)<0) { //unused & minima
        // i is point of a new found region
        countF = countB = 0;
        tempRegion[countB++] = i; // put point in queue
        int tempLevel = src.at(i);
        // find all points which have the same tempLevel like point i
        // and mark them with counter
        while(countF<countB) {
          tempi = tempRegion[countF++]; //get point from queue
          result.at(tempi) = counter; // label point
          for(n=0; n<neigh.size(); n++) {
            tempiNeigh = tempi + neigh[n];
            //tempiNeigh a valid image point
            if( validNeighbor(tempi,tempiNeigh) && 
                result.at(tempiNeigh)==-1 && // unused
                src.at(tempiNeigh)==tempLevel ) { //same level
              //each point only once in queue
              tempRegion[countB++] = tempiNeigh; // put point in queue
              result.at(tempiNeigh) = MASK; // prevent double in queue
            }
          }
        }
        counter++;
      }
    }
    
    delete[] tempRegion;

    _lti_debug("Found lakes/lokalMins: "<< counter << std::endl);

  }
  
  // rainfalling-method
  void  watershedSegmentation::letsRain(const matrix<int>& downPos,
                                        matrix<int>& result) {
    int i,c,tempi;
    vector<int> tempRegion(false,imgSize);  // uninitialized vector 

    int regionC;
    for(i=0;i<imgSize;i++) {
      regionC = 0;
      tempi = i;
      while(result.at(tempi) == -1) { // unassigned pixel
        tempRegion[regionC++] = tempi;
        tempi = downPos.at(tempi);
      }
      // a way found down to a lokalMin(lake/point)
      // set all points belong to the way down := tempi,
      // which is the counterNumber of the lokalMin
      int numOfLokalMin = result.at(tempi);
      for(c=0; c<regionC; c++) {
        result.at(tempRegion[c]) = numOfLokalMin;
      }
    }
  }

  // rainfalling-method
  void watershedSegmentation::findLowerNeigh(const channel8& src,
                                             matrix<int>& downPos,
                                             channel8& tSrc) {
    static const int lokalMin = -1;
    static const int saddle = -2;
    downPos.resize(src.size(),lokalMin,false,true);
    int i,n,max,pos,diff;
    for(i=0;i<imgSize;i++) {
      max = -1;
      for(n=0; n<neigh.size(); n++) {
        pos = i + neigh[n];
        if (invalidNeighbor(i,pos)) continue;
        diff = src.at(i)-src.at(pos);
        if(diff > max) { // neigh with lower level
          max = diff;
          downPos.at(i) = pos;
        }
      }
      if( max == 0 ) { // all neighours are on the same level = saddlePoint
        downPos.at(i) = saddle;
      }
    }
    // try if a saddlePoint have lower "neigh"
    bool change = true;
    while (change) {

      _lti_debug("saddle" << std::endl);

      change = false;
      for(i=0;i<imgSize;i++) {
        if(downPos.at(i) == saddle) {
          for(n=0; n<neigh.size(); n++) {
            pos = i + neigh[n];
            if (invalidNeighbor(i,pos)) continue;
            if(src.at(i)==src.at(pos) && downPos.at(pos) >= 0) { 
              // no more saddle, no lokalMin
              downPos.at(i) = downPos.at(pos);
              change = true;
              break; // next i
            }
          }
        }
      }
    }

    // remaining saddle points must be lokalMins
    // and all points<threshold are lokalMins
    int threshold = getParameters().threshold;
    tSrc.copy(src);
    for(i=0;i<imgSize;i++) {
      if (tSrc.at(i) < threshold) {
        downPos.at(i) = lokalMin;
        tSrc.at(i) = threshold;
      }
      else if(downPos.at(i) == saddle)
        downPos.at(i) = lokalMin;
    }
  }


  //-------------------------------------------------------------------
  //                  The apply-methods!
  //-------------------------------------------------------------------

  // On place apply for type channel8!
  bool watershedSegmentation::apply(channel8& srcdest) {
    return apply(srcdest, srcdest);
  };

  bool watershedSegmentation::apply(const channel8& src, channel8& dest) {
    matrix<int> result;
    if (apply(src, result)) {
      copyMatrixToChannel8(result, dest);//prepare result (show border)
      return true;
    }
    return false;    
  }

  bool watershedSegmentation::apply(const channel8& src, matrix<int> &result) {

    // the algorithm assumes a connected input image!
    if (src.getMode() != channel8::Connected) {
      // so, not a connected input channel => create a connected one
      const channel8 tmp(src);  
      return apply(src,result);
    }

    // compute offsets (4 or 8 dimensional vector)
    createNeighborhood(src.columns(), getParameters().neighborhood8);
    initBorderLUT(src.size(),borderLUT);
    
    // number of pixels of the image
    imgSize = src.columns() * src.rows();
    
    result.resize(src.size(),-1,false,true); // -1 = unused

    if( getParameters().rainfall ) { // rainfalling
      /*
       * according to idea of the rainfallingWatersheds from
       * P. De Smet and Rui Luis V.P.M.Pires
       * http://telin.rug.ac.be/ipi/watershed
       */
      matrix<int> downPos;
      channel8 tSrc;
      findLowerNeigh(src, downPos,tSrc);
      markMinimas(downPos, tSrc, result);
      letsRain(downPos, result);
    }
    else { // standard
      /*
       * according to pseudo code provided in "vincent and soille -
       * watersheds in digital spaces" IEEE Vol.13, No.6, p. 583f
       */
      std::vector<list_type> sortedPoints;
      sortPixels(src, sortedPoints);
      raiseWaterLevel(sortedPoints, result);
      // clear
      for (int waterLevel = 0; waterLevel < 256; waterLevel++) {
	sortedPoints[waterLevel].clear();
      }
      sortedPoints.clear();
    }

    return true;
  }

}
