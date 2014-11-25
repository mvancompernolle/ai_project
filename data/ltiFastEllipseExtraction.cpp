/*
 * Copyright (C) 2004, 2005, 2006
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
 * file .......: ltiFastEllipseExtraction.cpp
 * authors ....: Ingo Grothues (ingo@isdevelop.de)
 * organization: LTI, RWTH Aachen
 * creation ...: 23.4.2004
 * revisions ..: $Id: ltiFastEllipseExtraction.cpp,v 1.9 2006/09/05 17:02:10 ltilib Exp $
 */

#include "ltiFastEllipseExtraction.h"

#include "ltiCholeskyDecomposition.h"
#include "ltiMatrixInversion.h"
#include "ltiEigenSystem.h"

// number of pre-allocated vector elements
const int lti::fastEllipseExtraction::segmPreallocation = 5000;
const int lti::fastEllipseExtraction::linePreallocation = 1000;
const int lti::fastEllipseExtraction::ellArcPreallocation = 50;
const int lti::fastEllipseExtraction::ellExtArcPreallocation = 20;
const int lti::fastEllipseExtraction::ellipsePreallocation = 10;
const double lti::fastEllipseExtraction::PI = 3.14159265358979323846264338327;
const double lti::fastEllipseExtraction::INFINITE_SLOPE = 999999999.0;

namespace lti {
  // --------------------------------------------------
  // fastEllipseExtraction::parameters
  // --------------------------------------------------

  // default constructor
  fastEllipseExtraction::parameters::parameters()
    : featureExtractor::parameters() {

    minSegmLen = 2;
    minLineLen = 6;
    segmentTolerance = 0;
    maxSegmentGap = 1;
    maxQuantizationError = 0.74f;
    maxLineGap = 3;
    maxArcGap = 25;
    maxLineTangentError = 14.0f;
    maxArcTangentError = 18.0f;
    
    maxExtArcMismatch = 0.3f;
    maxCenterMismatch = 5.0f;
    minRadiusMatchRatio = 0.8f;
    minCoverage = 0.25f;
    
    maxArcDistanceAngle = 16.0f;
    minArcDistanceRatio = 0.56f;
    minGAPangleDistance = 2;
    maxGAPangle = 30.0f;
    maxArcOverlapGap = 1.0f;

    maxIntAngleMismatches = 1;
    maxTangentErrors = 1;
    maxLBcenterMismatch = 4.0f;
    
    ellExtArcExtractionStage = 3;
  }

  // copy constructor
  fastEllipseExtraction::parameters::parameters(const parameters& other)
    : featureExtractor::parameters() {
    copy(other);
  }

  // destructor
  fastEllipseExtraction::parameters::~parameters() {
  }

  // get type name
  const char* fastEllipseExtraction::parameters::getTypeName() const {
    return "fastEllipseExtraction::parameters";
  }

  // copy member

  fastEllipseExtraction::parameters&
    fastEllipseExtraction::parameters::copy(const parameters& other) {
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

    minSegmLen = other.minSegmLen;
    minLineLen = other.minLineLen;
    segmentTolerance = other.segmentTolerance;
    maxSegmentGap = other.maxSegmentGap;
    maxQuantizationError = other.maxQuantizationError;

    maxLineGap = other.maxLineGap;
    maxArcGap = other.maxArcGap;
    maxLineTangentError = other.maxLineTangentError;
    maxArcTangentError = other.maxArcTangentError;

    maxExtArcMismatch = other.maxExtArcMismatch;
    maxCenterMismatch = other.maxCenterMismatch;
    minRadiusMatchRatio = other.minRadiusMatchRatio;
    minCoverage = other.minCoverage;

    maxIntAngleMismatches = other.maxIntAngleMismatches;
    minGAPangleDistance = other.minGAPangleDistance;
    maxGAPangle = other.maxGAPangle;
    maxTangentErrors = other.maxTangentErrors;
    maxLBcenterMismatch = other.maxLBcenterMismatch;
    maxArcOverlapGap = other.maxArcOverlapGap;
    
    maxArcDistanceAngle = other.maxArcDistanceAngle;
    minArcDistanceRatio = other.minArcDistanceRatio;
    
    ellExtArcExtractionStage = other.ellExtArcExtractionStage;

    return *this;
  }

  // alias for copy member
  fastEllipseExtraction::parameters&
    fastEllipseExtraction::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fastEllipseExtraction::parameters::clone() const {
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
  bool fastEllipseExtraction::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool fastEllipseExtraction::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"minSegmLen",minSegmLen);
      lti::write(handler,"minLineLen",minLineLen);
      lti::write(handler,"segmentTolerance",segmentTolerance);
      lti::write(handler,"maxSegmentGap",maxSegmentGap);
      lti::write(handler,"maxQuantizationError",maxQuantizationError);
      
      lti::write(handler,"maxLineGap",maxLineGap);
      lti::write(handler,"maxArcGap",maxArcGap);
      lti::write(handler,"maxLineTangentError",maxLineTangentError);
      lti::write(handler,"maxArcTangentError",maxArcTangentError);

      lti::write(handler,"maxExtArcMismatch",maxExtArcMismatch);
      lti::write(handler,"maxCenterMismatch",maxCenterMismatch);
      lti::write(handler,"minRadiusMatchRatio",minRadiusMatchRatio);
      lti::write(handler,"minCoverage",minCoverage);
            
      lti::write(handler, "maxIntAngleMismatches", maxIntAngleMismatches);
      lti::write(handler, "minGAPangleDistance", minGAPangleDistance);
      lti::write(handler, "maxGAPangle", maxGAPangle);
      lti::write(handler, "maxTangentErrors", maxTangentErrors);
      lti::write(handler, "maxLBcenterMismatch", maxLBcenterMismatch);
      lti::write(handler, "maxArcOverlapGap", maxArcOverlapGap);
      
      lti::write(handler, "maxArcDistanceAngle", maxArcDistanceAngle);
      lti::write(handler, "minArcDistanceRatio", minArcDistanceRatio);
      
      lti::write(handler, "ellExtArcExtractionStage", ellExtArcExtractionStage);
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
  bool fastEllipseExtraction::parameters::write(ioHandler& handler,
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
  bool fastEllipseExtraction::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool fastEllipseExtraction::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"minSegmLen",minSegmLen);
      lti::read(handler,"minLineLen",minLineLen);
      lti::read(handler,"segmentTolerance",segmentTolerance);
      lti::read(handler,"maxSegmentGap",maxSegmentGap);
      lti::read(handler,"maxQuantizationError",maxQuantizationError);
      
      lti::read(handler,"maxLineGap",maxLineGap);
      lti::read(handler,"maxArcGap",maxArcGap);
      lti::read(handler,"maxLineTangentError",maxLineTangentError);
      lti::read(handler,"maxArcTangentError",maxArcTangentError);

      lti::read(handler,"maxExtArcMismatch",maxExtArcMismatch);
      lti::read(handler,"maxCenterMismatch",maxCenterMismatch);
      lti::read(handler,"minRadiusMatchRatio",minRadiusMatchRatio);
      lti::read(handler,"minCoverage",minCoverage);
      
      lti::read(handler, "maxIntAngleMismatches", maxIntAngleMismatches);
      lti::read(handler, "minGAPangleDistance", minGAPangleDistance);
      lti::read(handler, "maxGAPangle", maxGAPangle);
      lti::read(handler, "maxTangentErrors", maxTangentErrors);
      lti::read(handler, "maxLBcenterMismatch", maxLBcenterMismatch);
      lti::read(handler, "maxArcOverlapGap", maxArcOverlapGap);
      
      lti::read(handler, "maxArcDistanceAngle", maxArcDistanceAngle);
      lti::read(handler, "minArcDistanceRatio", minArcDistanceRatio);

      lti::read(handler, "ellExtArcExtractionStage", ellExtArcExtractionStage);
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
  bool fastEllipseExtraction::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // fastEllipseExtraction
  // --------------------------------------------------

  // default constructor
  fastEllipseExtraction::fastEllipseExtraction()
    : featureExtractor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
    // default initialization
    imageWidth = 0;
  }

  // default constructor
  fastEllipseExtraction::fastEllipseExtraction(const parameters& par)
    : featureExtractor() {

    // set the given parameters
    setParameters(par);
    // default initialization
    imageWidth = 0;
  }


  // copy constructor
  fastEllipseExtraction::fastEllipseExtraction(const fastEllipseExtraction& other)
    : featureExtractor() {
    copy(other);
  }

  // destructor
  fastEllipseExtraction::~fastEllipseExtraction() {
    for(int i=0; i<5; i++) {
      vecSegments[i].clear();
      clearLineList(i);
      clearEllArcList(i);
      clearEllExtArcList(i);
    }
    clearEllipseList();   
  }

  // returns the name of this type
  const char* fastEllipseExtraction::getTypeName() const {
    return "fastEllipseExtraction";
  }

  // copy member
  fastEllipseExtraction&
    fastEllipseExtraction::copy(const fastEllipseExtraction& other) {

    featureExtractor::copy(other);
    int i;
    for(i=0; i<5; i++) {
      vecLines[i] = other.vecLines[i];
      vecSegments[i] = other.vecSegments[i];
      // copy segmIdxLists
      const int nLines = static_cast<int>(vecLines[i].size());
      int j;
      for(j=0; j<nLines; j++) {
        vecLines[i][j].segmIdxList = new std::vector<int>(*other.vecLines[i][j].segmIdxList);
      }
    }

    for(i=0; i<9; i++) {
      vecEllArcs[i] = other.vecEllArcs[i];
      vecEllExtArcs[i] = other.vecEllExtArcs[i];
      // copy lineIdxLists
      const int nArcs = static_cast<int>(vecEllArcs[i].size());
      int j;
      for(j=0; j<nArcs; j++) {
        vecEllArcs[i][j].lineIdxList = new std::vector<int>(*other.vecEllArcs[i][j].lineIdxList);
      }
    }

    vecEllipses = other.vecEllipses;
    // copy arcIdxLists
    const int nEllipses = static_cast<int>(vecEllipses.size());
    for(i=0; i<nEllipses; i++) {
      vecEllipses[i].mergedArcs = new std::vector<int>(*other.vecEllipses[i].mergedArcs);
    }

    imageWidth = other.imageWidth;
    return (*this);
  }

  // alias for copy member
  fastEllipseExtraction&
    fastEllipseExtraction::operator=(const fastEllipseExtraction& other) {
    return (copy(other));
  }


  // clone member
  functor* fastEllipseExtraction::clone() const {
    return new fastEllipseExtraction(*this);
  }

  // return parameters
  const fastEllipseExtraction::parameters&
    fastEllipseExtraction::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }


////////////////////////////////////////////////////////////


  // returns vector with line segments
  std::vector<fastEllipseExtraction::segmEntry>& fastEllipseExtraction::getSegmentList(const int groupNumber) {
    if((groupNumber < 0)||(groupNumber > 4)) {
      return vecSegments[0];
    }
    return vecSegments[groupNumber];
  }

  // returns vector with lines
  std::vector<fastEllipseExtraction::lineEntry>& fastEllipseExtraction::getLineList(const int groupNumber) {
    if((groupNumber < 0)||(groupNumber > 4)) {
      return vecLines[0];
    }
    return vecLines[groupNumber];
  }

  // returns vector with arcs
  std::vector<fastEllipseExtraction::ellArcEntry>& fastEllipseExtraction::getEllArcList(const int groupNumber) {
    if((groupNumber < 0)||(groupNumber > 8)) {
      return vecEllArcs[0];
    }
    return vecEllArcs[groupNumber];
  }  

  // returns vector with extended arcs
  std::vector<fastEllipseExtraction::ellExtArcEntry>& fastEllipseExtraction::getEllExtArcList(const int groupNumber) {
    if((groupNumber < 0)||(groupNumber > 8)) {
      return vecEllExtArcs[0];
    }
    return vecEllExtArcs[groupNumber];
  }  

  // returns vector with ellipses
  std::vector<fastEllipseExtraction::ellipseEntry>& fastEllipseExtraction::getEllipseList() {
    return vecEllipses;
  }  

  // clears line lists
  void fastEllipseExtraction::clearLineList(const int groupNumber) {
      if(groupNumber > 0) {  // don't delete shared list[0] idx_vectors
        const int n = static_cast<int>(vecLines[groupNumber].size());
        for(int j=0; j<n; j++) {
          (*vecLines[groupNumber][j].segmIdxList).clear();
          delete vecLines[groupNumber][j].segmIdxList;
        }
      }
      vecLines[groupNumber].clear(); 
  }

  // clears arc lists
  void fastEllipseExtraction::clearEllArcList(const int groupNumber) {
      if(groupNumber > 0) {  // don't delete shared list[0] idx_vectors
        const int n = static_cast<int>(vecEllArcs[groupNumber].size());
        for(int j=0; j<n; j++) {
          (*vecEllArcs[groupNumber][j].lineIdxList).clear();
          delete vecEllArcs[groupNumber][j].lineIdxList;
        }
      }
      vecEllArcs[groupNumber].clear(); 
  }

  // clears extended arc lists
  void fastEllipseExtraction::clearEllExtArcList(const int groupNumber) {
      vecEllExtArcs[groupNumber].clear(); 
  }

  // clears the ellipse list
  void fastEllipseExtraction::clearEllipseList() {
      const int n = static_cast<int>(vecEllipses.size());
      for(int j=0; j<n; j++) {
        (*vecEllipses[j].mergedArcs).clear();
        delete vecEllipses[j].mergedArcs;
      }
      vecEllipses.clear(); 
  }

///////////////////////////////////////////////////////////////////////////////

  // creates segment lists
  void fastEllipseExtraction::detectGroup1Segments(const channel8& src) {
    
    // get parameters
    const parameters& par = getParameters();

    // get size of the input image
    const int rows = src.rows();
    const int cols = src.columns();
    imageWidth = cols;
    
    // calculate max ranges (horizontal lines)
    const int lineCount = rows;
    const int lineLength = cols;

    // clear old list and reserve memory
    vecSegments[1].clear(); 
    vecSegments[1].reserve(segmPreallocation);

    // create new list  
    for(int a=0; a<lineCount; a++) { // line scan

      const vector<ubyte> line = src.getRow(a);   // copy actual line (horizontal)

      int   prevVal = 0;   // value of the previous pixel
      int   start = 0;     // start of the actual segment
      int   len = 0;       // length of the actual segment
      int   startX = 0;    // starting x-coordinate
      int   startY = 0;    // starting y-coordinate

      for(int b=0; b<lineLength+1; b++) { // pixel scan

        ubyte val; 
        if(b < lineLength) val = line[b];  // actual pixel
        else val = 0;  // extra loop for lines ending at the border

        // calculate coordinates
        const int x = b;
        const int y = a;
        
        if(val > 0) {

          // segment detection
          if(len == 0) {  // start of segment
            start = b;
            startX = x;
            startY = y;
            len++;
          }
          else {          // part or end of segment

            if((val >= (prevVal - par.segmentTolerance))    // check value of this pixel
             &&(val <= (prevVal + par.segmentTolerance))) { // if in tolerance range -> segment part
              len++;
            }
            else {       // otherwise -> segment end

              if(len >= par.minSegmLen) {
                // add segment to group list
                segmEntry newSegment;
                newSegment.len = len;
                newSegment.used = 0;
                newSegment.start.x = start;
                newSegment.start.y = y;
                newSegment.end.x = b-1;
                newSegment.end.y = y;
                vecSegments[1].push_back(newSegment);
              }
              len = 0;
            }
          }
        }
        else {  // val == 0 

          if(len >= par.minSegmLen) {
            // add segment to group list
            segmEntry newSegment;
            newSegment.len = len;
            newSegment.used = 0;            
            newSegment.start.x = start;
            newSegment.start.y = y;
            newSegment.end.x = b-1;
            newSegment.end.y = y;
            vecSegments[1].push_back(newSegment); 
          }
          len = 0;
        }
        
        prevVal = val;
      } // pixel scan
    } // line scan
  }

///////////////////////////////////////////////////////////////////////////////

  // creates segment lists
  void fastEllipseExtraction::detectGroup2Segments(const channel8& src) {

    // get parameters
    const parameters& par = getParameters();

    // get size of the input image
    const int rows = src.rows();
    const int cols = src.columns();
    imageWidth = cols;
    
    // calculate max ranges (vertical)
    const int lineCount = cols;
    const int lineLength = rows;

    // clear old list and reserve memory
    vecSegments[2].clear(); 
    vecSegments[2].reserve(segmPreallocation);

    // create new list  
    for(int a=0; a<lineCount; a++) { // line scan
    
      const vector<ubyte> line = src.getColumnCopy(a);  // copy actual line (vertical)

      int   prevVal = 0;   // value of the previous pixel
      int   start = 0;     // start of the actual segment
      int   len = 0;       // length of the actual segment
      int   startX = 0;    // starting x-coordinate
      int   startY = 0;    // starting y-coordinate

      for(int b=0; b<lineLength+1; b++) { // pixel scan
      
        ubyte val; 
        if(b < lineLength) val = line[b];  // actual pixel
        else val = 0;  // extra loop for lines ending at the border

        // calculate coordinates
        const int x = a;
        const int y = b;
        
        if(val > 0) {

          // segment detection
          if(len == 0) {  // start of segment
          
            start = b;
            startX = x;
            startY = y;
            len++;
          }
          else {          // part or end of segment
          
            if((val >= (prevVal - par.segmentTolerance))    // check value of this pixel
             &&(val <= (prevVal + par.segmentTolerance))) { // if in tolerance range -> segment part
              len++;
            }
            else        // otherwise -> segment end
            {
              if(len >= par.minSegmLen) {
                // add segment to group list
                segmEntry newSegment;
                newSegment.len = len;
                newSegment.used = 0;
                newSegment.start.x = x;
                newSegment.start.y = start;
                newSegment.end.x = x;
                newSegment.end.y = b-1;
                vecSegments[2].push_back(newSegment); 
              }
              len = 0;
            }
          }
        }
        else {  // val == 0 
        
          if(len >= par.minSegmLen) {
            // add segment to group list
            segmEntry newSegment;
            newSegment.len = len;
            newSegment.used = 0;
            newSegment.start.x = x;
            newSegment.start.y = start;
            newSegment.end.x = x;
            newSegment.end.y = b-1;
            vecSegments[2].push_back(newSegment); 
          }
          len = 0;
        }
        
        prevVal = val;
      } // pixel scan
    } // line scan
  }

