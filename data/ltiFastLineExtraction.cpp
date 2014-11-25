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
 * file .......: ltiFastLineExtraction.cpp
 * authors ....: Ingo Grothues (ingo@isdevelop.de)
 * organization: LTI, RWTH Aachen
 * creation ...: 10.3.2004
 * revisions ..: $Id: ltiFastLineExtraction.cpp,v 1.10 2006/09/05 10:12:00 ltilib Exp $
 */

#include "ltiFastLineExtraction.h"

// number of pre-allocated vector entries
const int lti::fastLineExtraction::SegmPreallocation = 5000;
const int lti::fastLineExtraction::LinePreallocation = 1000;

namespace lti {
  // --------------------------------------------------
  // fastLineExtraction::parameters
  // --------------------------------------------------

  // default constructor
  fastLineExtraction::parameters::parameters()
    : featureExtractor::parameters() {

    minSegmLen = 2;
    minLineLen = 3;
    segmentTolerance = 0;
    maxSegmentGap = 1;
    maxQuantizationError = 1.0f;
  }

  // copy constructor
  fastLineExtraction::parameters::parameters(const parameters& other)
    : featureExtractor::parameters() {
    copy(other);
  }

  // destructor
  fastLineExtraction::parameters::~parameters() {
  }

  // get type name
  const char* fastLineExtraction::parameters::getTypeName() const {
    return "fastLineExtraction::parameters";
  }

  // copy member

  fastLineExtraction::parameters&
    fastLineExtraction::parameters::copy(const parameters& other) {
#ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    featureExtractor::parameters::copy(other);
#else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    featureExtractor::parameters& (featureExtractor::parameters::* p_copy)
      (const featureExtractor::parameters&) =
      featureExtractor::parameters::copy;
    (this->*p_copy)(other);
#endif

    minSegmLen = other.minSegmLen;
    minLineLen = other.minLineLen;
    segmentTolerance = other.segmentTolerance;
    maxSegmentGap = other.maxSegmentGap;
    maxQuantizationError = other.maxQuantizationError;
    return *this;
  }

  // alias for copy member
  fastLineExtraction::parameters&
    fastLineExtraction::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* fastLineExtraction::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
#ifndef _LTI_MSC_6
  bool fastLineExtraction::parameters::write(ioHandler& handler,
                                         const bool complete) const
#else
  bool fastLineExtraction::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
#endif
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
    }

#ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && featureExtractor::parameters::write(handler,false);
#else
    bool (featureExtractor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      featureExtractor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
#endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

#ifdef _LTI_MSC_6
  bool fastLineExtraction::parameters::write(ioHandler& handler,
                                         const bool complete) const {
    // ...we need this workaround to cope with another really awful MSVC bug.
    return writeMS(handler,complete);
  }
#endif

  /*
   * read the parameters from the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also read, otherwise only the data block will be read.
   * @return true if write was successful
   */
#ifndef _LTI_MSC_6
  bool fastLineExtraction::parameters::read(ioHandler& handler,
                                        const bool complete)
#else
  bool fastLineExtraction::parameters::readMS(ioHandler& handler,
                                          const bool complete)
#endif
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
    }

#ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && featureExtractor::parameters::read(handler,false);
#else
    bool (featureExtractor::parameters::* p_readMS)(ioHandler&,const bool) =
      featureExtractor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
#endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

#ifdef _LTI_MSC_6
  bool fastLineExtraction::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
#endif




