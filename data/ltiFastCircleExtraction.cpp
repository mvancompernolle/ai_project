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
 * file .......: ltiFastCircleExtraction.cpp
 * authors ....: Ingo Grothues (ingo@isdevelop.de)
 * organization: LTI, RWTH Aachen
 * creation ...: 22.4.2004
 * revisions ..: $Id: ltiFastCircleExtraction.cpp,v 1.8 2006/09/05 10:11:38 ltilib Exp $
 */

#include "ltiFastCircleExtraction.h"

// number of pre-allocated vector elements
const int lti::fastCircleExtraction::SegmPreallocation = 5000;
const int lti::fastCircleExtraction::LinePreallocation = 1000;
const int lti::fastCircleExtraction::ArcPreallocation = 100;
const int lti::fastCircleExtraction::ExtArcPreallocation = 50;
const int lti::fastCircleExtraction::CirclePreallocation = 10;
const double lti::fastCircleExtraction::PI = 3.14159265358979323846264338327;

namespace lti {
  // --------------------------------------------------
  // fastCircleExtraction::parameters
  // --------------------------------------------------

  // default constructor
  fastCircleExtraction::parameters::parameters()
    : featureExtractor::parameters() {

    minSegmLen = 2;
    minLineLen = 6;
    segmentTolerance = 0;
    maxSegmentGap = 1;
    maxQuantizationError = 0.79f;
    maxLineGap = 4;
    maxArcGap = 35;
    maxLineTangentError = 15.0f;
    maxArcTangentError = 12.0f;
    maxExtArcMismatch = 2.75f;
    minExtArcMatchRatio = 0.8f;
    oldExtArcCheck = false;
    oldExtArcThreshold = 100;
    maxCenterMismatch = 2.25f;
    minRadiusMatchRatio = 0.75f;
    minCoverage = 0.3f;
  }

  // copy constructor
  fastCircleExtraction::parameters::parameters(const parameters& other)
    : featureExtractor::parameters() {
    copy(other);
  }

  // destructor
  fastCircleExtraction::parameters::~parameters() {
  }

  // get type name
  const char* fastCircleExtraction::parameters::getTypeName() const {
    return "fastCircleExtraction::parameters";
  }

  // copy member

  fastCircleExtraction::parameters&
    fastCircleExtraction::parameters::copy(const parameters& other) {
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

    minRadiusMatchRatio = other.minRadiusMatchRatio;
    maxLineGap = other.maxLineGap;
    maxArcGap = other.maxArcGap;
    maxLineTangentError = other.maxLineTangentError;
    maxArcTangentError = other.maxArcTangentError;
    maxCenterMismatch = other.maxCenterMismatch;
    
    oldExtArcCheck = other.oldExtArcCheck;
    oldExtArcThreshold = other.oldExtArcThreshold;
    maxExtArcMismatch = other.maxExtArcMismatch;
    minExtArcMatchRatio = other.minExtArcMatchRatio;
    minCoverage = other.minCoverage;
    
    return *this;
  }

  // alias for copy member
  fastCircleExtraction::parameters&
    fastCircleExtraction::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fastCircleExtraction::parameters::clone() const {
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
  bool fastCircleExtraction::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool fastCircleExtraction::parameters::writeMS(ioHandler& handler,
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
      
      lti::write(handler,"minRadiusMatchRatio",minRadiusMatchRatio);
      lti::write(handler,"maxLineGap",maxLineGap);
      lti::write(handler,"maxArcGap",maxArcGap);
      lti::write(handler,"maxLineTangentError",maxLineTangentError);
      lti::write(handler,"maxArcTangentError",maxArcTangentError);
      lti::write(handler,"maxCenterMismatch",maxCenterMismatch);

      lti::write(handler,"oldExtArcCheck",oldExtArcCheck);
      lti::write(handler,"oldExtArcThreshold",oldExtArcThreshold);
      lti::write(handler,"maxExtArcMismatch",maxExtArcMismatch);
      lti::write(handler,"minExtArcMatchRatio",minExtArcMatchRatio);
      lti::write(handler,"minCoverage",minCoverage);
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
  bool fastCircleExtraction::parameters::write(ioHandler& handler,
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
  bool fastCircleExtraction::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool fastCircleExtraction::parameters::readMS(ioHandler& handler,
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
      
      lti::read(handler,"minRadiusMatchRatio",minRadiusMatchRatio);
      lti::read(handler,"maxLineGap",maxLineGap);
      lti::read(handler,"maxArcGap",maxArcGap);
      lti::read(handler,"maxLineTangentError",maxLineTangentError);
      lti::read(handler,"maxArcTangentError",maxArcTangentError);
      lti::read(handler,"maxCenterMismatch",maxCenterMismatch);

      lti::read(handler,"oldExtArcCheck",oldExtArcCheck);
      lti::read(handler,"oldExtArcThreshold",oldExtArcThreshold);
      lti::read(handler,"maxExtArcMismatch",maxExtArcMismatch);
      lti::read(handler,"minExtArcMatchRatio",minExtArcMatchRatio);
      lti::read(handler,"minCoverage",minCoverage);
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
  bool fastCircleExtraction::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


////////////////////////////////////////////////////////////////

  // --------------------------------------------------
  // fastCircleExtraction
  // --------------------------------------------------

  // default constructor
  fastCircleExtraction::fastCircleExtraction()
    : featureExtractor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
    // default initialization
    m_iWidth = 0;
  }

  // default constructor
  fastCircleExtraction::fastCircleExtraction(const parameters& par)
    : featureExtractor() {
    // set the given parameters
    setParameters(par);
    // default initialization
    m_iWidth = 0;
  }


  // copy constructor
  fastCircleExtraction::fastCircleExtraction(const fastCircleExtraction& other)
    : featureExtractor() {
    copy(other);
  }

  // destructor
  fastCircleExtraction::~fastCircleExtraction() {
    for(int i=0; i<5; i++) {
      clearLineList(i);
      clearArcList(i);
      clearExtArcList(i);
      m_vecSegments[i].clear();
    }
    m_vecCircles.clear();
  }

  // returns the name of this type
  const char* fastCircleExtraction::getTypeName() const {
    return "fastCircleExtraction";
  }

  // copy member
  fastCircleExtraction&
    fastCircleExtraction::copy(const fastCircleExtraction& other) {

    featureExtractor::copy(other);
    int i;
    for(i=0; i<5; i++) {
      m_vecLines[i] = other.m_vecLines[i];
      m_vecSegments[i] = other.m_vecSegments[i];
      m_vecArcs[i] = other.m_vecArcs[i];
      m_vecExtArcs[i] = other.m_vecExtArcs[i];
      // copy segmIdxLists
      const int nLines = static_cast<int>(m_vecLines[i].size());
      int j;
      for(j=0; j<nLines; j++) {
        m_vecLines[i][j].segmIdxList = new std::vector<int>(*other.m_vecLines[i][j].segmIdxList);
      }
      // copy lineIdxLists
      const int nArcs = static_cast<int>(m_vecArcs[i].size());
      for(j=0; j<nArcs; j++) {
        m_vecArcs[i][j].lineIdxList = new std::vector<int>(*other.m_vecArcs[i][j].lineIdxList);
      }
    }

    m_vecCircles = other.m_vecCircles;
    // copy arcIdxLists
    const int nCircles = static_cast<int>(m_vecCircles.size());
    for(i=0; i<nCircles; i++) {
      m_vecCircles[i].mergedArcs = new std::vector<int>(*other.m_vecCircles[i].mergedArcs);
    }

    m_iWidth = other.m_iWidth;
    return (*this);
  }

  // alias for copy member
  fastCircleExtraction&
    fastCircleExtraction::operator=(const fastCircleExtraction& other) {
    return (copy(other));
  }


  // clone member
  functor* fastCircleExtraction::clone() const {
    return new fastCircleExtraction(*this);
  }

  // return parameters
  const fastCircleExtraction::parameters&
    fastCircleExtraction::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // returns vector with line segments
  std::vector<fastCircleExtraction::segmEntry>& fastCircleExtraction::getSegmentList(const int group_number) {
    if((group_number < 0)||(group_number > 4)) {
      return m_vecSegments[0];
    }
    return m_vecSegments[group_number];
  }

  // returns vector with lines
  std::vector<fastCircleExtraction::lineEntry>& fastCircleExtraction::getLineList(const int group_number) {
    if((group_number < 0)||(group_number > 4)) {
      return m_vecLines[0];
    }
    return m_vecLines[group_number];
  }

  // returns vector with arcs
  std::vector<fastCircleExtraction::arcEntry>& fastCircleExtraction::getArcList(const int group_number) {
    if((group_number < 0)||(group_number > 4)) {
      return m_vecArcs[0];
    }
    return m_vecArcs[group_number];
  }  

  // returns vector with extended arcs
  std::vector<fastCircleExtraction::extArcEntry>& fastCircleExtraction::getExtArcList(const int group_number) {
    if((group_number < 0)||(group_number > 4)) {
      return m_vecExtArcs[0];
    }
    return m_vecExtArcs[group_number];
  }  

  // returns vector with circles
  std::vector<fastCircleExtraction::circleEntry>& fastCircleExtraction::getCircleList() {
    return m_vecCircles;
  }  

  // clears line lists
  void fastCircleExtraction::clearLineList(const int group_number) {
      if(group_number > 0) {  // don't delete shared list[0] idx_vectors
        const int n = static_cast<int>(m_vecLines[group_number].size());
        for(int j=0; j<n; j++) {
          (*m_vecLines[group_number][j].segmIdxList).clear();
          delete m_vecLines[group_number][j].segmIdxList;
        }
      }
      m_vecLines[group_number].clear(); 
  }

  // clears arc lists
  void fastCircleExtraction::clearArcList(const int group_number) {
      if(group_number > 0) {  // don't delete shared list[0] idx_vectors
        const int n = static_cast<int>(m_vecArcs[group_number].size());
        for(int j=0; j<n; j++) {
          (*m_vecArcs[group_number][j].lineIdxList).clear();
          delete m_vecArcs[group_number][j].lineIdxList;
        }
      }
      m_vecArcs[group_number].clear(); 
  }

  // clears extended arc lists
  void fastCircleExtraction::clearExtArcList(const int group_number) {
      m_vecExtArcs[group_number].clear(); 
  }

  // clears the circle list
  void fastCircleExtraction::clearCircleList() {
      const int n = static_cast<int>(m_vecCircles.size());
      for(int j=0; j<n; j++) {
        (*m_vecCircles[j].mergedArcs).clear();
        delete m_vecCircles[j].mergedArcs;
      }
      m_vecCircles.clear(); 
  }
  
///////////////////////////////////////////////////////////////////////////////

  // creates segment lists
  void fastCircleExtraction::detectGroup1Segments(const channel8& src) {
    
    // get parameters
    const parameters& par = getParameters();

    // get size of the input image
    const int rows = src.rows();
    const int cols = src.columns();
    m_iWidth = cols;
    
    // calculate max ranges (horizontal lines)
    const int lineCount = rows;
    const int lineLength = cols;

    // clear old list and reserve memory
    m_vecSegments[1].clear(); 
    m_vecSegments[1].reserve(SegmPreallocation);

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
                m_vecSegments[1].push_back(newSegment);
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
            m_vecSegments[1].push_back(newSegment); 
          }
          len = 0;
        }
        
        prevVal = val;
      } // pixel scan
    } // line scan
  }

///////////////////////////////////////////////////////////////////////////////

  // creates segment lists
  void fastCircleExtraction::detectGroup2Segments(const channel8& src) {

    // get parameters
    const parameters& par = getParameters();

    // get size of the input image
    const int rows = src.rows();
    const int cols = src.columns();
    m_iWidth = cols;
    
    // calculate max ranges (vertical)
    const int lineCount = cols;
    const int lineLength = rows;

    // clear old list and reserve memory
    m_vecSegments[2].clear(); 
    m_vecSegments[2].reserve(SegmPreallocation);

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
                m_vecSegments[2].push_back(newSegment); 
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
            m_vecSegments[2].push_back(newSegment); 
          }
          len = 0;
        }
        
        prevVal = val;
      } // pixel scan
    } // line scan
  }

///////////////////////////////////////////////////////////////////////////////

  // creates segment lists
  void fastCircleExtraction::detectGroup3Segments(const channel8& src) {
  
    // get parameters
    const parameters& par = getParameters();

    // get size of the input image
    const int rows = src.rows();
    const int cols = src.columns();
    m_iWidth = cols;
    
    // maximum values
    const int maxDiag   = (rows < cols) ? rows : cols;
    
    // calculate max ranges (diagonal)
    const int lineCount = rows+cols-1;  // scan from upper right over upper left to lower left corner
    int lineLength = 0;                 // line length depends on actual line scan position

    // clear old list and reserve memory
    m_vecSegments[3].clear(); 
    m_vecSegments[3].reserve(SegmPreallocation);

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
                m_vecSegments[3].push_back(newSegment); 
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
            m_vecSegments[3].push_back(newSegment); 
          }
          len = 0;
        }
        
        prevVal = val;
      } // pixel scan
    } // line scan
  }

///////////////////////////////////////////////////////////////////////////////

  // creates segment lists
  void fastCircleExtraction::detectGroup4Segments(const channel8& src) {
  
    // get parameters
    const parameters& par = getParameters();

    // get size of the input image
    const int rows = src.rows();
    const int cols = src.columns();
    m_iWidth = cols;
    
    // maximum values
    const int maxDiag   = (rows < cols) ? rows : cols;
    
    // calculate max ranges (diagonal)
    const int lineCount = rows+cols-1;  // scan from lower right over lower left to upper left corner
    int lineLength = 0;                 // line length depends on actual line scan position

    // clear old list and reserve memory
    m_vecSegments[4].clear(); 
    m_vecSegments[4].reserve(SegmPreallocation);

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
                m_vecSegments[4].push_back(newSegment); 
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
            m_vecSegments[4].push_back(newSegment); 
          }
          len = 0;
        }
        
        prevVal = val;
      } // pixel scan
    } // line scan
  }