///////////////////////////////////////////////////////////////////////////////

  // creates segment lists
  void fastEllipseExtraction::detectGroup3Segments(const channel8& src) {
  
    // get parameters
    const parameters& par = getParameters();

    // get size of the input image
    const int rows = src.rows();
    const int cols = src.columns();
    imageWidth = cols;
    
    // maximum values
    const int maxDiag   = (rows < cols) ? rows : cols;
    
    // calculate max ranges (diagonal)
    const int lineCount = rows+cols-1;  // scan from upper right over upper left to lower left corner
    int lineLength = 0;                 // line length depends on actual line scan position

    // clear old list and reserve memory
    vecSegments[3].clear(); 
    vecSegments[3].reserve(segmPreallocation);

    // create new list  
    for(int a=0; a<lineCount; a++) { // line scan
    
      // copy actual line (diagonal horizontal)
      vector<ubyte> line;
      
      if(a < cols) { // scan from upper_right to upper_left corner
      
        if(lineLength < maxDiag) lineLength++;     // set line length
        line.resize(lineLength, 0, false, false);  // resize vector

        for(int i=0; i<lineLength; i++) {          // fill vector
          line[i] = src.at(i, cols-1-a+i);         // copy diagonal right down
        }
      }
      else {  // scan from upper_left to lower_left corner
      
        // check if line length starts decreasing
        if((lineCount - a) < maxDiag) lineLength--; // set line length
        line.resize(lineLength, 0, false, false);   // resize vector

        for(int i=0; i<lineLength; i++) {           // fill vector
          line[i] = src.at(a-(cols-1)+i, i);        // copy diagonal right down
        }
      }

      int   prevVal = 0;   // value of the previous pixel
      int   start = 0;     // start of the actual segment
      int   len = 0;       // length of the actual segment
      int   startX = 0;    // starting x-coordinate
      int   startY = 0;    // starting y-coordinate

      for(int b=0; b<lineLength+1; b++) { // pixel scan
      
        ubyte val; 
        if(b < lineLength) val = line[b];  // actual pixel
        else val = 0;  // extra loop for lines ending at the border

        // calculate coordinates
        int x,y;
        if(a < cols) {
          x = cols-1-a+b;         // scan from upper right to upper left corner
          y = b;
        }
        else {                    // scan from upper left to lower left corner
          x = b;
          y = a-(cols-1)+b;
        }
        
        if(val > 0) {

          // segment detection
          if(len == 0) {  // start of segment
            start = b;
            startX = x;
            startY = y;
            len++;
          }
          else {          // part or end of segment

            if((val >= (prevVal - par.segmentTolerance))    // check value of this pixel
             &&(val <= (prevVal + par.segmentTolerance))) { // if in tolerance range -> segment part
              len++;
            }
            else {       // otherwise -> segment end

              if(len >= par.minSegmLen) {
                // add segment to group list
                segmEntry newSegment;
                newSegment.len = len;
                newSegment.used = 0;
                newSegment.start.x = startX;
                newSegment.start.y = startY;
                newSegment.end.x = x-1;
                newSegment.end.y = y-1;
                vecSegments[3].push_back(newSegment); 
              }
              len = 0;
            }
          }
        }
        else {  // val == 0 
        
          if(len >= par.minSegmLen) {
            // add segment to group list
            segmEntry newSegment;
            newSegment.len = len;
            newSegment.used = 0;
            newSegment.start.x = startX;
            newSegment.start.y = startY;
            newSegment.end.x = x-1;
            newSegment.end.y = y-1;
            vecSegments[3].push_back(newSegment); 
          }
          len = 0;
        }
        
        prevVal = val;
      } // pixel scan
    } // line scan
  }

///////////////////////////////////////////////////////////////////////////////

  // creates segment lists
  void fastEllipseExtraction::detectGroup4Segments(const channel8& src) {
  
    // get parameters
    const parameters& par = getParameters();

    // get size of the input image
    const int rows = src.rows();
    const int cols = src.columns();
    imageWidth = cols;
    
    // maximum values
    const int maxDiag   = (rows < cols) ? rows : cols;
    
    // calculate max ranges (diagonal)
    const int lineCount = rows+cols-1;  // scan from lower right over lower left to upper left corner
    int lineLength = 0;                 // line length depends on actual line scan position

    // clear old list and reserve memory
    vecSegments[4].clear(); 
    vecSegments[4].reserve(segmPreallocation);

    // create new list  
    for(int a=0; a<lineCount; a++) { // line scan

      // copy actual line (diagonal vertical)
      vector<ubyte> line;

      if(a < cols) { // scan from lower_right to lower_left corner

        if(lineLength < maxDiag) lineLength++;     // set line length
        line.resize(lineLength, 0, false, false);  // resize vector

        for(int i=0; i<lineLength; i++) {          // fill vector
          line[i] = src.at(rows-1-i, cols-1-a+i);  // copy diagonal right up
        }
      }
      else {  // scan from lower_left to upper_left corner
      
        // check if line length starts decreasing
        if((lineCount - a) < maxDiag) lineLength--; // set line length
        line.resize(lineLength, 0, false, false);   // resize vector
        
        for(int i=0; i<lineLength; i++) {                // fill vector
          line[i] = src.at((rows-1)-(a-(cols-1))-i, i);  // copy diagonal right up
        }
      }

      int   prevVal = 0;   // value of the previous pixel
      int   start = 0;     // start of the actual segment
      int   len = 0;       // length of the actual segment
      int   startX = 0;    // starting x-coordinate
      int   startY = 0;    // starting y-coordinate

      for(int b=0; b<lineLength+1; b++) { // pixel scan

        ubyte val; 
        if(b < lineLength) val = line[b];  // actual pixel
        else val = 0;  // extra loop for lines ending at the border

        // calculate coordinates (diagonal vertical)
        int x,y;
        if(a < cols) {            // scan from lower right to lower left corner
          x = cols-1-a+b;
          y = rows-1-b;
        }
        else {                    // scan from lower left to upper left corner
          x = b;
          y = (rows-1)-(a-(cols-1))-b;
        }
        
        
        if(val > 0) {

          // segment detection
          if(len == 0) {  // start of segment
            start = b;
            startX = x;
            startY = y;
            len++;
          }
          else {        // part or end of segment
            if((val >= (prevVal - par.segmentTolerance))    // check value of this pixel
             &&(val <= (prevVal + par.segmentTolerance))) { // if in tolerance range -> segment part
              len++;
            }
            else {      // otherwise -> segment end

              if(len >= par.minSegmLen) {
                // add segment to group list
                segmEntry newSegment;
                newSegment.len = len;
                newSegment.used = 0;
                newSegment.start.x = startX;
                newSegment.start.y = startY;
                newSegment.end.x = x-1;
                newSegment.end.y = y+1;
                vecSegments[4].push_back(newSegment); 
              }
              len = 0;
            }
          }
        }
        else {  // val == 0 
        
          if(len >= par.minSegmLen) {
            // add segment to group list
            segmEntry newSegment;
            newSegment.len = len;
            newSegment.used = 0;
            newSegment.start.x = startX;
            newSegment.start.y = startY;
            newSegment.end.x = x-1;
            newSegment.end.y = y+1;
            vecSegments[4].push_back(newSegment); 
          }
          len = 0;
        }
        
        prevVal = val;
      } // pixel scan
    } // line scan
  }


