/*
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 *
 * project ....: lti library
 * file .......: ltiSnakes.cpp
 * classes ....: lti::snake
 * types ......:
 * description.: segmentation algorithm based on snakes.
 *
 * authors ....: the Axiom development team
 * organization: LTI, RWTH Aachen
 * creation ...: 19.08.98
 * revisions ..: $Id: ltiSnake.cpp,v 1.11 2006/09/05 10:31:58 ltilib Exp $
 */

#include "ltiMacroSymbols.h"
#include "ltiSnake.h"
#include "ltiMath.h"

using std::string;

//#define _LTI_DEBUG

#ifdef _LTI_DEBUG
#include "ltiFastViewer.h"
#endif

namespace lti {

  /* ******************************
   * class for snake parameters
   * ******************************/

  snake::parameters::parameters()
    : segmentation::parameters() {
    lfilter = 5;
    variance.copy(frgbPixel(10.f,10.f,10.f));
    average.copy(variance);
    patchPlace.copy(rectangle(180,0,255,30)); // upper right  pixel block
                                              //todo general
    splitting = false;
  }

  snake::parameters::~parameters() {
  }

  // creates a clone of this object
  functor::parameters* snake::parameters::clone() const {
    parameters *p = new parameters;
    p->copy(*this);
    return p;
  }

  // become the other object
  snake::parameters& snake::parameters::copy(const parameters& other) {
#ifndef _LTI_MSC_6
    // for normal  ANSI C++
    segmentation::parameters::copy(other);
#else
    // this doesn't work with MS-VC++ 6.0 (once again... another bug)
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    segmentation::parameters&
      (segmentation::parameters::* p_copy)(const segmentation::parameters&) =
      segmentation::parameters::copy;
    (this->*p_copy)(other);
#endif

    lfilter = other.lfilter;

    average.copy(other.average);
    variance.copy(other.variance);

    splitting = other.splitting;

    return (*this);
  }