///////////////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractGroup1Lines() {
  
    // clear old complete line list and reserve memory
    m_vecLines[0].clear(); 
    m_vecLines[0].reserve(LinePreallocation);

    // clear old linelist and reserve memory
    clearLineList(1); 
    m_vecLines[1].reserve(LinePreallocation);
    
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(m_vecSegments[1].size());

    if(segmCount > 0) {

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        // create new segment grouping vector
        std::vector<int> *vecLineSegments = new std::vector<int>;

        bool positive_slope = true;   // sign of the slope of the first segment pair
        bool first_candidate = true;  // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments->push_back(Si);  // store Si in linelist (starting segment)
        SS = Si;                        // set Si as new target segment SS for search
        SL = Si;                        // set Si as actual longest linesegment SL
        Sq = Si + 1;                    // first candidate segment
        
        while(Sq < segmCount) {   // track line segments
          // STEP2
          // calculate window boundaries for target segment SS
          const segmEntry segSS = m_vecSegments[1][SS];
          const int GAPx = par.maxSegmentGap;
          const int GAPx_pos = segSS.end.x + GAPx;
          const int GAPx_neg = segSS.start.x - GAPx;
          const float deltaY = -0.5f; //segSS.end.y - segSS.start.y - 0.5;
          const float deltaX = static_cast<float>(segSS.end.x - segSS.start.x);
          const float min_slope = deltaY / deltaX;
          const float GAPy_fract = min_slope * (GAPx_neg - segSS.end.x);
          const float GAPy_round = floor(GAPy_fract + 0.5f);
          const int GAPy = static_cast<int>(segSS.end.y + GAPy_round);
          
          // search new candidate segment Sq (from top to bottom)
          while(Sq < segmCount) {
            // no multi-use of segments in circle extraction
            if(m_vecSegments[1][Sq].used > 0) {
              Sq++;
              continue;
            }
            const segmEntry segSq = m_vecSegments[1][Sq];
            
            if(segSq.start.y > GAPy) { // window boundary reached ?
              Sq = segmCount;          // no candidate segment found
              break;
            }

            // positive slope or first candidate
            if(positive_slope || first_candidate) {
              if((segSq.start.x <= GAPx_pos)                         // Sq starts in window 
              && (segSq.start.x >= segSS.end.x - segSS.len/2 + 1)) { // Sq starts right from center of SS
                positive_slope = true;
                break;
              }
            }

            // negative slope or first candidate
            if(!positive_slope || first_candidate) {
              if((segSq.end.x >= GAPx_neg)                           // Sq ends in window 
              && (segSq.end.x <= segSS.start.x + segSS.len/2 - 1)) { // Sq ends left from center of SS
                positive_slope = false;
                break;
              }
            }
            Sq++;
          }

          if(Sq == segmCount) break;    // no candidate segment found

          const segmEntry segSi = m_vecSegments[1][Si];
          const segmEntry segSq = m_vecSegments[1][Sq];

          // calculate longest line segment SL (old SL or new Sq)
          if(m_vecSegments[1][SL].len < segSq.len) {
            SL = Sq;
          }
          const segmEntry segSL = m_vecSegments[1][SL];
          
          // STEP3
          // calculate slope
          float slope;
          if(positive_slope) {
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
            if(first_candidate) {
              positive_slope = true;    // reset slope flag
            }
            break;   
          }
          else first_candidate = false;

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

        if(positive_slope) {
          const int x1 = m_vecSegments[1][(*vecLineSegments)[0]].start.x;
          const int x2 = m_vecSegments[1][(*vecLineSegments)[nSegments-1]].end.x;
          length = x2 - x1 + 1;
        }
        else {
          const int x1 = m_vecSegments[1][(*vecLineSegments)[nSegments-1]].start.x;
          const int x2 = m_vecSegments[1][(*vecLineSegments)[0]].end.x;
          length = x2 - x1 + 1;
        }

        // line length must be >= minLineLen for extraction
        if(length >= par.minLineLen) {
          
          // register grouped segments as extracted line
          lineEntry newline;
          if(positive_slope) {
            newline.start.x = m_vecSegments[1][(*vecLineSegments)[0]].start.x;
            newline.start.y = m_vecSegments[1][(*vecLineSegments)[0]].start.y;
            newline.end.x = m_vecSegments[1][(*vecLineSegments)[nSegments-1]].end.x;
            newline.end.y = m_vecSegments[1][(*vecLineSegments)[nSegments-1]].end.y;
          }
          else {
            newline.start.x = m_vecSegments[1][(*vecLineSegments)[nSegments-1]].start.x;
            newline.start.y = m_vecSegments[1][(*vecLineSegments)[nSegments-1]].start.y;
            newline.end.x = m_vecSegments[1][(*vecLineSegments)[0]].end.x;
            newline.end.y = m_vecSegments[1][(*vecLineSegments)[0]].end.y;
          }
          // calculate midpoint
          newline.mid.x = (newline.start.x + newline.end.x)/2;
          newline.mid.y = (newline.start.y + newline.end.y)/2;
          // calculate tangent
          newline.tangent = atan2((double)(newline.start.y-newline.end.y), (double)(newline.end.x-newline.start.x));
          // set other values
          newline.len = length;
          newline.used = 0;
          newline.group = 1;
          // store data
          newline.segmIdxList = vecLineSegments;
          m_vecLines[1].push_back(newline);
          m_vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecSegments[1][(*vecLineSegments)[i]].used++;
          }
        }
        else delete vecLineSegments;

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecSegments[1][Si].used == 0) break;
        }
        
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractGroup2Lines() {

    // clear old linelist and reserve memory
    clearLineList(2);
    m_vecLines[2].reserve(LinePreallocation);
  
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(m_vecSegments[2].size());

    if(segmCount > 0) {

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        // create new segment grouping vector
        std::vector<int> *vecLineSegments = new std::vector<int>;

        bool positive_slope = true;   // sign of the slope of the first segment pair
        bool first_candidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments->push_back(Si);  // store Si in linelist (starting segment)
        SS = Si;                        // set Si as new target segment SS for search
        SL = Si;                        // set Si as actual longest linesegment SL
        Sq = Si + 1;                    // first candidate segment


        while(Sq < segmCount) {   // track line segments
          // STEP2
          // calculate window boundaries for target segment SS
          const segmEntry segSS = m_vecSegments[2][SS];
          const int GAPx = par.maxSegmentGap;
          const int GAPx_pos = segSS.end.y + GAPx;
          const int GAPx_neg = segSS.start.y - GAPx;
          const float deltaY = -0.5f; //segSS.end.x - segSS.start.x - 0.5;
          const float deltaX = static_cast<float>(segSS.end.y - segSS.start.y);
          const float min_slope = deltaY / deltaX;
          const float GAPy_fract = min_slope * (GAPx_neg - segSS.end.y);
          const float GAPy_round = floor(GAPy_fract + 0.5f);
          const int GAPy = static_cast<int>(segSS.end.x + GAPy_round);


          // search new candidate segment Sq (from left to right)
          while(Sq < segmCount) {
          
            // no multi-use of segments in circle extraction
            if(m_vecSegments[2][Sq].used > 0) {
              Sq++;
              continue;
            }
            const segmEntry segSq = m_vecSegments[2][Sq];

            if(segSq.start.x > GAPy) {  // window boundary reached ?
              Sq = segmCount;   // no candidate segment found
              break;
            }

            // positive slope or first candidate
            if(positive_slope || first_candidate) {
              if((segSq.start.y <= GAPx_pos)                         // Sq starts in window 
              && (segSq.start.y >= segSS.end.y - segSS.len/2 + 1)) { // Sq starts below the center of SS
                  positive_slope = true;
                  break;
              }
            }

            // negative slope or first candidate
            if(!positive_slope || first_candidate) {
              if((segSq.end.y >= GAPx_neg)                           // Sq ends in window 
              && (segSq.end.y <= segSS.start.y + segSS.len/2 - 1)) { // Sq ends above the center of SS
                  positive_slope = false;
                  break;
              }
            }

            Sq++;
          }

          if(Sq == segmCount) break;    // no candidate segment found

          const segmEntry segSi = m_vecSegments[2][Si];
          const segmEntry segSq = m_vecSegments[2][Sq];

          // calculate longest line segment SL (old SL or new Sq)
          if(m_vecSegments[2][SL].len < segSq.len) {
            SL = Sq;
          }
          const segmEntry segSL = m_vecSegments[2][SL];

          // STEP3
          // calculate slope
          float slope;
          if(positive_slope) {
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
            if(first_candidate) {
              positive_slope = true;    // reset slope flag
            }
            break;   
          }
          else first_candidate = false;

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

        if(positive_slope) {
          const int y1 = m_vecSegments[2][(*vecLineSegments)[0]].start.y;
          const int y2 = m_vecSegments[2][(*vecLineSegments)[nSegments-1]].end.y;
          length = y2 - y1 + 1;
        }
        else {
          const int y1 = m_vecSegments[2][(*vecLineSegments)[nSegments-1]].start.y;
          const int y2 = m_vecSegments[2][(*vecLineSegments)[0]].end.y;
          length = y2 - y1 + 1;
        }

        // line length must be >= minLineLen for extraction
        if(length >= par.minLineLen) {
        
          // register grouped segments as extracted line
          lineEntry newline;
          if(positive_slope) {
            newline.start.x = m_vecSegments[2][(*vecLineSegments)[0]].start.x;
            newline.start.y = m_vecSegments[2][(*vecLineSegments)[0]].start.y;
            newline.end.x = m_vecSegments[2][(*vecLineSegments)[nSegments-1]].end.x;
            newline.end.y = m_vecSegments[2][(*vecLineSegments)[nSegments-1]].end.y;
          }
          else {
            newline.start.x = m_vecSegments[2][(*vecLineSegments)[nSegments-1]].start.x;
            newline.start.y = m_vecSegments[2][(*vecLineSegments)[nSegments-1]].start.y;
            newline.end.x = m_vecSegments[2][(*vecLineSegments)[0]].end.x;
            newline.end.y = m_vecSegments[2][(*vecLineSegments)[0]].end.y;
          }
          
          // calculate midpoint
          newline.mid.x = (newline.start.x + newline.end.x)/2;
          newline.mid.y = (newline.start.y + newline.end.y)/2;
          // calculate tangent
          newline.tangent = atan2((double)(newline.end.y-newline.start.y), (double)(newline.start.x-newline.end.x));
          // set other values
          newline.len = length;
          newline.used = 0;
          newline.group = 2;
          // store data
          newline.segmIdxList = vecLineSegments;
          m_vecLines[2].push_back(newline);
          m_vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecSegments[2][(*vecLineSegments)[i]].used++;
          }
        }
        else delete vecLineSegments;

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecSegments[2][Si].used == 0) break;
        }
        
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractGroup3Lines() {

    // clear old linelist and reserve memory
    clearLineList(3);
    m_vecLines[3].reserve(LinePreallocation);

    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(m_vecSegments[3].size());
    
    if(segmCount > 0) {

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        // create new segment grouping vector
        std::vector<int> *vecLineSegments = new std::vector<int>;
        
        bool positive_slope = true;   // sign of the slope of the first segment pair
        bool first_candidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments->push_back(Si);  // store Si in linelist (starting segment)
        SS = Si;                        // set Si as new target segment SS for search
        SL = Si;                        // set Si as actual longest linesegment SL
        Sq = Si + 1;                    // first candidate segment


        while(Sq < segmCount) {  // track line segments

          const segmEntry segSS = m_vecSegments[3][SS];

          // transform coordinates (diagonal -> horizontal)
          const int SS_start_x = segSS.start.x + segSS.start.y;
          const int SS_end_x   = segSS.end.x + segSS.end.y;
          const int SS_end_y   = m_iWidth - 1 - segSS.end.x + segSS.end.y;
          
          // STEP2
          // calculate window boundaries for target segment SS
          const int GAPx = par.maxSegmentGap;
          const int GAPx_pos = SS_end_x + 1 + GAPx*2;   // double gap + 1 
          const int GAPx_neg = SS_start_x - 1 - GAPx*2; // double gap + 1 
          const float deltaY = -0.5f; //SS_end_y - SS_start_y - 0.5;
          const float deltaX = static_cast<float>(SS_end_x - SS_start_x);
          const float min_slope = deltaY / deltaX;
          const float GAPy_fract = min_slope * (GAPx_neg - SS_end_x);
          const float GAPy_round = floor(GAPy_fract + 0.5f);
          const int GAPy = static_cast<int>(SS_end_y + GAPy_round);


          // search new candidate segment Sq (from top to bottom)
          while(Sq < segmCount) {
          
            // no multi-use of segments in circle extraction
            if(m_vecSegments[3][Sq].used > 0) {
              Sq++;
              continue;
            }
            const segmEntry segSq = m_vecSegments[3][Sq];

            // transform coordinates (diagonal -> horizontal)
            const int Sq_start_x = segSq.start.x + segSq.start.y;
            const int Sq_start_y = m_iWidth - 1 - segSq.start.x + segSq.start.y;
            const int Sq_end_x   = segSq.end.x + segSq.end.y;

            if(Sq_start_y > GAPy) { // window boundary reached ?
              Sq = segmCount;       // no candidate segment found
              break;
            }

            // positive slope or first candidate
            if(positive_slope || first_candidate) {
              if((Sq_start_x <= GAPx_pos)                    // Sq starts in window 
              && (Sq_start_x >= SS_end_x - segSS.len + 1)) { // Sq lies right from center of SS
                positive_slope = true;
                break;
              }
            }

            // negative slope or first candidate
            if(!positive_slope || first_candidate) {
              if((Sq_end_x >= GAPx_neg)                      // Sq ends in window 
              && (Sq_end_x <= SS_start_x + segSS.len - 1)) { // Sq lies left from center of SS
                positive_slope = false;
                break;
              }
            }

            Sq++;
          }

          if(Sq == segmCount) break;    // no candidate segment found

          const segmEntry segSi = m_vecSegments[3][Si];
          const segmEntry segSq = m_vecSegments[3][Sq];

          // calculate longest line segment SL (old SL or new Sq)
          if(m_vecSegments[3][SL].len < segSq.len) {
            SL = Sq;
          }
          const segmEntry segSL = m_vecSegments[3][SL];

          // STEP3
          // calculate slope
          float slope;
          if(positive_slope) {
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
          
          // if not satisfied
          if((deltaD > par.maxQuantizationError) || (deltaD < -par.maxQuantizationError)) {
            if(first_candidate) {
              positive_slope = true;    // reset slope flag
            }
            break;   
          }
          else first_candidate = false;

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

        if(positive_slope) {
          const int y1 = m_vecSegments[3][(*vecLineSegments)[0]].start.y;
          const int y2 = m_vecSegments[3][(*vecLineSegments)[nSegments-1]].end.y;
          length = y2 - y1 + 1;
        }
        else {
          const int x1 = m_vecSegments[3][(*vecLineSegments)[nSegments-1]].start.x;
          const int x2 = m_vecSegments[3][(*vecLineSegments)[0]].end.x;
          length = x2 - x1 + 1;
        }

        // line length must be >= minLineLen for extraction
        if(length >= par.minLineLen) {
        
          // register grouped segments as extracted line
          lineEntry newline;
          if(positive_slope) {
            newline.start.x = m_vecSegments[3][(*vecLineSegments)[0]].start.x;
            newline.start.y = m_vecSegments[3][(*vecLineSegments)[0]].start.y;
            newline.end.x = m_vecSegments[3][(*vecLineSegments)[nSegments-1]].end.x;
            newline.end.y = m_vecSegments[3][(*vecLineSegments)[nSegments-1]].end.y;
          }
          else {
            newline.end.x = m_vecSegments[3][(*vecLineSegments)[0]].end.x;
            newline.end.y = m_vecSegments[3][(*vecLineSegments)[0]].end.y;
            newline.start.x = m_vecSegments[3][(*vecLineSegments)[nSegments-1]].start.x;
            newline.start.y = m_vecSegments[3][(*vecLineSegments)[nSegments-1]].start.y;
          }
          // calculate midpoint
          newline.mid.x = (newline.start.x + newline.end.x)/2;
          newline.mid.y = (newline.start.y + newline.end.y)/2;
          // calculate tangent
          //newline.tangent = atan2((double)(newline.start.y-newline.end.y), (double)(newline.start.x-newline.end.x));
          newline.tangent = atan2((double)(newline.start.y-newline.end.y), (double)(newline.end.x-newline.start.x));
          // set other values
          newline.len = length;
          newline.used = 0;
          newline.group = 3;
          // store data
          newline.segmIdxList = vecLineSegments;
          m_vecLines[3].push_back(newline);
          m_vecLines[0].push_back(newline);


          for(int i=0; i<nSegments; i++) {
            // increase used-value of the grouped segments
            m_vecSegments[3][(*vecLineSegments)[i]].used++;
          }
        }
        else delete vecLineSegments;

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecSegments[3][Si].used == 0) break;
        }
        
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractGroup4Lines() {

    // clear old linelist and reserve memory
    clearLineList(4);
    m_vecLines[4].reserve(LinePreallocation);
  
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(m_vecSegments[4].size());
    
    if(segmCount > 0) {

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        // create new segment grouping vector
        std::vector<int> *vecLineSegments = new std::vector<int>;

        bool positive_slope = true;   // sign of the slope of the first segment pair
        bool first_candidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments->push_back(Si);  // store Si in linelist (starting segment)
        SS = Si;                        // set Si as new target segment SS for search
        SL = Si;                        // set Si as actual longest linesegment SL
        Sq = Si + 1;                    // first candidate segment


        while(Sq < segmCount) {  // track line segments
        
          const segmEntry segSS = m_vecSegments[4][SS];

          // transform coordinates (diagonal -> horizontal)
          const int SS_start_y = m_iWidth - 1 - segSS.start.x + segSS.start.y;
          const int SS_end_x   = segSS.end.x + segSS.end.y;
          const int SS_end_y   = m_iWidth - 1 - segSS.end.x + segSS.end.y;
          
          // STEP2
          // calculate window boundaries for target segment SS
          const int GAPx = par.maxSegmentGap;
          const int GAPx_pos = SS_start_y + 1 + GAPx*2;   // double gap + 1 
          const int GAPx_neg = SS_end_y - 1 - GAPx*2; // double gap + 1 
          const float deltaY = -0.5f; //SS_start_x - SS_end_x - 0.5;
          const float deltaX = static_cast<float>(SS_start_y - SS_end_y);
          const float min_slope = deltaY / deltaX;
          const float GAPy_fract = min_slope * (GAPx_neg - SS_start_y);
          const float GAPy_round = floor(GAPy_fract + 0.5f);
          const int GAPy = static_cast<int>(SS_end_x - GAPy_round);
          
          // search new candidate segment Sq (from right to left)
          while(Sq < segmCount) {
          
            // no multi-use of segments in circle extraction
            if(m_vecSegments[4][Sq].used > 0) {
              Sq++;
              continue;
            }
            const segmEntry segSq = m_vecSegments[4][Sq];

            // transform coordinates (diagonal -> horizontal)
            const int Sq_start_x = segSq.start.x + segSq.start.y;
            const int Sq_start_y = m_iWidth - 1 - segSq.start.x + segSq.start.y;
            const int Sq_end_y   = m_iWidth - 1 - segSq.end.x + segSq.end.y;

            if(Sq_start_x < GAPy) { // window boundary reached ?
              Sq = segmCount;       // no candidate segment found
              break;
            }

            // positive slope or first candidate
            if(positive_slope || first_candidate) {
              if((Sq_start_y >= GAPx_neg)                     // Sq starts in window 
              && (Sq_start_y <= SS_end_y + segSS.len - 1)) {  // Sq lies above from center of SS
                positive_slope = true;
                break;
              }
            }

            // negative slope or first candidate
            if(!positive_slope || first_candidate) {
              if((Sq_end_y <= GAPx_pos)                      // Sq ends in window 
              && (Sq_end_y >= SS_start_y - segSS.len + 1)) { // Sq lies below from center of SS
                positive_slope = false;
                break;
              }
            }

            Sq++;
          }

          if(Sq == segmCount) break;    // no candidate segment found

          const segmEntry segSi = m_vecSegments[4][Si];
          const segmEntry segSq = m_vecSegments[4][Sq];

          // calculate longest line segment SL (old SL or new Sq)
          if(m_vecSegments[4][SL].len < segSq.len) {
            SL = Sq;
          }
          const segmEntry segSL = m_vecSegments[4][SL];

          // STEP3
          // calculate slope
          float slope;
          if(positive_slope) {
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
          
          // if not satisfied
          if((deltaD > par.maxQuantizationError) || (deltaD < -par.maxQuantizationError)) {
            if(first_candidate) {
              positive_slope = true;    // reset slope flag
            }
            break;   
          }
          else first_candidate = false;

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

        if(positive_slope) {
          const int y1 = m_vecSegments[4][(*vecLineSegments)[nSegments-1]].end.y;
          const int y2 = m_vecSegments[4][(*vecLineSegments)[0]].start.y;
          length = y2 - y1 + 1;
        }
        else {
          const int x1 = m_vecSegments[4][(*vecLineSegments)[nSegments-1]].start.x;
          const int x2 = m_vecSegments[4][(*vecLineSegments)[0]].end.x;
          length = x2 - x1 + 1;
        }

        // line length must be >= minLineLen for extraction
        if(length >= par.minLineLen) {
        
          // register grouped segments as extracted line
          lineEntry newline;
          if(positive_slope) {
            newline.start.x = m_vecSegments[4][(*vecLineSegments)[nSegments-1]].end.x;
            newline.start.y = m_vecSegments[4][(*vecLineSegments)[nSegments-1]].end.y;
            newline.end.x = m_vecSegments[4][(*vecLineSegments)[0]].start.x;
            newline.end.y = m_vecSegments[4][(*vecLineSegments)[0]].start.y;
          }
          else {
            newline.start.x = m_vecSegments[4][(*vecLineSegments)[0]].end.x;
            newline.start.y = m_vecSegments[4][(*vecLineSegments)[0]].end.y;
            newline.end.x = m_vecSegments[4][(*vecLineSegments)[nSegments-1]].start.x;
            newline.end.y = m_vecSegments[4][(*vecLineSegments)[nSegments-1]].start.y;
          }
          // calculate midpoint
          newline.mid.x = (newline.start.x + newline.end.x)/2;
          newline.mid.y = (newline.start.y + newline.end.y)/2;
          // calculate tangent
          //newline.tangent = atan2((double)(newline.start.y-newline.end.y), (double)(newline.end.x-newline.start.x));
          newline.tangent = atan2((double)(newline.end.y-newline.start.y), (double)(newline.start.x-newline.end.x));
          // set other values
          newline.len = length;
          newline.used = 0;
          newline.group = 4;
          // store data
          newline.segmIdxList = vecLineSegments;
          m_vecLines[4].push_back(newline);
          m_vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecSegments[4][(*vecLineSegments)[i]].used++;
          }
        }
        else delete vecLineSegments;

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecSegments[4][Si].used == 0) break;
        }
        
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractGroup1Arcs() {

    // clear old arclist and reserve memory
    clearArcList(0);
    m_vecArcs[0].reserve(4*ArcPreallocation);
    clearArcList(1);
    m_vecArcs[1].reserve(ArcPreallocation);
    
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(m_vecLines[1].size());
    
    if(segmCount > 0) {

      int Si = 0;
      int Sq, SS, Sl;

      do {
        // create new line grouping vector
        std::vector<int> *vecArcSegments = new std::vector<int>;

        bool forward_search = false;  // search direction along the arc (forward = left->right)
        bool upward_search = false;   // search direction in the window (upward = bottom->up)
        bool extra_down = false;      // extra search cycle to continue inverted direction segments
        bool extra_up = false;        // extra search cycle to continue inverted direction segments

        // STEP1
        vecArcSegments->push_back(Si);  // store Si in arc segment list (starting line)
        SS = Si;                        // set Si as new target arc segment SS for search
        Sq = Si + 1;                    // first candidate segment
        Sl = -1;                        // last fitting candidate
        
        while(true) {   // track lines (line order: left->right, top->down)
          // STEP2
          const lineEntry segSS = m_vecLines[1][SS];
          
          int SSy, SSx;
          if(forward_search) {
            SSx = segSS.end.x;
            SSy = segSS.end.y;
          } else {
            SSx = segSS.start.x;
            SSy = segSS.start.y;
          }

          // calculate window
          const int GAPx_pos = par.maxLineGap;
          const int GAPx_neg = segSS.len/2 - 1;
          int GAPy_pos, GAPy_neg;
          if(upward_search) {
            GAPy_pos = par.maxLineGap - 1;
            GAPy_neg = 1;
          } else {
            GAPy_pos = 1;
            GAPy_neg = par.maxLineGap - 1;
          }
          const int minY = SSy - GAPy_pos;
          const int maxY = SSy + GAPy_neg;
          
          // search new candidate segment Sq
          while((Sq >= 0)&&(Sq < segmCount)) {

            const lineEntry segSq = m_vecLines[1][Sq];
            
            // vertical window boundary reached ? (both points outside the window)
            if(((segSq.start.y < minY)||(segSq.start.y > maxY)) 
            &&((segSq.end.y < minY)||(segSq.end.y > maxY))) { 
              Sq = -1;   // no candidate segment found
              break;
            }
            
            // calculate distance of Sq
            int dx,dy;
            if(forward_search) {
              dx = segSq.start.x - SSx;
              dy = segSq.start.y - SSy;
            } else {
              dx = SSx - segSq.end.x;
              dy = SSy - segSq.end.y;
            }             

            // calculate relative candidate angle
            const double angle = segSq.tangent - segSS.tangent;

            // check negative conditions
            if((dx > GAPx_pos)||(dx < -GAPx_neg)  // dx too large
            ||(dy > GAPy_pos)||(dy < -GAPy_neg)   // dy too large
            ||((dx <= 0)&&((dy > 1)||(dy < -1)))  // parallel gap too large
            ||((dx > 0)&&((dy > dx)||(dy < -dx))) // parallel gap too large
            ||((forward_search)&&(upward_search)&&(angle < 0))  // tangent directions
            ||((forward_search)&&(!upward_search)&&(angle > 0))
            ||((!forward_search)&&(upward_search)&&(angle > 0))
            ||((!forward_search)&&(!upward_search)&&(angle < 0))) {

              if(upward_search) {
                Sq--;
              } else {
                Sq++;
              }
              continue;
            }
            
            break;
          }

          if((Sq == segmCount)||(Sq < 0)) {   // no candidate segment found

            // next search cycle (STAGE 1: backward_down/extra_up -> backward_up/extra_down
            //                    STAGE 2: forward_down/extra_up -> forward_up/extra_down 
            //                    STAGE 3: end)
            if(upward_search) {
              if(extra_up) {            // NEXT: backward_up/forward_up
                extra_up = false;
                SS = Si;                // reset target segment
                Sq = Si - 1;            // first upward_search candidate
                Sl = -1;                // reset last fitting candidate
                continue;
              }
              if(!extra_down) {         // NEXT: extra_down
                extra_down = true;
                upward_search = false;  // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl + 1;
                }
                continue;
              }
            
            } else {  // down
              
              if(extra_down) {          // NEXT: forward_down/end
                extra_down = false;
                if(forward_search) {    // set next search stage
                  break;  // END
                } else {
                  forward_search = true;  // START FORWARD_SEARCH
                }
                SS = Si;                // reset target segment
                Sq = Si + 1;            // first forward_search candidate
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
              
              if(!extra_up) {           // !extra_up
                extra_up = true;        
                upward_search = true;   // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl - 1;
                }
                continue;
              }
            }
          }


        
          // STEP3: check interior angle
          const double tangent1 = m_vecLines[1][SS].tangent;
          const double tangent2 = m_vecLines[1][Sq].tangent;
          double interiorAngle = (tangent2-tangent1)*180/PI;
          if(interiorAngle < 0) interiorAngle = -interiorAngle;
          interiorAngle = 180 - interiorAngle;

          if((interiorAngle <= 180)&&(interiorAngle >= 135)) {
            double Xest, Yest, R2est;
            vecArcSegments->push_back(Sq);  // store Sq in linelist
            // estimate circle parameters
            if(!estimateCircle(*vecArcSegments, 1, Xest, Yest, R2est)) {
              vecArcSegments->pop_back();   // circle estimation failed
            } else {
              // STEP4: check tangents
              const double deltaX1 = Xest - m_vecLines[1][SS].mid.x;
              const double deltaY1 = Yest - m_vecLines[1][SS].mid.y;
              double TetaEst1 = atan2(deltaX1, deltaY1);
              if((deltaY1 < 0)&&(deltaX1 >= 0)) TetaEst1 -= PI;
              if((deltaY1 < 0)&&(deltaX1 < 0)) TetaEst1 += PI;
  
              const double deltaX2 = Xest - m_vecLines[1][Sq].mid.x;
              const double deltaY2 = Yest - m_vecLines[1][Sq].mid.y;
              double TetaEst2 = atan2(deltaX2, deltaY2);
              if((deltaY2 < 0)&&(deltaX2 >= 0)) TetaEst2 -= PI;
              if((deltaY2 < 0)&&(deltaX2 < 0)) TetaEst2 += PI;
  
              if(((TetaEst1-tangent1)*180/PI > par.maxLineTangentError)
              ||((TetaEst1-tangent1)*180/PI < -par.maxLineTangentError)
              ||((TetaEst2-tangent2)*180/PI > par.maxLineTangentError)
              ||((TetaEst2-tangent2)*180/PI < -par.maxLineTangentError)) {
                vecArcSegments->pop_back(); // error exceeds maximum
              } else {
                SS = Sq;    // set Sq as new target segment SS
                Sl = Sq;    // last fitting candidate
              }
            } //circle estimation
          } //interior angle
          
          if(upward_search) {
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
        double Xest, Yest, R2est;
        if(estimateCircle(*vecArcSegments, 1, Xest, Yest, R2est)) {
          x = static_cast<int>(floor(Xest + 0.5));
          y = static_cast<int>(floor(Yest + 0.5));
          r2 = static_cast<int>(floor(R2est + 0.5));
        } else {
          r2 = 0;   // estimation failed
        }

        // radius must be greater than 10 pixel for extraction
        // at least 2 segments are neccessary for circle estimation
        if((r2 >= 100)&&(nSegments >= 2)) {

      // EXTRACT ARC
          // register grouped segments as extracted arc
          arcEntry newarc;
          // start/end points
          const int x1 = m_vecLines[1][(*vecArcSegments)[0]].start.x;
          const int y1 = m_vecLines[1][(*vecArcSegments)[0]].start.y;
          const int x2 = m_vecLines[1][(*vecArcSegments)[nSegments-1]].end.x;
          const int y2 = m_vecLines[1][(*vecArcSegments)[nSegments-1]].end.y;
          
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
            newarc.firstVec = m_vecLines[1][(*vecArcSegments)[0]].end 
                            - m_vecLines[1][(*vecArcSegments)[0]].start;
            newarc.lastVec = m_vecLines[1][(*vecArcSegments)[nSegments-1]].end 
                           - m_vecLines[1][(*vecArcSegments)[nSegments-1]].start;
          } else {
            newarc.firstVec = m_vecLines[1][(*vecArcSegments)[0]].start
                            - m_vecLines[1][(*vecArcSegments)[0]].end;
            newarc.lastVec = m_vecLines[1][(*vecArcSegments)[nSegments-1]].start
                           - m_vecLines[1][(*vecArcSegments)[nSegments-1]].end;
          }
          
          // currently only 4 groups for circle arcs
          newarc.group = 1;
          // midpoint
          newarc.mid.x = x;
          newarc.mid.y = y;
          // other values
          newarc.r2 = r2;
          newarc.used = 0;
          // line index list
          newarc.lineIdxList = vecArcSegments;
          
          // store data
          m_vecArcs[newarc.group].push_back(newarc);
          m_vecArcs[0].push_back(newarc);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecLines[1][(*vecArcSegments)[i]].used++;
          }
        } else {
          delete vecArcSegments;
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecLines[1][Si].used == 0) break;
        }
      }
      while(Si < segmCount);    // extract next arc if possible
    }
  }


///////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractGroup2Arcs() {

    // clear old arclist and reserve memory
    clearArcList(2);
    m_vecArcs[2].reserve(ArcPreallocation);

    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(m_vecLines[2].size());
    
    if(segmCount > 0) {

      int Si = 0;
      int Sq, SS, Sl;

      do {
        // create new line grouping vector
        std::vector<int> *vecArcSegments = new std::vector<int>;

        bool forward_search = false;  // search direction along the arc (forward = top->down)
        bool upward_search = false;   // search direction in the window (upward = right->left)
        bool extra_down = false;      // extra search cycle to continue inverted direction segments
        bool extra_up = false;        // extra search cycle to continue inverted direction segments

        
        // STEP1
        vecArcSegments->push_back(Si);  // store Si in arc segment list (starting line)
        SS = Si;                        // set Si as new target arc segment SS for search
        Sq = Si + 1;                    // first candidate segment
        Sl = -1;                        // last fitting candidate
        
        while(true) {   // track lines (line order: top->down, left->right)
          // STEP2
          const lineEntry segSS = m_vecLines[2][SS];
          
          int SSy, SSx;
          if(forward_search) {
            SSx = segSS.end.x;
            SSy = segSS.end.y;
          } else {
            SSx = segSS.start.x;
            SSy = segSS.start.y;
          }

          // calculate window
          const int GAPy_pos = par.maxLineGap;
          const int GAPy_neg = segSS.len/2 - 1;
          int GAPx_pos, GAPx_neg;
          if(upward_search) {
            GAPx_pos = par.maxLineGap - 1;
            GAPx_neg = 1;
          } else {
            GAPx_pos = 1;
            GAPx_neg = par.maxLineGap - 1;
          }
          const int minX = SSx - GAPx_pos;
          const int maxX = SSx + GAPx_neg;

          // search new candidate segment Sq
          while((0 <= Sq)&&(Sq < segmCount)) {

            const lineEntry segSq = m_vecLines[2][Sq];

            // horizontal window boundary reached ? (both points outside the window)
            if(((segSq.end.x < minX)||(segSq.end.x > maxX)) 
            &&((segSq.start.x < minX)||(segSq.start.x > maxX))) { 
              Sq = -1;   // no candidate segment found
              break;
            }

            // calculate of Sq
            int dx,dy;
            if(forward_search) {
              dx = segSq.start.x - SSx;
              dy = segSq.start.y - SSy;
            } else {
              dx = SSx - segSq.end.x;
              dy = SSy - segSq.end.y;
            }             

            // calculate relative candidate angle
            const double angle = segSq.tangent - segSS.tangent;

            // check negative conditions
            if((dy > GAPy_pos)||(dy < -GAPy_neg)  // dy too large
            ||(dx > GAPx_pos)||(dx < -GAPx_neg)   // dx too large
            ||((dy <= 0)&&((dx > 1)||(dx < -1)))  // parallel gap too large
            ||((dy > 0)&&((dx > dy)||(dx < -dy))) // parallel gap too large
            ||((forward_search)&&(upward_search)&&(angle > 0))  // tangent directions (grp2: false tangent sign)
            ||((forward_search)&&(!upward_search)&&(angle < 0))
            ||((!forward_search)&&(upward_search)&&(angle < 0))
            ||((!forward_search)&&(!upward_search)&&(angle > 0))) {

              if(upward_search) {
                Sq--;
              } else {
                Sq++;
              }
              continue;
            }
            
            break;
          }

          if((Sq == segmCount)||(Sq < 0)) {   // no candidate segment found

            // next search cycle (STAGE 1: backward_down/extra_up -> backward_up/extra_down
            //                    STAGE 2: forward_down/extra_up -> forward_up/extra_down 
            //                    STAGE 3: end)
            if(upward_search) {
              if(extra_up) {            // NEXT: backward_up/forward_up
                extra_up = false;
                SS = Si;                // reset target segment
                Sq = Si - 1;            // first upward_search candidate
                Sl = -1;                // reset last fitting candidate
                continue;
              }
              if(!extra_down) {         // NEXT: extra_down
                extra_down = true;
                upward_search = false;  // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl + 1;
                }
                continue;
              }
            } else {  // down
              
              if(extra_down) {          // NEXT: forward_down/end
                extra_down = false;
                if(forward_search) {    // set next search stage
                  break;  // END
                } else {
                  forward_search = true;  // START FORWARD_SEARCH
                }
                SS = Si;                // reset target segment
                Sq = Si + 1;            // first forward_search candidate
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
              
              if(!extra_up) {           // extra_up
                extra_up = true;        
                upward_search = true;   // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl - 1;
                }
                continue;
              }
            }
          }
        
          // STEP3: check interior angle
          const double tangent1 = m_vecLines[2][SS].tangent;
          const double tangent2 = m_vecLines[2][Sq].tangent;
          double interiorAngle = (tangent2-tangent1)*180/PI;
          if(interiorAngle < 0) interiorAngle = -interiorAngle;
          interiorAngle = 180 - interiorAngle;

          if((interiorAngle <= 180)&&(interiorAngle >= 135)) {
            double Xest, Yest, R2est;
            vecArcSegments->push_back(Sq);  // store Sq in linelist
            // estimate circle parameters
            if(!estimateCircle(*vecArcSegments, 2, Xest, Yest, R2est)) {
              vecArcSegments->pop_back();   // circle estimation failed
            } else {
              // STEP4: check tangents
              const double deltaX1 = Xest - m_vecLines[2][SS].mid.x;
              const double deltaY1 = Yest - m_vecLines[2][SS].mid.y;
              double TetaEst1 = atan2(deltaX1, deltaY1);
              if((deltaX1 < 0)&&(deltaY1 >= 0)) TetaEst1 += PI;
              if((deltaX1 < 0)&&(deltaY1 < 0)) TetaEst1 += PI;
              
              const double deltaX2 = Xest - m_vecLines[2][Sq].mid.x;
              const double deltaY2 = Yest - m_vecLines[2][Sq].mid.y;
              double TetaEst2 = atan2(deltaX2, deltaY2);
              if((deltaX2 < 0)&&(deltaY2 >= 0)) TetaEst2 += PI;
              if((deltaX2 < 0)&&(deltaY2 < 0)) TetaEst2 += PI;
  
              if(((TetaEst1-tangent1)*180/PI > par.maxLineTangentError)
              ||((TetaEst1-tangent1)*180/PI < -par.maxLineTangentError)
              ||((TetaEst2-tangent2)*180/PI > par.maxLineTangentError)
              ||((TetaEst2-tangent2)*180/PI < -par.maxLineTangentError)) {
                // tangent estimation error exceeded maximum
                vecArcSegments->pop_back();
              } else {
                SS = Sq;    // set Sq as new target segment SS
                Sl = Sq;    // last fitting candidate
              }
            }
          }

          if(upward_search) {
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
        double Xest, Yest, R2est;
        if(estimateCircle(*vecArcSegments, 2, Xest, Yest, R2est)) {
          x = static_cast<int>(floor(Xest + 0.5));
          y = static_cast<int>(floor(Yest + 0.5));
          r2 = static_cast<int>(floor(R2est + 0.5));
        } else {
          r2 = 0;   // estimation failed
        }

        // radius must be greater than 10 pixel for extraction
        // at least 2 segments are neccessary for circle estimation
        if((r2 >= 100)&&(nSegments >= 2)) {
          
      // EXTRACT ARC
          // register grouped segments as extracted arc
          arcEntry newarc;
          // start/end points
          const int x1 = m_vecLines[2][(*vecArcSegments)[0]].start.x;
          const int y1 = m_vecLines[2][(*vecArcSegments)[0]].start.y;
          const int x2 = m_vecLines[2][(*vecArcSegments)[nSegments-1]].end.x;
          const int y2 = m_vecLines[2][(*vecArcSegments)[nSegments-1]].end.y;

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
            newarc.firstVec = m_vecLines[2][(*vecArcSegments)[0]].end 
                            - m_vecLines[2][(*vecArcSegments)[0]].start;
            newarc.lastVec = m_vecLines[2][(*vecArcSegments)[nSegments-1]].end 
                           - m_vecLines[2][(*vecArcSegments)[nSegments-1]].start;
          } else {
            newarc.firstVec = m_vecLines[2][(*vecArcSegments)[0]].start
                            - m_vecLines[2][(*vecArcSegments)[0]].end;
            newarc.lastVec = m_vecLines[2][(*vecArcSegments)[nSegments-1]].start
                           - m_vecLines[2][(*vecArcSegments)[nSegments-1]].end;
          }

          // currently only 4 groups for circle arcs
          newarc.group = 2;
          // midpoint
          newarc.mid.x = x;
          newarc.mid.y = y;
          // other values
          newarc.r2 = r2;
          newarc.used = 0;
          // line index list
          newarc.lineIdxList = vecArcSegments;
          
          // store data
          m_vecArcs[newarc.group].push_back(newarc);
          m_vecArcs[0].push_back(newarc);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecLines[2][(*vecArcSegments)[i]].used++;
          }
        } else {
          delete vecArcSegments;
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecLines[2][Si].used == 0) break;
        }
      }
      while(Si < segmCount);    // extract next arc if possible
    }
  }
  