/////////////////////////////////////////////////////////////////////////////////


  // --------------------------------------------------
  // fastLineExtraction
  // --------------------------------------------------

  // default constructor
  fastLineExtraction::fastLineExtraction()
    : featureExtractor(){
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
    // default initialization
    m_iWidth = 0;
  }

  // default constructor
  fastLineExtraction::fastLineExtraction(const parameters& par)
    : featureExtractor() {
    // set the given parameters
    setParameters(par);
    // default initialization
    m_iWidth = 0;
  }


  // copy constructor
  fastLineExtraction::fastLineExtraction(const fastLineExtraction& other)
    : featureExtractor() {
    copy(other);
  }

  // destructor
  fastLineExtraction::~fastLineExtraction() {
    for(int i=0; i<5; i++) {
      m_vecSegments[i].clear(); 
      m_vecLines[i].clear(); 
    }
  }

  // returns the name of this type
  const char* fastLineExtraction::getTypeName() const {
    return "fastLineExtraction";
  }

  // copy member
  fastLineExtraction&
    fastLineExtraction::copy(const fastLineExtraction& other) {
      featureExtractor::copy(other);

    for(int i=0; i<5; i++) {
      m_vecLines[i] = other.m_vecLines[i];
      m_vecSegments[i] = other.m_vecSegments[i];
    }
    m_iWidth = other.m_iWidth;
    return (*this);
  }

  // alias for copy member
  fastLineExtraction&
    fastLineExtraction::operator=(const fastLineExtraction& other) {
    return (copy(other));
  }


  // clone member
  functor* fastLineExtraction::clone() const {
    return new fastLineExtraction(*this);
  }

  // return parameters
  const fastLineExtraction::parameters&
    fastLineExtraction::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // returns vector with line segments
  std::vector<fastLineExtraction::segmEntry>& fastLineExtraction::getSegmentList(const int group_number) {
    if((group_number < 0)||(group_number > 4)) {
      return m_vecSegments[0];
    }
    return m_vecSegments[group_number];
  }

  // returns vector with lines
  std::vector<fastLineExtraction::segmEntry>& fastLineExtraction::getLineList(const int group_number) {
    if((group_number < 0)||(group_number > 4)) {
      return m_vecLines[0];
    }
    return m_vecLines[group_number];
  }

  // returns number of non-grouped segments
  const int fastLineExtraction::getNonGroupedSegmentCount(const int group_number) const {
    if((group_number < 1)||(group_number > 4)) {
      return -1;
    }
    
    int nonGrouped = 0;
    const int segmCount = m_vecSegments[group_number].size();

    for(int i=0; i<segmCount; i++) {
      if(m_vecSegments[group_number][i].used == 0) {
        nonGrouped++;
      }
    }
    return nonGrouped;
  }

  // returns number of multi-grouped segments
  const int fastLineExtraction::getMultiGroupedSegmentCount(const int group_number) const {
    if((group_number < 1)||(group_number > 4)) {
      return -1;
    }

    int multiGrouped = 0;
    const int segmCount = m_vecSegments[group_number].size();

    for(int i=0; i<segmCount; i++) {
      if(m_vecSegments[group_number][i].used > 1) {
        multiGrouped++;
      }
    }
    return multiGrouped;
  }