///////////////////////////////////////////////////////////////////////////////

  void fastEllipseExtraction::extractGroup1Lines() {
  
    // clear old complete line list and reserve memory
    vecLines[0].clear(); 
    vecLines[0].reserve(4*linePreallocation);

    // clear old linelist and reserve memory
    clearLineList(1); 
    vecLines[1].reserve(linePreallocation);
    
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(vecSegments[1].size());

    if(segmCount > 0) {

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        // create new segment grouping vector
        std::vector<int> *vecLineSegments = new std::vector<int>;

        bool positiveSlope = true;   // sign of the slope of the first segment pair
        bool firstCandidate = true;  // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments->push_back(Si);  // store Si in linelist (starting segment)
        SS = Si;                        // set Si as new target segment SS for search
        SL = Si;                        // set Si as actual longest linesegment SL
        Sq = Si + 1;                    // first candidate segment
        
        while(Sq < segmCount) {   // track line segments
          // STEP2
          // calculate window boundaries for target segment SS
          const segmEntry segSS = vecSegments[1][SS];
          const int GAPx = par.maxSegmentGap;
          const int GAPxPos = segSS.end.x + GAPx;
          const int GAPxNeg = segSS.start.x - GAPx;
          const float deltaY = -0.5f; //segSS.end.y - segSS.start.y - 0.5;
          const float deltaX = static_cast<float>(segSS.end.x - segSS.start.x);
          const float minSlope = deltaY / deltaX;
          const float GAPyFract = minSlope * (GAPxNeg - segSS.end.x);
          const float GAPyRound = floor(GAPyFract + 0.5f);
          const int GAPy = static_cast<int>(segSS.end.y + GAPyRound);
          
          // search new candidate segment Sq (from top to bottom)
          while(Sq < segmCount) {
            
            // no multi-use of segments in circle extraction
            if(vecSegments[1][Sq].used > 0) {
              Sq++;
              continue;
            }
            const segmEntry segSq = vecSegments[1][Sq];
            
            if(segSq.start.y > GAPy) { // window boundary reached ?
              Sq = segmCount;          // no candidate segment found
              break;
            }

            // positive slope or first candidate
            if(positiveSlope || firstCandidate) {
              if((segSq.start.x <= GAPxPos)                         // Sq starts in window 
              && (segSq.start.x >= segSS.end.x - segSS.len/2 + 1)) { // Sq starts right from center of SS
                positiveSlope = true;
                break;
              }
            }

            // negative slope or first candidate
            if(!positiveSlope || firstCandidate) {
              if((segSq.end.x >= GAPxNeg)                           // Sq ends in window 
              && (segSq.end.x <= segSS.start.x + segSS.len/2 - 1)) { // Sq ends left from center of SS
                positiveSlope = false;
                break;
              }
            }
            Sq++;
          }

          if(Sq == segmCount) break;    // no candidate segment found

          const segmEntry segSi = vecSegments[1][Si];
          const segmEntry segSq = vecSegments[1][Sq];

          // calculate longest line segment SL (old SL or new Sq)
          if(vecSegments[1][SL].len < segSq.len) {
            SL = Sq;
          }
          const segmEntry segSL = vecSegments[1][SL];
          
          // STEP3
          // calculate slope
          float slope;
          if(positiveSlope) {
            slope = ((float)(segSq.end.y - segSi.start.y)) / (segSq.end.x - segSi.start.x);
          }
          else {
            slope = ((float)(segSi.end.y - segSq.start.y)) / (segSi.end.x - segSq.start.x);
          }
          
          // STEP4
          // check if SL fits in estimated line
          //const float deltaD = segSL.start.y - (slope * segSL.start.x)
          //                   -(  segSL.end.y - (slope * segSL.end.x) );
          const float deltaD = slope * (segSL.end.x - segSL.start.x);

          // if not satisfied
          if((deltaD > par.maxQuantizationError) || (deltaD < -par.maxQuantizationError)) {
            if(firstCandidate) {
              positiveSlope = true;    // reset slope flag
            }
            break;   
          }
          else firstCandidate = false;

          // store Sq in linelist (actual finishing segment)
          vecLineSegments->push_back(Sq);
          
          // STEP5
          // set Sq as new target segment SS and increment candidate segment
          SS = Sq++;
        }

        // STEP7
        // calculate line length
        const int nSegments = static_cast<int>(vecLineSegments->size());
        int length = 0;

        if(positiveSlope) {
          const int x1 = vecSegments[1][(*vecLineSegments)[0]].start.x;
          const int x2 = vecSegments[1][(*vecLineSegments)[nSegments-1]].end.x;
          length = x2 - x1 + 1;
        }
        else {
          const int x1 = vecSegments[1][(*vecLineSegments)[nSegments-1]].start.x;
          const int x2 = vecSegments[1][(*vecLineSegments)[0]].end.x;
          length = x2 - x1 + 1;
        }

        // line length must be >= minLineLen for extraction
        if(length >= par.minLineLen) {
          
          // register grouped segments as extracted line
          lineEntry newline;
          if(positiveSlope) {
            newline.start.x = vecSegments[1][(*vecLineSegments)[0]].start.x;
            newline.start.y = vecSegments[1][(*vecLineSegments)[0]].start.y;
            newline.end.x = vecSegments[1][(*vecLineSegments)[nSegments-1]].end.x;
            newline.end.y = vecSegments[1][(*vecLineSegments)[nSegments-1]].end.y;
          }
          else {
            newline.start.x = vecSegments[1][(*vecLineSegments)[nSegments-1]].start.x;
            newline.start.y = vecSegments[1][(*vecLineSegments)[nSegments-1]].start.y;
            newline.end.x = vecSegments[1][(*vecLineSegments)[0]].end.x;
            newline.end.y = vecSegments[1][(*vecLineSegments)[0]].end.y;
          }
          // calculate midpoint
          newline.mid.x = (newline.start.x + newline.end.x)/2;
          newline.mid.y = (newline.start.y + newline.end.y)/2;
          // calculate tangent
          newline.tangent = atan2((double) -(newline.end.y-newline.start.y), (double)(newline.end.x-newline.start.x));
          //newline.tangent = atan((double) -(newline.end.y-newline.start.y) / (newline.end.x-newline.start.x));
          // set other values
          newline.len = length;
          newline.used = 0;
          newline.group = 1;
          // store data
          newline.segmIdxList = vecLineSegments;
          vecLines[1].push_back(newline);
          vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            vecSegments[1][(*vecLineSegments)[i]].used++;
          }
        }
        else delete vecLineSegments;

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(vecSegments[1][Si].used == 0) break;
        }
        
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////////////

  void fastEllipseExtraction::extractGroup2Lines() {

    // clear old linelist and reserve memory
    clearLineList(2);
    vecLines[2].reserve(linePreallocation);
  
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(vecSegments[2].size());

    if(segmCount > 0) {

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        // create new segment grouping vector
        std::vector<int> *vecLineSegments = new std::vector<int>;

        bool positiveSlope = true;   // sign of the slope of the first segment pair
        bool firstCandidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments->push_back(Si);  // store Si in linelist (starting segment)
        SS = Si;                        // set Si as new target segment SS for search
        SL = Si;                        // set Si as actual longest linesegment SL
        Sq = Si + 1;                    // first candidate segment


        while(Sq < segmCount) {   // track line segments
          // STEP2
          // calculate window boundaries for target segment SS
          const segmEntry segSS = vecSegments[2][SS];
          const int GAPx = par.maxSegmentGap;
          const int GAPxPos = segSS.end.y + GAPx;
          const int GAPxNeg = segSS.start.y - GAPx;
          const float deltaY = -0.5f; //segSS.end.x - segSS.start.x - 0.5;
          const float deltaX = static_cast<float>(segSS.end.y - segSS.start.y);
          const float minSlope = deltaY / deltaX;
          const float GAPyFract = minSlope * (GAPxNeg - segSS.end.y);
          const float GAPyRound = floor(GAPyFract + 0.5f);
          const int GAPy = static_cast<int>(segSS.end.x + GAPyRound);


          // search new candidate segment Sq (from left to right)
          while(Sq < segmCount) {
          
            // no multi-use of segments in circle extraction
            if(vecSegments[2][Sq].used > 0) {
              Sq++;
              continue;
            }
            const segmEntry segSq = vecSegments[2][Sq];

            if(segSq.start.x > GAPy) {  // window boundary reached ?
              Sq = segmCount;   // no candidate segment found
              break;
            }

            // positive slope or first candidate
            if(positiveSlope || firstCandidate) {
              if((segSq.start.y <= GAPxPos)                         // Sq starts in window 
              && (segSq.start.y >= segSS.end.y - segSS.len/2 + 1)) { // Sq starts below from center of SS
                  positiveSlope = true;
                  break;
              }
            }

            // negative slope or first candidate
            if(!positiveSlope || firstCandidate) {
              if((segSq.end.y >= GAPxNeg)                           // Sq ends in window 
              && (segSq.end.y <= segSS.start.y + segSS.len/2 - 1)) { // Sq ends above from center of SS
                  positiveSlope = false;
                  break;
              }
            }

            Sq++;
          }

          if(Sq == segmCount) break;    // no candidate segment found

          const segmEntry segSi = vecSegments[2][Si];
          const segmEntry segSq = vecSegments[2][Sq];

          // calculate longest line segment SL (old SL or new Sq)
          if(vecSegments[2][SL].len < segSq.len) {
            SL = Sq;
          }
          const segmEntry segSL = vecSegments[2][SL];

          // STEP3
          // calculate slope
          float slope;
          if(positiveSlope) {
            slope = ((float)(segSq.end.x - segSi.start.x)) / (segSq.end.y - segSi.start.y);
          }
          else {
            slope = ((float)(segSi.end.x - segSq.start.x)) / (segSi.end.y - segSq.start.y);
          }
          
          // STEP4
          // check if SL fits in estimated line
          //const float deltaD = segSL.start.x - (slope * segSL.start.y)
          //                   -(  segSL.end.x - (slope * segSL.end.y) );
          const float deltaD = slope * (segSL.end.y - segSL.start.y);
          
          // if not satisfied
          if((deltaD > par.maxQuantizationError) || (deltaD < -par.maxQuantizationError)) {
            if(firstCandidate) {
              positiveSlope = true;    // reset slope flag
            }
            break;   
          }
          else firstCandidate = false;

          // store Sq in linelist (actual finishing segment)
          vecLineSegments->push_back(Sq);
          
          // STEP5
          // set Sq as new target segment SS and increment candidate segment
          SS = Sq++;
        }

        // STEP7
        // calculate line length
        const int nSegments = static_cast<int>(vecLineSegments->size());
        int length = 0;

        if(positiveSlope) {
          const int y1 = vecSegments[2][(*vecLineSegments)[0]].start.y;
          const int y2 = vecSegments[2][(*vecLineSegments)[nSegments-1]].end.y;
          length = y2 - y1 + 1;
        }
        else {
          const int y1 = vecSegments[2][(*vecLineSegments)[nSegments-1]].start.y;
          const int y2 = vecSegments[2][(*vecLineSegments)[0]].end.y;
          length = y2 - y1 + 1;
        }

        // line length must be >= minLineLen for extraction
        if(length >= par.minLineLen) {
        
          // register grouped segments as extracted line
          lineEntry newline;
          if(positiveSlope) {
            newline.start.x = vecSegments[2][(*vecLineSegments)[0]].start.x;
            newline.start.y = vecSegments[2][(*vecLineSegments)[0]].start.y;
            newline.end.x = vecSegments[2][(*vecLineSegments)[nSegments-1]].end.x;
            newline.end.y = vecSegments[2][(*vecLineSegments)[nSegments-1]].end.y;
          }
          else {
            newline.start.x = vecSegments[2][(*vecLineSegments)[nSegments-1]].start.x;
            newline.start.y = vecSegments[2][(*vecLineSegments)[nSegments-1]].start.y;
            newline.end.x = vecSegments[2][(*vecLineSegments)[0]].end.x;
            newline.end.y = vecSegments[2][(*vecLineSegments)[0]].end.y;
          }
          // calculate midpoint
          newline.mid.x = (newline.start.x + newline.end.x)/2;
          newline.mid.y = (newline.start.y + newline.end.y)/2;
          // calculate tangent (wrong direction because of legacy implementation)
          newline.tangent = atan2((double)(newline.end.y-newline.start.y), (double) -(newline.end.x-newline.start.x));
          //newline.tangent = atan((double) -(newline.end.y-newline.start.y) / (newline.end.x-newline.start.x));
          // set other values
          newline.len = length;
          newline.used = 0;
          newline.group = 2;
          // store data
          newline.segmIdxList = vecLineSegments;
          vecLines[2].push_back(newline);
          vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            vecSegments[2][(*vecLineSegments)[i]].used++;
          }
        }
        else delete vecLineSegments;

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(vecSegments[2][Si].used == 0) break;
        }
        
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////////////

  void fastEllipseExtraction::extractGroup3Lines() {

    // clear old linelist and reserve memory
    clearLineList(3);
    vecLines[3].reserve(linePreallocation);

    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(vecSegments[3].size());
    
    if(segmCount > 0) {

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        // create new segment grouping vector
        std::vector<int> *vecLineSegments = new std::vector<int>;
        
        bool positiveSlope = true;   // sign of the slope of the first segment pair
        bool firstCandidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments->push_back(Si);  // store Si in linelist (starting segment)
        SS = Si;                        // set Si as new target segment SS for search
        SL = Si;                        // set Si as actual longest linesegment SL
        Sq = Si + 1;                    // first candidate segment


        while(Sq < segmCount) {  // track line segments

          const segmEntry segSS = vecSegments[3][SS];

          // transform coordinates (diagonal -> horizontal)
          const int SSStartX = segSS.start.x + segSS.start.y;
          const int SSEndX   = segSS.end.x + segSS.end.y;
          const int SSEndY   = imageWidth - 1 - segSS.end.x + segSS.end.y;
          
          // STEP2
          // calculate window boundaries for target segment SS
          const int GAPx = par.maxSegmentGap;
          const int GAPxPos = SSEndX + 1 + GAPx*2;   // double gap + 1 
          const int GAPxNeg = SSStartX - 1 - GAPx*2; // double gap + 1 
          const float deltaY = -0.5f; //SSEndY - SSStartY - 0.5;
          const float deltaX = static_cast<float>(SSEndX - SSStartX);
          const float minSlope = deltaY / deltaX;
          const float GAPyFract = minSlope * (GAPxNeg - SSEndX);
          const float GAPyRound = floor(GAPyFract + 0.5f);
          const int GAPy = static_cast<int>(SSEndY + GAPyRound);


          // search new candidate segment Sq (from top to bottom)
          while(Sq < segmCount) {
          
            // no multi-use of segments in circle extraction
            if(vecSegments[3][Sq].used > 0) {
              Sq++;
              continue;
            }
            const segmEntry segSq = vecSegments[3][Sq];

            // transform coordinates (diagonal -> horizontal)
            const int SqStartX = segSq.start.x + segSq.start.y;
            const int SqStartY = imageWidth - 1 - segSq.start.x + segSq.start.y;
            const int SqEndX   = segSq.end.x + segSq.end.y;

            if(SqStartY > GAPy) { // window boundary reached ?
              Sq = segmCount;       // no candidate segment found
              break;
            }

            // positive slope or first candidate
            if(positiveSlope || firstCandidate) {
              if((SqStartX <= GAPxPos)                    // Sq starts in window 
              && (SqStartX >= SSEndX - segSS.len + 1)) { // Sq lies right from center of SS
                positiveSlope = true;
                break;
              }
            }

            // negative slope or first candidate
            if(!positiveSlope || firstCandidate) {
              if((SqEndX >= GAPxNeg)                      // Sq ends in window 
              && (SqEndX <= SSStartX + segSS.len - 1)) { // Sq lies left from center of SS
                positiveSlope = false;
                break;
              }
            }

            Sq++;
          }

          if(Sq == segmCount) break;    // no candidate segment found

          const segmEntry segSi = vecSegments[3][Si];
          const segmEntry segSq = vecSegments[3][Sq];

          // calculate longest line segment SL (old SL or new Sq)
          if(vecSegments[3][SL].len < segSq.len) {
            SL = Sq;
          }
          const segmEntry segSL = vecSegments[3][SL];

          // STEP3
          // calculate slope
          float slope;
          if(positiveSlope) {
            slope = ((float)(segSq.end.y - segSi.start.y)) / (segSq.end.x - segSi.start.x);
          }
          else {
            slope = ((float)(segSi.end.y - segSq.start.y)) / (segSi.end.x - segSq.start.x);
          }
          
          // STEP4
          // check if SL fits in estimated line
          float deltaD = segSL.start.y - (slope * segSL.start.x)
                       -(  segSL.end.y - (slope * segSL.end.x) );
          //float deltaD = slope * (segSL.end.x - segSL.start.x);
          
          deltaD *= 0.5f; // check against deltaD/2 because of diagonal double quantization error
          
          // if not satisfied (check against deltaD/2 because of diagonal double quantization error)
          if((deltaD > par.maxQuantizationError) || (deltaD < -par.maxQuantizationError)) {
            if(firstCandidate) {
              positiveSlope = true;    // reset slope flag
            }
            break;   
          }
          else firstCandidate = false;

          // store Sq in linelist (actual finishing segment)
          vecLineSegments->push_back(Sq);
          
          // STEP5
          // set Sq as new target segment SS and increment candidate segment
          SS = Sq++;
        }

        // STEP7
        // calculate line length
        const int nSegments = static_cast<int>(vecLineSegments->size());
        int length = 0;

        if(positiveSlope) {
          const int y1 = vecSegments[3][(*vecLineSegments)[0]].start.y;
          const int y2 = vecSegments[3][(*vecLineSegments)[nSegments-1]].end.y;
          length = y2 - y1 + 1;
        }
        else {
          const int x1 = vecSegments[3][(*vecLineSegments)[nSegments-1]].start.x;
          const int x2 = vecSegments[3][(*vecLineSegments)[0]].end.x;
          length = x2 - x1 + 1;
        }

        // line length must be >= minLineLen for extraction
        if(length >= par.minLineLen) {
        
          // register grouped segments as extracted line
          lineEntry newline;
          if(positiveSlope) {
            newline.start.x = vecSegments[3][(*vecLineSegments)[0]].start.x;
            newline.start.y = vecSegments[3][(*vecLineSegments)[0]].start.y;
            newline.end.x = vecSegments[3][(*vecLineSegments)[nSegments-1]].end.x;
            newline.end.y = vecSegments[3][(*vecLineSegments)[nSegments-1]].end.y;
          }
          else {
            newline.end.x = vecSegments[3][(*vecLineSegments)[0]].end.x;
            newline.end.y = vecSegments[3][(*vecLineSegments)[0]].end.y;
            newline.start.x = vecSegments[3][(*vecLineSegments)[nSegments-1]].start.x;
            newline.start.y = vecSegments[3][(*vecLineSegments)[nSegments-1]].start.y;
          }
          // calculate midpoint
          newline.mid.x = (newline.start.x + newline.end.x)/2;
          newline.mid.y = (newline.start.y + newline.end.y)/2;
          // calculate tangent
          newline.tangent = atan2((double) -(newline.end.y-newline.start.y), (double)(newline.end.x-newline.start.x));
          //newline.tangent = atan((double) -(newline.end.y-newline.start.y) / (newline.end.x-newline.start.x));
          // set other values
          newline.len = length;
          newline.used = 0;
          newline.group = 3;
          // store data
          newline.segmIdxList = vecLineSegments;
          vecLines[3].push_back(newline);
          vecLines[0].push_back(newline);


          for(int i=0; i<nSegments; i++) {
            // increase used-value of the grouped segments
            vecSegments[3][(*vecLineSegments)[i]].used++;
          }
        }
        else delete vecLineSegments;

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(vecSegments[3][Si].used == 0) break;
        }
        
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////////////

  void fastEllipseExtraction::extractGroup4Lines() {

    // clear old linelist and reserve memory
    clearLineList(4);
    vecLines[4].reserve(linePreallocation);
  
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(vecSegments[4].size());
    
    if(segmCount > 0) {

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        // create new segment grouping vector
        std::vector<int> *vecLineSegments = new std::vector<int>;

        bool positiveSlope = true;   // sign of the slope of the first segment pair
        bool firstCandidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments->push_back(Si);  // store Si in linelist (starting segment)
        SS = Si;                        // set Si as new target segment SS for search
        SL = Si;                        // set Si as actual longest linesegment SL
        Sq = Si + 1;                    // first candidate segment


        while(Sq < segmCount) {  // track line segments
        
          const segmEntry segSS = vecSegments[4][SS];

          // transform coordinates (diagonal -> horizontal)
          const int SSStartY = imageWidth - 1 - segSS.start.x + segSS.start.y;
          const int SSEndX   = segSS.end.x + segSS.end.y;
          const int SSEndY   = imageWidth - 1 - segSS.end.x + segSS.end.y;
          
          // STEP2
          // calculate window boundaries for target segment SS
          const int GAPx = par.maxSegmentGap;
          const int GAPxPos = SSStartY + 1 + GAPx*2;   // double gap + 1 
          const int GAPxNeg = SSEndY - 1 - GAPx*2; // double gap + 1 
          const float deltaY = -0.5f; //SSStartX - SSEndX - 0.5;
          const float deltaX = static_cast<float>(SSStartY - SSEndY);
          const float minSlope = deltaY / deltaX;
          const float GAPyFract = minSlope * (GAPxNeg - SSStartY);
          const float GAPyRound = floor(GAPyFract + 0.5f);
          const int GAPy = static_cast<int>(SSEndX - GAPyRound);
          
          // search new candidate segment Sq (from right to left)
          while(Sq < segmCount) {
          
            // no multi-use of segments in circle extraction
            if(vecSegments[4][Sq].used > 0) {
              Sq++;
              continue;
            }
            const segmEntry segSq = vecSegments[4][Sq];

            // transform coordinates (diagonal -> horizontal)
            const int SqStartX = segSq.start.x + segSq.start.y;
            const int SqStartY = imageWidth - 1 - segSq.start.x + segSq.start.y;
            const int SqEndY   = imageWidth - 1 - segSq.end.x + segSq.end.y;

            if(SqStartX < GAPy) { // window boundary reached ?
              Sq = segmCount;       // no candidate segment found
              break;
            }

            // positive slope or first candidate
            if(positiveSlope || firstCandidate) {
              if((SqStartY >= GAPxNeg)                     // Sq starts in window 
              && (SqStartY <= SSEndY + segSS.len - 1)) {  // Sq lies above from center of SS
                positiveSlope = true;
                break;
              }
            }

            // negative slope or first candidate
            if(!positiveSlope || firstCandidate) {
              if((SqEndY <= GAPxPos)                      // Sq ends in window 
              && (SqEndY >= SSStartY - segSS.len + 1)) { // Sq lies below from center of SS
                positiveSlope = false;
                break;
              }
            }

            Sq++;
          }

          if(Sq == segmCount) break;    // no candidate segment found

          const segmEntry segSi = vecSegments[4][Si];
          const segmEntry segSq = vecSegments[4][Sq];

          // calculate longest line segment SL (old SL or new Sq)
          if(vecSegments[4][SL].len < segSq.len) {
            SL = Sq;
          }
          const segmEntry segSL = vecSegments[4][SL];

          // STEP3
          // calculate slope
          float slope;
          if(positiveSlope) {
            slope = ((float)(segSq.end.y - segSi.start.y)) / (segSq.end.x - segSi.start.x);
          }
          else {
            slope = ((float)(segSi.end.y - segSq.start.y)) / (segSi.end.x - segSq.start.x);
          }
          
          // STEP4
          // check if SL fits in estimated line
          float deltaD = segSL.start.y - (slope * segSL.start.x)
                       -(  segSL.end.y - (slope * segSL.end.x) );
          //float deltaD = slope * (segSL.end.x - segSL.start.x);
          
          deltaD *= 0.5f; // check against deltaD/2 because of diagonal double quantization error
          
          // if not satisfied (check against deltaD/2 because of diagonal double quantization error)
          if((deltaD > par.maxQuantizationError) || (deltaD < -par.maxQuantizationError)) {
            if(firstCandidate) {
              positiveSlope = true;    // reset slope flag
            }
            break;   
          }
          else firstCandidate = false;

          // store Sq in linelist (actual finishing segment)
          vecLineSegments->push_back(Sq);
          
          // STEP5
          // set Sq as new target segment SS and increment candidate segment
          SS = Sq++;
        }

        // STEP7
        // calculate line length
        const int nSegments = static_cast<int>(vecLineSegments->size());
        int length = 0;

        if(positiveSlope) {
          const int y1 = vecSegments[4][(*vecLineSegments)[nSegments-1]].end.y;
          const int y2 = vecSegments[4][(*vecLineSegments)[0]].start.y;
          length = y2 - y1 + 1;
        }
        else {
          const int x1 = vecSegments[4][(*vecLineSegments)[nSegments-1]].start.x;
          const int x2 = vecSegments[4][(*vecLineSegments)[0]].end.x;
          length = x2 - x1 + 1;
        }

        // line length must be >= minLineLen for extraction
        if(length >= par.minLineLen) {
        
          // register grouped segments as extracted line
          lineEntry newline;
          if(positiveSlope) {
            newline.start.x = vecSegments[4][(*vecLineSegments)[nSegments-1]].end.x;
            newline.start.y = vecSegments[4][(*vecLineSegments)[nSegments-1]].end.y;
            newline.end.x = vecSegments[4][(*vecLineSegments)[0]].start.x;
            newline.end.y = vecSegments[4][(*vecLineSegments)[0]].start.y;
          }
          else {
            newline.start.x = vecSegments[4][(*vecLineSegments)[0]].end.x;
            newline.start.y = vecSegments[4][(*vecLineSegments)[0]].end.y;
            newline.end.x = vecSegments[4][(*vecLineSegments)[nSegments-1]].start.x;
            newline.end.y = vecSegments[4][(*vecLineSegments)[nSegments-1]].start.y;
          }
          // calculate midpoint
          newline.mid.x = (newline.start.x + newline.end.x)/2;
          newline.mid.y = (newline.start.y + newline.end.y)/2;
          // calculate tangent (wrong direction because of legacy implementation)
          newline.tangent = atan2((double)(newline.end.y-newline.start.y), (double) -(newline.end.x-newline.start.x));
          //newline.tangent = atan((double) -(newline.end.y-newline.start.y) / (newline.end.x-newline.start.x));
          
          // set other values
          newline.len = length;
          newline.used = 0;
          newline.group = 4;
          // store data
          newline.segmIdxList = vecLineSegments;
          vecLines[4].push_back(newline);
          vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            vecSegments[4][(*vecLineSegments)[i]].used++;
          }
        }
        else delete vecLineSegments;

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(vecSegments[4][Si].used == 0) break;
        }
        
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////

  void fastEllipseExtraction::extractGroup1and5Arcs() {

    // clear old arclist and reserve memory
    clearEllArcList(0);
    vecEllArcs[0].reserve(8*ellArcPreallocation);
    clearEllArcList(1);
    vecEllArcs[1].reserve(ellArcPreallocation);
    clearEllArcList(5);
    vecEllArcs[5].reserve(ellArcPreallocation);
    
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(vecLines[1].size());
    
    if(segmCount > 0) {

      int Si = 0;
      int Sq, SS, Sl;

      do {
        // create new line grouping vector
        std::vector<int> *vecArcSegments = new std::vector<int>;

        bool forwardSearch = false;  // search direction along the arc (forward = left->right)
        bool upwardSearch = false;   // search direction in the window (upward = bottom->up)
        bool extraDown = false;      // extra search cycle to continue inverted direction segments
        bool extraUp = false;        // extra search cycle to continue inverted direction segments

        // STEP1
        vecArcSegments->push_back(Si);  // store Si in arc segment list (starting line)
        SS = Si;                        // set Si as new target arc segment SS for search
        Sq = Si + 1;                    // first candidate segment
        Sl = -1;                        // last fitting candidate
        
        while(true) {   // track lines (line order: left->right, top->down)
          // STEP2
          const lineEntry segSS = vecLines[1][SS];
          
          int SSy, SSx;
          if(forwardSearch) {
            SSx = segSS.end.x;
            SSy = segSS.end.y;
          } else {
            SSx = segSS.start.x;
            SSy = segSS.start.y;
          }

          // calculate window
          const int GAPxPos = par.maxLineGap;
          const int GAPxNeg = segSS.len/2 - 1;
          int GAPyPos, GAPyNeg;
          if(upwardSearch) {
            GAPyPos = par.maxLineGap - 1;
            GAPyNeg = 1;
          } else {
            GAPyPos = 1;
            GAPyNeg = par.maxLineGap - 1;
          }
          const int minY = SSy - GAPyPos;
          const int maxY = SSy + GAPyNeg;
          
          // search new candidate segment Sq
          while((Sq >= 0)&&(Sq < segmCount)) {

            const lineEntry segSq = vecLines[1][Sq];
            
            // vertical window boundary reached ? (both points outside the window)
            if(((segSq.start.y < minY)||(segSq.start.y > maxY)) 
            &&((segSq.end.y < minY)||(segSq.end.y > maxY))) { 
              Sq = -1;   // no candidate segment found
              break;
            }
            
            // calculate distance of Sq
            int dx,dy;
            if(forwardSearch) {
              dx = segSq.start.x - SSx;
              dy = segSq.start.y - SSy;
            } else {
              dx = SSx - segSq.end.x;
              dy = SSy - segSq.end.y;
            }             

            // calculate relative candidate angle
            const double angle = segSq.tangent - segSS.tangent;

            // check negative conditions
            if((dx > GAPxPos)||(dx < -GAPxNeg)  // dx too large
            ||(dy > GAPyPos)||(dy < -GAPyNeg)   // dy too large
            ||((dx <= 0)&&((dy > 1)||(dy < -1)))  // parallel gap too large
            ||((dx > 0)&&((dy > dx)||(dy < -dx))) // parallel gap too large
            ||((forwardSearch)&&(upwardSearch)&&(angle < 0))  // tangent directions
            ||((forwardSearch)&&(!upwardSearch)&&(angle > 0))
            ||((!forwardSearch)&&(upwardSearch)&&(angle > 0))
            ||((!forwardSearch)&&(!upwardSearch)&&(angle < 0))) {

              if(upwardSearch) {
                Sq--;
              } else {
                Sq++;
              }
              continue;
            }
            
            break;
          }

          if((Sq == segmCount)||(Sq < 0)) {   // no candidate segment found

            // next search cycle (STAGE 1: backwardDown/extraUp -> backwardUp/extraDown
            //                    STAGE 2: forwardDown/extraUp -> forwardUp/extraDown 
            //                    STAGE 3: end)
            if(upwardSearch) {
              if(extraUp) {            // NEXT: backwardUp/forwardUp
                extraUp = false;
                SS = Si;                // reset target segment
                Sq = Si - 1;            // first upwardSearch candidate
                Sl = -1;                // reset last fitting candidate
                continue;
              }
              if(!extraDown) {         // NEXT: extraDown
                extraDown = true;
                upwardSearch = false;  // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl + 1;
                }
                continue;
              }
            
            } else {  // down
              
              if(extraDown) {          // NEXT: forwardDown/end
                extraDown = false;
                if(forwardSearch) {    // set next search stage
                  break;  // END
                } else {
                  forwardSearch = true;  // START FORWARDSEARCH
                }
                SS = Si;                // reset target segment
                Sq = Si + 1;            // first forwardSearch candidate
                Sl = -1;                // reset last fitting candidate
                // resort segment list
                const int n = static_cast<int>(vecArcSegments->size());
                for(int i=0; i<(n/2); i++) {  
                  const int tmp = (*vecArcSegments)[i];
                  (*vecArcSegments)[i] = (*vecArcSegments)[n-1-i];
                  (*vecArcSegments)[n-1-i] = tmp;
                }
                continue;
              }
              
              if(!extraUp) {           // !extraUp
                extraUp = true;        
                upwardSearch = true;   // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl - 1;
                }
                continue;
              }
            }
          }


        
          // STEP3: check interior angle
          const double tangent1 = vecLines[1][SS].tangent;
          const double tangent2 = vecLines[1][Sq].tangent;
          double interiorAngle = (tangent2-tangent1)*180/PI;
          if(interiorAngle < 0) interiorAngle = -interiorAngle;
          interiorAngle = 180 - interiorAngle;

          if((interiorAngle <= 180)&&(interiorAngle >= 135)) {
            double estX, estY, estR2;
            vecArcSegments->push_back(Sq);  // store Sq in linelist
            // estimate circle parameters
            if(!estimateCircle(*vecArcSegments, 1, estX, estY, estR2)) {
              vecArcSegments->pop_back();   // circle estimation failed
            } else {
              // STEP4: check tangents
              const double deltaX1 = estX - vecLines[1][SS].mid.x;
              const double deltaY1 = estY - vecLines[1][SS].mid.y;
              double estTheta1 = atan2(deltaX1, deltaY1);
              if((deltaY1 < 0)&&(deltaX1 >= 0)) estTheta1 -= PI;
              if((deltaY1 < 0)&&(deltaX1 < 0)) estTheta1 += PI;
  
              const double deltaX2 = estX - vecLines[1][Sq].mid.x;
              const double deltaY2 = estY - vecLines[1][Sq].mid.y;
              double estTheta2 = atan2(deltaX2, deltaY2);
              if((deltaY2 < 0)&&(deltaX2 >= 0)) estTheta2 -= PI;
              if((deltaY2 < 0)&&(deltaX2 < 0)) estTheta2 += PI;
  
              if(((estTheta1-tangent1)*180/PI > par.maxLineTangentError)
              ||((estTheta1-tangent1)*180/PI < -par.maxLineTangentError)
              ||((estTheta2-tangent2)*180/PI > par.maxLineTangentError)
              ||((estTheta2-tangent2)*180/PI < -par.maxLineTangentError)) {
                vecArcSegments->pop_back(); // error exceeds maximum
              } else {
                SS = Sq;    // set Sq as new target segment SS
                Sl = Sq;    // last fitting candidate
              }
            } //circle estimation
          } //interior angle
          
          if(upwardSearch) {
            Sq--;
          } else {
            Sq++;
          }
        } //line tracking


      // CHECK ARC EXTRACTION CONDITIONS
        const int nSegments = static_cast<int>(vecArcSegments->size());
        
        int x = 0;
        int y = 0;
        int r2 = 0;
        double estX, estY, estR2;
        if(estimateCircle(*vecArcSegments, 1, estX, estY, estR2)) {
          x = static_cast<int>(floor(estX + 0.5));
          y = static_cast<int>(floor(estY + 0.5));
          r2 = static_cast<int>(floor(estR2 + 0.5));
        } else {
          r2 = 0;   // estimation failed
        }

        // radius must be greater than 10 pixel for extraction
        // at least 2 segments are neccessary for circle estimation
        if((r2 >= 100)&&(nSegments >= 2)) {

      // EXTRACT ARC
          // register grouped segments as extracted arc
          ellArcEntry newarc;
          // start/end points
          const int x1 = vecLines[1][(*vecArcSegments)[0]].start.x;
          const int y1 = vecLines[1][(*vecArcSegments)[0]].start.y;
          const int x2 = vecLines[1][(*vecArcSegments)[nSegments-1]].end.x;
          const int y2 = vecLines[1][(*vecArcSegments)[nSegments-1]].end.y;
          
          bool reverse = false;
          // check direction and group
          if(y >= y1) {
            newarc.group = 1; // top
            if(x1 > x2) {
              reverse = true;
            }
          } else {
            newarc.group = 5; // bottom
            if(x1 < x2) {
              reverse = true;
            }
          }
          
          if(reverse) {     // reverse idxlist
            int i;
            for(i=0; i<nSegments/2; i++) {
              int swap = (*vecArcSegments)[i];
              (*vecArcSegments)[i] = (*vecArcSegments)[nSegments-1-i];
              (*vecArcSegments)[nSegments-1-i] = swap;
            }
            // start/end points
            newarc.start.x = x2;
            newarc.start.y = y2;
            newarc.end.x = x1;
            newarc.end.y = y1;
          } else {
            newarc.start.x = x1;
            newarc.start.y = y1;
            newarc.end.x = x2;
            newarc.end.y = y2;
          }
          
          // first/last line vectors
          if(newarc.group == 1) {
            newarc.firstVec = vecLines[1][(*vecArcSegments)[0]].end 
                            - vecLines[1][(*vecArcSegments)[0]].start;
            newarc.lastVec = vecLines[1][(*vecArcSegments)[nSegments-1]].end 
                           - vecLines[1][(*vecArcSegments)[nSegments-1]].start;
          } else {
            newarc.firstVec = vecLines[1][(*vecArcSegments)[0]].start
                            - vecLines[1][(*vecArcSegments)[0]].end;
            newarc.lastVec = vecLines[1][(*vecArcSegments)[nSegments-1]].start
                           - vecLines[1][(*vecArcSegments)[nSegments-1]].end;
          }
          
          // midpoint
          newarc.mid.x = x;
          newarc.mid.y = y;
          // other values
          newarc.r2 = r2;
          newarc.used = 0;
          // line index list
          newarc.lineIdxList = vecArcSegments;
          
          // store data
          vecEllArcs[newarc.group].push_back(newarc);
          vecEllArcs[0].push_back(newarc);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            vecLines[1][(*vecArcSegments)[i]].used++;
          }
        } else {
          delete vecArcSegments;
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(vecLines[1][Si].used == 0) break;
        }
      }
      while(Si < segmCount);    // extract next arc if possible
    }
  }