///////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractGroup3Arcs() {
    
    // clear old arclist and reserve memory
    clearArcList(3);
    m_vecArcs[3].reserve(ArcPreallocation);

    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(m_vecLines[3].size());
    
    if(segmCount > 0) {

      int Si = 0;
      int Sq, SS, Sl;

      do {
        // create new line grouping vector
        std::vector<int> *vecArcSegments = new std::vector<int>;

        bool forward_search = false;  // search direction along the arc (forward = left->right)
        bool upward_search = false;   // search direction in the window (upward = bottom->up)
        bool extra_down = false;      // extra search cycle to continue inverted direction segments
        bool extra_up = false;        // extra search cycle to continue inverted direction segments

        // STEP1
        vecArcSegments->push_back(Si);  // store Si in arc segment list (starting line)
        SS = Si;                        // set Si as new target arc segment SS for search
        Sq = Si + 1;                    // first candidate segment
        Sl = -1;                        // last fitting candidate
        
        while(true) {   // track lines (line order: left->right, top->down)
          // STEP2
          const lineEntry segSS = m_vecLines[3][SS];

          // transform coordinates (diagonal -> horizontal)
          const int SS_start_x = segSS.start.x + segSS.start.y;
          const int SS_start_y = m_iWidth - 1 - segSS.start.x + segSS.start.y;
          const int SS_end_x   = segSS.end.x + segSS.end.y;
          const int SS_end_y   = m_iWidth - 1 - segSS.end.x + segSS.end.y;
          
          int SSy, SSx;
          if(forward_search) {
            SSx = SS_end_x;
            SSy = SS_end_y;
          } else {
            SSx = SS_start_x;
            SSy = SS_start_y;
          }

          // calculate window
          const int GAPx_pos = par.maxLineGap*2 + 1;
          const int GAPx_neg = segSS.len/2;
          int GAPy_pos, GAPy_neg;
          if(upward_search) {
            GAPy_pos = par.maxLineGap;
            GAPy_neg = 1;
          } else {
            GAPy_pos = 1;
            GAPy_neg = par.maxLineGap;
          }
          const int minY = SSy - GAPy_pos;
          const int maxY = SSy + GAPy_neg;


          // search new candidate segment Sq
          while((0 <= Sq)&&(Sq < segmCount)) {

            const lineEntry segSq = m_vecLines[3][Sq];

            // transform coordinates (diagonal -> horizontal)
            const int Sq_start_x = segSq.start.x + segSq.start.y;
            const int Sq_start_y = m_iWidth - 1 - segSq.start.x + segSq.start.y;
            const int Sq_end_x   = segSq.end.x + segSq.end.y;
            const int Sq_end_y   = m_iWidth - 1 - segSq.end.x + segSq.end.y;

            // vertical window boundary reached ? (both points outside the window)
            if(((Sq_start_y < minY)||(Sq_start_y > maxY)) 
            &&((Sq_end_y < minY)||(Sq_end_y > maxY))) { 
              Sq = -1;   // no candidate segment found
              break;
            }

            // calculate distance of Sq
            int dx,dy;
            if(forward_search) {
              dx = Sq_start_x - SSx;
              dy = Sq_start_y - SSy;
            } else {
              dx = SSx - Sq_end_x;
              dy = SSy - Sq_end_y;
            }             

            // calculate relative candidate angle
            const double angle = segSq.tangent - segSS.tangent;

            // check negative conditions
            if((dx > GAPx_pos)||(dx < -GAPx_neg)      // dx too large
            ||(dy > GAPy_pos)||(dy < -GAPy_neg)       // dy too large
            ||((dx <= 1)&&((dy > 1)||(dy < -1)))      // parallel gap too large
            ||((dx > 1)&&((dy > dx/2)||(dy < -dx/2))) // parallel gap too large
            ||((forward_search)&&(upward_search)&&(angle < 0))  // tangent directions
            ||((forward_search)&&(!upward_search)&&(angle > 0))
            ||((!forward_search)&&(upward_search)&&(angle > 0))
            ||((!forward_search)&&(!upward_search)&&(angle < 0))) {

              if(upward_search) {
                Sq--;
              } else {
                Sq++;
              }
              continue;
            }
            
            break;
          }

          if((Sq == segmCount)||(Sq < 0)) {   // no candidate segment found

            // next search cycle (STAGE 1: backward_down/extra_up -> backward_up/extra_down
            //                    STAGE 2: forward_down/extra_up -> forward_up/extra_down 
            //                    STAGE 3: end)
            if(upward_search) {
              if(extra_up) {            // NEXT: backward_up/forward_up
                extra_up = false;
                SS = Si;                // reset target segment
                Sq = Si - 1;            // first upward_search candidate
                Sl = -1;                // reset last fitting candidate
                continue;
              }
              if(!extra_down) {         // NEXT: extra_down
                extra_down = true;
                upward_search = false;  // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl + 1;
                }
                continue;
              }
            } else {  // down
              
              if(extra_down) {          // NEXT: forward_down/end
                extra_down = false;
                if(forward_search) {    // set next search stage
                  break;  // END
                } else {
                  forward_search = true;  // START FORWARD_SEARCH
                }
                SS = Si;                // reset target segment
                Sq = Si + 1;            // first forward_search candidate
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
              
              if(!extra_up) {           // extra_up
                extra_up = true;        
                upward_search = true;   // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl - 1;
                }
                continue;
              }
            }
          }

          // STEP3: check interior angle
          const double tangent1 = m_vecLines[3][SS].tangent;
          const double tangent2 = m_vecLines[3][Sq].tangent;
          double interiorAngle = (tangent2-tangent1)*180/PI;
          if(interiorAngle < 0) interiorAngle = -interiorAngle;
          interiorAngle = 180 - interiorAngle;

          if((interiorAngle <= 180)&&(interiorAngle >= 135)) {
            double Xest, Yest, R2est;
            vecArcSegments->push_back(Sq);  // store Sq in linelist
            // estimate circle parameters
            if(!estimateCircle(*vecArcSegments, 3, Xest, Yest, R2est)) {
              vecArcSegments->pop_back();   // circle estimation failed
            } else {
              // STEP4: check tangents
              const double deltaX1 = Xest - m_vecLines[3][SS].mid.x;
              const double deltaY1 = Yest - m_vecLines[3][SS].mid.y;
              double TetaEst1 = atan2(deltaX1, deltaY1);
              if((deltaY1 < 0)&&(deltaX1 >= 0)) TetaEst1 -= PI;
              if((deltaY1 < 0)&&(deltaX1 < 0)) TetaEst1 += PI;
  
              const double deltaX2 = Xest - m_vecLines[3][Sq].mid.x;
              const double deltaY2 = Yest - m_vecLines[3][Sq].mid.y;
              double TetaEst2 = atan2(deltaX2, deltaY2);
              if((deltaY2 < 0)&&(deltaX2 >= 0)) TetaEst2 -= PI;
              if((deltaY2 < 0)&&(deltaX2 < 0)) TetaEst2 += PI;
  
              if(((TetaEst1-tangent1)*180/PI > par.maxLineTangentError)
              ||((TetaEst1-tangent1)*180/PI < -par.maxLineTangentError)
              ||((TetaEst2-tangent2)*180/PI > par.maxLineTangentError)
              ||((TetaEst2-tangent2)*180/PI < -par.maxLineTangentError)) {
                // tangent estimation error exceeded maximum
                vecArcSegments->pop_back();
              } else {
                SS = Sq;    // set Sq as new target segment SS
                Sl = Sq;    // last fitting candidate
              }
            }
          }

          if(upward_search) {
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
        double Xest, Yest, R2est;
        if(estimateCircle(*vecArcSegments, 3, Xest, Yest, R2est)) {
          x = static_cast<int>(floor(Xest + 0.5));
          y = static_cast<int>(floor(Yest + 0.5));
          r2 = static_cast<int>(floor(R2est + 0.5));
        } else {
          r2 = 0;   // estimation failed
        }

        // radius must be greater than 10 pixel for extraction
        // at least 2 segments are neccessary for circle estimation
        if((r2 >= 100)&&(nSegments >= 2)) {

      // EXTRACT ARC
          // register grouped segments as extracted arc
          arcEntry newarc;
          // start/end points
          const int x1 = m_vecLines[3][(*vecArcSegments)[0]].start.x;
          const int y1 = m_vecLines[3][(*vecArcSegments)[0]].start.y;
          const int x2 = m_vecLines[3][(*vecArcSegments)[nSegments-1]].end.x;
          const int y2 = m_vecLines[3][(*vecArcSegments)[nSegments-1]].end.y;

          // transform coordinates (diagonal -> horizontal)
          const int X1 = x1 + y1;
          const int X2 = x2 + y2;
          const int Y1 = m_iWidth - 1 - x1 + y1;
          const int Y = m_iWidth - 1 - x + y;

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
            newarc.firstVec = m_vecLines[3][(*vecArcSegments)[0]].end 
                            - m_vecLines[3][(*vecArcSegments)[0]].start;
            newarc.lastVec = m_vecLines[3][(*vecArcSegments)[nSegments-1]].end 
                           - m_vecLines[3][(*vecArcSegments)[nSegments-1]].start;
          } else {
            newarc.firstVec = m_vecLines[3][(*vecArcSegments)[0]].start
                            - m_vecLines[3][(*vecArcSegments)[0]].end;
            newarc.lastVec = m_vecLines[3][(*vecArcSegments)[nSegments-1]].start
                           - m_vecLines[3][(*vecArcSegments)[nSegments-1]].end;
          }
          
          // currently only 4 groups for circle arcs
          newarc.group = 3;
          // midpoint
          newarc.mid.x = x;
          newarc.mid.y = y;
          // other values
          newarc.r2 = r2;
          newarc.used = 0;
          // line index list
          newarc.lineIdxList = vecArcSegments;
          
          // store data
          m_vecArcs[newarc.group].push_back(newarc);
          m_vecArcs[0].push_back(newarc);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecLines[3][(*vecArcSegments)[i]].used++;
          }
        } else {
          delete vecArcSegments;
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecLines[3][Si].used == 0) break;
        }
      }
      while(Si < segmCount);    // extract next arc if possible
    }
  }