///////////////////////////////////////////////////////////////////////////////

  // creates segment lists
  void fastLineExtraction::detectGroup1Segments(const channel8& src) {
    
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
  void fastLineExtraction::detectGroup2Segments(const channel8& src) {

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
  void fastLineExtraction::detectGroup3Segments(const channel8& src) {
  
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
  void fastLineExtraction::detectGroup4Segments(const channel8& src) {
  
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

  void fastLineExtraction::extractGroup1Lines() {
  
    // clear old complete line list and reserve memory
    m_vecLines[0].clear(); 
    m_vecLines[0].reserve(LinePreallocation);

    // clear old linelist and reserve memory
    m_vecLines[1].clear(); 
    m_vecLines[1].reserve(LinePreallocation);
    
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = m_vecSegments[1].size();

    if(segmCount > 0) {
      // create line segment grouping vector
      std::vector<int>  vecLineSegments;

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;
      
      do {
        bool positive_slope = true;   // sign of the slope of the first segment pair
        bool first_candidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments.push_back(Si);  // store Si in linelist (starting segment)
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

            const segmEntry segSq = m_vecSegments[1][Sq];
            
            if(segSq.start.y > GAPy) { // window boundary reached ?
              Sq = segmCount;          // no candidate segment found
              break;
            }

            // positive slope or first candidate
            if(positive_slope || first_candidate) {
              if((segSq.start.x <= GAPx_pos)                         // Sq starts in window 
              && (segSq.start.x >= segSS.end.x - segSS.len/2 + 1)) { // Sq starts right the center of SS
                positive_slope = true;
                break;
              }
            }

            // negative slope or first candidate
            if(!positive_slope || first_candidate) {
              if((segSq.end.x >= GAPx_neg)                           // Sq ends in window 
              && (segSq.end.x <= segSS.start.x + segSS.len/2 - 1)) { // Sq ends left the center of SS
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
          vecLineSegments.push_back(Sq);
          
          // STEP5
          // set Sq as new target segment SS and increment candidate segment
          SS = Sq++;
        }

        // STEP7
        // calculate line length
        const int nSegments = vecLineSegments.size();
        int length = 0;

        if(positive_slope) {
          const int x1 = m_vecSegments[1][vecLineSegments[0]].start.x;
          const int x2 = m_vecSegments[1][vecLineSegments[nSegments-1]].end.x;
          length = x2 - x1 + 1;
        }
        else {
          const int x1 = m_vecSegments[1][vecLineSegments[nSegments-1]].start.x;
          const int x2 = m_vecSegments[1][vecLineSegments[0]].end.x;
          length = x2 - x1 + 1;
        }
/*
				// check line coverage (if maxSegmentGap > 1)
				float coverage = 1;
				if(par.maxSegmentGap > 1) {
					int sum = 0;
					for(int i=0; i<nSegments; i++) {
						sum += m_vecSegments[1][vecLineSegments[i]].len;
					}
					coverage = (float)sum/length;
				}

        // line length must be >= minLineLen for extraction
        if((length >= par.minLineLen)&&(coverage > 0.9)) {
*/			if(length >= par.minLineLen) {
	          
          // register grouped segments as extracted line
          segmEntry newline;
          if(positive_slope) {
            newline.start.x = m_vecSegments[1][vecLineSegments[0]].start.x;
            newline.start.y = m_vecSegments[1][vecLineSegments[0]].start.y;
            newline.end.x = m_vecSegments[1][vecLineSegments[nSegments-1]].end.x;
            newline.end.y = m_vecSegments[1][vecLineSegments[nSegments-1]].end.y;
          }
          else {
            newline.start.x = m_vecSegments[1][vecLineSegments[nSegments-1]].start.x;
            newline.start.y = m_vecSegments[1][vecLineSegments[nSegments-1]].start.y;
            newline.end.x = m_vecSegments[1][vecLineSegments[0]].end.x;
            newline.end.y = m_vecSegments[1][vecLineSegments[0]].end.y;
          }
          newline.len = length;
          m_vecLines[1].push_back(newline);
          m_vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecSegments[1][vecLineSegments[i]].used++;
          }
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecSegments[1][Si].used == 0) break;
        }
        
        // clear segment grouping vector
        vecLineSegments.clear();
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////////////

  void fastLineExtraction::extractGroup2Lines() {
  
    // clear old linelist and reserve memory
    m_vecLines[2].clear(); 
    m_vecLines[2].reserve(LinePreallocation);

    // get parameters
    const parameters& par = getParameters();

    const int segmCount = m_vecSegments[2].size();

    if(segmCount > 0) {
      // create line segment grouping vector
      std::vector<int>  vecLineSegments;

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        bool positive_slope = true;   // sign of the slope of the first segment pair
        bool first_candidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments.push_back(Si);  // store Si in linelist (starting segment)
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
          vecLineSegments.push_back(Sq);
          
          // STEP5
          // set Sq as new target segment SS and increment candidate segment
          SS = Sq++;
        }

        // STEP7
        // calculate line length
        const int nSegments = vecLineSegments.size();
        int length = 0;

        if(positive_slope) {
          const int y1 = m_vecSegments[2][vecLineSegments[0]].start.y;
          const int y2 = m_vecSegments[2][vecLineSegments[nSegments-1]].end.y;
          length = y2 - y1 + 1;
        }
        else {
          const int y1 = m_vecSegments[2][vecLineSegments[nSegments-1]].start.y;
          const int y2 = m_vecSegments[2][vecLineSegments[0]].end.y;
          length = y2 - y1 + 1;
        }
/*
				// check line coverage (if maxSegmentGap > 1)
				float coverage = 1;
				if(par.maxSegmentGap > 1) {
					int sum = 0;
					for(int i=0; i<nSegments; i++) {
						sum += m_vecSegments[2][vecLineSegments[i]].len;
					}
					coverage = (float)sum/length;
				}

        // line length must be >= minLineLen for extraction
        if((length >= par.minLineLen)&&(coverage > 0.9)) {
*/			if(length >= par.minLineLen) {
	        
          // register grouped segments as extracted line
          segmEntry newline;
          if(positive_slope) {
            newline.start.x = m_vecSegments[2][vecLineSegments[0]].start.x;
            newline.start.y = m_vecSegments[2][vecLineSegments[0]].start.y;
            newline.end.x = m_vecSegments[2][vecLineSegments[nSegments-1]].end.x;
            newline.end.y = m_vecSegments[2][vecLineSegments[nSegments-1]].end.y;
          }
          else {
            newline.start.x = m_vecSegments[2][vecLineSegments[nSegments-1]].start.x;
            newline.start.y = m_vecSegments[2][vecLineSegments[nSegments-1]].start.y;
            newline.end.x = m_vecSegments[2][vecLineSegments[0]].end.x;
            newline.end.y = m_vecSegments[2][vecLineSegments[0]].end.y;
          }
          newline.len = length;
          m_vecLines[2].push_back(newline);
          m_vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecSegments[2][vecLineSegments[i]].used++;
          }
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecSegments[2][Si].used == 0) break;
        }
        
        // clear segment grouping vector
        vecLineSegments.clear();
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////////////

  void fastLineExtraction::extractGroup3Lines() {

    // clear old linelist and reserve memory
    m_vecLines[3].clear(); 
    m_vecLines[3].reserve(LinePreallocation);
  
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = m_vecSegments[3].size();
    
    if(segmCount > 0) {
      // create line segment grouping vector
      std::vector<int>  vecLineSegments;

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        bool positive_slope = true;   // sign of the slope of the first segment pair
        bool first_candidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments.push_back(Si);  // store Si in linelist (starting segment)
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
              && (Sq_start_x >= SS_end_x - segSS.len + 1)) { // Sq lies right the center of SS
                positive_slope = true;
                break;
              }
            }

            // negative slope or first candidate
            if(!positive_slope || first_candidate) {
              if((Sq_end_x >= GAPx_neg)                      // Sq ends in window 
              && (Sq_end_x <= SS_start_x + segSS.len - 1)) { // Sq lies left the center of SS
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
          vecLineSegments.push_back(Sq);
          
          // STEP5
          // set Sq as new target segment SS and increment candidate segment
          SS = Sq++;
        }

        // STEP7
        // calculate line length
        const int nSegments = vecLineSegments.size();
        int length = 0;

        if(positive_slope) {
          const int y1 = m_vecSegments[3][vecLineSegments[0]].start.y;
          const int y2 = m_vecSegments[3][vecLineSegments[nSegments-1]].end.y;
          length = y2 - y1 + 1;
        }
        else {
          const int x1 = m_vecSegments[3][vecLineSegments[nSegments-1]].start.x;
          const int x2 = m_vecSegments[3][vecLineSegments[0]].end.x;
          length = x2 - x1 + 1;
        }
/*
				// check line coverage (if maxSegmentGap > 1)
				float coverage = 1;
				if(par.maxSegmentGap > 1) {
					int sum = 0;
					for(int i=0; i<nSegments; i++) {
						sum += m_vecSegments[3][vecLineSegments[i]].len;
					}
					coverage = (float)sum/length;
				}

        // line length must be >= minLineLen for extraction
        if((length >= par.minLineLen)&&(coverage > 0.9)) {
*/			if(length >= par.minLineLen) {
	        
          // register grouped segments as extracted line
          segmEntry newline;
          if(positive_slope) {
            newline.start.x = m_vecSegments[3][vecLineSegments[0]].start.x;
            newline.start.y = m_vecSegments[3][vecLineSegments[0]].start.y;
            newline.end.x = m_vecSegments[3][vecLineSegments[nSegments-1]].end.x;
            newline.end.y = m_vecSegments[3][vecLineSegments[nSegments-1]].end.y;
          }
          else {
            newline.end.x = m_vecSegments[3][vecLineSegments[0]].end.x;
            newline.end.y = m_vecSegments[3][vecLineSegments[0]].end.y;
            newline.start.x = m_vecSegments[3][vecLineSegments[nSegments-1]].start.x;
            newline.start.y = m_vecSegments[3][vecLineSegments[nSegments-1]].start.y;          
          }
          newline.len = length;
          m_vecLines[3].push_back(newline);
          m_vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecSegments[3][vecLineSegments[i]].used++;
          }
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecSegments[3][Si].used == 0) break;
        }
        
        // clear segment grouping vector
        vecLineSegments.clear();
      }
      while(Si < segmCount);
    }
  }

///////////////////////////////////////////////////////////////////////////////

  void fastLineExtraction::extractGroup4Lines() {

    // clear old linelist and reserve memory
    m_vecLines[4].clear(); 
    m_vecLines[4].reserve(LinePreallocation);
  
    // get parameters
    const parameters& par = getParameters();

    const int segmCount = m_vecSegments[4].size();
    
    if(segmCount > 0) {
      // create line segment grouping vector
      std::vector<int>  vecLineSegments;

      // set first element as starting segment Si
      int Si = 0;
      int Sq, SS, SL;

      do {
        bool positive_slope = true;   // sign of the slope of the first segment pair
        bool first_candidate = true;   // only the first segment pair defines the slope
        
        // STEP1
        vecLineSegments.push_back(Si);  // store Si in linelist (starting segment)
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
              && (Sq_start_y <= SS_end_y + segSS.len - 1)) {  // Sq lies above the center of SS
                positive_slope = true;
                break;
              }
            }

            // negative slope or first candidate
            if(!positive_slope || first_candidate) {
              if((Sq_end_y <= GAPx_pos)                      // Sq ends in window 
              && (Sq_end_y >= SS_start_y - segSS.len + 1)) { // Sq lies below the center of SS
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
          vecLineSegments.push_back(Sq);
          
          // STEP5
          // set Sq as new target segment SS and increment candidate segment
          SS = Sq++;
        }

        // STEP7
        // calculate line length
        const int nSegments = vecLineSegments.size();
        int length = 0;

        if(positive_slope) {
          const int y1 = m_vecSegments[4][vecLineSegments[nSegments-1]].end.y;
          const int y2 = m_vecSegments[4][vecLineSegments[0]].start.y;
          length = y2 - y1 + 1;
        }
        else {
          const int x1 = m_vecSegments[4][vecLineSegments[nSegments-1]].start.x;
          const int x2 = m_vecSegments[4][vecLineSegments[0]].end.x;
          length = x2 - x1 + 1;
        }
/*
				// check line coverage (if maxSegmentGap > 1)
				float coverage = 1;
				if(par.maxSegmentGap > 1) {
					int sum = 0;
					for(int i=0; i<nSegments; i++) {
						sum += m_vecSegments[4][vecLineSegments[i]].len;
					}
					coverage = (float)sum/length;
				}

        // line length must be >= minLineLen for extraction
        if((length >= par.minLineLen)&&(coverage > 0.9)) {
*/        if(length >= par.minLineLen) {
        
          // register grouped segments as extracted line
          segmEntry newline;
          if(positive_slope) {
            newline.start.x = m_vecSegments[4][vecLineSegments[nSegments-1]].end.x;
            newline.start.y = m_vecSegments[4][vecLineSegments[nSegments-1]].end.y;
            newline.end.x = m_vecSegments[4][vecLineSegments[0]].start.x;
            newline.end.y = m_vecSegments[4][vecLineSegments[0]].start.y;
          } else {
            newline.start.x = m_vecSegments[4][vecLineSegments[0]].end.x;
            newline.start.y = m_vecSegments[4][vecLineSegments[0]].end.y;
            newline.end.x = m_vecSegments[4][vecLineSegments[nSegments-1]].start.x;
            newline.end.y = m_vecSegments[4][vecLineSegments[nSegments-1]].start.y;
          }
          newline.len = length;
          m_vecLines[4].push_back(newline);
          m_vecLines[0].push_back(newline);

          // increase used-value of the grouped segments
          for(int i=0; i<nSegments; i++) {
            m_vecSegments[4][vecLineSegments[i]].used++;
          }
        }

        // set next non-used segment as new starting segment Si
        while(++Si < segmCount) {
          if(m_vecSegments[4][Si].used == 0) break;
        }
        
        // clear segment grouping vector
        vecLineSegments.clear();
      }
      while(Si < segmCount);
    }
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  
  // On place apply for type channel8!
  bool fastLineExtraction::apply(const channel8& srcdest) {

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

    return true;
  };


  // On place apply for type channel!
  bool fastLineExtraction::apply(const channel& srcdest) {
  
    lti::channel8 m_temp;
    m_temp.castFrom(srcdest, false, false);   // channel -> channel8
    apply(m_temp);
    return true;
  };

}
