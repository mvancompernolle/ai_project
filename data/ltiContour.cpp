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
 * file .......: ltiContour.cpp
 * authors ....: AXON development team
 * organization: LTI, RWTH Aachen
 * creation ...: 26.04.2000
 * revisions ..: $Id: ltiContour.cpp,v 1.6 2006/02/07 18:40:10 ltilib Exp $
 */


#include "ltiObject.h"
#include "ltiContour.h"
#include "ltiPolygonPoints.h"
#include "ltiObjectsFromMask.h"
#include "ltiDraw.h"
#include "ltiCubicSpline.h"


#define _DEBUG_MASK 0

#if _DEBUG_MASK
#include "ltiExternViewer.h"
#endif


namespace lti {

  /* 
   * Storable interface
   * Members for the storable interface
   */

  /*
   * read the chainCode element from the given ioHandler.  The
   * complete flag will be ignored
   */
  bool read(ioHandler& handler,chainCode& cc,const bool complete) {
    bool b;
    std::string symb;
    b = handler.readSymbol(symb);

    if (b) {
      if (symb == "noMove") {
        cc = chainCode::NOMOVE;
      } else if (symb == "E") {
        cc = chainCode::E;
      } else if (symb == "NE") {
        cc = chainCode::NE;
      } else if (symb == "N") {
        cc = chainCode::N;
      } else if (symb == "NW") {
        cc = chainCode::NW;
      } else if (symb == "W") {
        cc = chainCode::W;
      } else if (symb == "SW") {
        cc = chainCode::SW;
      } else if (symb == "S") {
        cc = chainCode::S;
      } else if (symb == "SE") {
        cc = chainCode::SE;
      } else {
        cc == chainCode::UNKNOWN;
      }
    }

    return b;
  }