///////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractGroup4Arcs() {

    // clear old arclist and reserve memory
    clearArcList(4);
    m_vecArcs[4].reserve(ArcPreallocation);

    // get parameters
    const parameters& par = getParameters();

    const int segmCount = static_cast<int>(m_vecLines[4].size());
    
    if(segmCount > 0) {

      int Si = 0;
      int Sq, SS, Sl;

      do {
        // create new line grouping vector
        std::vector<int> *vecArcSegments = new std::vector<int>;

        bool forward_search = false;  // search direction along the arc (forward = bottom->up)
        bool upward_search = false;   // search direction in the window (upward = left->right)
        bool extra_down = false;      // extra search cycle to continue inverted direction segments
        bool extra_up = false;        // extra search cycle to continue inverted direction segments

        // STEP1
        vecArcSegments->push_back(Si);  // store Si in arc segment list (starting line)
        SS = Si;                        // set Si as new target arc segment SS for search
        Sq = Si + 1;                    // first candidate segment
        Sl = -1;                        // last fitting candidate

        while(true) {   // track lines (line order: bottom->up, right->left)
          // STEP2
          const lineEntry segSS = m_vecLines[4][SS];

          // transform coordinates (diagonal -> horizontal)
          const int SS_start_x = segSS.start.x + segSS.start.y;
          const int SS_start_y = m_iWidth - 1 - segSS.start.x + segSS.start.y;
          const int SS_end_x   = segSS.end.x + segSS.end.y;
          const int SS_end_y   = m_iWidth - 1 - segSS.end.x + segSS.end.y;

          int SSy, SSx;
          if(forward_search) {
            SSx = SS_start_x;
            SSy = SS_start_y;
          } else {
            SSx = SS_end_x;
            SSy = SS_end_y;
          }
          
          // calculate window
          const int GAPy_pos = par.maxLineGap*2 + 1;
          const int GAPy_neg = segSS.len/2;
          int GAPx_pos, GAPx_neg;
          if(upward_search) {
            GAPx_pos = 1;
            GAPx_neg = par.maxLineGap;
          } else {
            GAPx_pos = par.maxLineGap;
            GAPx_neg = 1;
          }
          const int minX = SSx - GAPx_pos;      // grpIV: inverted search direction (compared to grpII)
          const int maxX = SSx + GAPx_neg;


          // search new candidate segment Sq
          while((0 <= Sq)&&(Sq < segmCount)) {

            const lineEntry segSq = m_vecLines[4][Sq];

            // transform coordinates (diagonal -> horizontal)
            const int Sq_start_x = segSq.start.x + segSq.start.y;
            const int Sq_start_y = m_iWidth - 1 - segSq.start.x + segSq.start.y;
            const int Sq_end_x   = segSq.end.x + segSq.end.y;
            const int Sq_end_y   = m_iWidth - 1 - segSq.end.x + segSq.end.y;


            // horizontal window boundary reached ? (both points outside the window)
            if(((Sq_end_x < minX)||(Sq_end_x > maxX)) 
            &&((Sq_start_x < minX)||(Sq_start_x > maxX))) { 
              Sq = -1;   // no candidate segment found
              break;
            }

            // calculate distance of Sq
            int dx,dy;
            if(forward_search) {
              dx = SSx - Sq_end_x;
              dy = SSy - Sq_end_y;
            } else {
              dx = Sq_start_x - SSx;
              dy = Sq_start_y - SSy;
            }

            // calculate relative candidate angle
            const double angle = segSq.tangent - segSS.tangent;

            // check negative conditions
            if((dy > GAPy_pos)||(dy < -GAPy_neg)      // dy too large
            ||(dx > GAPx_pos)||(dx < -GAPx_neg)       // dx too large
            ||((dy <= 1)&&((dx > 1)||(dx < -1)))      // parallel gap too large
            ||((dy > 1)&&((dx > dy/2)||(dx > dy/2)))  // parallel gap too large
            ||((forward_search)&&(upward_search)&&(angle > 0))  // tangent directions
            ||((forward_search)&&(!upward_search)&&(angle < 0))
            ||((!forward_search)&&(upward_search)&&(angle < 0))
            ||((!forward_search)&&(!upward_search)&&(angle > 0))) {

              if(upward_search) {
                Sq--;
              } else {
                Sq++;
              }
              continue;
            }
            
            break;
          }

          if((Sq == segmCount)||(Sq < 0)) {   // no candidate segment found

            // next search cycle (STAGE 1: backward_down/extra_up -> backward_up/extra_down
            //                    STAGE 2: forward_down/extra_up -> forward_up/extra_down 
            //                    STAGE 3: end)
            if(upward_search) {
              if(extra_up) {            // NEXT: backward_up/forward_up
                extra_up = false;
                SS = Si;                // reset target segment
                Sq = Si - 1;            // first upward_search candidate
                Sl = -1;                // reset last fitting candidate
                continue;
              }
              if(!extra_down) {         // NEXT: extra_down
                extra_down = true;
                upward_search = false;  // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl + 1;
                }
                continue;
              }
            } else {  // down
              
              if(extra_down) {          // NEXT: forward_down/end
                extra_down = false;
                if(forward_search) {    // set next search stage
                  break;  // END
                } else {
                  forward_search = true;  // START FORWARD_SEARCH
                }
                SS = Si;                // reset target segment
                Sq = Si + 1;            // first forward_search candidate
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
              
              if(!extra_up) {           // extra_up
                extra_up = true;        
                upward_search = true;   // invert search direction
                if(Sl != -1) {
                  SS = Sl;              // if possible continue with last fitting candidate Sl
                  Sq = Sl - 1;
                }
                continue;
              }
            }
          }

          // STEP3: check interior angle
          const double tangent1 = m_vecLines[4][SS].tangent;
          const double tangent2 = m_vecLines[4][Sq].tangent;
          double interiorAngle = (tangent2-tangent1)*180/PI;
          if(interiorAngle < 0) interiorAngle = -interiorAngle;
          interiorAngle = 180 - interiorAngle;

          if((interiorAngle <= 180)&&(interiorAngle >= 135)) {
            double Xest, Yest, R2est;
            vecArcSegments->push_back(Sq);  // store Sq in linelist
            // estimate circle parameters
            if(!estimateCircle(*vecArcSegments, 4, Xest, Yest, R2est)) {
              vecArcSegments->pop_back();   // circle estimation failed
            } else {
              // STEP4: check tangents
              const double deltaX1 = Xest - m_vecLines[4][SS].mid.x;
              const double deltaY1 = Yest - m_vecLines[4][SS].mid.y;
              double TetaEst1 = atan2(deltaX1, deltaY1);
              if((deltaX1 < 0)&&(deltaY1 >= 0)) TetaEst1 += PI;
              if((deltaX1 < 0)&&(deltaY1 < 0)) TetaEst1 += PI;
  
              const double deltaX2 = Xest - m_vecLines[4][Sq].mid.x;
              const double deltaY2 = Yest - m_vecLines[4][Sq].mid.y;
              double TetaEst2 = atan2(deltaX2, deltaY2);
              if((deltaX2 < 0)&&(deltaY2 >= 0)) TetaEst2 += PI;
              if((deltaX2 < 0)&&(deltaY2 < 0)) TetaEst2 += PI;
  
              if(((TetaEst1-tangent1)*180/PI > par.maxLineTangentError)
              ||((TetaEst1-tangent1)*180/PI < -par.maxLineTangentError)
              ||((TetaEst2-tangent2)*180/PI > par.maxLineTangentError)
              ||((TetaEst2-tangent2)*180/PI < -par.maxLineTangentError)) {
                // tangent estimation error exceeded maximum
                vecArcSegments->pop_back();
              } else {
                SS = Sq;    // set Sq as new target segment SS
                Sl = Sq;    // last fitting candidate
              }
            }
          }

          if(upward_search) {
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
        double Xest, Yest, R2est;
        if(estimateCircle(*vecArcSegments, 4, Xest, Yest, R2est)) {
          x = static_cast<int>(floor(Xest + 0.5));
          y = static_cast<int>(floor(Yest + 0.5));
          r2 = static_cast<int>(floor(R2est + 0.5));
        } else {
          r2 = 0;   // estimation failed
        }

        // radius must be greater than 10 pixel for extraction
        // at least 2 segments are neccessary for circle estimation
        if((r2 >= 100)&&(nSegments >= 2)) {

      // EXTRACT ARC
          // register grouped segments as extracted arc
          arcEntry newarc;
          // start/end points

          // TODO: fix wrong segment order
          for(int j=0; j<nSegments/2; j++) {
            int swap = (*vecArcSegments)[j];
            (*vecArcSegments)[j] = (*vecArcSegments)[nSegments-1-j];
            (*vecArcSegments)[nSegments-1-j] = swap;
          }

          const int x1 = m_vecLines[4][(*vecArcSegments)[0]].start.x;
          const int y1 = m_vecLines[4][(*vecArcSegments)[0]].start.y;
          const int x2 = m_vecLines[4][(*vecArcSegments)[nSegments-1]].end.x;
          const int y2 = m_vecLines[4][(*vecArcSegments)[nSegments-1]].end.y;

          // transform coordinates (diagonal -> horizontal)
          const int X1 = x1 + y1;
          const int Y1 = m_iWidth - 1 - x1 + y1;
          const int Y2 = m_iWidth - 1 - x2 + y2;
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
            newarc.firstVec = m_vecLines[4][(*vecArcSegments)[0]].end 
                            - m_vecLines[4][(*vecArcSegments)[0]].start;
            newarc.lastVec = m_vecLines[4][(*vecArcSegments)[nSegments-1]].end 
                           - m_vecLines[4][(*vecArcSegments)[nSegments-1]].start;
          } else {
            newarc.firstVec = m_vecLines[4][(*vecArcSegments)[0]].start
                            - m_vecLines[4][(*vecArcSegments)[0]].end;
            newarc.lastVec = m_vecLines[4][(*vecArcSegments)[nSegments-1]].start
                           - m_vecLines[4][(*vecArcSegments)[nSegments-1]].end;
          }
          
          // currently only 4 groups for circle arcs
          newarc.group = 4;
          // midpoint
          newarc.mid.x = x;
          newarc.mid.y = y;
          // other values
          newarc.r2 = r2;
          newarc.used = 0;
          // line index list
          newarc.lineIdxList = vecArcSegments;
          
          // store data
          m_vecArcs[newarc.group].push_back(newarc);
          m_vecArcs[0].push_back(newarc);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecLines[4][(*vecArcSegments)[i]].used++;
          }
        } else {
          delete vecArcSegments;
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecLines[4][Si].used == 0) break;
        }
      }
      while(Si < segmCount);    // extract next arc if possible
    }
  }