///////////////////////////////////////////////////////////////////////

  void fastEllipseExtraction::extractGroup3and7Arcs() {

    // clear old arclist and reserve memory
    clearEllArcList(3);
    vecEllArcs[3].reserve(ellArcPreallocation);
    clearEllArcList(7);
    vecEllArcs[7].reserve(ellArcPreallocation);

    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(vecLines[2].size());
    
    if(segmCount > 0) {

      int Si = 0;
      int Sq, SS, Sl;

      do {
        // create new line grouping vector
        std::vector<int> *vecArcSegments = new std::vector<int>;

        bool forwardSearch = false;  // search direction along the arc (forward = top->down)
        bool upwardSearch = false;   // search direction in the window (upward = right->left)
        bool extraDown = false;      // extra search cycle to continue inverted direction segments
        bool extraUp = false;        // extra search cycle to continue inverted direction segments

        
        // STEP1
        vecArcSegments->push_back(Si);  // store Si in arc segment list (starting line)
        SS = Si;                        // set Si as new target arc segment SS for search
        Sq = Si + 1;                    // first candidate segment
        Sl = -1;                        // last fitting candidate
        
        while(true) {   // track lines (line order: top->down, left->right)
          // STEP2
          const lineEntry segSS = vecLines[2][SS];
          
          int SSy, SSx;
          if(forwardSearch) {
            SSx = segSS.end.x;
            SSy = segSS.end.y;
          } else {
            SSx = segSS.start.x;
            SSy = segSS.start.y;
          }

          // calculate window
          const int GAPyPos = par.maxLineGap;
          const int GAPyNeg = segSS.len/2 - 1;
          int GAPxPos, GAPxNeg;
          if(upwardSearch) {
            GAPxPos = par.maxLineGap - 1;
            GAPxNeg = 1;
          } else {
            GAPxPos = 1;
            GAPxNeg = par.maxLineGap - 1;
          }
          const int minX = SSx - GAPxPos;
          const int maxX = SSx + GAPxNeg;

          // search new candidate segment Sq
          while((0 <= Sq)&&(Sq < segmCount)) {

            const lineEntry segSq = vecLines[2][Sq];

            // horizontal window boundary reached ? (both points outside the window)
            if(((segSq.end.x < minX)||(segSq.end.x > maxX)) 
            &&((segSq.start.x < minX)||(segSq.start.x > maxX))) { 
              Sq = -1;   // no candidate segment found
              break;
            }

            // calculate of Sq
            int dx,dy;
            if(forwardSearch) {
              dx = segSq.start.x - SSx;
              dy = segSq.start.y - SSy;
            } else {
              dx = SSx - segSq.end.x;
              dy = SSy - segSq.end.y;
            }             

            // calculate relative candidate angle
            const double angle = segSq.tangent - segSS.tangent;

            // check negative conditions
            if((dy > GAPyPos)||(dy < -GAPyNeg)  // dy too large
            ||(dx > GAPxPos)||(dx < -GAPxNeg)   // dx too large
            ||((dy <= 0)&&((dx > 1)||(dx < -1)))  // parallel gap too large
            ||((dy > 0)&&((dx > dy)||(dx < -dy))) // parallel gap too large
            ||((forwardSearch)&&(upwardSearch)&&(angle > 0))  // tangent directions (grp2: false tangent sign)
            ||((forwardSearch)&&(!upwardSearch)&&(angle < 0))
            ||((!forwardSearch)&&(upwardSearch)&&(angle < 0))
            ||((!forwardSearch)&&(!upwardSearch)&&(angle > 0))) {

              if(upwardSearch) {
                Sq--;
              } else {
                Sq++;
              }
              continue;
            }
            
            break;
          }

          if((Sq == segmCount)||(Sq < 0)) {   // no candidate segment found

            // next search cycle (STAGE 1: backwardDown/extraUp -> backwardUp/extraDown
            //                    STAGE 2: forwardDown/extraUp -> forwardUp/extraDown 
            //                    STAGE 3: end)
            if(upwardSearch) {
              if(extraUp) {            // NEXT: backwardUp/forwardUp
                extraUp = false;
                SS = Si;                // reset target segment
                Sq = Si - 1;            // first upwardSearch candidate
                Sl = -1;                // reset last fitting candidate
                continue;
              }
              if(!extraDown) {         // NEXT: extraDown
                extraDown = true;
                upwardSearch = false;  // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl + 1;
                }
                continue;
              }
            } else {  // down
              
              if(extraDown) {          // NEXT: forwardDown/end
                extraDown = false;
                if(forwardSearch) {    // set next search stage
                  break;  // END
                } else {
                  forwardSearch = true;  // START FORWARDSEARCH
                }
                SS = Si;                // reset target segment
                Sq = Si + 1;            // first forwardSearch candidate
                Sl = -1;                // reset last fitting candidate
                // resort segment list
                const int n = static_cast<int>(vecArcSegments->size());
                for(int i=0; i<(n/2); i++) {  
                  const int tmp = (*vecArcSegments)[i];
                  (*vecArcSegments)[i] = (*vecArcSegments)[n-1-i];
                  (*vecArcSegments)[n-1-i] = tmp;
                }
                continue;
              }
              
              if(!extraUp) {           // extraUp
                extraUp = true;        
                upwardSearch = true;   // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl - 1;
                }
                continue;
              }
            }
          }
        
          // STEP3: check interior angle
          const double tangent1 = vecLines[2][SS].tangent;
          const double tangent2 = vecLines[2][Sq].tangent;
          double interiorAngle = (tangent2-tangent1)*180/PI;
          if(interiorAngle < 0) interiorAngle = -interiorAngle;
          interiorAngle = 180 - interiorAngle;

          if((interiorAngle <= 180)&&(interiorAngle >= 135)) {
            double estX, estY, estR2;
            vecArcSegments->push_back(Sq);  // store Sq in linelist
            // estimate circle parameters
            if(!estimateCircle(*vecArcSegments, 2, estX, estY, estR2)) {
              vecArcSegments->pop_back();   // circle estimation failed
            } else {
              // STEP4: check tangents
              const double deltaX1 = estX - vecLines[2][SS].mid.x;
              const double deltaY1 = estY - vecLines[2][SS].mid.y;
              double estTheta1 = atan2(deltaX1, deltaY1);
              if((deltaX1 < 0)&&(deltaY1 >= 0)) estTheta1 += PI;
              if((deltaX1 < 0)&&(deltaY1 < 0)) estTheta1 += PI;
              
              const double deltaX2 = estX - vecLines[2][Sq].mid.x;
              const double deltaY2 = estY - vecLines[2][Sq].mid.y;
              double estTheta2 = atan2(deltaX2, deltaY2);
              if((deltaX2 < 0)&&(deltaY2 >= 0)) estTheta2 += PI;
              if((deltaX2 < 0)&&(deltaY2 < 0)) estTheta2 += PI;
  
              if(((estTheta1-tangent1)*180/PI > par.maxLineTangentError)
              ||((estTheta1-tangent1)*180/PI < -par.maxLineTangentError)
              ||((estTheta2-tangent2)*180/PI > par.maxLineTangentError)
              ||((estTheta2-tangent2)*180/PI < -par.maxLineTangentError)) {
                // tangent estimation error exceeded maximum
                vecArcSegments->pop_back();
              } else {
                SS = Sq;    // set Sq as new target segment SS
                Sl = Sq;    // last fitting candidate
              }
            }
          }

          if(upwardSearch) {
            Sq--;
          } else {
            Sq++;
          }
        }

      // CHECK ARC EXTRACTION CONDITIONS
        const int nSegments = static_cast<int>(vecArcSegments->size());
        
        int x = 0;
        int y = 0;
        int r2 = 0;
        double estX, estY, estR2;
        if(estimateCircle(*vecArcSegments, 2, estX, estY, estR2)) {
          x = static_cast<int>(floor(estX + 0.5));
          y = static_cast<int>(floor(estY + 0.5));
          r2 = static_cast<int>(floor(estR2 + 0.5));
        } else {
          r2 = 0;   // estimation failed
        }

        // radius must be greater than 10 pixel for extraction
        // at least 2 segments are neccessary for circle estimation
        if((r2 >= 100)&&(nSegments >= 2)) {
          
      // EXTRACT ARC
          // register grouped segments as extracted arc
          ellArcEntry newarc;
          // start/end points
          const int x1 = vecLines[2][(*vecArcSegments)[0]].start.x;
          const int y1 = vecLines[2][(*vecArcSegments)[0]].start.y;
          const int x2 = vecLines[2][(*vecArcSegments)[nSegments-1]].end.x;
          const int y2 = vecLines[2][(*vecArcSegments)[nSegments-1]].end.y;

          bool reverse = false;
          // check direction and group
          if(x <= x1) {
            newarc.group = 3; // right
            if(y1 > y2) {
              reverse = true;
            }
          } else {
            newarc.group = 7; // left
            if(y1 < y2) {
              reverse = true;
            }
          }
          
          if(reverse) {     // reverse idxlist
            int i;
            for(i=0; i<nSegments/2; i++) {
              int swap = (*vecArcSegments)[i];
              (*vecArcSegments)[i] = (*vecArcSegments)[nSegments-1-i];
              (*vecArcSegments)[nSegments-1-i] = swap;
            }
            // start/end points
            newarc.start.x = x2;
            newarc.start.y = y2;
            newarc.end.x = x1;
            newarc.end.y = y1;
          } else {
            newarc.start.x = x1;
            newarc.start.y = y1;
            newarc.end.x = x2;
            newarc.end.y = y2;
          }

          // first/last line vectors
          if(newarc.group == 3) {
            newarc.firstVec = vecLines[2][(*vecArcSegments)[0]].end 
                            - vecLines[2][(*vecArcSegments)[0]].start;
            newarc.lastVec = vecLines[2][(*vecArcSegments)[nSegments-1]].end 
                           - vecLines[2][(*vecArcSegments)[nSegments-1]].start;
          } else {
            newarc.firstVec = vecLines[2][(*vecArcSegments)[0]].start
                            - vecLines[2][(*vecArcSegments)[0]].end;
            newarc.lastVec = vecLines[2][(*vecArcSegments)[nSegments-1]].start
                           - vecLines[2][(*vecArcSegments)[nSegments-1]].end;
          }

          // midpoint
          newarc.mid.x = x;
          newarc.mid.y = y;
          // other values
          newarc.r2 = r2;
          newarc.used = 0;
          // line index list
          newarc.lineIdxList = vecArcSegments;
          
          // store data
          vecEllArcs[newarc.group].push_back(newarc);
          vecEllArcs[0].push_back(newarc);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            vecLines[2][(*vecArcSegments)[i]].used++;
          }
        } else {
          delete vecArcSegments;
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(vecLines[2][Si].used == 0) break;
        }
      }
      while(Si < segmCount);    // extract next arc if possible
    }
  }
  