  /*
   * write the matrix in the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,const chainCode& cc,const bool complete) {
    bool b;
    switch (cc.getDirection()) {
        case chainCode::NOMOVE:
          b = handler.writeSymbol("noMove");
          break;
        case chainCode::E:
          b = handler.writeSymbol("E");
          break;
        case chainCode::NE:
          b = handler.writeSymbol("NE");
          break;
        case chainCode::N:
          b = handler.writeSymbol("N");
          break;
        case chainCode::NW:
          b = handler.writeSymbol("NW");
          break;
        case chainCode::W:
          b = handler.writeSymbol("W");
          break;
        case chainCode::SW:
          b = handler.writeSymbol("SW");
          break;
        case chainCode::S:
          b = handler.writeSymbol("S");
          break;
        case chainCode::SE:
          b = handler.writeSymbol("SE");
          break;
        default:
          b = handler.writeSymbol("unknown");
    }
    return b;
  }

  // static members of chainCode
  const ubyte chainCode::Nothing = 0;
  const ubyte chainCode::Top     = 1;
  const ubyte chainCode::Right   = 2;
  const ubyte chainCode::Bottom  = 4;
  const ubyte chainCode::Left    = 8;

  // -----------------------------------------------------------------
  // IO POINTS
  // -----------------------------------------------------------------

  ioPoints::ioPoints() : pointList() {
  }

  ioPoints::ioPoints(const ioPoints& other)
    : pointList() {
    copy(other);
  }

  ioPoints::~ioPoints() {
  }

  ioPoints& ioPoints::castFrom(const areaPoints& theAreaPoints) {

    if (theAreaPoints.size() == 1) {
      // special case with just one pixel can be done much faster..
      clear();
      push_back(*theAreaPoints.begin()); // in
      push_back(*theAreaPoints.begin()); // out
      return *this;
    }
    //draws the borderPoints in a channel and generates ioPoints from
    //this channel this function removes the offset from the points so
    //you can cast negativ points
    areaPoints ap; //the points without offset

    pointList::const_iterator itc;
    pointList::iterator it;

    //calculates the real size of the area without offset
    rectangle maxRect = theAreaPoints.computeBoundary();

    //creates a channel8 as big as the ioPoints
    channel8 mask(maxRect.getDimensions(),lti::ubyte(0));
    //remove the offset and copy the points to new list
    for (itc=theAreaPoints.begin();itc!=theAreaPoints.end();itc++){
      ap.push_back((*itc) - maxRect.ul);
    }

    //draw the points in a channel and get the io points from the channel
    ap.setBoundary(rectangle(point(0,0),maxRect.br-maxRect.ul));
    ap.generateMask(mask,false);
    getFromMask(mask);

    //add the offset to the ioPoints
    for (it=begin();it!=end();it++) {
      (*it) += maxRect.ul;
    }

    return *this;
  }

  ioPoints& ioPoints::castFrom(const borderPoints& theBorderPoints) {
    //draws the borderPoints in a channel and generates ioPoints from
    //this channel this function removes the offset from the points so
    //you can cast negativ points
    borderPoints bp; //i need non const points for calculate boundary
    pointList::const_iterator itc;
    pointList::iterator it;

    //calculates the real size of the border without offset
    rectangle maxRect = theBorderPoints.computeBoundary();

    //creates a channel8 as big as the borderPoints
    channel8 mask(maxRect.getDimensions(),lti::ubyte(0));

    //remove the offset and copy the points to new list
    for (itc=theBorderPoints.begin();itc!=theBorderPoints.end();itc++){
      bp.push_back((*itc) - maxRect.ul);
    }

    //draw the points in a chanel and get the ioPoints from the channel
    bp.setBoundary(rectangle(point(0,0),maxRect.br-maxRect.ul));
    bp.generateMask(mask,false);
    getFromMask(mask);

    //add the offset to the ioPoints
    for (it=begin();it!=end();it++) {
      (*it) += maxRect.ul;
    }

    return *this;
  }

  bool ioPoints::getFromMask(const channel8& mask) {
    point p,q;

    // calculate io points
    clear();

    bool out=true;
    static const ubyte off = 0;

    q = point(0,0);

    for (p.y=0;p.y<mask.rows();p.y++) {
      for (p.x=0;p.x<mask.columns();q=p,p.x++) {
        if (out && mask.at(p)!=off) {
          // IN point
          push_back(p);
          out = false;
        } else if (!out && mask.at(p) == off) {
          // OUT point
          push_back(q);
          out = true;
        }
      }

      if (!out) { // end of line! => force OUT point
        push_back(q);
        out = true;
      }
    }

    return true;
  }

  bool ioPoints::generateMask(channel8& mask,
                              const bool computeBoundary,
                              const bool exactBoundaryDimensions,
                              const bool keepMaskData) const {
    const_iterator it;
    point in,out;
    int x;

    static const ubyte off = 0;
    static const ubyte on  = 255;

    if (empty()) {
      return true;
    }

    lti::trectangle<int> tmpBoundary = 
      computeBoundary ? this->computeBoundary() : getBoundary();

    mask.resize(max(tmpBoundary.br.y+1,mask.rows()),
                max(tmpBoundary.br.x+1,mask.columns()),off,keepMaskData,true);

    it = begin();

    while (it!=end()) {
      // get in and out
      in = *it;
      it++;
      if (it==end()) {
        // no output point? -> there must be an error!
        return false;
      }

      out = *it;
      it++;

      if (out.y!=in.y) {
        // in and out must lie in the same line! -> error
        return false;
      }

      if (static_cast<unsigned int>(in.y) <
          static_cast<unsigned int>(mask.rows())) {
        vector<channel8::value_type>& vct = mask.getRow(in.y);
        const int maxx=min(out.x,mask.lastColumn());
        for (x=max(0,in.x);x<=maxx;x++) {
          vct.at(x)=on;
        }
      }
    }

    // This is a quick-and-dirty hack. The fastest implementation
    // would be to completely copy the above code for both cases
    // (exactBoundaryDimensions == false / true) and adapt the matrix
    // resize and access operations accordingly.
    if (exactBoundaryDimensions) {
      lti::channel8 mask2(mask);

      mask.resize(
        tmpBoundary.br.y-tmpBoundary.ul.y+1,
        tmpBoundary.br.x-tmpBoundary.ul.x+1,
        off,false,true);

      mask.fill(mask2,0,0,
        MaxInt32,MaxInt32,tmpBoundary.ul.y,tmpBoundary.ul.x);
    }

    return true;
  }

  void ioPoints::sort() {
    if (size() < 2) {
      return;
    }

    thePointList.sort();
  }

  // -----------------------------------------------------------------
  // BORDER POINTS
  // -----------------------------------------------------------------

  borderPoints::borderPoints() : pointList() {
  }

  borderPoints::borderPoints(const borderPoints& other) : pointList() {
    copy(other);
  }

  borderPoints::~borderPoints() {
  }

  borderPoints& borderPoints::castFrom(const ioPoints& theIOPoints) {
    //draws the ioPoints in a channel and generates borderPoints from
    //this channel this function removes the offset from the points so
    //you can cast negativ points
    ioPoints ioP; //i need non const ioPoints for calculate boundary
    pointList::iterator it;
    pointList::const_iterator itc;

    //calculates the real size of the ioPoints without offset
    rectangle maxRect = theIOPoints.computeBoundary();

    //creates a channel8 as big as the ioPoints
    channel8 mask(maxRect.getDimensions(),lti::ubyte(0));

    //remove the offset and copy the points to new list
    for (itc=theIOPoints.begin();itc!=theIOPoints.end();itc++){
      ioP.push_back((*itc) - maxRect.ul);
    }

    //draw the points in a chanel and get the borderPoints from the channel
    ioP.setBoundary(rectangle(point(0,0),maxRect.br-maxRect.ul));
    ioP.generateMask(mask,false);
    getFromMask(mask);

    //add the offset to the borderPoints
    for (it=begin();it!=end();it++) {
      (*it) += maxRect.ul;
    }
    return *this;
  }

  borderPoints& borderPoints::castFrom(const areaPoints& theAreaPoints) {
    //draws the areaPoints in a channel and generates borderPoints
    //from this channel this function removes the offset from the
    //points so you can cast negativ points
    areaPoints aP; //i need non const areaPoints for calculate boundary
    pointList::iterator it;
    pointList::const_iterator itc;

    //calculates the real size of the polygon without offset
    rectangle maxRect = theAreaPoints.computeBoundary();

    //creates a channel8 as big as the areaPoints
    channel8 mask(maxRect.getDimensions(),lti::ubyte(0));

    //remove the offset and copy the points to new list
    for (itc=theAreaPoints.begin();itc!=theAreaPoints.end();itc++){
      aP.push_back((*itc) - maxRect.ul);
    }

    //draw the points in a channel and get the border points from the channel
    aP.setBoundary(rectangle(point(0,0),maxRect.br-maxRect.ul));
    aP.generateMask(mask);
    getFromMask(mask);

    //add the offset to the borderPoints
    for (it=begin();it!=end();it++) {
      (*it) += maxRect.ul;
    }

    return *this;
  }

  borderPoints& borderPoints::castFrom(const polygonPoints& thePolygonPoints){
    // this function draws the polygon into a channel8 and extracts
    // the borderPoints with the getFromMask function of the
    // borderPoints
    borderPoints bp;
    draw<channel8::value_type> drawer;
    polygonPoints::iterator it;
    pointList::const_iterator itc;
    borderPoints::iterator itbp;
    rectangle maxRect;

    // a non const polygon is need
    polygonPoints pPs;

    // calculates the real size of the polygon without offset
    maxRect = thePolygonPoints.computeBoundary();

    // creates a channel8 as big as the polygon
    channel8 mask(maxRect.getDimensions(),lti::ubyte(0));

    //remove the offset
    for (itc=thePolygonPoints.begin();itc!=thePolygonPoints.end();itc++){
      pPs.push_back((*itc) - maxRect.ul);
    }

    drawer.use(mask);
    drawer.setColor(255);
    drawer.set(*pPs.begin());
    //connect the polygonPoints with lines
    for (it=pPs.begin();it!=pPs.end();it++)
    {
      drawer.lineTo((*it));
    }//close the polygon
    drawer.lineTo(*pPs.begin());

    //extract the object as borderPointList
    std::list<borderPoints> objectList;
    std::list<borderPoints>::iterator it_bPl;
    objectsFromMask extractor;
    objectsFromMask::parameters extractorParams;
    extractorParams.level = 0;
    extractorParams.meltHoles = true;
    extractor.setParameters(extractorParams);
    extractor.apply(mask,objectList);
    // if more than one object in the channel -> abort
    if(objectList.size() == 1){
      it_bPl= objectList.begin();
      copy(*it_bPl);
      //add the offset to the borderPoints
      for (itbp=begin();itbp!=end();itbp++) {
        (*itbp) += maxRect.ul;
      }
    }
    else clear();

    return *this;
  }

  borderPoints& borderPoints::castFrom(const polygonPoints& thePolygonPoints,
                                       const int& segments) {

    double x,y,di,dt;
    long int lt=0;
    dt = 1.0 / (abs(segments)+1);

    cubicSpline<double>::parameters sParamX,sParamY;
    cubicSpline<double> splineX,splineY;

    polygonPoints::const_iterator itc;
    tpointList<double> plx,ply;
    polygonPoints result;

    lt=0;
    for(itc=thePolygonPoints.begin();itc!=thePolygonPoints.end();itc++) {
      //copy the x and y function of the 2d curve in 2 lists
      plx.push_back(tpoint<double>(lt,(*itc).x));
      ply.push_back(tpoint<double>(lt,(*itc).y));
      lt++;
    }
    for(itc=thePolygonPoints.begin();itc!=thePolygonPoints.end();itc++) {
      //copy the x and y function of the 2d curve in 2 lists
      plx.push_back(tpoint<double>(lt,(*itc).x));
      ply.push_back(tpoint<double>(lt,(*itc).y));
      lt++;
    }
    for(itc=thePolygonPoints.begin();itc!=thePolygonPoints.end();itc++) {
      //copy the x and y function of the 2d curve in 2 lists
      plx.push_back(tpoint<double>(lt,(*itc).x));
      ply.push_back(tpoint<double>(lt,(*itc).y));
      lt++;
    }

    sParamX.samplingPoints = plx;           sParamY.samplingPoints = ply;
    splineX.setParameters(sParamX);         splineY.setParameters(sParamY);

    result.clear();
    for(lt=thePolygonPoints.size();lt<(thePolygonPoints.size()*2);lt++)
    {
      for(di=0;di<1;di+=dt)
      {
        splineX.apply(lt+di,x);
        splineY.apply(lt+di,y);
        result.push_back(point(int(x+0.5),int(y+0.5)));
      }
    }

    castFrom(result);
    return *this;
  }


  bool borderPoints::getFromMask(const channel8& mask) {

    point p,q,anchor;

    static const ubyte off = 0;

    // calculate border points
    clear();

    //  look for anchor
    anchor.x = -1;

    p.y = 0;
    while ((anchor.x<0) && (p.y<mask.rows())) {
      p.x=0;
      while (p.x<mask.columns()) {
        if (mask.at(p) != off) {
          anchor.copy(p);
          p.x = mask.columns(); // flag for exit
        }
        p.x++;
      }
      p.y++;
    }

    if (anchor.x<0) {
      // no object in the mask!
      return true;
    }

    // current direction
    chainCode current(chainCode::E);
    p=anchor;

    push_back(p);

    // search border points
    do {
      int dirCounter = 0;
      q = current.getNext(p); // next point
      // repeat until masked point found or all directions have been checked
      while ((!inside(q, mask) || (mask.at(q) == off)) && (dirCounter < 8)) {
        // rotate clockwise
        current.subtract(chainCode(chainCode::NE));
        q = current.getNext(p);
        dirCounter++;
      }
      // no border point found
      if (dirCounter == 8) {
        return true;
      }

      // new point is a border point
      p = q;
      push_back(p);

      // turn 90 degrees counterclockwise and continue searching clockwise
      current.add(chainCode(chainCode::N));
    } while (p!=anchor);

    // the last element is again the anchor element,
    // which was inserted at the beginning:
    pop_back(); // delete duplicated anchor!

    return true;
  }

  bool borderPoints::generateMask(channel8& mask,
                                  const bool computeBoundary,
                                  const bool exactBoundaryDimensions,
                                  const bool keepMaskData) const {

    static const ubyte off     = chainCode::Nothing;
    static const ubyte on      = 255;

    if (empty()) {
      return true;
    }

    // boundary is not updated automatically -> compute it on user
    // request, else use stored boundary (possibly inaccurate)
    lti::trectangle<int> tmpBoundary =
      computeBoundary ? this->computeBoundary() : getBoundary();

    mask.resize(max(tmpBoundary.br.y+1,mask.rows()),
                max(tmpBoundary.br.x+1,mask.columns()),off,keepMaskData,true);

    const_iterator it;

    if (size() < 4) {
      // three points are at least required for the next algorithms, so
      // if there are too few points, just draw them in the mask at that
      // was it!
      for (it = begin(); it != end(); ++it) {
        mask.at(*it) = on;
      }
      return true;
    }

    point p,tst;

    // for each three points in the contour create the canzler-codes map
    it = begin();
    p = *it;
    ++it;
    tst = *it;
    ++it;
    chainCode oldC(p,tst);

    for (;it!=end();++it) {
      p = *it;
      chainCode c(tst,p);
      mask.at(tst) |= oldC.getCanzlerCode(c);
      oldC = c;
      tst = p;
    }

    // the last things (end of the list and the begining
    it = begin();
    p = *it;
    chainCode c(tst,p);
    mask.at(tst) |= oldC.getCanzlerCode(c); // the last element in the list
    oldC = c;
    tst = p;
    ++it;
    p = *it;
    c = chainCode(tst,p);
    mask.at(tst) |= oldC.getCanzlerCode(c); // the first element in the list

    // now we have the canzler map, which contains the io-points, we can
    // just search for them and fill the contents.

    int y;
    bool drawFromBegin;
    bool drawing;
    ubyte val;

    vector<channel8::value_type>::iterator mit,eit,xit;
    for (y=0;y<mask.rows();y++) {
      drawFromBegin = true;
      drawing = false;

      vector<channel8::value_type>& vct = mask.getRow(y);
      for (mit=vct.begin(),eit=vct.end();mit!=eit;++mit) {
        val = (*mit);
        if ((val & chainCode::Left) == chainCode::Left) {
          drawFromBegin = false;
          drawing = true;
        }

        if (drawing) {
          (*mit) = on;
        }

        if ((val & chainCode::Right) == chainCode::Right) {
          drawing = false;
          if (drawFromBegin) { // ups! an "out" pixel found first..
            for (xit=vct.begin();xit!=mit;++xit) {
              (*xit) = on;
            }
            drawFromBegin = false;
          }
        }

      }
    }

    // This is a quick-and-dirty hack. The fastest implementation
    // would be to completely copy the above code for both cases
    // (exactBoundaryDimensions == false / true) and adapt the matrix
    // resize and access operations accordingly.
    if (exactBoundaryDimensions) {
      lti::channel8 mask2(mask);

      mask.resize(
        tmpBoundary.br.y-tmpBoundary.ul.y+1,
        tmpBoundary.br.x-tmpBoundary.ul.x+1,
        off,false,true);

      mask.fill(mask2,0,0,
        MaxInt32,MaxInt32,tmpBoundary.ul.y,tmpBoundary.ul.x);
    }

    return true;
  }

  void borderPoints::invert() {
    borderPoints tmp(*this);
    borderPoints::iterator it;
    clear();
    for (it=tmp.begin();it!=tmp.end();++it) {
      push_front(*it);
    }
  }

  bool borderPoints::isConsistent() const {
    if (size() == 0 || size() == 1)
      return true;
    borderPoints::const_iterator it = begin();
    lti::point p;
    p = *it - back();
    for (;;) {
      if (p.x < -1 || p.x > 1 || p.y < -1 || p.y > 1 || (p.x == 0 && p.y == 0))
        return false;
      p = *it;
      if (++it == end())
        return true; // no inconsistency detected
      p = *it - p;
    }
  }

  // -----------------------------------------------------------------
  // AREA POINTS
  // -----------------------------------------------------------------

  areaPoints::areaPoints() : pointList() {
  }

  areaPoints::areaPoints(const areaPoints& other) : pointList() {
    copy(other);
  }

  areaPoints::~areaPoints() {
  }

  areaPoints& areaPoints::castFrom(const ioPoints& theIOPoints) {

    pointList::const_iterator itc;

    clear();
    point in,out;
    int x;
    itc = theIOPoints.begin();
    while (itc!=theIOPoints.end()) {
      // get in and out
      in = *itc;
      itc++;
      if (itc==theIOPoints.end()) {
        // no output point? -> there must be an error!
        return *this;
      }
      out = *itc;
      itc++;
      if (out.y!=in.y) {
        // in and out must lie in the same line! -> error
        return *this;
      }
      for (x=in.x;x<=out.x;x++) {
        push_back(point(x,in.y));
      }
    }

    return *this;
  }

  areaPoints& areaPoints::castFrom(const borderPoints& theBorderPoints) {
    // draws the borderPoints in a channel and generates areaPoints
    // from this channel

    // this function removes the offset from the points so you can cast
    // negativ points
    borderPoints bp; //non const borderPoints for calculate boundary

    pointList::iterator it,ite;
    pointList::const_iterator itc,itce;

    //calculates the real size of the border without offset
    rectangle maxRect = theBorderPoints.computeBoundary();

    //creates a channel8 as big as the borderPoints
    channel8 mask;

    //remove the offset and copy the points to new list
    for (itc=theBorderPoints.begin(),itce=theBorderPoints.end();
         itc!=itce;
         itc++) {
      bp.push_back((*itc)-maxRect.ul);
    }

    //draw the points in a channel and get the areaPoints from the channel
    bp.setBoundary(rectangle(point(0,0),maxRect.br-maxRect.ul));
    bp.generateMask(mask,false);

    getFromMask(mask);
    //add the offset to the areaPoints
    for (it=begin(),ite=end();it!=ite;it++) {
      (*it)+=maxRect.ul;
    }

    return *this;
  }

  areaPoints& areaPoints::castFrom(const polygonPoints& thePolygonPoints){
    // this function draws the polygon into a channel8 and extracts
    // the areaPoints with the getFromMask function of the
    // areaPoints
    areaPoints bp;
    draw<channel8::value_type> drawer;
    polygonPoints::iterator it;
    pointList::const_iterator itc,ite;
    rectangle maxRect;

    // a non const polygon is need
    polygonPoints pPs;

    // calculates the real size of the polygon without offset
    maxRect = thePolygonPoints.computeBoundary();

    // creates a channel8 as big as the polygon
    channel8 mask(maxRect.getDimensions(),lti::ubyte(0));

    //remove the offset
    for (itc=thePolygonPoints.begin();itc!=thePolygonPoints.end();itc++) {
      pPs.push_back((*itc) - maxRect.ul);
    }

    drawer.use(mask);
    drawer.setColor(255);
    drawer.set(*pPs.begin());
    //connect the polygonPoints with lines
    for (it=pPs.begin();it!=pPs.end();it++) {
      drawer.lineTo((*it));
    }//close the polygon
    drawer.lineTo(*pPs.begin());

    //extract the object as areaPointList
    std::list<areaPoints> objectList;
    objectsFromMask extractor;
    objectsFromMask::parameters extractorParams;
    extractorParams.level = 0;
    extractorParams.meltHoles = true;
    extractor.setParameters(extractorParams);
    extractor.apply(mask,objectList);
    // if more than one object in the channel -> abort
    if (objectList.size() == 1) {
      itc = (*objectList.begin()).begin();
      ite = (*objectList.begin()).end();
      clear();
      // add offset to the areaPoints, while copying
      for (;itc!=ite;itc++) {
        push_back((*itc) + maxRect.ul);
      }
    }
    else {
      clear();
    }

    return *this;
  }


  bool areaPoints::getFromMask(const channel8& mask) {
    point p;

    clear();

    static const ubyte off = 0;

    for (p.y=0;p.y<mask.rows();p.y++) {
      for (p.x=0;p.x<mask.columns();p.x++) {
        if (mask.at(p)!=off) {
          push_back(p);
        }
      }
    }
    return true;
  }

  bool areaPoints::generateMask(channel8& mask,
                                const bool computeBoundary,
                                const bool exactBoundaryDimensions,
                                const bool keepMaskData) const {

    static const ubyte off = 0;
    static const ubyte on  = 255;

    if (empty()) {
      return true;
    }

    rectangle maxRect = 
      computeBoundary ? this->computeBoundary() : getBoundary();

    if (exactBoundaryDimensions) {

      mask.resize(
        maxRect.br.y-maxRect.ul.y+1,
        maxRect.br.x-maxRect.ul.x+1,
        off,false,true);

      const_iterator it;
      if ((maxRect.ul.x < 0) || (maxRect.ul.y < 0)) {
        for (it=begin();it!=end();++it) {
          if (((*it).x >= 0) && ((*it).y >= 0)) {
            mask.at(*it-maxRect.ul) = on;
          }
        }
      } else {
        for (it=begin();it!=end();++it) {
          mask.at(*it-maxRect.ul) = on;
        }
      }

    } else {

      mask.resize(max(maxRect.br.y+1,mask.rows()),
                  max(maxRect.br.x+1,mask.columns()),
                  off,keepMaskData,true);

      const_iterator it;
      if ((maxRect.ul.x < 0) || (maxRect.ul.y < 0)) {
        for (it=begin();it!=end();++it) {
          if (((*it).x >= 0) && ((*it).y >= 0)) {
            mask.at(*it) = on;
          }
        }
      } else {
        for (it=begin();it!=end();++it) {
          mask.at(*it) = on;
        }
      }

    }

    return true;
  }

  void areaPoints::sort() {
    if (size() < 2) {
      return;
    }

    thePointList.sort();
  }

  int areaPoints::intersectionArea(const rectangle& rect) const {
    int count = 0;
    const_iterator it;
    for (it = begin(); it != end(); it++)
      if (rect.isInside(*it))
        count++;
    return count;
  }

} // end of namespace lti::