///////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractExtGroup1Arcs() {
    
    // get parameters
    const parameters& par = getParameters();
    const float maxLineTangentError = par.maxArcTangentError;

    const int targetCount = static_cast<int>(m_vecArcs[1].size());
    const int candidateCount = static_cast<int>(m_vecArcs[3].size());

    // clear old arclist and reserve memory
    clearExtArcList(0);
    clearExtArcList(1);
    m_vecExtArcs[0].reserve(ExtArcPreallocation);
    m_vecExtArcs[1].reserve(ExtArcPreallocation);
    
    if((targetCount == 0)||(candidateCount == 0)) {
      return;
    }

    int TA;   // target arc index
    int TS;   // target segment index
    int CA;   // candidate arc index

    for(TA=0; TA<targetCount; TA++)
    {
      bool match = false;
      arcEntry &arcTA = m_vecArcs[1][TA];
      //if(arcTA.used > 0) continue;
      
      const std::vector<int> &segmList = *arcTA.lineIdxList;
      const int segmCount = static_cast<int>(segmList.size());
      const int maxGap = par.maxArcGap;
      
      // check only first and last segments
      int segmRange = segmCount/2;
      if(segmRange > 3) segmRange = 3;
      for(TS=0; TS<segmCount; TS++)
      {
        if((TS>segmRange-1)&&(TS<segmCount-segmRange)) continue;
        
        const lineEntry segTS = m_vecLines[1][segmList[TS]];
        for(CA=0; CA<candidateCount; CA++)
        {
          int found = 0;
          arcEntry &arcCA = m_vecArcs[3][CA];
          //if(arcCA.used > 0) continue;

          if((arcCA.start.x <= segTS.end.x + maxGap)    // CA starts in window 
          &&(arcCA.start.x >= segTS.end.x - maxGap)     // CA starts in window 
          &&(arcCA.start.y <= segTS.end.y + maxGap)     // CA starts in window
          &&(arcCA.start.y >= segTS.end.y - maxGap))    // CA starts in window 
          {
            if(arcCA.end.y > arcTA.end.y) {
              found = 1;
            } else {
              found = 2;
            }
            const lineEntry segStart = segTS;
            const lineEntry segEnd = m_vecLines[3][(*arcCA.lineIdxList)[0]];

            // check interior angle
            const double tangent1 = segStart.tangent;
            const double tangent2 = segEnd.tangent;
            double interiorAngle = (tangent2-tangent1)*180/PI;
            if(interiorAngle < 0) interiorAngle = -interiorAngle;
            interiorAngle = 180 - interiorAngle;
            if((interiorAngle <= 180)&&(interiorAngle >= 135)) {

              // estimate circle parameters
              double Xest, Yest, R2est;
              if(estimateCircle(*arcTA.lineIdxList, 1, *arcCA.lineIdxList, 3, Xest, Yest, R2est)) {
                // check tangents
                const double deltaX1 = Xest - segStart.mid.x;
                const double deltaY1 = Yest - segStart.mid.y;
                double TetaEst1 = atan2(deltaX1, deltaY1);
                if((deltaY1 < 0)&&(deltaX1 >= 0)) TetaEst1 -= PI;
                if((deltaY1 < 0)&&(deltaX1 < 0)) TetaEst1 += PI;
    
                const double deltaX2 = Xest - segEnd.mid.x;
                const double deltaY2 = Yest - segEnd.mid.y;
                double TetaEst2 = atan2(deltaX2, deltaY2);
                if((deltaY2 < 0)&&(deltaX2 >= 0)) TetaEst2 -= PI;
                if((deltaY2 < 0)&&(deltaX2 < 0)) TetaEst2 += PI;

                const double tang1err = (TetaEst1-tangent1)*180/PI;
                const double tang2err = (TetaEst2-tangent2)*180/PI;
                
                if(((tang1err > maxLineTangentError)
                  ||(tang1err < -maxLineTangentError))
                ||((tang2err > maxLineTangentError)
                  ||(tang2err < -maxLineTangentError))) {
                  // tangent estimation error exceeded maximum
                } else {

                  // extra checks for better extarc extraction
                  if(par.oldExtArcCheck == true) {

                    double deltaX, deltaY, squDiff;
                    deltaX = arcCA.start.x - Xest;
                    deltaY = arcCA.start.y - Yest;
                    squDiff = R2est - (deltaX*deltaX + deltaY*deltaY);  // Pythagoras
                    if((squDiff > par.oldExtArcThreshold)||(squDiff < -par.oldExtArcThreshold)) {
                      continue;
                    }
  
                    deltaX = arcCA.end.x - Xest;
                    deltaY = arcCA.end.y - Yest;
                    squDiff = R2est - (deltaX*deltaX + deltaY*deltaY);  // Pythagoras
                    if((squDiff > par.oldExtArcThreshold)||(squDiff < -par.oldExtArcThreshold)) {
                      continue;
                    }

                  } else {
                    
                    // check arc/estimation fitting
                    double deltaX, deltaY, d2a, d2b, d2c, d2d, ratio, mismatch;
                    deltaX = arcTA.start.x - Xest;
                    deltaY = arcTA.start.y - Yest;
                    // squared distance (TA.start <-> estimated center)
                    d2a = deltaX*deltaX + deltaY*deltaY;
                    if(d2a > R2est) {
                      ratio = R2est/d2a;
                    } else {
                      ratio = d2a/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }
  
                    deltaX = arcTA.end.x - Xest;
                    deltaY = arcTA.end.y - Yest;
                    // squared distance (TA.end <-> estimated center)
                    d2b = deltaX*deltaX + deltaY*deltaY;
                    if(d2b > R2est) {
                      ratio = R2est/d2b;
                    } else {
                      ratio = d2b/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }
  
                    deltaX = arcCA.start.x - Xest;
                    deltaY = arcCA.start.y - Yest;
                    // squared distance (CA.start <-> estimated center)
                    d2c = deltaX*deltaX + deltaY*deltaY;
                    if(d2c > R2est) {
                      ratio = R2est/d2c;
                    } else {
                      ratio = d2c/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }
  
                    deltaX = arcCA.end.x - Xest;
                    deltaY = arcCA.end.y - Yest;
                    // squared distance (CA.end <-> estimated center)
                    d2d = deltaX*deltaX + deltaY*deltaY;
                    if(d2d > R2est) {
                      ratio = R2est/d2d;
                    } else {
                      ratio = d2d/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }
  
                    // check radial displacement
                    mismatch = sqrt(d2b) - sqrt(d2c);

                    if((mismatch > par.maxExtArcMismatch)
                     ||(mismatch < -par.maxExtArcMismatch)) {
                      continue;
                    }
                  }

                  // create new extarc entry
                  extArcEntry entry;
                  entry.start = arcTA.start;
                  entry.end = arcCA.end;
                  const int n = static_cast<int>(arcCA.lineIdxList->size());
                  entry.firstTang = m_vecLines[1][(*arcTA.lineIdxList)[0]].tangent;
                  entry.firstMid = m_vecLines[1][(*arcTA.lineIdxList)[0]].mid;
                  entry.lastTang = m_vecLines[3][(*arcCA.lineIdxList)[n-1]].tangent;
                  entry.lastMid = m_vecLines[3][(*arcCA.lineIdxList)[n-1]].mid;
                  entry.mid.x = static_cast<int>(floor(Xest + 0.5));
                  entry.mid.y = static_cast<int>(floor(Yest + 0.5));
                  entry.r2    = static_cast<int>(floor(R2est + 0.5));
                  entry.used  = 0;
                  entry.group = 1;
                  //entry.type  = found;
                  entry.arcIdx[0] = TA;
                  entry.arcIdx[1] = CA;
                  m_vecExtArcs[0].push_back(entry);
                  m_vecExtArcs[1].push_back(entry);
                  arcTA.used++;
                  arcCA.used++;
                  match = true;
                  //break;
                  continue;
                }
              }// circle estimation
            }// interior angle
          }// candidate found
        }// CA Loop
        if(match) break;
      }// TS Loop
    }// TA Loop
  }