///////////////////////////////////////////////////////////////////////

  void fastEllipseExtraction::extractGroup2and6Arcs() {
    
    // clear old arclist and reserve memory
    clearEllArcList(2);
    vecEllArcs[2].reserve(ellArcPreallocation);
    clearEllArcList(6);
    vecEllArcs[6].reserve(ellArcPreallocation);

    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(vecLines[3].size());
    
    if(segmCount > 0) {

      int Si = 0;
      int Sq, SS, Sl;

      do {
        // create new line grouping vector
        std::vector<int> *vecArcSegments = new std::vector<int>;

        bool forwardSearch = false;  // search direction along the arc (forward = left->right)
        bool upwardSearch = false;   // search direction in the window (upward = bottom->up)
        bool extraDown = false;      // extra search cycle to continue inverted direction segments
        bool extraUp = false;        // extra search cycle to continue inverted direction segments

        // STEP1
        vecArcSegments->push_back(Si);  // store Si in arc segment list (starting line)
        SS = Si;                        // set Si as new target arc segment SS for search
        Sq = Si + 1;                    // first candidate segment
        Sl = -1;                        // last fitting candidate
        
        while(true) {   // track lines (line order: left->right, top->down)
          // STEP2
          const lineEntry segSS = vecLines[3][SS];

          // transform coordinates (diagonal -> horizontal)
          const int SSStartX = segSS.start.x + segSS.start.y;
          const int SSStartY = imageWidth - 1 - segSS.start.x + segSS.start.y;
          const int SSEndX   = segSS.end.x + segSS.end.y;
          const int SSEndY   = imageWidth - 1 - segSS.end.x + segSS.end.y;
          
          int SSy, SSx;
          if(forwardSearch) {
            SSx = SSEndX;
            SSy = SSEndY;
          } else {
            SSx = SSStartX;
            SSy = SSStartY;
          }

          // calculate window
          const int GAPxPos = par.maxLineGap*2 + 1;
          const int GAPxNeg = segSS.len/2;
          int GAPyPos, GAPyNeg;
          if(upwardSearch) {
            GAPyPos = par.maxLineGap;
            GAPyNeg = 1;
          } else {
            GAPyPos = 1;
            GAPyNeg = par.maxLineGap;
          }
          const int minY = SSy - GAPyPos;
          const int maxY = SSy + GAPyNeg;


          // search new candidate segment Sq
          while((0 <= Sq)&&(Sq < segmCount)) {

            const lineEntry segSq = vecLines[3][Sq];

            // transform coordinates (diagonal -> horizontal)
            const int SqStartX = segSq.start.x + segSq.start.y;
            const int SqStartY = imageWidth - 1 - segSq.start.x + segSq.start.y;
            const int SqEndX   = segSq.end.x + segSq.end.y;
            const int SqEndY   = imageWidth - 1 - segSq.end.x + segSq.end.y;

            // vertical window boundary reached ? (both points outside the window)
            if(((SqStartY < minY)||(SqStartY > maxY)) 
            &&((SqEndY < minY)||(SqEndY > maxY))) { 
              Sq = -1;   // no candidate segment found
              break;
            }

            // calculate distance of Sq
            int dx,dy;
            if(forwardSearch) {
              dx = SqStartX - SSx;
              dy = SqStartY - SSy;
            } else {
              dx = SSx - SqEndX;
              dy = SSy - SqEndY;
            }             

            // calculate relative candidate angle
            const double angle = segSq.tangent - segSS.tangent;

            // check negative conditions
            if((dx > GAPxPos)||(dx < -GAPxNeg)      // dx too large
            ||(dy > GAPyPos)||(dy < -GAPyNeg)       // dy too large
            ||((dx <= 1)&&((dy > 1)||(dy < -1)))      // parallel gap too large
            ||((dx > 1)&&((dy > dx/2)||(dy < -dx/2))) // parallel gap too large
            ||((forwardSearch)&&(upwardSearch)&&(angle < 0))  // tangent directions
            ||((forwardSearch)&&(!upwardSearch)&&(angle > 0))
            ||((!forwardSearch)&&(upwardSearch)&&(angle > 0))
            ||((!forwardSearch)&&(!upwardSearch)&&(angle < 0))) {

              if(upwardSearch) {
                Sq--;
              } else {
                Sq++;
              }
              continue;
            }
            
            break;
          }

          if((Sq == segmCount)||(Sq < 0)) {   // no candidate segment found

            // next search cycle (STAGE 1: backwardDown/extraUp -> backwardUp/extraDown
            //                    STAGE 2: forwardDown/extraUp -> forwardUp/extraDown 
            //                    STAGE 3: end)
            if(upwardSearch) {
              if(extraUp) {            // NEXT: backwardUp/forwardUp
                extraUp = false;
                SS = Si;                // reset target segment
                Sq = Si - 1;            // first upwardSearch candidate
                Sl = -1;                // reset last fitting candidate
                continue;
              }
              if(!extraDown) {         // NEXT: extraDown
                extraDown = true;
                upwardSearch = false;  // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl + 1;
                }
                continue;
              }
            } else {  // down
              
              if(extraDown) {          // NEXT: forwardDown/end
                extraDown = false;
                if(forwardSearch) {    // set next search stage
                  break;  // END
                } else {
                  forwardSearch = true;  // START FORWARDSEARCH
                }
                SS = Si;                // reset target segment
                Sq = Si + 1;            // first forwardSearch candidate
                Sl = -1;                // reset last fitting candidate
                // resort segment list
                const int n = static_cast<int>(vecArcSegments->size());
                for(int i=0; i<(n/2); i++) {  
                  const int tmp = (*vecArcSegments)[i];
                  (*vecArcSegments)[i] = (*vecArcSegments)[n-1-i];
                  (*vecArcSegments)[n-1-i] = tmp;
                }
                continue;
              }
              
              if(!extraUp) {           // extraUp
                extraUp = true;        
                upwardSearch = true;   // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl - 1;
                }
                continue;
              }
            }
          }

          // STEP3: check interior angle
          const double tangent1 = vecLines[3][SS].tangent;
          const double tangent2 = vecLines[3][Sq].tangent;
          double interiorAngle = (tangent2-tangent1)*180/PI;
          if(interiorAngle < 0) interiorAngle = -interiorAngle;
          interiorAngle = 180 - interiorAngle;

          if((interiorAngle <= 180)&&(interiorAngle >= 135)) {
            double estX, estY, estR2;
            vecArcSegments->push_back(Sq);  // store Sq in linelist
            // estimate circle parameters
            if(!estimateCircle(*vecArcSegments, 3, estX, estY, estR2)) {
              vecArcSegments->pop_back();   // circle estimation failed
            } else {
              // STEP4: check tangents
              const double deltaX1 = estX - vecLines[3][SS].mid.x;
              const double deltaY1 = estY - vecLines[3][SS].mid.y;
              double estTheta1 = atan2(deltaX1, deltaY1);
              if((deltaY1 < 0)&&(deltaX1 >= 0)) estTheta1 -= PI;
              if((deltaY1 < 0)&&(deltaX1 < 0)) estTheta1 += PI;
  
              const double deltaX2 = estX - vecLines[3][Sq].mid.x;
              const double deltaY2 = estY - vecLines[3][Sq].mid.y;
              double estTheta2 = atan2(deltaX2, deltaY2);
              if((deltaY2 < 0)&&(deltaX2 >= 0)) estTheta2 -= PI;
              if((deltaY2 < 0)&&(deltaX2 < 0)) estTheta2 += PI;
  
              if(((estTheta1-tangent1)*180/PI > par.maxLineTangentError)
              ||((estTheta1-tangent1)*180/PI < -par.maxLineTangentError)
              ||((estTheta2-tangent2)*180/PI > par.maxLineTangentError)
              ||((estTheta2-tangent2)*180/PI < -par.maxLineTangentError)) {
                // tangent estimation error exceeded maximum
                vecArcSegments->pop_back();
              } else {
                SS = Sq;    // set Sq as new target segment SS
                Sl = Sq;    // last fitting candidate
              }
            }
          }

          if(upwardSearch) {
            Sq--;
          } else {
            Sq++;
          }
        }


      // CHECK ARC EXTRACTION CONDITIONS
        const int nSegments = static_cast<int>(vecArcSegments->size());
        
        int x = 0;
        int y = 0;
        int r2 = 0;
        double estX, estY, estR2;
        if(estimateCircle(*vecArcSegments, 3, estX, estY, estR2)) {
          x = static_cast<int>(floor(estX + 0.5));
          y = static_cast<int>(floor(estY + 0.5));
          r2 = static_cast<int>(floor(estR2 + 0.5));
        } else {
          r2 = 0;   // estimation failed
        }

        // radius must be greater than 10 pixel for extraction
        // at least 2 segments are neccessary for circle estimation
        if((r2 >= 100)&&(nSegments >= 2)) {

      // EXTRACT ARC
          // register grouped segments as extracted arc
          ellArcEntry newarc;
          // start/end points
          const int x1 = vecLines[3][(*vecArcSegments)[0]].start.x;
          const int y1 = vecLines[3][(*vecArcSegments)[0]].start.y;
          const int x2 = vecLines[3][(*vecArcSegments)[nSegments-1]].end.x;
          const int y2 = vecLines[3][(*vecArcSegments)[nSegments-1]].end.y;

          // transform coordinates (diagonal -> horizontal)
          const int X1 = x1 + y1;
          const int X2 = x2 + y2;
          const int Y1 = imageWidth - 1 - x1 + y1;
          const int Y = imageWidth - 1 - x + y;

          bool reverse = false;
          // check direction and group
          if(Y >= Y1) {
            newarc.group = 2; // top right
            if(X1 > X2) {
              reverse = true;
            }
          } else {
            newarc.group = 6; // bottom left
            if(X1 < X2) {
              reverse = true;
            }
          }
          
          if(reverse) {     // reverse idxlist
            int i;
            for(i=0; i<nSegments/2; i++) {
              int swap = (*vecArcSegments)[i];
              (*vecArcSegments)[i] = (*vecArcSegments)[nSegments-1-i];
              (*vecArcSegments)[nSegments-1-i] = swap;
            }
            // start/end points
            newarc.start.x = x2;
            newarc.start.y = y2;
            newarc.end.x = x1;
            newarc.end.y = y1;
          } else {
            newarc.start.x = x1;
            newarc.start.y = y1;
            newarc.end.x = x2;
            newarc.end.y = y2;
          }

          // first/last line vectors
          if(newarc.group == 2) {
            newarc.firstVec = vecLines[3][(*vecArcSegments)[0]].end 
                            - vecLines[3][(*vecArcSegments)[0]].start;
            newarc.lastVec = vecLines[3][(*vecArcSegments)[nSegments-1]].end 
                           - vecLines[3][(*vecArcSegments)[nSegments-1]].start;
          } else {
            newarc.firstVec = vecLines[3][(*vecArcSegments)[0]].start
                            - vecLines[3][(*vecArcSegments)[0]].end;
            newarc.lastVec = vecLines[3][(*vecArcSegments)[nSegments-1]].start
                           - vecLines[3][(*vecArcSegments)[nSegments-1]].end;
          }
          
          // midpoint
          newarc.mid.x = x;
          newarc.mid.y = y;
          // other values
          newarc.r2 = r2;
          newarc.used = 0;
          // line index list
          newarc.lineIdxList = vecArcSegments;
          
          // store data
          vecEllArcs[newarc.group].push_back(newarc);
          vecEllArcs[0].push_back(newarc);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            vecLines[3][(*vecArcSegments)[i]].used++;
          }
        } else {
          delete vecArcSegments;
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(vecLines[3][Si].used == 0) break;
        }
      }
      while(Si < segmCount);    // extract next arc if possible
    }
  }

  
///////////////////////////////////////////////////////////////////////

  void fastEllipseExtraction::extractGroup4and8Arcs() {

    // clear old arclist and reserve memory
    clearEllArcList(4);
    vecEllArcs[4].reserve(ellArcPreallocation);
    clearEllArcList(8);
    vecEllArcs[8].reserve(ellArcPreallocation);

    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(vecLines[4].size());
    
    if(segmCount > 0) {

      int Si = 0;
      int Sq, SS, Sl;

      do {
        // create new line grouping vector
        std::vector<int> *vecArcSegments = new std::vector<int>;

        bool forwardSearch = false;  // search direction along the arc (forward = bottom->up)
        bool upwardSearch = false;   // search direction in the window (upward = left->right)
        bool extraDown = false;      // extra search cycle to continue inverted direction segments
        bool extraUp = false;        // extra search cycle to continue inverted direction segments

        // STEP1
        vecArcSegments->push_back(Si);  // store Si in arc segment list (starting line)
        SS = Si;                        // set Si as new target arc segment SS for search
        Sq = Si + 1;                    // first candidate segment
        Sl = -1;                        // last fitting candidate

        while(true) {   // track lines (line order: bottom->up, right->left)
          // STEP2
          const lineEntry segSS = vecLines[4][SS];

          // transform coordinates (diagonal -> horizontal)
          const int SSStartX = segSS.start.x + segSS.start.y;
          const int SSStartY = imageWidth - 1 - segSS.start.x + segSS.start.y;
          const int SSEndX   = segSS.end.x + segSS.end.y;
          const int SSEndY   = imageWidth - 1 - segSS.end.x + segSS.end.y;

          int SSy, SSx;
          if(forwardSearch) {
            SSx = SSStartX;
            SSy = SSStartY;
          } else {
            SSx = SSEndX;
            SSy = SSEndY;
          }
          
          // calculate window
          const int GAPyPos = par.maxLineGap*2 + 1;
          const int GAPyNeg = segSS.len/2;
          int GAPxPos, GAPxNeg;
          if(upwardSearch) {
            GAPxPos = 1;
            GAPxNeg = par.maxLineGap;
          } else {
            GAPxPos = par.maxLineGap;
            GAPxNeg = 1;
          }
          const int minX = SSx - GAPxPos;      // grpIV: inverted search direction (compared to grpII)
          const int maxX = SSx + GAPxNeg;


          // search new candidate segment Sq
          while((0 <= Sq)&&(Sq < segmCount)) {

            const lineEntry segSq = vecLines[4][Sq];

            // transform coordinates (diagonal -> horizontal)
            const int SqStartX = segSq.start.x + segSq.start.y;
            const int SqStartY = imageWidth - 1 - segSq.start.x + segSq.start.y;
            const int SqEndX   = segSq.end.x + segSq.end.y;
            const int SqEndY   = imageWidth - 1 - segSq.end.x + segSq.end.y;


            // horizontal window boundary reached ? (both points outside the window)
            if(((SqEndX < minX)||(SqEndX > maxX)) 
            &&((SqStartX < minX)||(SqStartX > maxX))) { 
              Sq = -1;   // no candidate segment found
              break;
            }

            // calculate distance of Sq
            int dx,dy;
            if(forwardSearch) {
              dx = SSx - SqEndX;
              dy = SSy - SqEndY;
            } else {
              dx = SqStartX - SSx;
              dy = SqStartY - SSy;
            }

            // calculate relative candidate angle
            const double angle = segSq.tangent - segSS.tangent;

            // check negative conditions
            if((dy > GAPyPos)||(dy < -GAPyNeg)      // dy too large
            ||(dx > GAPxPos)||(dx < -GAPxNeg)       // dx too large
            ||((dy <= 1)&&((dx > 1)||(dx < -1)))      // parallel gap too large
            ||((dy > 1)&&((dx > dy/2)||(dx > dy/2)))  // parallel gap too large
            ||((forwardSearch)&&(upwardSearch)&&(angle > 0))  // tangent directions
            ||((forwardSearch)&&(!upwardSearch)&&(angle < 0))
            ||((!forwardSearch)&&(upwardSearch)&&(angle < 0))
            ||((!forwardSearch)&&(!upwardSearch)&&(angle > 0))) {

              if(upwardSearch) {
                Sq--;
              } else {
                Sq++;
              }
              continue;
            }
            
            break;
          }

          if((Sq == segmCount)||(Sq < 0)) {   // no candidate segment found

            // next search cycle (STAGE 1: backwardDown/extraUp -> backwardUp/extraDown
            //                    STAGE 2: forwardDown/extraUp -> forwardUp/extraDown 
            //                    STAGE 3: end)
            if(upwardSearch) {
              if(extraUp) {            // NEXT: backwardUp/forwardUp
                extraUp = false;
                SS = Si;                // reset target segment
                Sq = Si - 1;            // first upwardSearch candidate
                Sl = -1;                // reset last fitting candidate
                continue;
              }
              if(!extraDown) {         // NEXT: extraDown
                extraDown = true;
                upwardSearch = false;  // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl + 1;
                }
                continue;
              }
            } else {  // down
              
              if(extraDown) {          // NEXT: forwardDown/end
                extraDown = false;
                if(forwardSearch) {    // set next search stage
                  break;  // END
                } else {
                  forwardSearch = true;  // START FORWARDSEARCH
                }
                SS = Si;                // reset target segment
                Sq = Si + 1;            // first forwardSearch candidate
                Sl = -1;                // reset last fitting candidate
                // resort segment list
                const int n = static_cast<int>(vecArcSegments->size());
                for(int i=0; i<(n/2); i++) {  
                  const int tmp = (*vecArcSegments)[i];
                  (*vecArcSegments)[i] = (*vecArcSegments)[n-1-i];
                  (*vecArcSegments)[n-1-i] = tmp;
                }
                continue;
              }
              
              if(!extraUp) {           // extraUp
                extraUp = true;        
                upwardSearch = true;   // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl - 1;
                }
                continue;
              }
            }
          }

          // STEP3: check interior angle
          const double tangent1 = vecLines[4][SS].tangent;
          const double tangent2 = vecLines[4][Sq].tangent;
          double interiorAngle = (tangent2-tangent1)*180/PI;
          if(interiorAngle < 0) interiorAngle = -interiorAngle;
          interiorAngle = 180 - interiorAngle;

          if((interiorAngle <= 180)&&(interiorAngle >= 135)) {
            double estX, estY, estR2;
            vecArcSegments->push_back(Sq);  // store Sq in linelist
            // estimate circle parameters
            if(!estimateCircle(*vecArcSegments, 4, estX, estY, estR2)) {
              vecArcSegments->pop_back();   // circle estimation failed
            } else {
              // STEP4: check tangents
              const double deltaX1 = estX - vecLines[4][SS].mid.x;
              const double deltaY1 = estY - vecLines[4][SS].mid.y;
              double estTheta1 = atan2(deltaX1, deltaY1);
              if((deltaX1 < 0)&&(deltaY1 >= 0)) estTheta1 += PI;
              if((deltaX1 < 0)&&(deltaY1 < 0)) estTheta1 += PI;
  
              const double deltaX2 = estX - vecLines[4][Sq].mid.x;
              const double deltaY2 = estY - vecLines[4][Sq].mid.y;
              double estTheta2 = atan2(deltaX2, deltaY2);
              if((deltaX2 < 0)&&(deltaY2 >= 0)) estTheta2 += PI;
              if((deltaX2 < 0)&&(deltaY2 < 0)) estTheta2 += PI;
  
              if(((estTheta1-tangent1)*180/PI > par.maxLineTangentError)
              ||((estTheta1-tangent1)*180/PI < -par.maxLineTangentError)
              ||((estTheta2-tangent2)*180/PI > par.maxLineTangentError)
              ||((estTheta2-tangent2)*180/PI < -par.maxLineTangentError)) {
                // tangent estimation error exceeded maximum
                vecArcSegments->pop_back();
              } else {
                SS = Sq;    // set Sq as new target segment SS
                Sl = Sq;    // last fitting candidate
              }
            }
          }

          if(upwardSearch) {
            Sq--;
          } else {
            Sq++;
          }
        }


      // CHECK ARC EXTRACTION CONDITIONS
        const int nSegments = static_cast<int>(vecArcSegments->size());
        
        int x = 0;
        int y = 0;
        int r2 = 0;
        double estX, estY, estR2;
        if(estimateCircle(*vecArcSegments, 4, estX, estY, estR2)) {
          x = static_cast<int>(floor(estX + 0.5));
          y = static_cast<int>(floor(estY + 0.5));
          r2 = static_cast<int>(floor(estR2 + 0.5));
        } else {
          r2 = 0;   // estimation failed
        }

        // radius must be greater than 10 pixel for extraction
        // at least 2 segments are neccessary for circle estimation
        if((r2 >= 100)&&(nSegments >= 2)) {

      // EXTRACT ARC
          // register grouped segments as extracted arc
          ellArcEntry newarc;
          // start/end points

          // TODO: fix wrong segment order
          for(int j=0; j<nSegments/2; j++) {
            int swap = (*vecArcSegments)[j];
            (*vecArcSegments)[j] = (*vecArcSegments)[nSegments-1-j];
            (*vecArcSegments)[nSegments-1-j] = swap;
          }

          const int x1 = vecLines[4][(*vecArcSegments)[0]].start.x;
          const int y1 = vecLines[4][(*vecArcSegments)[0]].start.y;
          const int x2 = vecLines[4][(*vecArcSegments)[nSegments-1]].end.x;
          const int y2 = vecLines[4][(*vecArcSegments)[nSegments-1]].end.y;

          // transform coordinates (diagonal -> horizontal)
          const int X1 = x1 + y1;
          const int Y1 = imageWidth - 1 - x1 + y1;
          const int Y2 = imageWidth - 1 - x2 + y2;
          const int X = x + y;

          bool reverse = false;
          // check direction and group
          if(X <= X1) {
            newarc.group = 4; // bottom right
            if(Y1 > Y2) {
              reverse = true;
            }
          } else {
            newarc.group = 8; // top left
            if(Y1 < Y2) {
              reverse = true;
            }
          }
          
          if(reverse) {     // reverse idxlist
            int i;
            for(i=0; i<nSegments/2; i++) {
              int swap = (*vecArcSegments)[i];
              (*vecArcSegments)[i] = (*vecArcSegments)[nSegments-1-i];
              (*vecArcSegments)[nSegments-1-i] = swap;
            }
            // start/end points
            newarc.start.x = x2;
            newarc.start.y = y2;
            newarc.end.x = x1;
            newarc.end.y = y1;
          } else {
            newarc.start.x = x1;
            newarc.start.y = y1;
            newarc.end.x = x2;
            newarc.end.y = y2;
          }

          // first/last line vectors
          if(newarc.group == 4) {
            newarc.firstVec = vecLines[4][(*vecArcSegments)[0]].end 
                            - vecLines[4][(*vecArcSegments)[0]].start;
            newarc.lastVec = vecLines[4][(*vecArcSegments)[nSegments-1]].end 
                           - vecLines[4][(*vecArcSegments)[nSegments-1]].start;
          } else {
            newarc.firstVec = vecLines[4][(*vecArcSegments)[0]].start
                            - vecLines[4][(*vecArcSegments)[0]].end;
            newarc.lastVec = vecLines[4][(*vecArcSegments)[nSegments-1]].start
                           - vecLines[4][(*vecArcSegments)[nSegments-1]].end;
          }
          
          // midpoint
          newarc.mid.x = x;
          newarc.mid.y = y;
          // other values
          newarc.r2 = r2;
          newarc.used = 0;
          // line index list
          newarc.lineIdxList = vecArcSegments;
          
          // store data
          vecEllArcs[newarc.group].push_back(newarc);
          vecEllArcs[0].push_back(newarc);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            vecLines[4][(*vecArcSegments)[i]].used++;
          }
        } else {
          delete vecArcSegments;
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(vecLines[4][Si].used == 0) break;
        }
      }
      while(Si < segmCount);    // extract next arc if possible
    }
  }






/////////////////////////////////////////////////////////////////////