  const char* snake::parameters::getTypeName() const {
    return ("snake::parameters");
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool snake::parameters::write(ioHandler& handler,
                                const bool complete) const
# else
    bool snake::parameters::writeMS(ioHandler& handler,
                                    const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"patchPlace",patchPlace);

      lti::write(handler,"lfilter",lfilter);

      lti::write(handler,"average",average);
      lti::write(handler,"variance",variance);
      lti::write(handler,"splitting",splitting);
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
  bool snake::parameters::write(ioHandler& handler,
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
  bool snake::parameters::read(ioHandler& handler,
                               const bool complete)
# else
    bool snake::parameters::readMS(ioHandler& handler,
                                   const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"patchPlace",patchPlace);

      lti::read(handler,"lfilter",lfilter);

      lti::read(handler,"average",average);
      lti::read(handler,"variance",variance);

      lti::read(handler,"splitting",splitting);
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
  bool snake::parameters::read(ioHandler& handler,
                               const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  //-----------------------------------------------------------------------------------
  // lti::snake
  //-----------------------------------------------------------------------------------

  snake::snake() {
    parameters param;     // creates default parameters
    setParameters(param); // set default parameters
    size.copy(point(0,0));
  }

  snake::~snake() {
  }

  // returns the current parameters
  const snake::parameters& snake::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(params))
      throw invalidParametersException(getTypeName());
    return *params;
  }

  // returns the current parameters
  snake::parameters& snake::getParameters() {
    parameters* params = dynamic_cast<parameters*>(&functor::getParameters());
    if (isNull(params))
      throw invalidParametersException(getTypeName());
    return *params;
  }

  //-----------------------------------------------------------------------------------
  // copy the snake
  snake& snake::copy(const snake& other) {
    segmentation::copy(other); // copy parameters and all inherited members...
    snakeField.copy(other.snakeField);
    size = other.size;
    img = other.img;
    anchorPoint.copy(other.anchorPoint);
    return (*this);
  }

  functor* snake::clone() const {
    return (new snake(*this));
  }

  //-----------------------------------------------------------------------------------
  //
  bool snake::apply(const image& imgIn, areaPoints& newAreaPoints) {

    // init img
    img = &imgIn;
    size = point(img->columns(),img->rows());
    // clear snakeField (copy data = false, init data = false)
    element unknown;
    unknown.prev = chainCode::UNKNOWN;
    unknown.next = chainCode::UNKNOWN;
    unknown.fixed = false;
    snakeField.resize(size.y,size.x,unknown,false,true);

    const parameters& param = getParameters();
    point patchSize = param.patchPlace.getDimensions();

    borderPoints preSnake;

    // is there any reference color patch?
    if ((patchSize.x>0) || (patchSize.y>0)) {
      // find the reference color
      initReference(preSnake); // create contour around the reference
      setSnake(preSnake); // set the snake using the created contour
    }

    initObject(preSnake);
    setSnake(preSnake);
    findObjects();

    // get filledsnakes into newAreaPoints!
    if (!getContour(newAreaPoints))
      return false;

    return  true;
  }

  //-----------------------------------------------------------------------
  // sets a snake
  void snake::setSnake(const borderPoints& aSnake) {

    if (aSnake.empty())
      throw(exception("Invalid initialization snake"));

    // initialize snakeField with the data in "firstSnake"
    // for each point in the border-points list
    anchorPoint.push_back(*(aSnake.begin()));
    borderPoints::const_iterator it = aSnake.begin();
    point pnext;
    point pos = (*it);
    it++;
    while (it != aSnake.end()) {
      // find next element ...
      pnext=(*it);
      // and link with the actual element
      snakeField.at(pos).next = chainCode(pos,pnext);
      snakeField.at(pnext).prev = chainCode(pnext,pos);
      pos = pnext;
      it++;
    }
    // link begin and end of the snake
    snakeField.at(pos).next                    = chainCode(pos,*(aSnake.begin()));
    snakeField.at(*(aSnake.begin())).prev = chainCode(*(aSnake.begin()),pos);
  }

  //-----------------------------------------------------------------------------------
  // get the from snake enclosed areas
  bool snake::getContour(areaPoints& newAreaPoints) {

    point p,q;
    lti::borderPoints tempContour;
    lti::areaPoints   tempArea;
    lti::areaPoints::iterator tempit;
    lti::channel8 mask;
    mask.resize(size,0,false,true);
    lti::pointList::iterator it;
    for(it = anchorPoint.begin();it!=anchorPoint.end();it++) {
      tempContour.clear();
      p = *it;
      do {
        tempContour.push_back(p);
        q = p; // flag to check if 'next' returns a valid new point!
        p = snakeField.at(p).next.getNext(p);
      } while ((p!=*it) && (p!=q));

      if (p==q) { // error!! invalid snake!!!
        setStatusString("invalid snake");
        return false;
      }
      tempArea.clear();
      tempArea.castFrom(tempContour);
      for(tempit = tempArea.begin();tempit!=tempArea.end();tempit++)
        mask.at(*tempit) = 1;
    }
    newAreaPoints.getFromMask(mask);
    return true;
  }

  //-----------------------------------------------------------------------------------
  // Set reference snake
  void snake::initReference(borderPoints& colorReference) {

    // alias for parameters
    const parameters& param = getParameters();

    bool fixTop, fixBottom, fixRight, fixLeft;
    fixTop = fixBottom = fixRight = fixLeft = false;

    rectangle patch(param.patchPlace);
    patch.ensureConsistency();

    colorReference.clear();

    // 1. Make sure that the boudaries of the reference patch
    //    are at least a "param.detail" number of pixels away from
    //    the border of the image.

    if (patch.ul.y < param.lfilter) {
      patch.ul.y = param.lfilter;
      fixTop=true;
    }
    if (patch.ul.x < param.lfilter) {
      patch.ul.x = param.lfilter;
      fixLeft=true;
    }
    if (patch.br.y > size.y-1-param.lfilter) {
      patch.br.y = size.y-1-param.lfilter;
      fixBottom=true;
    }
    if (patch.br.x > size.x-1-param.lfilter) {
      patch.br.x = size.x-1-param.lfilter;
      fixRight=true;
    }

    // 2. Initialize contour
    int x,y;
    point p;
    pointList fixedPoints;
    frgbPixel mean = frgbPixel(0,0,0); // mean value of color
    int n = 0;
    // upper border
    for (x=patch.ul.x;x<patch.br.x;x++) {
      p = point(x,patch.ul.y);
      colorReference.push_back(p);
      if (fixTop)
        fixedPoints.push_back(p);
      mean.add(img->at(p));
      n++;
    }
    // right border
    for (y=patch.ul.y;y<patch.br.y;y++) {
      p = point(patch.br.x,y);
      colorReference.push_back(p);
      if (fixRight)
        fixedPoints.push_back(p);
      mean.add(img->at(p));
      n++;
    }
    // bottom border
    for (x=patch.br.x;x>patch.ul.x;x--) {
      p = point(x,patch.br.y);
      colorReference.push_back(p);
      if (fixBottom)
        fixedPoints.push_back(p);
      mean.add(img->at(p));
      n++;
    }
    // left border
    for (y=patch.br.y;y>patch.ul.y;y--) {
      p = point(patch.ul.x,y);
      colorReference.push_back(p);
      if (fixLeft)
        fixedPoints.push_back(p);
      mean.add(img->at(p));
      n++;
    }
    // mean is the mean-color of the elements in the border of the patch
    mean.divide(static_cast<float>(n));

    // set fixed points
    pointList::iterator it;
    for (it = fixedPoints.begin();it != fixedPoints.end();it++)
      snakeField.at(*it).fixed = true;

  } // end of initReference

  //-----------------------------------------------------------------------------------
  // create object snake without the reference patch.
  void snake::initObject(borderPoints& firstSnake) {
    const byte off = 0;
    const byte on  = 1;

    // 1. create a map with the same dimensions as the image
    channel8 theMap(img->rows(),img->columns(),channel8::value_type(off));

    // 2. the snake must have a border to apply the filter in an
    //    efficient way.

    // alias for parameters
    const parameters& param = getParameters();

    rectangle patch(0,0,img->columns(),img->rows());
    patch.ensureConsistency();

    if (patch.ul.y < param.lfilter) {
      patch.ul.y = param.lfilter;
    }
    if (patch.ul.x < param.lfilter) {
      patch.ul.x = param.lfilter;
    }
    if (patch.br.y > size.y-1-param.lfilter) {
      patch.br.y = size.y-1-param.lfilter;
    }
    if (patch.br.x > size.x-1-param.lfilter) {
      patch.br.x = size.x-1-param.lfilter;
    }

    // 3. fill the map with this initial rectangle
    theMap.fill(on,patch);

    // 4. clean the reference patch place
    int x,y;
    for(x = param.patchPlace.ul.x; x<= param.patchPlace.br.x;x++)
      for(y = param.patchPlace.ul.y; y<= param.patchPlace.br.y;y++){
        theMap.at(y,x) = off;
      }

    firstSnake.clear();
    firstSnake.getFromMask(theMap);
  }

  //-----------------------------------------------------------------------------------
  // Starts the snake to shrink
  void snake::findObjects() {

#ifdef _LTI_DEBUG
    static fastViewer view("findObject");
    static channel8 chnl;
#endif
    bool splitting = getParameters().splitting;
    bool changed;
    pointList::iterator actAnchor = anchorPoint.begin();
    point pos,nextPos;

    do {
      changed  = true;
      pos = *actAnchor;  //snake at the beginning
      while( changed ) {
        //if changed -> once again
        changed = false;
        do { // move the snake (one element at the time)
          nextPos = snakeField.at(pos).next.getNext(pos);
          if(pos == *actAnchor)
            *actAnchor = snakeField.at(pos).prev.getNext(pos);
          if ( move(pos) ){
            changed = true;
            pos = nextPos;
          }
          else if (splitting && AreLastTwo(pos,actAnchor) ) {
            changed = true;
          }
          else if (splitting && TryToSplit(pos,actAnchor) ) {
            changed = true;
          }
          else { // pos should not be changed by AreToLast,TryToSplit
            pos = nextPos;
          }
        }while (pos!=*actAnchor); //continue Snake-iterations?
#ifdef _LTI_DEBUG
        // show snake movement
        chnl.castFrom(snakeField);
        view.show(chnl);
#endif
      }
      actAnchor++;
    }while(actAnchor != anchorPoint.end());
  }

  //-----------------------------------------------------------------------------------
  // Delete Snake if it consists of two points
  bool snake::AreLastTwo(point& pos, pointList::iterator& actAnchor) {

    int next = snakeField.at(pos).next.getDirection();
    int prev = snakeField.at(pos).prev.getDirection();

    if (prev == next) { // snake with only two points
      point nextPos(chainCode(next).getNext(pos));
      snakeField.at(    pos).next = chainCode::NOMOVE;
      snakeField.at(    pos).prev = chainCode::NOMOVE;
      snakeField.at(nextPos).next = chainCode::NOMOVE;
      snakeField.at(nextPos).prev = chainCode::NOMOVE;
      actAnchor = anchorPoint.erase(actAnchor);
      if (actAnchor != anchorPoint.begin())
        actAnchor--;
      // the anchor points now on the element,
      // which is before the erased (if possible)
      pos = *actAnchor;
      //pos = snakeField.at(pos).next.getNext(pos);
      return true;
    }
    return false;
  }


  //------------------------------------------------------------------------------
  //
  bool snake::move(point pos) {

    int next = snakeField.at(pos).next.getDirection();
    int prev = snakeField.at(pos).prev.getDirection();

    if (prev == next) { // snake with only two points
      return false;
    }

    int i,j;
    i=prev;
    bool moveable = !snakeField.at(pos).fixed;
    // 1. check if it is posible to move the snake
    while ((i!=next) && moveable) {
      // look for next posible element
      j = (i+1)%8;
      if ((j!=next) && (i%2 == 0))
        j = (j+1)%8;

      if (j != next) {
        moveable = freeAndAllowed(pos,chainCode(j).getNext(pos));
      }
      i=j;
    }

    // 2. move the snake
    if (!moveable) {
      return false;
    }
    else {
      // erase element at pos
      snakeField.at(pos).next = chainCode::NOMOVE;
      snakeField.at(pos).prev = chainCode::NOMOVE;
      i=prev;
      while (i!=next) {
        // look for next posible element
        j = (i+1)%8;
        if ((j!=next) && (i%2 == 0))
          j = (j+1)%8;

        // position of points
        point pj(chainCode(j).getNext(pos));
        point pi(chainCode(i).getNext(pos));

        // actualize links between chain-elements
        snakeField.at(pi).next = chainCode(pi,pj);
        snakeField.at(pj).prev = chainCode(pj,pi);

        i=j; // next point...
      }
      return  true;
    }
  }

  //-----------------------------------------------------------------------------------
  // Split Snake if it has contact to itself
  bool snake::TryToSplit(point& pos, pointList::iterator& actAnchor)
  {
    //  |   |         |   |
    //  p1  p3        p1--p3
    //  |   |    =>
    //  p0  p2        p0--p2
    //  |   |         |   |

    int next = snakeField.at(pos).next.getDirection();
    int prev = snakeField.at(pos).prev.getDirection();
    point p0,p1,p2,p3;
    bool notSplit = true;
    p0 = pos;
    // search for neighbour points which are !=next !=prev
    for(int i = (prev+1)%8; i!= next && notSplit; i = (i+1)%8) {
      p2 = chainCode(i).getNext(p0); // possible
      if(snakeField.at(p2).next.isDirection()) {
        // the 1. foreign neighbour is found
        p1 = chainCode(snakeField.at(p0).next).getNext(p0);
        p3 = chainCode(i).getNext(p1);  // possible
        if( snakeField.at(p3).next.isDirection()) {
          // the 2. foreign neighbour is found
          if( snakeField.at(p3).next.getNext(p3) == p2) {
            // the two new neighbours are also neighbours
            // => Snake can be splitted
            snakeField.at(p0).next = chainCode(p0,p2);
            snakeField.at(p1).prev = chainCode(p1,p3);
            snakeField.at(p2).prev = chainCode(p2,p0);
            snakeField.at(p3).next = chainCode(p3,p1);
            *actAnchor = p0;
            actAnchor = anchorPoint.insert(actAnchor,p3); // actAnchor now on the new insert element
            pos = *actAnchor;
            pos = snakeField.at(pos).next.getNext(pos);
            notSplit = false;
          }
        }
      }
    }
    return !notSplit;
  }

  //-----------------------------------------------------------------------------------
  // check if for the given position the field is free
  bool snake::freeAndAllowed(const point pos, const point p) {
    // p: possible new point ; pos: actual point
    if(  (p.x<0)||(p.y<0)||(p.x>=size.x)||(p.y>=size.y))
      // is a valid point? -> no (not within the image)
      return false;

    if((snakeField.at(p).next.isDirection()))
      // the point belongs already to the snake!
      return false;

    // calculate the movement of pos
    chainCode tmpCC(snakeField.at(pos).prev);
    tmpCC.subtract(snakeField.at(pos).next);
    tmpCC.divide(2);
    tmpCC.add(snakeField.at(pos).next);
    point changeXY(tmpCC.delta());

    // alias for parameters
    const parameters& param = getParameters();
    rgbPixel pixel;
    frgbPixel backMean,
      backVar,
      picMean,
      picVar;
    int i;
    for( i = 0; i< param.lfilter;i++) {
      // apply line filter
      //cout << pos<< (pos+changeXY*i) << p <<endl;
      pixel = img->at(pos+changeXY*i); // Neighbour
      backMean.add(pixel);
      backVar.add(frgbPixel((float)pixel.getRed()*pixel.getRed(),
                            (float)pixel.getGreen()*pixel.getGreen(),
                            (float)pixel.getBlue()*pixel.getBlue()));

      pixel = img->at(p+changeXY*i);   // Actual element in Image
      picMean.add(pixel);
      picVar.add(frgbPixel((float)pixel.getRed()*pixel.getRed(),
                           (float)pixel.getGreen()*pixel.getGreen(),
                           (float)pixel.getBlue()*pixel.getBlue()));
    }

    backMean.divide(static_cast<float>(param.lfilter)); // Average Neighbour
    backVar.divide(static_cast<float>(param.lfilter));  // Variance Neighbour
    backVar.subtract(frgbPixel((float)backMean.getRed()*backMean.getRed(),
                               (float)backMean.getGreen()*backMean.getGreen(),
                               (float)backMean.getBlue()*backMean.getBlue()));

    picMean.divide(static_cast<float>(param.lfilter)); // Average
    picVar.divide(static_cast<float>(param.lfilter));  // Variance
    picVar.subtract(frgbPixel((float)picMean.getRed()*picMean.getRed(),
                              (float)picMean.getGreen()*picMean.getGreen(),
                              (float)picMean.getBlue()*picMean.getBlue()));

    if((abs(backMean.red-picMean.red)     <= param.average.red)   &&
       (abs(backMean.green-picMean.green) <= param.average.green) &&
       (abs(backMean.blue-picMean.blue)   <= param.average.blue)  &&
       (abs(backVar.red-picVar.red)       <= param.variance.red)  &&
       (abs(backVar.green-picVar.green)   <= param.variance.green)&&
       (abs(backVar.blue-picVar.blue)     <= param.variance.blue))
      // Are all values below the threshold?
      return true;

    for(i=1;i<param.lfilter;i++) {
      //Punktuelles Maximum?
      if(snakeField.at(p-changeXY*i).next.isDirection())
        return true;
    }

    return false;
  }

  //--------------------------------------------------------------------------
  // read the snake element from the given ioHandler.  The
  // complete flag will be ignored
  bool snake::element::read(ioHandler& handler,const bool complete) {

    bool b = true;

    if (complete) {
      b = handler.readBegin();
    }

    b = b && lti::read(handler,"next",next);
    b = b && lti::read(handler,"prev",prev);
    b = b && lti::read(handler,"fixed",fixed);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

  //----------------------------------------------------------------------
  /*
   * write the snake element in the given ioHandler.
   * The complete flag indicates if the enclosing begin and end
   * should be also be written or not
   */
  bool snake::element::write(ioHandler& handler,const bool complete) const {

    bool b = true;

    if (complete) {
      b = handler.writeBegin();
    }

    b = b && lti::write(handler,"next",next);
    b = b && lti::write(handler,"prev",prev);
    b = b && lti::write(handler,"fixed",fixed);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }


}