///////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractExtGroup2Arcs() {
    
    // get parameters
    const parameters& par = getParameters();
    const float maxLineTangentError = par.maxArcTangentError;

    const int targetCount = static_cast<int>(m_vecArcs[3].size());
    const int candidateCount = static_cast<int>(m_vecArcs[2].size());

    // clear old arclist and reserve memory
    clearExtArcList(2);
    m_vecExtArcs[2].reserve(ExtArcPreallocation);
    
    if((targetCount == 0)||(candidateCount == 0)) {
      return;
    }

    int TA;   // target arc index
    int TS;   // target segment index
    int CA;   // candidate arc index

    for(TA=0; TA<targetCount; TA++) 
    {
      bool match = false;
      arcEntry &arcTA = m_vecArcs[3][TA];
      //if(arcTA.used > 0) continue;
      
      const std::vector<int> &segmList = *arcTA.lineIdxList;
      const int segmCount = static_cast<int>(segmList.size());
      const int maxGap = par.maxArcGap;
      
      // check only first and last segments
      int segmRange = segmCount/2;
      if(segmRange > 3) segmRange = 3;
      for(TS=0; TS<segmCount; TS++)
      {
        if((TS>segmRange-1)&&(TS<segmCount-segmRange)) continue;
        
        const lineEntry segTS = m_vecLines[3][segmList[TS]];
        for(CA=0; CA<candidateCount; CA++)
        {
          int found = 0;
          arcEntry &arcCA = m_vecArcs[2][CA];
          //if(arcCA.used > 0) continue;

          if((arcCA.start.x <= segTS.end.x + maxGap)    // CA starts in window 
          &&(arcCA.start.x >= segTS.end.x - maxGap)     // CA starts in window 
          &&(arcCA.start.y <= segTS.end.y + maxGap)     // CA starts in window
          &&(arcCA.start.y >= segTS.end.y - maxGap))    // CA starts in window 
          {
            if(arcCA.end.y > arcTA.end.y) {
              found = 1;
            } else {
              found = 2;
            }
            const lineEntry segStart = segTS;
            const lineEntry segEnd = m_vecLines[2][(*arcCA.lineIdxList)[0]];

            // check interior angle
            const double tangent1 = segStart.tangent;
            const double tangent2 = segEnd.tangent;
            double interiorAngle = (tangent2-tangent1)*180/PI;
            if(interiorAngle < 0) interiorAngle = -interiorAngle;
            //interiorAngle = 180 - interiorAngle;
            if((interiorAngle <= 180)&&(interiorAngle >= 135)) {

              // estimate circle parameters
              double Xest, Yest, R2est;
              if(estimateCircle(*arcTA.lineIdxList, 3, *arcCA.lineIdxList, 2, Xest, Yest, R2est)) {
                // check tangents
                const double deltaX1 = Xest - segStart.mid.x;
                const double deltaY1 = Yest - segStart.mid.y;
                double TetaEst1 = atan2(deltaX1, deltaY1);
                if((deltaY1 < 0)&&(deltaX1 >= 0)) TetaEst1 -= PI;
                if((deltaY1 < 0)&&(deltaX1 < 0)) TetaEst1 += PI;

                const double deltaX2 = Xest - segEnd.mid.x;
                const double deltaY2 = Yest - segEnd.mid.y;
                double TetaEst2 = atan2(deltaX2, deltaY2);
                if((deltaX2 < 0)&&(deltaY2 >= 0)) TetaEst2 += PI;
                if((deltaX2 < 0)&&(deltaY2 < 0)) TetaEst2 += PI;

                const double tang1err = (TetaEst1-tangent1)*180/PI;
                const double tang2err = (TetaEst2-tangent2)*180/PI;
                
                if(((tang1err > maxLineTangentError)
                  ||(tang1err < -maxLineTangentError))
                ||((tang2err > maxLineTangentError)
                  ||(tang2err < -maxLineTangentError))) {
                  // tangent estimation error exceeded maximum
                } else {

                  // extra checks for better extarc extraction
                  if(par.oldExtArcCheck == true) {

                    double deltaX, deltaY, squDiff;
                    deltaX = arcCA.start.x - Xest;
                    deltaY = arcCA.start.y - Yest;
                    squDiff = R2est - (deltaX*deltaX + deltaY*deltaY);  // Pythagoras
                    if((squDiff > par.oldExtArcThreshold)||(squDiff < -par.oldExtArcThreshold)) {
                      continue;
                    }
  
                    deltaX = arcCA.end.x - Xest;
                    deltaY = arcCA.end.y - Yest;
                    squDiff = R2est - (deltaX*deltaX + deltaY*deltaY);  // Pythagoras
                    if((squDiff > par.oldExtArcThreshold)||(squDiff < -par.oldExtArcThreshold)) {
                      continue;
                    }

                  } else {

                    // check arc/estimation fitting
                    double deltaX, deltaY, d2a, d2b, d2c, d2d, ratio, mismatch;
                    deltaX = arcTA.start.x - Xest;
                    deltaY = arcTA.start.y - Yest;
                    // squared distance (TA.start <-> estimated center)
                    d2a = deltaX*deltaX + deltaY*deltaY;
                    if(d2a > R2est) {
                      ratio = R2est/d2a;
                    } else {
                      ratio = d2a/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }
  
                    deltaX = arcTA.end.x - Xest;
                    deltaY = arcTA.end.y - Yest;
                    // squared distance (TA.end <-> estimated center)
                    d2b = deltaX*deltaX + deltaY*deltaY;
                    if(d2b > R2est) {
                      ratio = R2est/d2b;
                    } else {
                      ratio = d2b/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }

                    deltaX = arcCA.start.x - Xest;
                    deltaY = arcCA.start.y - Yest;
                    // squared distance (CA.start <-> estimated center)
                    d2c = deltaX*deltaX + deltaY*deltaY;
                    if(d2c > R2est) {
                      ratio = R2est/d2c;
                    } else {
                      ratio = d2c/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }
  
                    deltaX = arcCA.end.x - Xest;
                    deltaY = arcCA.end.y - Yest;
                    // squared distance (CA.end <-> estimated center)
                    d2d = deltaX*deltaX + deltaY*deltaY;
                    if(d2d > R2est) {
                      ratio = R2est/d2d;
                    } else {
                      ratio = d2d/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }

                    // check radial displacement
                    mismatch = sqrt(d2b) - sqrt(d2c);

                    if((mismatch > par.maxExtArcMismatch)
                     ||(mismatch < -par.maxExtArcMismatch)) {
                      continue;
                    }
                  }

                  // create new extarc entry
                  extArcEntry entry;
                  entry.start = arcTA.start;
                  entry.end = arcCA.end;
                  const int n = static_cast<int>(arcCA.lineIdxList->size());
                  entry.firstTang = m_vecLines[3][(*arcTA.lineIdxList)[0]].tangent;
                  entry.firstMid = m_vecLines[3][(*arcTA.lineIdxList)[0]].mid;
                  entry.lastTang = m_vecLines[2][(*arcCA.lineIdxList)[n-1]].tangent;
                  entry.lastMid = m_vecLines[2][(*arcCA.lineIdxList)[n-1]].mid;
                  entry.mid.x = static_cast<int>(floor(Xest + 0.5));
                  entry.mid.y = static_cast<int>(floor(Yest + 0.5));
                  entry.r2    = static_cast<int>(floor(R2est + 0.5));
                  entry.used  = 0;
                  entry.group = 2;
                  //entry.type  = found;
                  entry.arcIdx[0] = TA;
                  entry.arcIdx[1] = CA;
                  m_vecExtArcs[0].push_back(entry);
                  m_vecExtArcs[2].push_back(entry);
                  arcTA.used++;
                  arcCA.used++;
                  match = true;
                  //break;
                  continue;
                }
              }
            }
          }
        }
        if(match) break;
      }
    }
  }


///////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractExtGroup3Arcs() {
    
    // get parameters
    const parameters& par = getParameters();
    const float maxLineTangentError = par.maxArcTangentError;

    const int targetCount = static_cast<int>(m_vecArcs[2].size());
    const int candidateCount = static_cast<int>(m_vecArcs[4].size());

    // clear old arclist and reserve memory
    clearExtArcList(3);
    m_vecExtArcs[3].reserve(ExtArcPreallocation);

    if((targetCount == 0)||(candidateCount == 0)) {
      return;
    }

    int TA;   // target arc index
    int TS;   // target segment index
    int CA;   // candidate arc index

    for(TA=0; TA<targetCount; TA++) 
    {
      bool match = false;
      arcEntry &arcTA = m_vecArcs[2][TA];
      //if(arcTA.used > 0) continue;
      
      const std::vector<int> &segmList = *arcTA.lineIdxList;
      const int segmCount = static_cast<int>(segmList.size());
      const int maxGap = par.maxArcGap;
      
      // check only first and last segments
      int segmRange = segmCount/2;
      if(segmRange > 3) segmRange = 3;
      for(TS=0; TS<segmCount; TS++)
      {
        if((TS>segmRange-1)&&(TS<segmCount-segmRange)) continue;
        
        const lineEntry segTS = m_vecLines[2][segmList[TS]];
        for(CA=0; CA<candidateCount; CA++)
        {
          int found = 0;
          arcEntry &arcCA = m_vecArcs[4][CA];
          //if(arcCA.used > 0) continue;

          if((arcCA.start.x <= segTS.end.x + maxGap)    // CA starts in window 
          &&(arcCA.start.x >= segTS.end.x - maxGap)     // CA starts in window 
          &&(arcCA.start.y <= segTS.end.y + maxGap)     // CA starts in window
          &&(arcCA.start.y >= segTS.end.y - maxGap))    // CA starts in window 
          {
            if(arcCA.start.y > arcTA.start.y) {
              found = 1;
            } else {
              found = 2;
            }
            const lineEntry segStart = segTS;
            const lineEntry segEnd = m_vecLines[4][(*arcCA.lineIdxList)[0]];

            // check interior angle
            const double tangent1 = segStart.tangent;
            const double tangent2 = segEnd.tangent;
            double interiorAngle = (tangent2-tangent1)*180/PI;
            if(interiorAngle < 0) interiorAngle = -interiorAngle;
            interiorAngle = 180 - interiorAngle;
            if((interiorAngle <= 180)&&(interiorAngle >= 135)) {

              // estimate circle parameters
              double Xest, Yest, R2est;
              if(estimateCircle(*arcTA.lineIdxList, 2, *arcCA.lineIdxList, 4, Xest, Yest, R2est)) {
                // check tangents
                const double deltaX1 = Xest - segStart.mid.x;
                const double deltaY1 = Yest - segStart.mid.y;
                double TetaEst1 = atan2(deltaX1, deltaY1);
                if((deltaY1 < 0)&&(deltaX1 >= 0)) TetaEst1 -= PI;
                if((deltaY1 < 0)&&(deltaX1 < 0)) TetaEst1 += PI;
    
                const double deltaX2 = Xest - segEnd.mid.x;
                const double deltaY2 = Yest - segEnd.mid.y;
                double TetaEst2 = atan2(deltaX2, deltaY2);
                if((deltaX2 < 0)&&(deltaY2 >= 0)) TetaEst2 += PI;
                if((deltaX2 < 0)&&(deltaY2 < 0)) TetaEst2 += PI;

                const double tang1err = (TetaEst1-tangent1)*180/PI;
                const double tang2err = (TetaEst2-tangent2)*180/PI;
                
                if(((tang1err > maxLineTangentError)
                  ||(tang1err < -maxLineTangentError))
                ||((tang2err > maxLineTangentError)
                  ||(tang2err < -maxLineTangentError))) {
                  // tangent estimation error exceeded maximum
                } else {

                if(par.oldExtArcCheck == true) {

                  double deltaX, deltaY, squDiff;
                  deltaX = arcCA.end.x - Xest;
                  deltaY = arcCA.end.y - Yest;
                  squDiff = R2est - (deltaX*deltaX + deltaY*deltaY);  // Pythagoras
                  if((squDiff > par.oldExtArcThreshold)||(squDiff < -par.oldExtArcThreshold)) {
                    continue;
                  }

                  deltaX = arcCA.start.x - Xest;
                  deltaY = arcCA.start.y - Yest;
                  squDiff = R2est - (deltaX*deltaX + deltaY*deltaY);  // Pythagoras
                  if((squDiff > par.oldExtArcThreshold)||(squDiff < -par.oldExtArcThreshold)) {
                    continue;
                  }

                } else {

                  // check arc/estimation fitting
                  double deltaX, deltaY, d2a, d2b, d2c, d2d, ratio, mismatch;
                  deltaX = arcTA.start.x - Xest;
                  deltaY = arcTA.start.y - Yest;
                  // squared distance (TA.start <-> estimated center)
                  d2a = deltaX*deltaX + deltaY*deltaY;
                  if(d2a > R2est) {
                    ratio = R2est/d2a;
                  } else {
                    ratio = d2a/R2est;
                  }
                  if(ratio < par.minExtArcMatchRatio) {
                    continue;
                  }

                  deltaX = arcTA.end.x - Xest;
                  deltaY = arcTA.end.y - Yest;
                  // squared distance (TA.end <-> estimated center)
                  d2b = deltaX*deltaX + deltaY*deltaY;
                  if(d2b > R2est) {
                    ratio = R2est/d2b;
                  } else {
                    ratio = d2b/R2est;
                  }
                  if(ratio < par.minExtArcMatchRatio) {
                    continue;
                  }

                  deltaX = arcCA.end.x - Xest;
                  deltaY = arcCA.end.y - Yest;
                  // squared distance (CA.start <-> estimated center)
                  d2c = deltaX*deltaX + deltaY*deltaY;
                  if(d2c > R2est) {
                    ratio = R2est/d2c;
                  } else {
                    ratio = d2c/R2est;
                  }
                  if(ratio < par.minExtArcMatchRatio) {
                    continue;
                  }

                  deltaX = arcCA.start.x - Xest;
                  deltaY = arcCA.start.y - Yest;
                  // squared distance (CA.end <-> estimated center)
                  d2d = deltaX*deltaX + deltaY*deltaY;
                  if(d2d > R2est) {
                    ratio = R2est/d2d;
                  } else {
                    ratio = d2d/R2est;
                  }
                  if(ratio < par.minExtArcMatchRatio) {
                    continue;
                  }

                  // check radial displacement
                  mismatch = sqrt(d2b) - sqrt(d2c);

                  if((mismatch > par.maxExtArcMismatch)
                   ||(mismatch < -par.maxExtArcMismatch)) {
                    continue;
                  }
                }

                  // create new extarc entry
                  extArcEntry entry;
                  entry.start = arcTA.start;
                  entry.end = arcCA.end;
                  const int n = static_cast<int>(arcCA.lineIdxList->size());
                  entry.firstTang = m_vecLines[2][(*arcTA.lineIdxList)[0]].tangent;
                  entry.firstMid = m_vecLines[2][(*arcTA.lineIdxList)[0]].mid;
                  entry.lastTang = m_vecLines[4][(*arcCA.lineIdxList)[n-1]].tangent;
                  entry.lastMid = m_vecLines[4][(*arcCA.lineIdxList)[n-1]].mid;
                  entry.mid.x = static_cast<int>(floor(Xest + 0.5));
                  entry.mid.y = static_cast<int>(floor(Yest + 0.5));
                  entry.r2    = static_cast<int>(floor(R2est + 0.5));
                  entry.used  = 0;
                  entry.group = 3;
                  //entry.type  = found;
                  entry.arcIdx[0] = TA;
                  entry.arcIdx[1] = CA;
                  m_vecExtArcs[0].push_back(entry);
                  m_vecExtArcs[3].push_back(entry);
                  arcTA.used++;
                  arcCA.used++;
                  match = true;
                  //break;
                  continue;
                }
              }
            }
          }
        }
        if(match) break;
      }
    }
  }