void fastEllipseExtraction::extractEllExtArcs(const int grp) {

  clearEllExtArcList(grp);

  // get parameters
  const parameters& par = getParameters();

  // extended group classification for ellipses (XeCG = eXtended elliptic Circle Groups)
  const int XeCG[9] = { 0, 1,3,2,4, 1,3,2,4 };
  const int XXXeEG[9][3] = {{0,0,0}, {8,1,2},{1,2,3},{2,3,4},{3,4,5}, {4,5,6},{5,6,7},{6,7,8},{7,8,1}};

  const int grpA = XXXeEG[grp][0];
  const int grpB = XXXeEG[grp][1];
  const int grpC = XXXeEG[grp][2];
  const std::vector<lineEntry> &linesA = getLineList(XeCG[grpA]);
  const std::vector<lineEntry> &linesC = getLineList(XeCG[grpC]);
  const std::vector<ellArcEntry> &arcsA = vecEllArcs[grpA];
  const std::vector<ellArcEntry> &arcsB = vecEllArcs[grpB];
  const std::vector<ellArcEntry> &arcsC = vecEllArcs[grpC];
  const int nArcsA = arcsA.size();
  const int nArcsB = arcsB.size();
  const int nArcsC = arcsC.size();

  /////////////////////
  /////// ARC B ///////
  /////////////////////
  int b;
  for(b=0; b<nArcsB; b++)
  {
    // new 2nd arc (B)
    std::vector<int> listA;
    std::vector<int> listC;
    
    /////////////////////
    /////// ARC A ///////
    /////////////////////
    int a;
    for(a=0; a<nArcsA; a++)
    {
      //////////////////
      // check window
      const int Xdiff = arcsB[b].start.x - arcsA[a].end.x;
      const int Ydiff = arcsB[b].start.y - arcsA[a].end.y;
      if((Xdiff > par.maxArcGap)||(Xdiff < -par.maxArcGap)
       ||(Ydiff > par.maxArcGap)||(Ydiff < -par.maxArcGap)) {
        continue;
      }

      // calculate startpoint distance ratio
      const int ABx = arcsB[b].start.x - arcsA[a].start.x;
      const int ABy = arcsB[b].start.y - arcsA[a].start.y;
      const double ABlen = sqrt(static_cast<double>(ABx*ABx + ABy*ABy));
      const int Ax = arcsA[a].end.x - arcsA[a].start.x;
      const int Ay = arcsA[a].end.y - arcsA[a].start.y;
      const double Alen = sqrt(static_cast<double>(Ax*Ax + Ay*Ay));
      const double ratio = ABlen / Alen;
      if(ratio < par.minArcDistanceRatio) {
        continue;
      }
      
      const double phi = acos(static_cast<double>(Ax*ABx + Ay*ABy)/sqrt(static_cast<double>((Ax*Ax+Ay*Ay)*(ABx*ABx+ABy*ABy))))*180/PI;
      if(phi > par.maxArcDistanceAngle) {
        continue;
      }

      if((Xdiff > par.minGAPangleDistance)||(Xdiff < -par.minGAPangleDistance)
       ||(Ydiff > par.minGAPangleDistance)||(Ydiff < -par.minGAPangleDistance)) {
        //////////////////////////
        // check GAP angles
        const int LAx = arcsA[a].lastVec.x;
        const int LAy = arcsA[a].lastVec.y;
        const int LBx = arcsB[b].firstVec.x;
        const int LBy = arcsB[b].firstVec.y;
        const double GAPangleA = acos(static_cast<double>(LAx*Xdiff + LAy*Ydiff)/sqrt(static_cast<double>((LAx*LAx+LAy*LAy)*(Xdiff*Xdiff+Ydiff*Ydiff))))*180/PI;
        const double GAPangleB = acos(static_cast<double>(LBx*Xdiff + LBy*Ydiff)/sqrt(static_cast<double>((LBx*LBx+LBy*LBy)*(Xdiff*Xdiff+Ydiff*Ydiff))))*180/PI;

        if((GAPangleA > par.maxGAPangle)||(GAPangleB > par.maxGAPangle)) {
          if((GAPangleA > 180-par.maxGAPangle)||(GAPangleB > 180-par.maxGAPangle)) {
            const int dx = arcsA[a].end.x - arcsB[b].mid.x;
            const int dy = arcsA[a].end.y - arcsB[b].mid.y;
            const double d = sqrt(dx*dx + dy*dy) - sqrt(arcsB[b].r2);
            if((d > par.maxArcOverlapGap)||(d < -par.maxArcOverlapGap)) {
              continue;
            }
          } else {
            continue;
          }
        }
      }
      ///////////////////////////////
      // check interior angles (A,B)
      if(checkInteriorAngles(a, grpA, b, grpB) <= par.maxIntAngleMismatches) {
        listA.push_back(a);
      }
    }//arcA

    const int sizeA = listA.size();
    if(sizeA == 0) continue;  // check next B arc

    /////////////////////
    /////// ARC C ///////
    /////////////////////
    int c;
    for(c=0; c<nArcsC; c++)
    {
      //////////////////
      // check window
      int Xdiff = arcsC[c].start.x - arcsB[b].end.x;
      int Ydiff = arcsC[c].start.y - arcsB[b].end.y;
      if((Xdiff > par.maxArcGap)||(Xdiff < -par.maxArcGap)
       ||(Ydiff > par.maxArcGap)||(Ydiff < -par.maxArcGap)) {
        continue;
      }

      // calculate startpoint distance ratio
      const int BCx = arcsC[c].start.x - arcsB[b].start.x;
      const int BCy = arcsC[c].start.y - arcsB[b].start.y;
      const double BClen = sqrt(static_cast<double>(BCx*BCx + BCy*BCy));
      const int Bx = arcsB[b].end.x - arcsB[b].start.x;
      const int By = arcsB[b].end.y - arcsB[b].start.y;
      const double Blen = sqrt(static_cast<double>(Bx*Bx + By*By));
      const double ratio = BClen / Blen;
      if(ratio < par.minArcDistanceRatio) {
        continue;
      }
      const double phi = acos(static_cast<double>(Bx*BCx + By*BCy)/sqrt(static_cast<double>((Bx*Bx+By*By)*(BCx*BCx+BCy*BCy))))*180/PI;
      if(phi > par.maxArcDistanceAngle) {
        continue;
      }

      if((Xdiff > par.minGAPangleDistance)||(Xdiff < -par.minGAPangleDistance)
       ||(Ydiff > par.minGAPangleDistance)||(Ydiff < -par.minGAPangleDistance)) {
        //////////////////////////
        // check GAP angles
        const int LBx = arcsB[b].lastVec.x;
        const int LBy = arcsB[b].lastVec.y;
        const int LCx = arcsC[c].firstVec.x;
        const int LCy = arcsC[c].firstVec.y;
        const double GAPangleB = acos(static_cast<double>(LBx*Xdiff + LBy*Ydiff)/sqrt(static_cast<double>((LBx*LBx+LBy*LBy)*(Xdiff*Xdiff+Ydiff*Ydiff))))*180/PI;
        const double GAPangleC = acos(static_cast<double>(LCx*Xdiff + LCy*Ydiff)/sqrt(static_cast<double>((LCx*LCx+LCy*LCy)*(Xdiff*Xdiff+Ydiff*Ydiff))))*180/PI;

        if((GAPangleB > par.maxGAPangle)||(GAPangleC > par.maxGAPangle)) {
          if((GAPangleB > 180-par.maxGAPangle)||(GAPangleC > 180-par.maxGAPangle)) {
            const int dx = arcsC[c].start.x - arcsB[b].mid.x;
            const int dy = arcsC[c].start.y - arcsB[b].mid.y;
            const double d = sqrt(dx*dx + dy*dy) - sqrt(arcsB[b].r2);
            if((d > par.maxArcOverlapGap)||(d < -par.maxArcOverlapGap)) {
              continue;
            }
          } else {
            continue;
          }
        }
      }
      ///////////////////////////////
      // check interior angles (B,C)
      if(checkInteriorAngles(b, grpB, c, grpC) <= par.maxIntAngleMismatches) {
        listC.push_back(c);
      }
    }//arcC

    const int sizeC = listC.size();
    if(sizeC == 0) continue;  // check next B arc

    int i;
    for(i=0; i<sizeA; i++) {
      int a = listA[i];

      int j;
      for(j=0; j<sizeC; j++) {
        int c = listC[j];

        ////////////////////////////////
        // check interior angles (A,C)      
        if(checkInteriorAngles(a, grpA, c, grpC) <= par.maxIntAngleMismatches) {

          /////////////////////
          // estimate ellipse
          double estX, estY, estA, estB, estT;
          if(!estimateEllipse(arcsA[a], arcsB[b], arcsC[c], estX, estY, estA, estB, estT)) {
            continue;
          }

          // prepare ellExtArc
          ellExtArcEntry entry;
          entry.used = 0;
          entry.group = grpB;
          entry.arcIdx[0] = a;
          entry.arcIdx[1] = b;
          entry.arcIdx[2] = c;
          entry.x = estX;
          entry.y = estY;
          entry.a = estA;
          entry.b = estB;
          entry.t = estT;

          ///////////////////
          // check tangents
          const int mismatches = checkEllExtArcTangents(entry, estX, estY, estA, estB, estT);
          if(mismatches <= par.maxTangentErrors) {
          
            ////////////////////////////////////////
            // calculate line beam center distance
            double distance;
            const std::vector<int> &LidxA = *arcsA[a].lineIdxList;
            const std::vector<int> &LidxC = *arcsC[c].lineIdxList;
            const int nC = LidxC.size();
            if((lineBeamCenterDistance(linesA[LidxA[0]], linesC[LidxC[nC-1]], entry, distance, estX, estY)
            &&(distance < par.maxLBcenterMismatch)
            &&(distance > -par.maxLBcenterMismatch))
            ||(par.ellExtArcExtractionStage < 3)) {
              // store ellExtArc
              vecEllExtArcs[grpB].push_back(entry);
              vecEllExtArcs[0].push_back(entry);
            }

          }//tangent test successful
        }//angle test successful
      }//arcC* (j)
    }//arcA* (i)
  }//arcB
}


///////////////////////////////////////////////////////////////////////

void fastEllipseExtraction::extractEllipses() {

  clearEllipseList();

  // get parameters
  const parameters& par = getParameters();

  const int XXXeEG[9][3] = {{0,0,0}, {8,1,2},{1,2,3},{2,3,4},{3,4,5}, {4,5,6},{5,6,7},{6,7,8},{7,8,1}};
  std::vector<ellExtArcEntry> &extarcs = vecEllExtArcs[0];
  const int nExtArcs = extarcs.size();
  int i, j;

  //////////////////////////////////////////
  // try merging of overlapping extarcs

  for(i=0; i<nExtArcs; i++)
  {
    if(extarcs[i].used > 0) continue;
    
    // new estimations only once for each group
    int grpEstimations[9] = {0};
    
    // prepare ellipse entry
    ellipseEntry entry;
    entry.x = extarcs[i].x;
    entry.y = extarcs[i].y;
    entry.a = extarcs[i].a;
    entry.b = extarcs[i].b;
    entry.t = extarcs[i].t;
    entry.coverage = 0;
    entry.mergedArcs = new std::vector<int>;

    std::vector<int> &mergeList = (*entry.mergedArcs);
    mergeList.push_back(i);
    extarcs[i].used++;

    // step through actual mergeList
    for(j=0; j<static_cast<int>(mergeList.size()); j++) {

      const int targetIdx = mergeList[j];
      const int targetGrp = extarcs[targetIdx].group;
      
      // step through the three base arcs
      int k;
      for(k=0; k<3; k++) {
        const int targetArcIdx = extarcs[targetIdx].arcIdx[k];
        const int targetArcGrp = XXXeEG[targetGrp][k];

        // search other extArcs
        int o;
        for(o=0; o<nExtArcs; o++) {
          if(o == i) continue;
          const int candGrp = extarcs[o].group;

          // compare base arcs
          int l;
          for(l=0; l<3; l++) {
            const int candArcIdx = extarcs[o].arcIdx[l];
            const int candArcGrp = XXXeEG[candGrp][l];
            
            if((candArcGrp == targetArcGrp)&&(candArcIdx == targetArcIdx)) {
              // MATCH
              if(extarcs[o].used == 0) {

                // insert candArc temporarily in mergeList
                mergeList.push_back(o);
                bool keep = false;

                // estimate new ellipse for first new group arcs
                bool estimationOK = true;
                bool newEstimation = false;
                double estX, estY, estA, estB, estT;

                if(grpEstimations[candGrp] < 1) {
                  estimationOK = estimateEllipse(entry.mergedArcs, estX, estY, estA, estB, estT);
                  grpEstimations[candGrp]++;
                  newEstimation = true;
                } else {
                  estX = entry.x;
                  estY = entry.y;
                  estA = entry.a;
                  estB = entry.b;
                  estT = entry.t;
                }

                if(estimationOK) {
                  const int tangentErrors = checkEllExtArcTangents(extarcs[o], estX, estY, estA, estB, estT);
                  const double pointMismatch = ellipsePointMismatch(extarcs[o], estX, estY, estA, estB, estT);
                  const double centerDistance = linePointDistance(estX, estY, extarcs[o].ix, extarcs[o].iy, extarcs[o].slopeLB);
                  
                  if((tangentErrors <= par.maxTangentErrors)
                  &&(pointMismatch < par.maxExtArcMismatch)
                  &&(centerDistance < par.maxLBcenterMismatch)
                  &&(centerDistance > -par.maxLBcenterMismatch)) {
                    keep = true;
                    extarcs[o].used++;
                  } else {
                    // something is wrong with this arc - ignore it in next passes
                    extarcs[o].used++;
                  }
                }//estimation
                
                if(keep) {
                  if(newEstimation) {
                    entry.x = estX;
                    entry.y = estY;
                    entry.a = estA;
                    entry.b = estB;
                    entry.t = estT;
                  }
                } else {  // !keep
                  mergeList.pop_back();
                  if(newEstimation) {
                    grpEstimations[candGrp]--;
                  }
                }
                
              }//used==0
              break;
            }// MATCH
          }//l (candBaseArcs)
        }//o (candExtArcs)
      }//k (targetBaseArcs)
    }//j (mergedExtArcs)


    const int nMergedArcs = mergeList.size();
    if(nMergedArcs > 1) {
      // estimate new ellipse
      double estX, estY, estA, estB, estT;
      if(estimateEllipse(entry.mergedArcs, estX, estY, estA, estB, estT)) {
        entry.x = estX;
        entry.y = estY;
        entry.a = estA;
        entry.b = estB;
        entry.t = estT;
        calculateEllipseCoverage(entry);
      }
    }

    if(entry.coverage > par.minCoverage) {
      // store ellipse
      vecEllipses.push_back(entry);
    } else {
      // reset used count
      for(int m=0; m<nMergedArcs; m++) {
        extarcs[mergeList[m]].used = 0;
      }
      delete entry.mergedArcs;
    }

  }//i


  ///////////////////////////////////////
  // ellipse search for unused extarc
  
  std::vector<ellipseEntry> &ellipses = vecEllipses;
  const int nCurrentEllipses = ellipses.size();
  for(i=0; i<nCurrentEllipses; i++) { // target index

    ellipseEntry &ellipse = vecEllipses[i];
    
    for(j=0; j<nExtArcs; j++) {     // candidate index

      ellExtArcEntry &candArc = vecEllExtArcs[0][j];
      if(candArc.used > 0) continue;

      // check extarc/ellipse matching
      const int tangentErrors = checkEllExtArcTangents(candArc, ellipse.x, ellipse.y, ellipse.a, ellipse.b, ellipse.t);
      const double pointMismatch = ellipsePointMismatch(candArc, ellipse.x, ellipse.y, ellipse.a, ellipse.b, ellipse.t);
      //const double centerDistance = linePointDistance(ellipse.x, ellipse.y, candArc.ix, candArc.iy, candArc.slopeLB);
      
      if((tangentErrors <= par.maxTangentErrors)
      &&(pointMismatch < par.maxExtArcMismatch)) {
      //&&(centerDistance < par.maxLBcenterMismatch)
      //&&(centerDistance > -par.maxLBcenterMismatch)) {
        // estimate new ellipse
        (*ellipse.mergedArcs).push_back(j);
        double estX, estY, estA, estB, estT;
        if(estimateEllipse(ellipse.mergedArcs, estX, estY, estA, estB, estT)) {
          ellipse.x = estX;
          ellipse.y = estY;
          ellipse.a = estA;
          ellipse.b = estB;
          ellipse.t = estT;
          calculateEllipseCoverage(ellipse);
          candArc.used++;
        }
      }
    
    }//j (candArc)
  }//i (targetEllipse)



  /////////////////////////////////////////
  // try cross merging of unused extarcs

  for(i=0; i<nExtArcs; i++) {     // target index

    const ellExtArcEntry &targetArc = vecEllExtArcs[0][i];
    if(targetArc.used > 0) continue;
    
    // prepare ellipse entry
    ellipseEntry entry;
    entry.x = targetArc.x;
    entry.y = targetArc.y;
    entry.a = targetArc.a;
    entry.b = targetArc.b;
    entry.t = targetArc.t;
    entry.coverage = 0;
    entry.mergedArcs = new std::vector<int>;
    std::vector<int> &mergeList = (*entry.mergedArcs);
    mergeList.push_back(i);
    
    for(j=0; j<nExtArcs; j++) {     // candidate index
      if(i==j) continue;
      const ellExtArcEntry &candArc = vecEllExtArcs[0][j];
      if(candArc.used > 0) continue;

      // compare parameters
      const double diffX = targetArc.x - candArc.x;
      const double diffY = targetArc.y - candArc.y;
      double ratioA, ratioB;
      if(targetArc.a > candArc.a) {
        ratioA = candArc.a / targetArc.a;
      } else {
        ratioA = targetArc.a / candArc.a;
      }

      if(targetArc.b > candArc.b) {
        ratioB = candArc.b / targetArc.b;
      } else {
        ratioB = targetArc.b / candArc.b;
      }
      
      if(((diffX < par.maxCenterMismatch)&&(diffX > -par.maxCenterMismatch))
      &&((diffY < par.maxCenterMismatch)&&(diffY > -par.maxCenterMismatch))
      &&(ratioA > par.minRadiusMatchRatio)
      &&(ratioB > par.minRadiusMatchRatio)) {
        // MATCH
        mergeList.push_back(j);
      }
    }//j (candArc)

    const int nMergedArcs = mergeList.size();

    if(nMergedArcs > 1) {
      // estimate new ellipse
      double estX, estY, estA, estB, estT;
      if(estimateEllipse(entry.mergedArcs, estX, estY, estA, estB, estT)) {
        entry.x = estX;
        entry.y = estY;
        entry.a = estA;
        entry.b = estB;
        entry.t = estT;
        calculateEllipseCoverage(entry);
      }
    } else {
      // sincle arc coverage
      calculateEllipseCoverage(entry);
    }

    if(entry.coverage > par.minCoverage) {
      // store ellipse
      vecEllipses.push_back(entry);
      // mark used extarcs
      for(int m=0; m<nMergedArcs; m++) {
        vecEllExtArcs[0][mergeList[m]].used++;
      }
    }
    
  }//i (targetArc)
}


///////////////////////////////////////////////////////////////////////