///////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractExtGroup4Arcs() {
    
    // get parameters
    const parameters& par = getParameters();
    const float maxLineTangentError = par.maxArcTangentError;

    const int targetCount = static_cast<int>(m_vecArcs[4].size());
    const int candidateCount = static_cast<int>(m_vecArcs[1].size());
    
    // clear old arclist and reserve memory
    clearExtArcList(4);
    m_vecExtArcs[4].reserve(ExtArcPreallocation);

    if((targetCount == 0)||(candidateCount == 0)) {
      return;
    }

    int TA;   // target arc index
    int TS;   // target segment index
    int CA;   // candidate arc index

    for(TA=0; TA<targetCount; TA++) 
    {
      bool match = false;
      arcEntry &arcTA = m_vecArcs[4][TA];
      //if(arcTA.used > 0) continue;
      
      const std::vector<int> &segmList = *arcTA.lineIdxList;
      const int segmCount = static_cast<int>(segmList.size());
      const int maxGap = par.maxArcGap;
      
      // check only first and last segments
      int segmRange = segmCount/2;
      if(segmRange > 3) segmRange = 3;
      for(TS=0; TS<segmCount; TS++)
      {
        if((TS>segmRange-1)&&(TS<segmCount-segmRange)) continue;
        
        const lineEntry segTS = m_vecLines[4][segmList[TS]];
        for(CA=0; CA<candidateCount; CA++)
        {
          int found = 0;
          arcEntry &arcCA = m_vecArcs[1][CA];
          //if(arcCA.used > 0) continue;

          if((arcCA.start.x <= segTS.end.x + maxGap)    // CA starts in window 
          &&(arcCA.start.x >= segTS.end.x - maxGap)     // CA starts in window 
          &&(arcCA.start.y <= segTS.end.y + maxGap)     // CA starts in window
          &&(arcCA.start.y >= segTS.end.y - maxGap))    // CA starts in window 
          {
            if(arcCA.end.x < arcTA.end.x) {
              found = 1;
            } else {
              found = 2;
            }
            const lineEntry segStart = segTS;
            const lineEntry segEnd = m_vecLines[1][(*arcCA.lineIdxList)[0]];

            // check interior angle
            const double tangent1 = segStart.tangent;
            const double tangent2 = segEnd.tangent;
            double interiorAngle = (tangent2-tangent1)*180/PI;
            if(interiorAngle < 0) interiorAngle = -interiorAngle;
            interiorAngle = 180 - interiorAngle;
            if((interiorAngle <= 180)&&(interiorAngle >= 135)) {

              // estimate circle parameters
              double Xest, Yest, R2est;
              if(estimateCircle(*arcTA.lineIdxList, 4, *arcCA.lineIdxList, 1, Xest, Yest, R2est)) {
                // check tangents
                const double deltaX1 = Xest - segStart.mid.x;
                const double deltaY1 = Yest - segStart.mid.y;
                double TetaEst1 = atan2(deltaX1, deltaY1);
                if((deltaY1 < 0)&&(deltaX1 >= 0)) TetaEst1 -= PI;
                if((deltaY1 < 0)&&(deltaX1 < 0)) TetaEst1 += PI;
    
                const double deltaX2 = Xest - segEnd.mid.x;
                const double deltaY2 = Yest - segEnd.mid.y;
                double TetaEst2 = atan2(deltaX2, deltaY2);
                if((deltaY2 < 0)&&(deltaX2 >= 0)) TetaEst2 -= PI;
                if((deltaY2 < 0)&&(deltaX2 < 0)) TetaEst2 += PI;

                const double tang1err = (TetaEst1-tangent1)*180/PI;
                const double tang2err = (TetaEst2-tangent2)*180/PI;
                
                if(((tang1err > maxLineTangentError)
                  ||(tang1err < -maxLineTangentError))
                ||((tang2err > maxLineTangentError)
                  ||(tang2err < -maxLineTangentError))) {
                  // tangent estimation error exceeded maximum
                } else {

                  if(par.oldExtArcCheck == true) {

                    double deltaX, deltaY, squDiff;
                    deltaX = arcCA.start.x - Xest;
                    deltaY = arcCA.start.y - Yest;
                    squDiff = R2est - (deltaX*deltaX + deltaY*deltaY);  // Pythagoras
                    if((squDiff > par.oldExtArcThreshold)||(squDiff < -par.oldExtArcThreshold)) {
                      continue;
                    }
  
                    deltaX = arcCA.end.x - Xest;
                    deltaY = arcCA.end.y - Yest;
                    squDiff = R2est - (deltaX*deltaX + deltaY*deltaY);  // Pythagoras
                    if((squDiff > par.oldExtArcThreshold)||(squDiff < -par.oldExtArcThreshold)) {
                      continue;
                    }

                  } else {

                    // check arc/estimation fitting
                    double deltaX, deltaY, d2a, d2b, d2c, d2d, ratio, mismatch;
                    deltaX = arcTA.end.x - Xest;
                    deltaY = arcTA.end.y - Yest;
                    // squared distance (TA.start <-> estimated center)
                    d2a = deltaX*deltaX + deltaY*deltaY;
                    if(d2a > R2est) {
                      ratio = R2est/d2a;
                    } else {
                      ratio = d2a/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }
  
                    deltaX = arcTA.start.x - Xest;
                    deltaY = arcTA.start.y - Yest;
                    // squared distance (TA.end <-> estimated center)
                    d2b = deltaX*deltaX + deltaY*deltaY;
                    if(d2b > R2est) {
                      ratio = R2est/d2b;
                    } else {
                      ratio = d2b/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }

                    deltaX = arcCA.start.x - Xest;
                    deltaY = arcCA.start.y - Yest;
                    // squared distance (CA.start <-> estimated center)
                    d2c = deltaX*deltaX + deltaY*deltaY;
                    if(d2c > R2est) {
                      ratio = R2est/d2c;
                    } else {
                      ratio = d2c/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }
  
                    deltaX = arcCA.end.x - Xest;
                    deltaY = arcCA.end.y - Yest;
                    // squared distance (CA.end <-> estimated center)
                    d2d = deltaX*deltaX + deltaY*deltaY;
                    if(d2d > R2est) {
                      ratio = R2est/d2d;
                    } else {
                      ratio = d2d/R2est;
                    }
                    if(ratio < par.minExtArcMatchRatio) {
                      continue;
                    }

                    // check radial displacement
                    mismatch = sqrt(d2b) - sqrt(d2c);

                    if((mismatch > par.maxExtArcMismatch)
                     ||(mismatch < -par.maxExtArcMismatch)) {
                      continue;
                    }
                  }

                  // create new extarc entry
                  extArcEntry entry;
                  entry.start = arcTA.start;
                  entry.end = arcCA.end;
                  const int n = static_cast<int>(arcCA.lineIdxList->size());
                  entry.firstTang = m_vecLines[4][(*arcTA.lineIdxList)[0]].tangent;
                  entry.firstMid = m_vecLines[4][(*arcTA.lineIdxList)[0]].mid;
                  entry.lastTang = m_vecLines[1][(*arcCA.lineIdxList)[n-1]].tangent;
                  entry.lastMid = m_vecLines[1][(*arcCA.lineIdxList)[n-1]].mid;
                  entry.mid.x = static_cast<int>(floor(Xest + 0.5));
                  entry.mid.y = static_cast<int>(floor(Yest + 0.5));
                  entry.r2    = static_cast<int>(floor(R2est + 0.5));
                  entry.used  = 0;
                  entry.group = 4;
                  //entry.type  = found;
                  entry.arcIdx[0] = TA;
                  entry.arcIdx[1] = CA;
                  m_vecExtArcs[0].push_back(entry);
                  m_vecExtArcs[4].push_back(entry);
                  arcTA.used++;
                  arcCA.used++;
                  match = true;
                  //break;
                  continue;
                }
              }
            }
          }
        }
        if(match) break;
      }
    }
  }

/////////////////////////////////////////////////////////////////////

  int fastCircleExtraction::checkExtArcTangents(const extArcEntry &extarc1,
                                                const extArcEntry &extarc2,
                                                const double Xest,
                                                const double Yest) {
    int tangErrors = 0;
    const parameters& par = getParameters();

    const double deltaX1 = Xest - extarc1.firstMid.x;
    const double deltaY1 = Yest - extarc1.firstMid.y;
    double TetaEst1 = atan2(deltaX1, deltaY1);
    if(extarc1.group == 1) {
      if((deltaY1 < 0)&&(deltaX1 >= 0)) TetaEst1 -= PI;
      if((deltaY1 < 0)&&(deltaX1 < 0)) TetaEst1 += PI;
    }
    else if(extarc1.group == 2) {
      if((deltaY1 < 0)&&(deltaX1 >= 0)) TetaEst1 -= PI;
      if((deltaY1 < 0)&&(deltaX1 < 0)) TetaEst1 += PI;      // not verified
    }
    else if(extarc1.group == 3) {
      if((deltaX1 < 0)&&(deltaY1 >= 0)) TetaEst1 += PI;
      if((deltaX1 < 0)&&(deltaY1 < 0)) TetaEst1 += PI;
    }
    else if(extarc1.group == 4) {
      if((deltaX1 < 0)&&(deltaY1 >= 0)) TetaEst1 += PI;     // not verified
      if((deltaX1 < 0)&&(deltaY1 < 0)) TetaEst1 += PI;
    }       

    const double deltaX2 = Xest - extarc1.lastMid.x;
    const double deltaY2 = Yest - extarc1.lastMid.y;
    double TetaEst2 = atan2(deltaX2, deltaY2);
    if(extarc1.group == 1) {
      if((deltaY2 < 0)&&(deltaX2 >= 0)) TetaEst2 -= PI;
      if((deltaY2 < 0)&&(deltaX2 < 0)) TetaEst2 += PI;      // not verified
    }
    else if(extarc1.group == 2) {
      if((deltaX2 < 0)&&(deltaY2 >= 0)) TetaEst2 += PI;
      if((deltaX2 < 0)&&(deltaY2 < 0)) TetaEst2 += PI;
    }
    else if(extarc1.group == 3) {
      if((deltaX2 < 0)&&(deltaY2 >= 0)) TetaEst2 += PI;     // not verified
      if((deltaX2 < 0)&&(deltaY2 < 0)) TetaEst2 += PI;
    }
    else if(extarc1.group == 4) {
      if((deltaY2 < 0)&&(deltaX2 >= 0)) TetaEst2 -= PI;
      if((deltaY2 < 0)&&(deltaX2 < 0)) TetaEst2 += PI;      // not verified
    }
    
    const double deltaX3 = Xest - extarc2.firstMid.x;
    const double deltaY3 = Yest - extarc2.firstMid.y;
    double TetaEst3 = atan2(deltaX3, deltaY3);
    if(extarc2.group == 1) {
      if((deltaY3 < 0)&&(deltaX3 >= 0)) TetaEst3 -= PI;
      if((deltaY3 < 0)&&(deltaX3 < 0)) TetaEst3 += PI;
    }
    else if(extarc2.group == 2) {
      if((deltaY3 < 0)&&(deltaX3 >= 0)) TetaEst3 -= PI;
      if((deltaY3 < 0)&&(deltaX3 < 0)) TetaEst3 += PI;      // not verified
    }
    else if(extarc2.group == 3) {
      if((deltaX3 < 0)&&(deltaY3 >= 0)) TetaEst3 += PI;
      if((deltaX3 < 0)&&(deltaY3 < 0)) TetaEst3 += PI;
    }
    else if(extarc2.group == 4) {
      if((deltaX3 < 0)&&(deltaY3 >= 0)) TetaEst3 += PI;     // not verified
      if((deltaX3 < 0)&&(deltaY3 < 0)) TetaEst3 += PI;
    }

    const double deltaX4 = Xest - extarc2.lastMid.x;
    const double deltaY4 = Yest - extarc2.lastMid.y;
    double TetaEst4 = atan2(deltaX4, deltaY4);
    if(extarc2.group == 1) {
      if((deltaY4 < 0)&&(deltaX4 >= 0)) TetaEst4 -= PI;
      if((deltaY4 < 0)&&(deltaX4 < 0)) TetaEst4 += PI;      // not verified
    }
    else if(extarc2.group == 2) {
      if((deltaX4 < 0)&&(deltaY4 >= 0)) TetaEst4 += PI;
      if((deltaX4 < 0)&&(deltaY4 < 0)) TetaEst4 += PI;
    }
    else if(extarc2.group == 3) {
      if((deltaX4 < 0)&&(deltaY4 >= 0)) TetaEst4 += PI;
      if((deltaX4 < 0)&&(deltaY4 < 0)) TetaEst4 += PI;
    }
    else if(extarc2.group == 4) {
      if((deltaY4 < 0)&&(deltaX4 >= 0)) TetaEst4 -= PI;
      if((deltaY4 < 0)&&(deltaX4 < 0)) TetaEst4 += PI;
    }

    if((((TetaEst1-extarc1.firstTang)*180/PI > par.maxArcTangentError)
      ||((TetaEst1-extarc1.firstTang)*180/PI < -par.maxArcTangentError))) {
        tangErrors++;
    }
    if((((TetaEst2-extarc1.lastTang)*180/PI > par.maxArcTangentError)
      ||((TetaEst2-extarc1.lastTang)*180/PI < -par.maxArcTangentError))) {
        tangErrors++;
    }
    if((((TetaEst3-extarc2.firstTang)*180/PI > par.maxArcTangentError)
      ||((TetaEst3-extarc2.firstTang)*180/PI < -par.maxArcTangentError))) {
        tangErrors++;
    }
    if((((TetaEst4-extarc2.lastTang)*180/PI > par.maxArcTangentError)
      ||((TetaEst4-extarc2.lastTang)*180/PI < -par.maxArcTangentError))) {
        tangErrors++;
    }
    return tangErrors;
  }

/////////////////////////////////////////////////////////////////////

  void fastCircleExtraction::extractCircles() {

    // get parameters
    const parameters& par = getParameters();

    // clear old arclist and reserve memory
    m_vecCircles.clear();
    m_vecCircles.reserve(CirclePreallocation);

    std::vector<int> idxlist;

    // compare extarcs
    int nExtArcs = static_cast<int>(m_vecExtArcs[0].size());
    for(int i=0; i<nExtArcs; i++)
    {
      const extArcEntry &arc1 = m_vecExtArcs[0][i];
      if(arc1.used > 0) continue;
      idxlist.clear();
      idxlist.push_back(i);
  
      for(int j=0; j<nExtArcs; j++)
      {
        if(j==i) continue;
        const extArcEntry &arc2 = m_vecExtArcs[0][j];
        if(arc2.used > 0) continue;

        double Xest, Yest, R2est;
        //if(!estimateCircle(m_vecExtArcs[0][i].start, m_vecExtArcs[0][i].end, m_vecExtArcs[0][j].start, m_vecExtArcs[0][j].end, Xest, Yest, R2est)) {
        if(!estimateCircle(m_vecExtArcs[0][i], m_vecExtArcs[0][j], Xest, Yest, R2est)) {
          continue;   // estimation failed
        } 

        if(R2est >= m_vecExtArcs[0][i].r2) {
          if((double)m_vecExtArcs[0][i].r2/R2est < par.minRadiusMatchRatio) continue;
        } else {
          if((double)R2est/m_vecExtArcs[0][i].r2 < par.minRadiusMatchRatio) continue;
        }

        if(R2est >= m_vecExtArcs[0][j].r2) {
          if((double)m_vecExtArcs[0][j].r2/R2est < par.minRadiusMatchRatio) continue;
        } else {
          if((double)R2est/m_vecExtArcs[0][j].r2 < par.minRadiusMatchRatio) continue;
        }

        int mismatch = 0;
        const double Xdiff1 = Xest - m_vecExtArcs[0][i].mid.x;
        const double Ydiff1 = Yest - m_vecExtArcs[0][i].mid.y;
        if((Xdiff1 > par.maxCenterMismatch)||(Xdiff1 < -par.maxCenterMismatch)) {
          mismatch++;
          //continue;
        }
        if((Ydiff1 > par.maxCenterMismatch)||(Ydiff1 < -par.maxCenterMismatch)) {
          mismatch++;
          //continue;
        }

        const double Xdiff2 = Xest - m_vecExtArcs[0][j].mid.x;
        const double Ydiff2 = Yest - m_vecExtArcs[0][j].mid.y;
        if((Xdiff2 > par.maxCenterMismatch)||(Xdiff2 < -par.maxCenterMismatch)) {
          mismatch++;
          //continue;
        }
        if((Ydiff2 > par.maxCenterMismatch)||(Ydiff2 < -par.maxCenterMismatch)) {
          mismatch++;
          //continue;
        }

        if(mismatch > 1) {
          continue;
        }

        // check tangents
        if(checkExtArcTangents(arc1, arc2, Xest, Yest) > 1) {
          continue;
        }
        idxlist.push_back(j);     
      }
      
      // extract circle
      int n = static_cast<int>(idxlist.size());
      if(n > 1) {
        // collect unique circle lines
        std::vector<lineEntry> circlelines;

        double length = 0;
        for(int k=0; k<n; k++) {

          for(int a=0; a<2; a++)
          {
	          const int XG[5][2] = {{0,0},{1,3},{3,2},{2,4},{4,1}};
            const int group = XG[m_vecExtArcs[0][idxlist[k]].group][a];
            const int index = m_vecExtArcs[0][idxlist[k]].arcIdx[a];
          
            // Referenz auf Arcliste holen
            std::vector<arcEntry> &arcs = m_vecArcs[group];
    
            // Referenz auf Linienliste holen
            std::vector<lineEntry> &lines = m_vecLines[group];
            
            // Referenz auf LineIndexliste holen
            std::vector<int> &lineidx = *arcs[index].lineIdxList;
          
            int n = static_cast<int>(lineidx.size());
            for(int b=0; b<n; b++) 
            {
              if(lines[lineidx[b]].used >= 0) {
                circlelines.push_back(lines[lineidx[b]]);
                lines[lineidx[b]].used = -1;
  
                // Gesamtlänge berechnen
                length += lines[lineidx[b]].len;
              }
            }
          }
        }
        
        double Xest, Yest, R2est;
        if(estimateCircle(circlelines, Xest, Yest, R2est)) {
  
          const int x = static_cast<int>(floor(Xest+0.5));
          const int y = static_cast<int>(floor(Yest+0.5));
          const int r = static_cast<int>(floor(sqrt(R2est)+0.5));
          double coverage = length / (2*PI*r);
    
          if(coverage >= par.minCoverage) {
            // extract circle
            circleEntry entry;
            entry.center.x = x;
            entry.center.y = y;
            entry.radius = r;
            entry.coverage = coverage;
            entry.mergedArcs = new std::vector<int>(idxlist);
            m_vecCircles.push_back(entry);
            
            // increase extarc used-counters
            for(int q=0; q<n; q++) {
              m_vecExtArcs[0][idxlist[q]].used++;
            }
          }
        }
      }//gefundenen Kreis berechnen
    }//Kreise suchen
}


/////////////////////////////////////////////////////////////////////
  
  bool fastCircleExtraction::estimateCircle(std::vector<int> &idxlist, const int group,
                                            double &Xest, double &Yest, double &R2est) {
    lti::int64 N = 0;
    lti::int64 x_sum = 0;
    lti::int64 y_sum = 0; 
    lti::int64 x2_sum = 0;
    lti::int64 y2_sum = 0; 
    lti::int64 xy_sum = 0;
    lti::int64 x3_sum = 0;
    lti::int64 y3_sum = 0;
    lti::int64 xy2_sum = 0;
    lti::int64 x2y_sum = 0;
  
    const int nLines = static_cast<int>(idxlist.size());
  
    // accumulate all lines
    for(int l=0; l<nLines; l++) {
      // get number of line segments and segment group
      const int nSegments = static_cast<int>((*m_vecLines[group][idxlist[l]].segmIdxList).size());

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*m_vecLines[group][idxlist[l]].segmIdxList)[i];
        
        // starting point of segment[i]
        int x = m_vecSegments[group][idx].start.x;
        int y = m_vecSegments[group][idx].start.y;
      
        // segments length
        const int len = m_vecSegments[group][idx].len;
        
        // accumulate all segment pixels
        for(int j=0; j<len; j++) {
          // calculate sums
          x_sum += x;
          y_sum += y;
          x2_sum += x*x;
          y2_sum += y*y;
          xy_sum += x*y;
          x3_sum += x*x*x;
          y3_sum += y*y*y;
          xy2_sum += x*y*y;
          x2y_sum += x*x*y;
          
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
          N++;  // increase number of all pixels
        
        }// next pixel
      }// next segment
    }// next line
  
    const double  a1 = static_cast<double>(2*(x_sum * x_sum - N * x2_sum));
    const double  b1 = static_cast<double>(2*(x_sum * y_sum - N * xy_sum));
    const double  a2 = b1;
    const double  b2 = static_cast<double>(2*(y_sum * y_sum - N * y2_sum));
    const double  c1 = static_cast<double>(x2_sum * x_sum - N * x3_sum + x_sum * y2_sum - N * xy2_sum);
    const double  c2 = static_cast<double>(x2_sum * y_sum - N * y3_sum + y_sum * y2_sum - N * x2y_sum);
      
    // calculate circle parameters
    if((a1*b2 - a2*b1) != 0) {      // avoid division by zero
      Xest = (c1*b2 - c2*b1)/(a1*b2 - a2*b1);
      Yest = (a1*c2 - a2*c1)/(a1*b2 - a2*b1);
      R2est = (double)(x2_sum - 2*Xest*x_sum + N*Xest*Xest + y2_sum - 2*Yest*y_sum + N*Yest*Yest)/N;
      return true;
    }
    
    return false;   // division by zero
  }




  bool fastCircleExtraction::estimateCircle(std::vector<int> &idxlist1, const int group1,
                                            std::vector<int> &idxlist2, const int group2,
                                            double &Xest, double &Yest, double &R2est) {
    lti::int64 N = 0;
    lti::int64 x_sum = 0;
    lti::int64 y_sum = 0; 
    lti::int64 x2_sum = 0;
    lti::int64 y2_sum = 0; 
    lti::int64 xy_sum = 0;
    lti::int64 x3_sum = 0;
    lti::int64 y3_sum = 0;
    lti::int64 xy2_sum = 0;
    lti::int64 x2y_sum = 0;
  
    // accumulate all lines of idxlist1
    const int nLines1 = static_cast<int>(idxlist1.size());
    for(int l=0; l<nLines1; l++) {
      // get number of line segments and segment group
      const int nSegments = static_cast<int>((*m_vecLines[group1][idxlist1[l]].segmIdxList).size());

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*m_vecLines[group1][idxlist1[l]].segmIdxList)[i];
        
        // starting point of segment[i]
        int x = m_vecSegments[group1][idx].start.x;
        int y = m_vecSegments[group1][idx].start.y;
      
        // segments length
        const int len = m_vecSegments[group1][idx].len;
        
        // accumulate all segment pixels
        for(int j=0; j<len; j++) {
          // calculate sums
          x_sum += x;
          y_sum += y;
          x2_sum += x*x;
          y2_sum += y*y;
          xy_sum += x*y;
          x3_sum += x*x*x;
          y3_sum += y*y*y;
          xy2_sum += x*y*y;
          x2y_sum += x*x*y;
          
          // set next segment pixel coordinates
          // orientation:  ---x
          //               |
          //               y
          switch(group1) {
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
          N++;  // increase number of all pixels
        
        }// next pixel
      }// next segment
    }// next line


    // accumulate all lines of idxlist2
    const int nLines2 = static_cast<int>(idxlist2.size());
    for(int q=0; q<nLines2; q++) {
      // get number of line segments and segment group
      const int nSegments = static_cast<int>((*m_vecLines[group2][idxlist2[q]].segmIdxList).size());

      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*m_vecLines[group2][idxlist2[q]].segmIdxList)[i];
        
        // starting point of segment[i]
        int x = m_vecSegments[group2][idx].start.x;
        int y = m_vecSegments[group2][idx].start.y;
      
        // segments length
        const int len = m_vecSegments[group2][idx].len;
        
        // accumulate all segment pixels
        for(int j=0; j<len; j++) {
          // calculate sums
          x_sum += x;
          y_sum += y;
          x2_sum += x*x;
          y2_sum += y*y;
          xy_sum += x*y;
          x3_sum += x*x*x;
          y3_sum += y*y*y;
          xy2_sum += x*y*y;
          x2y_sum += x*x*y;
          
          // set next segment pixel coordinates
          // orientation:  ---x
          //               |
          //               y
          switch(group2) {
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
          N++;  // increase number of all pixels
        
        }// next pixel
      }// next segment
    }// next line

  
    const double  a1 = static_cast<double>(2*(x_sum * x_sum - N * x2_sum));
    const double  b1 = static_cast<double>(2*(x_sum * y_sum - N * xy_sum));
    const double  a2 = b1;
    const double  b2 = static_cast<double>(2*(y_sum * y_sum - N * y2_sum));
    const double  c1 = static_cast<double>(x2_sum * x_sum - N * x3_sum + x_sum * y2_sum - N * xy2_sum);
    const double  c2 = static_cast<double>(x2_sum * y_sum - N * y3_sum + y_sum * y2_sum - N * x2y_sum);
      
    // calculate circle parameters
    if((a1*b2 - a2*b1) != 0) {      // avoid division by zero
      Xest = (c1*b2 - c2*b1)/(a1*b2 - a2*b1);
      Yest = (a1*c2 - a2*c1)/(a1*b2 - a2*b1);
      R2est = (double)(x2_sum - 2*Xest*x_sum + N*Xest*Xest + y2_sum - 2*Yest*y_sum + N*Yest*Yest)/N;
      return true;
    }
    
    return false;   // division by zero
  }



 bool fastCircleExtraction::estimateCircle(const extArcEntry &arc1, const extArcEntry &arc2, 
                                            double &Xest, double &Yest, double &R2est) {
    lti::int64 N = 0;
    lti::int64 x_sum = 0;
    lti::int64 y_sum = 0; 
    lti::int64 x2_sum = 0;
    lti::int64 y2_sum = 0; 
    lti::int64 xy_sum = 0;
    lti::int64 x3_sum = 0;
    lti::int64 y3_sum = 0;
    lti::int64 xy2_sum = 0;
    lti::int64 x2y_sum = 0;
  
  	const extArcEntry extarcs[2] = { arc1, arc2 };
  
    for(int e=0; e<2; e++)
    {
	    for(int b=0; b<2; b++)
	    {
		    const int XG[5][2] = {{0,0},{1,3},{3,2},{2,4},{4,1}};
		    const int group = XG[extarcs[e].group][b];
				const std::vector<int> *lineIdxList = m_vecArcs[group][extarcs[e].arcIdx[b]].lineIdxList; 
	    
	      // accumulate all lines of actual arc
	      const int nLines = static_cast<int>((*lineIdxList).size());
	      for(int l=0; l<nLines; l++) {
	        // get number of line segments and segment group
	        const int nSegments = static_cast<int>((*m_vecLines[group][(*lineIdxList)[l]].segmIdxList).size());
	  
	        // accumulate all segments
	        for(int i=0; i<nSegments; i++) {
	          // segment index
	          const int idx = (*m_vecLines[group][(*lineIdxList)[l]].segmIdxList)[i];
	          
	          // starting point of segment[i]
	          int x = m_vecSegments[group][idx].start.x;
	          int y = m_vecSegments[group][idx].start.y;
	        
	          // segments length
	          const int len = m_vecSegments[group][idx].len;
	          
	          // accumulate all segment pixels
	          for(int j=0; j<len; j++) {
	            // calculate sums
	            x_sum += x;
	            y_sum += y;
	            x2_sum += x*x;
	            y2_sum += y*y;
	            xy_sum += x*y;
	            x3_sum += x*x*x;
	            y3_sum += y*y*y;
	            xy2_sum += x*y*y;
	            x2y_sum += x*x*y;
	            
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
	            N++;  // increase number of all pixels
	          
	          }// next pixel
	        }// next segment
	      }// next line
	    }// next basearc
		}// next extarc
  
    const double  a1 = static_cast<double>(2*(x_sum * x_sum - N * x2_sum));
    const double  b1 = static_cast<double>(2*(x_sum * y_sum - N * xy_sum));
    const double  a2 = b1;
    const double  b2 = static_cast<double>(2*(y_sum * y_sum - N * y2_sum));
    const double  c1 = static_cast<double>(x2_sum * x_sum - N * x3_sum + x_sum * y2_sum - N * xy2_sum);
    const double  c2 = static_cast<double>(x2_sum * y_sum - N * y3_sum + y_sum * y2_sum - N * x2y_sum);
      
    // calculate circle parameters
    if((a1*b2 - a2*b1) != 0) {      // avoid division by zero
      Xest = (c1*b2 - c2*b1)/(a1*b2 - a2*b1);
      Yest = (a1*c2 - a2*c1)/(a1*b2 - a2*b1);
      R2est = (double)(x2_sum - 2*Xest*x_sum + N*Xest*Xest + y2_sum - 2*Yest*y_sum + N*Yest*Yest)/N;
      return true;
    }
    
    return false;   // division by zero
  }



 bool fastCircleExtraction::estimateCircle(const ipoint &start1, const ipoint &end1, 
                                            const ipoint &start2, const ipoint &end2,
                                            double &Xest, double &Yest, double &R2est) {
    const int x1 = start1.x;
    const int y1 = start1.y;
    const int x2 = end1.x;
    const int y2 = end1.y;
    const int x3 = start2.x;
    const int y3 = start2.y;
    const int x4 = end2.x;
    const int y4 = end2.y;

    lti::int64 N = 4;
    lti::int64 x_sum = x1 + x2 + x3 + x4;
    lti::int64 y_sum = y1 + y2 + y3 + y4; 
    lti::int64 x2_sum = x1*x1 + x2*x2 + x3*x3 + x4*x4;
    lti::int64 y2_sum = y1*y1 + y2*y2 + y3*y3 + y4*y4;
    lti::int64 xy_sum = x1*y1 + x2*y2 + x3*y3 + x4*y4;
    lti::int64 x3_sum = x1*x1*x1 + x2*x2*x2 + x3*x3*x3 + x4*x4*x4;
    lti::int64 y3_sum = y1*y1*y1 + y2*y2*y2 + y3*y3*y3 + y4*y4*y4;
    lti::int64 xy2_sum = x1*y1*y1 + x2*y2*y2 + x3*y3*y3 + x4*y4*y4;
    lti::int64 x2y_sum = x1*x1*y1 + x2*x2*y2 + x3*x3*y3 + x4*x4*y4;

    const double  a1 = static_cast<double>(2*(x_sum * x_sum - N * x2_sum));
    const double  b1 = static_cast<double>(2*(x_sum * y_sum - N * xy_sum));
    const double  a2 = b1;
    const double  b2 = static_cast<double>(2*(y_sum * y_sum - N * y2_sum));
    const double  c1 = static_cast<double>(x2_sum * x_sum - N * x3_sum + x_sum * y2_sum - N * xy2_sum);
    const double  c2 = static_cast<double>(x2_sum * y_sum - N * y3_sum + y_sum * y2_sum - N * x2y_sum);
      
    // calculate circle parameters
    if((a1*b2 - a2*b1) != 0) {      // avoid division by zero
      Xest = (c1*b2 - c2*b1)/(a1*b2 - a2*b1);
      Yest = (a1*c2 - a2*c1)/(a1*b2 - a2*b1);
      R2est = (double)(x2_sum - 2*Xest*x_sum + N*Xest*Xest + y2_sum - 2*Yest*y_sum + N*Yest*Yest)/N;
      return true;
    }
    
    return false;   // division by zero
  }



  bool fastCircleExtraction::estimateCircle(std::vector<lineEntry> &lines,
                                            double &Xest, double &Yest, double &R2est) {
    lti::int64 N = 0;
    lti::int64 x_sum = 0;
    lti::int64 y_sum = 0; 
    lti::int64 x2_sum = 0;
    lti::int64 y2_sum = 0; 
    lti::int64 xy_sum = 0;
    lti::int64 x3_sum = 0;
    lti::int64 y3_sum = 0;
    lti::int64 xy2_sum = 0;
    lti::int64 x2y_sum = 0;
  
    const int nLines = static_cast<int>(lines.size());
  
    // accumulate all lines
    for(int l=0; l<nLines; l++) {
      // get number of line segments and segment group
      const int nSegments = static_cast<int>((*lines[l].segmIdxList).size());
      const int group = lines[l].group;
  
      // accumulate all segments
      for(int i=0; i<nSegments; i++) {
        // segment index
        const int idx = (*lines[l].segmIdxList)[i];
        
        // starting point of segment[i]
        int x = m_vecSegments[group][idx].start.x;
        int y = m_vecSegments[group][idx].start.y;
      
        // segments length
        const int len = m_vecSegments[group][idx].len;
        
        // accumulate all segment pixels
        for(int j=0; j<len; j++) {
          // calculate sums
          x_sum += x;
          y_sum += y;
          x2_sum += x*x;
          y2_sum += y*y;
          xy_sum += x*y;
          x3_sum += x*x*x;
          y3_sum += y*y*y;
          xy2_sum += x*y*y;
          x2y_sum += x*x*y;
          
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
          N++;  // increase number of all pixels
        
        }// next pixel
      }// next segment
    }// next line
  
    const double  a1 = static_cast<double>(2*(x_sum * x_sum - N * x2_sum));
    const double  b1 = static_cast<double>(2*(x_sum * y_sum - N * xy_sum));
    const double  a2 = b1;
    const double  b2 = static_cast<double>(2*(y_sum * y_sum - N * y2_sum));
    const double  c1 = static_cast<double>(x2_sum * x_sum - N * x3_sum + x_sum * y2_sum - N * xy2_sum);
    const double  c2 = static_cast<double>(x2_sum * y_sum - N * y3_sum + y_sum * y2_sum - N * x2y_sum);
      
    // calculate circle parameters
    if((a1*b2 - a2*b1) != 0) {      // avoid division by zero
      Xest = (c1*b2 - c2*b1)/(a1*b2 - a2*b1);
      Yest = (a1*c2 - a2*c1)/(a1*b2 - a2*b1);
      R2est = (double)(x2_sum - 2*Xest*x_sum + N*Xest*Xest + y2_sum - 2*Yest*y_sum + N*Yest*Yest)/N;
      return true;
    }
    
    return false;   // division by zero
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  
  // On place apply for type channel8!
  bool fastCircleExtraction::apply(const channel8& srcdest) {

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

    // extract arcs
    extractGroup1Arcs();
    extractGroup2Arcs();
    extractGroup3Arcs();
    extractGroup4Arcs();

    // extract extended arcs
    extractExtGroup1Arcs();
    extractExtGroup2Arcs();
    extractExtGroup3Arcs();
    extractExtGroup4Arcs();

    // extract circles
    extractCircles();

    return true;
  };


  // On place apply for type channel!
  bool fastCircleExtraction::apply(const channel& srcdest) {
  
    lti::channel8 m_temp;
    m_temp.castFrom(srcdest, false, false);   // channel -> channel8
    apply(m_temp);
    return true;
  };

}