int fastEllipseExtraction::checkInteriorAngles(int a, int grpA, int b, int grpB) {

  // get parameters
  const parameters& par = getParameters();

  const int XeCG[9] = { 0, 1,3,2,4, 1,3,2,4 };

  const std::vector<lineEntry> &linesA = getLineList(XeCG[grpA]);
  const std::vector<int> &idxA = *vecEllArcs[grpA][a].lineIdxList;
  const int nIdxA = idxA.size();

  const std::vector<lineEntry> &linesB = getLineList(XeCG[grpB]);
  const std::vector<int> &idxB = *vecEllArcs[grpB][b].lineIdxList;
  const int nIdxB = idxB.size();

  ////////////////////////////////////////////
  // check interior angles between all lines
  int mismatches = 0;
  
  int iB;
  for(iB=0; iB<nIdxB; iB++) 
  {
    int iA;
    for(iA=0; iA<nIdxA; iA++)   // inner loop: first arc
    {
      // calculate line vectors
      int AstartX, AstartY, AendX, AendY;
      int BstartX, BstartY, BendX, BendY;
      if(grpA <= 4) {
        AstartX = linesA[idxA[iA]].start.x;
        AstartY = linesA[idxA[iA]].start.y;
        AendX = linesA[idxA[iA]].end.x;
        AendY = linesA[idxA[iA]].end.y;
      } else {
        AstartX = linesA[idxA[iA]].end.x;
        AstartY = linesA[idxA[iA]].end.y;
        AendX = linesA[idxA[iA]].start.x;
        AendY = linesA[idxA[iA]].start.y;
      }
      if(grpB <= 4) {
        BstartX = linesB[idxB[iB]].start.x;
        BstartY = linesB[idxB[iB]].start.y;
        BendX = linesB[idxB[iB]].end.x;
        BendY = linesB[idxB[iB]].end.y;
      } else {
        BstartX = linesB[idxB[iB]].end.x;
        BstartY = linesB[idxB[iB]].end.y;
        BendX = linesB[idxB[iB]].start.x;
        BendY = linesB[idxB[iB]].start.y;
      }

      // calculate vectors A and B
      const int Ax = AendX - AstartX;
      const int Ay = -(AendY - AstartY);  // invert y-axis (to normal cartesian)
      const int Bx = BendX - BstartX;
      const int By = -(BendY - BstartY);  // invert y-axis (to normal cartesian)

      // calculate normal vectors AN and BN
      const int ANx = Ay;
      const int ANy = -Ax;
      const int BNx = By;
      const int BNy = -Bx;

      // calculate vectors AB and BA
      const int ABx = BstartX - AstartX;
      const int ABy = -(BstartY - AstartY);  // invert y-axis (to normal cartesian)
      const int BAx = -ABx;
      const int BAy = -ABy;

      // calculate interior angles
      const double TH1 = acos((ABx*ANx + ABy*ANy) / sqrt(static_cast<double>((ABx*ABx + ABy*ABy) * (ANx*ANx + ANy*ANy))));
      const double TH2 = acos((BAx*BNx + BAy*BNy) / sqrt(static_cast<double>((BAx*BAx + BAy*BAy) * (BNx*BNx + BNy*BNy))));

      const double Alen = sqrt(static_cast<double>(Ax*Ax + Ay*Ay));
      const double ABlen = sqrt(static_cast<double>(ABx*ABx + ABy*ABy));
      const double ABdivA = ABlen/Alen;
      if(ABdivA < 1.0) {
        continue;
      }

      // check max value
      if((TH1 > PI/2) || (TH2 > PI/2)) {
        mismatches++;
      }

      if(mismatches > par.maxIntAngleMismatches) {
        break;
      }
    }//iA
    if(mismatches > par.maxIntAngleMismatches) {
      break;
    }
  }//iB
  
  return mismatches;
}


///////////////////////////////////////////////////////////////////////

double fastEllipseExtraction::lineBeamIsecCenterDistance(const ellExtArcEntry &extarcA,
                                                         const ellExtArcEntry &extarcB,
                                                         double estX, double estY) {
  const double x1 = extarcA.ix;
  const double x2 = extarcB.ix;
  const double y1 = extarcA.iy;
  const double y2 = extarcB.iy;
  const double m1 = extarcA.slopeLB;
  const double m2 = extarcB.slopeLB;
  const double det0 = -m1 + m2;
  const double det1 = -(y2-x2*m2) + y1-x1*m1;
  const double det2 = (y1-x1*m1)*m2 - m1*(y2-x2*m2);
            
  if(det0 == 0) return -1;  // parallel lines
  
  // intersection point of line beams
  const double ix = det1 / det0;
  const double iy = det2 / det0;
  
  // center distance
  return sqrt((ix-estX)*(ix-estX) + (iy-estY)*(iy-estY));
}


///////////////////////////////////////////////////////////////////////

double fastEllipseExtraction::linePointDistance(double testPtX, double testPtY, 
                                                double linePtX, double linePtY,
                                                double lineSlope) {
  const double C = linePtY - linePtX * lineSlope;
  double u = 1 / sqrt(lineSlope * lineSlope + 1);   // scale factor for HESSEsche normalform
  if(C > 0) u = -u;
  return (lineSlope * testPtX - testPtY + C) * u;
}


///////////////////////////////////////////////////////////////////////

double fastEllipseExtraction::ellipsePointMismatch(ellExtArcEntry &extarc,
                                                   double estX, 
                                                   double estY, 
                                                   double estA, 
                                                   double estB, 
                                                   double estT) {
  
  const int XXXeEG[9][3] = {{0,0,0}, 
                            {8,1,2},
                            {1,2,3},
                            {2,3,4},
                            {3,4,5}, 
                            {4,5,6},
                            {5,6,7},
                            {6,7,8},
                            {7,8,1}};
  double extArcMismatch = 0;

  // correct orientation angle
  if(estT*180/PI < -89.5) {
    estT = -estT;
  }
  const double phi = PI/2 - estT;
  double x, y, X, Y, xterm, yterm;
  double cosa, sina;

  int a;
  for(a=0; a<3; a++) {
    const int idx = extarc.arcIdx[a];
    const int grp = XXXeEG[extarc.group][a];
    
    const ellArcEntry &arc = vecEllArcs[grp][idx];

    x = static_cast<double>(arc.start.x) - estX;
    y = estY - static_cast<double>(arc.start.y);
    // rotate coordinate system
    sincos(phi, sina, cosa);
    X =  x*cosa + y*sina;
    Y = -x*sina + y*cosa;
    // ellipse equation
    xterm = X / estA;
    yterm = Y / estB;
    const double startMismatch = fabs(xterm*xterm + yterm*yterm - 1);
    if(startMismatch > extArcMismatch) {
      extArcMismatch = startMismatch;
    }

    x = static_cast<double>(arc.end.x) - estX;
    y = estY - static_cast<double>(arc.end.y);
    // rotate coordinate system
    X = x*cos(phi) + y*sin(phi);
    Y = -x*sin(phi) + y*cos(phi);
    // ellipse equation
    xterm = X / estA;
    yterm = Y / estB;
    const double endMismatch = fabs(xterm*xterm + yterm*yterm - 1);
    if(endMismatch > extArcMismatch) {
      extArcMismatch = endMismatch;
    }
  }
  
  return extArcMismatch;
}


///////////////////////////////////////////////////////////////////////

int fastEllipseExtraction::checkEllExtArcTangents(const ellExtArcEntry &extarc,
                                                  double estX,
                                                  double estY,
                                                  double estA,
                                                  double estB,
                                                  double estT) {
  // get parameters
  const parameters& par = getParameters();

  const int XeCG[9] = { 0, 1,3,2,4, 1,3,2,4 };
  const int XXXeEG[9][3] = {{0,0,0},
                            {8,1,2},
                            {1,2,3},
                            {2,3,4},
                            {3,4,5}, 
                            {4,5,6},
                            {5,6,7},
                            {6,7,8},
                            {7,8,1}};

  // correct orientation angle
  if(estT*180/PI < -89.5) {
    estT = -estT;
  }
  const double phi = PI/2 - estT;
  
  double ellTang;
  double lineTang;
  double tangErr;
  double sinp, cosp;
  int mismatches = 0;
  int k;
  int l;
  for(k=0; k<3; k++)  // check all lines of the three arcs
  {
    const int grp = XXXeEG[extarc.group][k];    // grpA, grpB, grpC
    
    // references to lists
    const std::vector<lineEntry> &lines = getLineList(XeCG[grp]);
    const std::vector<ellArcEntry> &arcs = vecEllArcs[grp];
    const std::vector<int> &Lidx = *arcs[extarc.arcIdx[k]].lineIdxList;
    const int nIdx = Lidx.size();

    for(l=0; l<nIdx; l++)
    {
      lineTang = lines[Lidx[l]].tangent;

      // wrong grpII tangent direction 
      //if(((grp == 3)||(grp == 7))&&(lineTang > PI/2)) lineTang -= PI;

      
      // rotate coordinate system
      const double x = static_cast<double>(lines[Lidx[l]].mid.x - estX);
      const double y = static_cast<double>(estY - lines[Lidx[l]].mid.y);

      sincos(phi, sinp, cosp);
      const double X =  x*cosp + y*sinp;
      const double Y = -x*sinp + y*cosp;
      
      const double BBX = estB*estB*X;
      const double AAY = estA*estA*Y;
      
      //  calculate ellipse tangent and avoid division by zero
      if      ((AAY == 0)&&(BBX >= 0)) {
        ellTang = -PI/2;
      } else if ((AAY == 0)&&(BBX < 0)) {
        ellTang = PI/2;
      } else {
        ellTang = atan(-BBX/AAY) + phi;
      }

      if(ellTang > PI/2) {
        ellTang -= PI;
      }

      // wrong grpII tangent direction 
      if(((grp == 3)||(grp == 7))&&(ellTang < 0)) ellTang += PI;

      tangErr = (ellTang-lineTang)*180/PI;
      
      if     ((tangErr > 178)&&(tangErr < 182)) {
        tangErr -=180;
      } else if((tangErr < -178)&&(tangErr > -182)) {
        tangErr +=180;
      }
      
      if((tangErr > par.maxArcTangentError)
        ||(tangErr < -par.maxArcTangentError)) {
        // tangent estimation error exceeded maximum
        if(par.ellExtArcExtractionStage > 1) {
          mismatches++;
        }

        if(mismatches > par.maxTangentErrors) {
          break;
        }
      }
    }//l
    if(mismatches > par.maxTangentErrors) {
      break;
    }
  }//k (basearcs)
  
  return mismatches;
} 


///////////////////////////////////////////////////////////////////////

bool fastEllipseExtraction::lineBeamCenterDistance(const lineEntry &firstLine,
                                                   const lineEntry &lastLine,
                                                   ellExtArcEntry &extarc,
                                                   double &distance, 
                                                   const double estX, 
                                                   const double estY) {
  const double x1 = firstLine.mid.x;
  const double y1 = firstLine.mid.y;
  const double x2 = lastLine.mid.x;
  const double y2 = lastLine.mid.y;
  const double dx1 = firstLine.end.x - firstLine.start.x;
  const double dy1 = firstLine.end.y - firstLine.start.y;
  const double dx2 = lastLine.end.x - lastLine.start.x;
  const double dy2 = lastLine.end.y - lastLine.start.y;

  double m1, m2;
  if((dx1 == 0)&&(dy1 > 0)) m1 = INFINITE_SLOPE;
  else if((dx1 == 0)&&(dy1 < 0)) m1 = -INFINITE_SLOPE;
  else m1 = dy1/dx1;

  if((dx2 == 0)&&(dy2 > 0)) m2 = INFINITE_SLOPE;
  else if((dx2 == 0)&&(dy2 < 0)) m2 = -INFINITE_SLOPE;
  else m2 = dy2/dx2;

  // calculate midpoint of endpoint connection
  const double cx = (x1 + x2) / 2;
  const double cy = (y1 + y2) / 2;
    
  // calculate intersection point
  const double det0 = -m1 + m2;
  const double det1 = -(y2-x2*m2) + y1-x1*m1;
  const double det2 = (y1-x1*m1)*m2 - m1*(y2-x2*m2);

  if(det0 == 0) {   // parallel lines
    return false;
  }
  
  const double ix = det1 / det0;
  const double iy = det2 / det0;

  // calculate line beam
  double m;
  const double dx = cx - ix;
  const double dy = cy - iy;
  if((dx == 0)&&(dy > 0)) m = INFINITE_SLOPE;
  else if((dx == 0)&&(dy < 0)) m = -INFINITE_SLOPE;
  else m = dy/dx;
  
  // calculate center distance
  const double C = iy - ix*m;
  double u = 1 / sqrt(m*m + 1);   // scale factor for HESSEsche normalform
  if(C > 0) u = -u;

  // results  
  extarc.ix = ix;
  extarc.iy = iy;
  extarc.slopeLB = m;
  distance = (m*estX - estY + C) * u;
  return true;
}


///////////////////////////////////////////////////////////////////////

void fastEllipseExtraction::calculateEllipseCoverage(ellipseEntry &ellipse)
{
  const int XeCG[9] = { 0, 1,3,2,4, 1,3,2,4 };
  const int XXXeEG[9][3] = {{0,0,0}, {8,1,2},{1,2,3},{2,3,4},{3,4,5}, {4,5,6},{5,6,7},{6,7,8},{7,8,1}};
  const std::vector<int> &mergeList = *ellipse.mergedArcs;
  const int nExtArcs = mergeList.size();
  
  int pixels = 0;

  // RESET OLD USED COUNT
    
  // iterate over all merged extarcs
  int e;
  for(e=0; e<nExtArcs; e++) {
    const ellExtArcEntry &extarc = vecEllExtArcs[0][mergeList[e]];
    
    // iterate over all arcs
    for(int a=0; a<3; a++) {
      const int arcGrp = XXXeEG[extarc.group][a];
      const ellArcEntry &arc = vecEllArcs[arcGrp][extarc.arcIdx[a]];
      const std::vector<int> &lineList = *arc.lineIdxList;
      const int nLines = lineList.size();
      std::vector<lineEntry> &lines = getLineList(XeCG[arcGrp]);
      
      // iterate over all lines
      for(int l=0; l<nLines; l++) {
        // Reset used-count of extracted lines
        lines[lineList[l]].used = 0;
      }//lines
    }//arcs
  }//extarcs


  // COUNT EXTRACTED EDGE PIXELS      

  // iterate over all merged extarcs
  for(e=0; e<nExtArcs; e++) {
    const ellExtArcEntry &extarc = vecEllExtArcs[0][mergeList[e]];
    
    // iterate over all arcs
    for(int a=0; a<3; a++) {
      const int arcGrp = XXXeEG[extarc.group][a];
      const int lineGrp = XeCG[arcGrp];
      const ellArcEntry &arc = vecEllArcs[arcGrp][extarc.arcIdx[a]];
      const std::vector<int> &lineList = *arc.lineIdxList;
      const int nLines = lineList.size();
      std::vector<lineEntry> &lines = getLineList(lineGrp);
      
      // iterate over all lines
      for(int l=0; l<nLines; l++) {
        // count edge pixels
        if(lines[lineList[l]].used == 0) {
          pixels += lines[lineList[l]].len;
          lines[lineList[l]].used++;
        }
      }//lines
    }//arcs
  }//extarcs
  
  // approximate ideal ellipse circumference
  const double circumference = PI * ( 1.5 * (ellipse.a + ellipse.b) - sqrt(ellipse.a * ellipse.b));
  
  //const double lambda = (ellipse.a - ellipse.b) / (ellipse.a + ellipse.b);
  //const double temp = (64 - 3*lambda*lambda*lambda*lambda) / (64 - 16*lambda*lambda);
  //const double temp = 1 + (3*lambda*lambda / (10 + sqrt(4- 3*lambda*lambda)));
  //const double circumference = PI * (ellipse.a + ellipse.b) * temp;

  ellipse.coverage = static_cast<double>(pixels) / circumference;
}


/////////////////////////////////////////////////////////////////////

bool fastEllipseExtraction::estimateEllipse(const std::vector<int> *mergedArcs, double &estX, double &estY, double &estA, double &estB, double &estT)
  {
    const int XXXeEG[9][3] = {{0,0,0}, {8,1,2},{1,2,3},{2,3,4},{3,4,5}, {4,5,6},{5,6,7},{6,7,8},{7,8,1}};
    const int XeCG[9] = { 0, 1,3,2,4, 1,3,2,4 };

    std::vector<ellExtArcEntry> &extarcs = vecEllExtArcs[0];
    const int nMerged = mergedArcs->size();
    int m;
    
    // COUNT INPUT PIXELS
    int nPixels = 0;
    
    for(m=0; m<nMerged; m++) 
    {
      int k;
      for(k=0; k<3; k++) 
      {
        const int arcGrp = XXXeEG[extarcs[(*mergedArcs)[m]].group][k];
        const int index = extarcs[(*mergedArcs)[m]].arcIdx[k];
        const ellArcEntry &arc = vecEllArcs[arcGrp][index];

        const std::vector<lineEntry> &lines = getLineList(XeCG[arc.group]);
        const std::vector<int> &lineidx = *arc.lineIdxList;
        const int nLines = lineidx.size();

        int l;
        for(l=0; l<nLines; l++) 
        {
          // get number of line segments and segment group
          const int nSegments = (*lines[lineidx[l]].segmIdxList).size();
          const int group = lines[lineidx[l]].group;
      
          const std::vector<segmEntry>& segmList = getSegmentList(group);
    
          // accumulate all segments
          for(int i=0; i<nSegments; i++) {
            // segment index
            const int idx = (*lines[lineidx[l]].segmIdxList)[i];
        
            // segments length
            nPixels += segmList[idx].len;
          }//i
        }//l
      }//k
    }//m


  // FILL DESIGN MATRIX
    lti::matrix<double> Design(false, nPixels, 6);
    int row = 0;


    for(m=0; m<nMerged; m++) 
    {
      int k;
      for(k=0; k<3; k++) 
      {
        const int arcGrp = XXXeEG[extarcs[(*mergedArcs)[m]].group][k];
        const int index = extarcs[(*mergedArcs)[m]].arcIdx[k];
        const ellArcEntry &arc = vecEllArcs[arcGrp][index];

        const std::vector<lineEntry> &lines = getLineList(XeCG[arc.group]);
        const std::vector<int> &lineidx = *arc.lineIdxList;
        const int nLines = lineidx.size();

        int l;
        for(l=0; l<nLines; l++) 
        {
          // get number of line segments and segment group
          const int nSegments = (*lines[lineidx[l]].segmIdxList).size();
          const int group = lines[lineidx[l]].group;
      
          const std::vector<segmEntry>& segmList = getSegmentList(group);
    
          // accumulate all segments
          for(int i=0; i<nSegments; i++) {
            // segment index
            const int idx = (*lines[lineidx[l]].segmIdxList)[i];
        
            // starting point of segment[i]
            int x = segmList[idx].start.x;
            int y = segmList[idx].start.y;
          
            // segments length
            const int len = segmList[idx].len;
            
            // accumulate all segment pixels
            for(int j=0; j<len; j++) {
    
              // Now fill design matrix
              double data[6] = { x*x, x*y, y*y, x, y, 1.0 };
              lti::vector<double> line(6, data);
              Design.setRow(row, line);
           
              row++;
              
              // set next segment pixel coordinates
              // orientation:  ---x
              //               |
              //               y
              switch(group) {
                case 1: {   // horizontal
                  x++;
                  break;
                }
                case 2: {   // vertical
                  y++;
                  break;
                }
                case 3: {   // diagonal horizontal: \ (starts top left)
                  x++;
                  y++;
                  break;
                }
                case 4: {   // diagonal vertical: / (starts bottom left)
                  x++;
                  y--;
                  break;
                }
              }
            }// next pixel (j)
          }//i
        }//l
      }//k
    }//m

    return directEllipseFitting(Design, estX, estY, estA, estB, estT);
  }


///////////////////////////////////////////////////////////////////////

  bool fastEllipseExtraction::estimateEllipse(const ellArcEntry &arcA, 
                                              const ellArcEntry &arcB, 
                                              const ellArcEntry &arcC, 
                                              double &estX, double &estY, double &estA, double &estB, double &estT)
  {
    const int XeCG[9] = { 0, 1,3,2,4, 1,3,2,4 };
    
    // COUNT INPUT PIXELS
    int nPixels = 0;
    int l;

    // arcA
    const std::vector<lineEntry> &linesA = getLineList(XeCG[arcA.group]);
    const std::vector<int> &lineidxA = *arcA.lineIdxList;
  
    const int nLinesA = lineidxA.size();
    for(l=0; l<nLinesA; l++) 
    {
      // get number of line segments and segment group
      const int nSegments = (*linesA[lineidxA[l]].segmIdxList).size();
      const int group = linesA[lineidxA[l]].group;
  
      const std::vector<segmEntry>& segmList = getSegmentList(group);

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*linesA[lineidxA[l]].segmIdxList)[i];
    
        // segments length
        nPixels += segmList[idx].len;
      }
    }


    // arcB
    const std::vector<lineEntry> &linesB = getLineList(XeCG[arcB.group]);
    const std::vector<int> &lineidxB = *arcB.lineIdxList;
  
    const int nLinesB = lineidxB.size();
    for(l=0; l<nLinesB; l++) 
    {
      // get number of line segments and segment group
      const int nSegments = (*linesB[lineidxB[l]].segmIdxList).size();
      const int group = linesB[lineidxB[l]].group;
  
      const std::vector<segmEntry>& segmList = getSegmentList(group);

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*linesB[lineidxB[l]].segmIdxList)[i];
    
        // segments length
        nPixels += segmList[idx].len;
      }
    }


    // arcC
    const std::vector<lineEntry> &linesC = getLineList(XeCG[arcC.group]);
    const std::vector<int> &lineidxC = *arcC.lineIdxList;
  
    const int nLinesC = lineidxC.size();
    for(l=0; l<nLinesC; l++) 
    {
      // get number of line segments and segment group
      const int nSegments = (*linesC[lineidxC[l]].segmIdxList).size();
      const int group = linesC[lineidxC[l]].group;
  
      const std::vector<segmEntry>& segmList = getSegmentList(group);

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*linesC[lineidxC[l]].segmIdxList)[i];
    
        // segments length
        nPixels += segmList[idx].len;
      }
    }


  // FILL DESIGN MATRIX
    lti::matrix<double> Design(false, nPixels, 6);
    int row = 0;


    // arcA
    for(l=0; l<nLinesA; l++) 
    {
      // get number of line segments and segment group
      const int nSegments = (*linesA[lineidxA[l]].segmIdxList).size();
      const int group = linesA[lineidxA[l]].group;
  
      const std::vector<segmEntry>& segmList = getSegmentList(group);

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*linesA[lineidxA[l]].segmIdxList)[i];
        
        // starting point of segment[i]
        int x = segmList[idx].start.x;
        int y = segmList[idx].start.y;
      
        // segments length
        const int len = segmList[idx].len;
        
        // accumulate all segment pixels
        for(int j=0; j<len; j++) {

          // Now fill design matrix
          double data[6] = { x*x, x*y, y*y, x, y, 1.0 };
          lti::vector<double> line(6, data);
          Design.setRow(row, line);
       
          row++;
          
          // set next segment pixel coordinates
          // orientation:  ---x
          //               |
          //               y
          switch(group) {
            case 1: {   // horizontal
              x++;
              break;
            }
            case 2: {   // vertical
              y++;
              break;
            }
            case 3: {   // diagonal horizontal: \ (starts top left)
              x++;
              y++;
              break;
            }
            case 4: {   // diagonal vertical: / (starts bottom left)
              x++;
              y--;
              break;
            }
          }
        }// next pixel
      }
    }


    // arcB
    for(l=0; l<nLinesB; l++) 
    {
      // get number of line segments and segment group
      const int nSegments = (*linesB[lineidxB[l]].segmIdxList).size();
      const int group = linesB[lineidxB[l]].group;
  
      const std::vector<segmEntry>& segmList = getSegmentList(group);

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*linesB[lineidxB[l]].segmIdxList)[i];
        
        // starting point of segment[i]
        int x = segmList[idx].start.x;
        int y = segmList[idx].start.y;
      
        // segments length
        const int len = segmList[idx].len;
        
        // accumulate all segment pixels
        for(int j=0; j<len; j++) {

          // Now fill design matrix
          double data[6] = { x*x, x*y, y*y, x, y, 1.0 };
          lti::vector<double> line(6, data);
          Design.setRow(row, line);
       
          row++;
          
          // set next segment pixel coordinates
          // orientation:  ---x
          //               |
          //               y
          switch(group) {
            case 1: {   // horizontal
              x++;
              break;
            }
            case 2: {   // vertical
              y++;
              break;
            }
            case 3: {   // diagonal horizontal: \ (starts top left)
              x++;
              y++;
              break;
            }
            case 4: {   // diagonal vertical: / (starts bottom left)
              x++;
              y--;
              break;
            }
          }
        }// next pixel
      }
    }



    // arcC
    for(l=0; l<nLinesC; l++) 
    {
      // get number of line segments and segment group
      const int nSegments = (*linesC[lineidxC[l]].segmIdxList).size();
      const int group = linesC[lineidxC[l]].group;
  
      const std::vector<segmEntry>& segmList = getSegmentList(group);

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*linesC[lineidxC[l]].segmIdxList)[i];
        
        // starting point of segment[i]
        int x = segmList[idx].start.x;
        int y = segmList[idx].start.y;
      
        // segments length
        const int len = segmList[idx].len;
        
        // accumulate all segment pixels
        for(int j=0; j<len; j++) {

          // Now fill design matrix
          double data[6] = { x*x, x*y, y*y, x, y, 1.0 };
          lti::vector<double> line(6, data);
          Design.setRow(row, line);
       
          row++;
          
          // set next segment pixel coordinates
          // orientation:  ---x
          //               |
          //               y
          switch(group) {
            case 1: {   // horizontal
              x++;
              break;
            }
            case 2: {   // vertical
              y++;
              break;
            }
            case 3: {   // diagonal horizontal: \ (starts top left)
              x++;
              y++;
              break;
            }
            case 4: {   // diagonal vertical: / (starts bottom left)
              x++;
              y--;
              break;
            }
          }
        }// next pixel
      }
    }


    return directEllipseFitting(Design, estX, estY, estA, estB, estT);
  }


///////////////////////////////////////////////////////////////////////

  bool fastEllipseExtraction::directEllipseFitting(const lti::matrix<double> &Design, 
                                                   double &estX, double &estY, double &estA, double &estB, double &estT)
  {
    //printLtiMatrix(Design, "Design matrix:");

    // Create 6x6 constraint matrix (applet version is *(-1) ?)
    lti::matrix<double> CO(6, 6, 0.0);
    CO.at(0,2) = -2;
    CO.at(1,1) = 1;
    CO.at(2,0) = -2;
    //printLtiMatrix(CO, "Constraint matrix:");


    // Create transposed design matrix Dt
    lti::matrix<double> Dt(false, Design.columns(), Design.rows());
    Dt.transpose(Design);
    
    // Create scatter matrix  S
    lti::matrix<double> S(Dt);
    S.multiply(Design);
    //printLtiMatrix(S, "Scatter matrix:");

    // Cholesky decomposition (lti version rounds values ?)
    lti::choleskyDecomposition<double> colesDcFunctor;
    if(!colesDcFunctor.apply(S)) {
      return false;
    }
    S.transpose();  // modified dc in java applet ?
    //printLtiMatrix(S, "Cholesky decomposed scatter matrix (transposed):");

    // Create inverted cholesky decomposed scatter matrix Si
    lti::matrix<double> Si(S);
    lti::matrixInversion<double> invFunctor;
    if(!invFunctor.apply(Si)) {
      return false;
    }
    //printLtiMatrix(Si, "Inverted cholesky decomposed scatter matrix:");

    // Create transposed inverted scatter matrix Sit
    lti::matrix<double> Sit(false, Si.columns(), Si.rows());
    Sit.transpose(Si);
    ////printLtiMatrix(Sit, "Transposed cholesky decomposed inverted scatter matrix:");

    // Multiply CO and Sit
    lti::matrix<double> COSit(CO);
    COSit.multiply(Sit);
    //printLtiMatrix(COSit, "Temporary matrix:");

    // Multiply Si and COSit
    lti::matrix<double> SiCOSit(Si);
    SiCOSit.multiply(COSit);
    //printLtiMatrix(SiCOSit, "System matrix:");


    // Solve eigensystem
    lti::jacobi<double> jacobiFunctor;
    lti::vector<double> EVals(6);
    lti::matrix<double> EVecs(false, 6, 6);
    jacobiFunctor.apply(SiCOSit, EVals, EVecs);

    //printLtiVector(EVals, "Eigenvalues");
    //printLtiMatrix(EVecs, "Eigenvectors:");


    // Multiply Sit and EVecs
    lti::matrix<double> Sol(Sit);
    Sol.multiply(EVecs);
    //printLtiMatrix(Sol, "Unnormalized solution:");
    
    // Normalization
    for(int j=0; j<6; j++) {  // scan columns
      int i;
      double mod = 0.0;
      for(i=0; i<6; i++) {
        mod += Sol[i][j] * Sol[i][j];
      }
      for(i=0; i<6; i++) {
        Sol[i][j] /= sqrt(mod); 
      }
    }
    //printLtiMatrix(Sol, "Normalized solution:");


    // Find negative non-zero EVal
    int index = -1;
    const double zero = 10e-20;   // ?
    for(int i=0; i<6; i++) {
      //double x = EVals[i];
      if(EVals[i] < 0) {
        if(-EVals[i] > zero) {
          index = i;
        }
      }
    }
    if(index == -1) {
      return false;
    }

    // Fetch the right solution
    lti::vector<double> solvEVec(6);
    Sol.getColumnCopy(index, solvEVec);

    //printLtiVector(solvEVec, "The solving Eigenvector:");



    // Calculate ellipse parameters
    const double A = solvEVec[0];
    const double B = solvEVec[1]/2;
    const double C = solvEVec[2];
    const double D = solvEVec[3]/2;
    const double E = solvEVec[4]/2;
    const double F = solvEVec[5];

    // Invarianten der Kegelschnittgleichung (Bronstein S.213)
    const double delta = A*C - B*B;
    const double DELTA = A*C*F + B*E*D + D*B*E - A*E*E - B*B*F - D*C*D;

    if(delta == 0) {
      return false;   // division by zero
    }

    // Ellipsen-Mittelpunkt und Orientierung
    estX = (B*E - C*D) / delta;
    estY = (B*D - A*E) / delta;
    estT = atan2(2*B, A-C)/2;
    //estT = PI/2 - atan2(2*B, A-C)/2;
    
    // Koeffizienten der Bronstein-Normalform nach Transformation (Bronstein S.212)
    const double sqrtResult = sqrt((A-C)*(A-C) + 4*B*B);
    const double a = (A+C + sqrtResult) / 2;
    const double c = (A+C - sqrtResult) / 2;

    if((a == 0)||(c == 0)) {
      return false;   // division by zero
    }

    // Umrechnung Bronstein-Normalform in Ellipsen-Normalform
    const double Ddivd = DELTA / delta;
    const double a2 = -Ddivd / a;
    const double b2 = -Ddivd / c;

    if((a2 < 0)||(b2 < 0)) {
      return false;   // negative sqrt argument
    }

    //estA = sqrt(a2);
    //estB = sqrt(b2);
    estA = sqrt(b2);
    estB = sqrt(a2);
    
    return true;
  }


///////////////////////////////////////////////////////////////////////

  bool fastEllipseExtraction::directEllipseCoefficientFitting(const lti::matrix<double> &Design, 
                                                              double &A, double &B, double &C, double &D, double &E, double &F)
  {
    //printLtiMatrix(Design, "Design matrix:");

    // Create 6x6 constraint matrix (applet version is *(-1) ?)
    lti::matrix<double> CO(6, 6, 0.0);
    CO.at(0,2) = -2;
    CO.at(1,1) = 1;
    CO.at(2,0) = -2;
    //printLtiMatrix(CO, "Constraint matrix:");


    // Create transposed design matrix Dt
    lti::matrix<double> Dt(false, Design.columns(), Design.rows());
    Dt.transpose(Design);
    
    // Create scatter matrix  S
    lti::matrix<double> S(Dt);
    S.multiply(Design);
    //printLtiMatrix(S, "Scatter matrix:");

    // Cholesky decomposition (lti version rounds values ?)
    lti::choleskyDecomposition<double> colesDcFunctor;
    if(!colesDcFunctor.apply(S)) {
      return false;
    }
    S.transpose();  // modified dc in java applet ?
    //printLtiMatrix(S, "Cholesky decomposed scatter matrix (transposed):");

    // Create inverted cholesky decomposed scatter matrix Si
    lti::matrix<double> Si(S);
    lti::matrixInversion<double> invFunctor;
    if(!invFunctor.apply(Si)) {
      return false;
    }
    //printLtiMatrix(Si, "Inverted cholesky decomposed scatter matrix:");

    // Create transposed inverted scatter matrix Sit
    lti::matrix<double> Sit(false, Si.columns(), Si.rows());
    Sit.transpose(Si);
    ////printLtiMatrix(Sit, "Transposed cholesky decomposed inverted scatter matrix:");

    // Multiply CO and Sit
    lti::matrix<double> COSit(CO);
    COSit.multiply(Sit);
    //printLtiMatrix(COSit, "Temporary matrix:");

    // Multiply Si and COSit
    lti::matrix<double> SiCOSit(Si);
    SiCOSit.multiply(COSit);
    //printLtiMatrix(SiCOSit, "System matrix:");


    // Solve eigensystem
    lti::jacobi<double> jacobiFunctor;
    lti::vector<double> EVals(6);
    lti::matrix<double> EVecs(false, 6, 6);
    jacobiFunctor.apply(SiCOSit, EVals, EVecs);

    //printLtiVector(EVals, "Eigenvalues");
    //printLtiMatrix(EVecs, "Eigenvectors:");


    // Multiply Sit and EVecs
    lti::matrix<double> Sol(Sit);
    Sol.multiply(EVecs);
    //printLtiMatrix(Sol, "Unnormalized solution:");
    
    // Normalization
    for(int j=0; j<6; j++) {  // scan columns
      int i;
      double mod = 0.0;
      for(i=0; i<6; i++) {
        mod += Sol[i][j] * Sol[i][j];
      }
      for(i=0; i<6; i++) {
        Sol[i][j] /= sqrt(mod); 
      }
    }
    //printLtiMatrix(Sol, "Normalized solution:");


    // Find negative non-zero EVal
    int index = -1;
    const double zero = 10e-20;   // ?
    for(int i=0; i<6; i++) {
      //double x = EVals[i];
      if(EVals[i] < 0) {
        if(-EVals[i] > zero) {
          index = i;
        }
      }
    }
    if(index == -1) {
      return false;
    }

    // Fetch the right solution
    lti::vector<double> solvEVec(6);
    Sol.getColumnCopy(index, solvEVec);

    //printLtiVector(solvEVec, "The solving Eigenvector:");



    // Return coefficients
    A = solvEVec[0];
    B = solvEVec[1]/2;
    C = solvEVec[2];
    D = solvEVec[3]/2;
    E = solvEVec[4]/2;
    F = solvEVec[5];

    return true;
  }


/////////////////////////////////////////////////////////////////////
  
  bool fastEllipseExtraction::estimateCircle(std::vector<int> &idxlist, const int group,
                                             double &estX, double &estY, double &estR2) {
    lti::int64 pixelCount = 0;
    lti::int64 xSum = 0;
    lti::int64 ySum = 0; 
    lti::int64 x2Sum = 0;
    lti::int64 y2Sum = 0; 
    lti::int64 xySum = 0;
    lti::int64 x3Sum = 0;
    lti::int64 y3Sum = 0;
    lti::int64 xy2Sum = 0;
    lti::int64 x2ySum = 0;
  
    int x;
    int y;

    // set how x and y are followed in the inner loop
    // orientation:  ---x
    //               |
    //               y
    int xInc;
    int yInc;
    switch (group) {
    case 1:     // horizontal
      xInc = 1;
      yInc = 0;
      break;

    case 2:      // vertical
      xInc = 0;
      yInc = 1;
      break;

    case 3:     // diagonal horizontal: \ (starts top left)
      xInc = 1;
      yInc = 1;
      break;

    case 4:     // diagonal vertical: / (starts bottom left)
      xInc = 1;
      yInc = -1;
      break;

    default:
      xInc = 0;
      yInc = 0;
    }

    // save first endpoint to use later to avoid overflow
    int x0 = vecSegments[group][(*vecLines[group][idxlist[0]].segmIdxList)[0]].start.x;
    int y0 = vecSegments[group][(*vecLines[group][idxlist[0]].segmIdxList)[0]].start.y;

    const int nLines = static_cast<int>(idxlist.size());
  
    // accumulate all lines
    for(int l=0; l<nLines; l++) {
      // get number of line segments and segment group
      const int nSegments = static_cast<int>((*vecLines[group][idxlist[l]].segmIdxList).size());

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*vecLines[group][idxlist[l]].segmIdxList)[i];
        
        // starting point of segment[i], translated to near the origin
        x = vecSegments[group][idx].start.x - x0;
        y = vecSegments[group][idx].start.y - y0;
      
        // segments length
        const int len = vecSegments[group][idx].len;
        
        // accumulate all segment pixels
        for(int j=0; j<len; j++) {
          // calculate sums
	  const lti::int64 xx = x*x;
	  const lti::int64 yy = y*y;

          xSum += x;
          ySum += y;
          x2Sum += xx;
          y2Sum += yy;
          xySum += x*y;
          x3Sum += x*xx;
          y3Sum += y*yy;
          xy2Sum += x*yy;
          x2ySum += xx*y;
          
          // set next segment pixel coordinates
	  x += xInc;
	  y += yInc;
          pixelCount++;  // increase number of all pixels
        
        }// next pixel
      }// next segment
    }// next line
  
    const double  a1 = static_cast<double>(2*(xSum * xSum - pixelCount * x2Sum));
    const double  b1 = static_cast<double>(2*(xSum * ySum - pixelCount * xySum));
    const double  a2 = b1;
    const double  b2 = static_cast<double>(2*(ySum * ySum - pixelCount * y2Sum));
    const double  c1 = static_cast<double>(x2Sum * xSum - pixelCount * x3Sum + xSum * y2Sum - pixelCount * xy2Sum);
    const double  c2 = static_cast<double>(x2Sum * ySum - pixelCount * y3Sum + ySum * y2Sum - pixelCount * x2ySum);
      
    // calculate circle parameters
    if((a1*b2 - a2*b1) != 0) {      // avoid division by zero
      estX = (c1*b2 - c2*b1)/(a1*b2 - a2*b1);
      estY = (a1*c2 - a2*c1)/(a1*b2 - a2*b1);
      estR2 = (double)(x2Sum - 2*estX*xSum + pixelCount*estX*estX + y2Sum - 2*estY*ySum + pixelCount*estY*estY)/pixelCount;

      // translate back to original position
      estX += x0;
      estY += y0;
      return true;
    }
    
    return false;   // division by zero
  }



  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  
  // On place apply for type channel8!
  bool fastEllipseExtraction::apply(const channel8& srcdest) {

    // extract line segments
    detectGroup1Segments(srcdest);
    detectGroup2Segments(srcdest);
    detectGroup3Segments(srcdest);
    detectGroup4Segments(srcdest);
    
    // extract lines
    extractGroup1Lines();
    extractGroup2Lines();
    extractGroup3Lines();
    extractGroup4Lines();

    // extract elliptic arcs
    extractGroup1and5Arcs();
    extractGroup2and6Arcs();
    extractGroup3and7Arcs();
    extractGroup4and8Arcs();

    // extract elliptic extended arcs
    clearEllExtArcList(0);
    for(int g=1; g<=8; g++) {
      extractEllExtArcs(g);
    }

    // extract ellipses
    extractEllipses();

    return true;
  };


  // On place apply for type channel!
  bool fastEllipseExtraction::apply(const channel& srcdest) {
  
    lti::channel8 temp;
    temp.castFrom(srcdest, false, false);   // channel -> channel8
    apply(temp);
    return true;
  };


}
