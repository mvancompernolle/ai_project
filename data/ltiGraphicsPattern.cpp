/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiGraphicsPattern.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 05.02.2003
 * revisions ..: $Id: ltiGraphicsPattern.cpp,v 1.9 2006/02/08 11:13:20 ltilib Exp $
 */

#ifdef _LTI_MSC_6
// disable warning for vector<bool> and matrix<bool>
#pragma warning(disable:4800)
#endif

#include "ltiObject.h"
#include "ltiMath.h"
#include "ltiGraphicsPattern.h"
#include "ltiLispStreamHandler.h"
#include "ltiImage.h"
#include "ltiDraw.h"
#include "ltiContour.h"

namespace lti {
  // implementation of functions

  // -------------------------------------------------------------------
  // graphicsPattern
  // -------------------------------------------------------------------

  // default constructor

  graphicsPattern::graphicsPattern() {
  }

  // destructor
  graphicsPattern::~graphicsPattern(){
  }

  // creates a clone of this basicObject

  object* graphicsPattern::clone() const {
    return 0;
  }

  // read the vector from the given ioHandler.  The complete flag indicates
  // whether the enclosing begin and end should also be read

  bool graphicsPattern::read(ioHandler& handler,const bool complete) {

    bool b = true;
    return b;
  };

  // write the vector in the given ioHandler.  The complete flag indicates
  // if the enclosing begin and end should be also be written or not

  bool graphicsPattern::write(ioHandler& handler,const bool complete) const {

    bool b=true;

    return b;
  };



  // ---------------------------------------------------------------------
  // linePattern
  // ---------------------------------------------------------------------

  linePattern::linePattern()
    : graphicsPattern() {
    clear();
  }

  linePattern::linePattern(const linePattern& other)
    : graphicsPattern() {
    copy(other);
  }



  linePattern::linePattern(const ivector& d, int o) {
    dash=d;
    offset=o;
    length=dash.sumOfElements();
    mask.resize(length);
    mask.fill(false);
    genericVector<bool> tmp(length);
    // first, ignore offset
    int k=0;
    int i=0;
    for (i=0; i<dash.size(); i+=2) {
      for (int j=0; j<dash.at(i); ++j) {
        tmp.at(k++)=true;
      }
      if (i < dash.size()-1) {
        for (int j=0; j<dash.at(i+1); ++j) {
          tmp.at(k++)=false;
        }
      }
    }
    // now, shift vector by offset to left
    for (i=0; i<tmp.size(); ++i) {
      mask.at(i)=tmp.at((i+offset)%length);
    }
  }


  linePattern::~linePattern() {
    // do nothing
  }

  /*
   * Returns the bounding box of this pattern.
   */
  rectangle linePattern::getBoundingBox() const {
    rectangle result(0,0,length,0);
    return result;
  }

  /*
   * write the pattern to the given ioHandler
   */
  bool linePattern::write(ioHandler& handler,const bool complete) const {
    bool b=true;
    if (complete) {
      b=handler.writeBegin();
    }
    b=b && lti::write(handler, "dash", dash);
    b=b && lti::write(handler, "mask", mask);
    b=b && lti::write(handler, "length", length);
    b=b && lti::write(handler, "offset", offset);
    if (complete) {
      b=b && handler.writeEnd();
    }

    return b;
  }

    /*
     * read the pattern from the given ioHandler
     */
  bool linePattern::read(ioHandler& handler,const bool complete) {
    bool b=true;
    if (complete) {
      b=handler.readBegin();
    }
    b=b && lti::read(handler, "dash", dash);
    b=b && lti::read(handler, "mask", mask);
    b=b && lti::read(handler, "length", length);
    b=b && lti::write(handler, "offset", offset);
    if (complete) {
      b=b && handler.readEnd();
    }

    return b;

  }

  linePattern* linePattern::createDashed(int d) {
    ivector tmp(2);
    tmp.at(0)=d;
    tmp.at(1)=d;
    return new linePattern(tmp,0);
  }

  object* linePattern::clone() const {
    return new linePattern(*this);
  }

  void linePattern::clear() {
    mask.resize(2);
    mask.at(0)=true;
    mask.at(1)=true;
    dash.resize(1);
    dash.at(0)=2;
    offset=0;
    length=2;
  }

  linePattern& linePattern::copy(const linePattern& other) {
    mask=other.mask;
    dash=other.dash;
    offset=other.offset;
    length=other.length;
    return *this;
  }

  int linePattern::compareTo(const graphicsPattern& other) const {
    const linePattern* tmp=dynamic_cast<const linePattern*>(&other);
    if (length < tmp->length) return -1;
    if (length > tmp->length) return 1;

    // same length; look at the offset
    if (offset < tmp->offset) return -1;
    if (offset > tmp->offset) return 1;

    // same length& offset; so sort according to dash vector
    ivector::const_iterator i1=dash.begin();
    ivector::const_iterator i2=tmp->dash.begin();
    while (i1 != dash.end()) {
      if (*i1 < *i2) return -1;
      if (*i1 > *i2) return 1;
      i1++;
      i2++;
    }
    // if we get here, all things are equal
    return 0;
  }


  // ---------------------------------------------------------------------
  // fillPattern
  // ---------------------------------------------------------------------

  fillPattern::fillPattern()
    : graphicsPattern() {
    clear();
  }

  fillPattern::~fillPattern() {
    // delete the mask object list
    clear();
  }

  fillPattern::fillPattern(const fillPattern& other)
    : graphicsPattern() {
    copy(other);
  }

  fillPattern::fillPattern(const basicObject& base, 
                           contourType type,
                           float scale)
    : graphicsPattern() {
    // BUG here: nothing is done

    // strategy: convert base to <type>
    // at first, only polygon points
    clear();
    internalObject* poly=new internalObject();
    poly->castFrom(base);
    poly->updateBoundary();
    objects.push_back(poly);
    updateMask();
  }

  void fillPattern::add(const basicObject& o) {
    internalObject* poly=new internalObject();
    poly->castFrom(o);
    poly->updateBoundary();
    objects.push_back(poly);
    updateMask();
  }


  /*
   * updates mask and bounding box
   */
  void fillPattern::updateMask() {
    // first, update bounding box.
    bb=rectangle(0,0,0,0);
    objectList::const_iterator i;
    for (i=objects.begin(); i != objects.end(); ++i) {
      // the boundary of each object is computed when it is entered
      // into the list, so we can just get it here
      rectangle tmp;
      tmp.castFrom((*i)->getBoundary());
      bb.join(tmp);
    }

    dim=bb.getDimensions();
    pixeldim=dim;
    pixeldim.x--;
    pixeldim.y--;

    channel8 tmp(dim.y,dim.x,ubyte(0));
    draw<ubyte> d;
    d.use(tmp);
    d.setColor(1);

    for (i=objects.begin(); i != objects.end(); ++i) {
      polygonPoints p;
      for (internalObject::const_iterator j=(*i)->begin(); 
           j != (*i)->end();
           ++j) {
        point p2;
        p2.castFrom(*j);
        p.push_back(p2);
      }
      borderPoints b;
      b.castFrom(p);
      areaPoints a;
      // create an area object
      a.castFrom(b);
      // draw it
      d.set(a);
    }
    // now we have a ubyte mask, use it for the bool mask
    setMask(tmp);
  }


  object* fillPattern::clone() const {
    return new fillPattern(*this);
  }


  void fillPattern::clear() {
    std::list<const basicObject*>::iterator i;
    for (i=objects.begin(); i != objects.end(); ++i) {
      delete *i;
    }
    objects.clear();
    mask.clear();
    bb=rectangle(0,0,0,0);
    dim=point(0,0);
    pixeldim=point(0,0);
  }

  fillPattern& fillPattern::copy(const fillPattern& other) {

    bb=other.bb;
    dim=other.dim;
    pixeldim=other.pixeldim;
    objectList::iterator j;
    for (j=objects.begin(); j != objects.end(); ++j) {
      delete *j;
    }
    objects.clear();
    for (objectList::const_iterator i=other.objects.begin();
         i != other.objects.end(); ++i) {
      basicObject *tmp=dynamic_cast<basicObject*>((*i)->clone());
      objects.push_back(tmp);
    }
    mask=other.mask;

    return *this;
  }

  int fillPattern::compareTo(const graphicsPattern& other) const {
    const fillPattern* tmp=dynamic_cast<const fillPattern*>(&other);
    if (dim.x < tmp->dim.x) return -1;
    if (dim.x > tmp->dim.x) return 1;

    // same width; look at the height
    if (dim.y < tmp->dim.y) return -1;
    if (dim.y > tmp->dim.y) return 1;

    // same dimension; so now look at the number of basic objects
    if (objects.size() < tmp->objects.size()) return -1;
    if (objects.size() > tmp->objects.size()) return 1;

    // now everything else being equal, compare the number
    // of ones...
    int s1=0,s2=0;
    matrix<ubyte>::const_iterator i1,i2;
    for (i1=mask.begin(); i1 != mask.end(); ++i1) {
      if ((*i1)!=0) {
        s1++;
      }
    }
    for (i2=tmp->mask.begin(); i2 != tmp->mask.end(); ++i2) {
      if ((*i2)!=0) {
        s2++;
      }
    }
    if (s1 < s2) return -1;
    if (s1 > s2) return 1;

    // if we get here, all things yet are equal
    // now, this is rather arbitrary: simply do a lexicographic
    // comparison on the mask
    i1=mask.begin();
    i2=tmp->mask.begin();
    while (i1 != mask.end()) {
      if (((*i1)==0) && ((*i2)!=0)) return -1;
      if (((*i1)!=0) && ((*i2)==0)) return 1;
      i1++;
      i2++;
    }
    // if we ever get here, both objects are equal
    return 0;
  }

  rectangle fillPattern::getBoundingBox() const {
    return bb;
  }

  template <class T>
  void fillPattern::drawObject(matrix<T>& pane, const internalObject& p) {
    // draws a filled polygon into the given pane.
    for (int y=0; y<pane.rows(); ++y) {
      // step 1: find intersections of the polygon with the scan line
      std::list<float> tx,ty;
      internalObject::const_iterator i=p.begin();
      while (i != p.end()) {


        i++;
      }
    }
  }


  // read the vector from the given ioHandler.  The complete flag indicates
  // whether the enclosing begin and end should also be read

  bool fillPattern::read(ioHandler& handler,const bool complete) {

    bool b = true;

    if (complete) {
      b=handler.readBegin();
    }
    clear();
    b=b && lti::read(handler, "bbox",bb);
    b=b && lti::read(handler, "dim",dim);
    b=b && lti::read(handler, "pixdim",pixeldim);
    b=b && handler.readBegin();
    std::string tmp;
    b=b && lti::read(handler, tmp);
    if (tmp != "objects") {
      b=false;
    }
    b=b && handler.readBegin();
    do {
      internalObject *tmp=new internalObject();
      b=b && tmp->read(handler);
      objects.push_back(tmp);
    } while (!handler.tryEnd());
    b=b && handler.readEnd();

    b=b && lti::read(handler, "mask",mask);

    if (complete) {
      b=b && handler.readEnd();
    }

    return b;
  };

  // write the vector in the given ioHandler.  The complete flag indicates
  // if the enclosing begin and end should be also be written or not

  bool fillPattern::write(ioHandler& handler,const bool complete) const {

    bool b=true;

    if (complete) {
      b=handler.writeBegin();
    }
    b=b && lti::write(handler, "bbox",bb);
    b=b && lti::write(handler, "dim",dim);
    b=b && lti::write(handler, "pixdim",pixeldim);
    b=b && handler.writeBegin();
    b=b && lti::write(handler, "objects");
    b=b && handler.writeBegin();
    for (objectList::const_iterator i=objects.begin();
         i != objects.end(); ++i) {
      b=b && (*i)->write(handler);
    }
    b=b && handler.writeEnd();
    b=b && handler.writeEnd();

    b=b && lti::write(handler, "mask",mask);

    if (complete) {
      b=b && handler.writeEnd();
    }

    return b;
  };

  fillPattern* fillPattern::createHatching(int dx, int dy, int w) {
    internalObject p1,p2;

    float ystep=1.0f;
    float yoff=0.0f;

    if (dx*dy < 0) {
      ystep=-1.0f;
      yoff=static_cast<float>(lti::abs(dy));
    }
    dx=lti::abs(dx);
    dy=lti::abs(dy);

    // first, create the upper right part
    p1.push_back(pointType(dx/2.0f,yoff));
    p1.push_back(pointType(dx/2.0f+w,yoff));
    p1.push_back(pointType(static_cast<float>(dx),yoff+ystep*(dy/2.0f-w)));
    p1.push_back(pointType(static_cast<float>(dx),yoff+ystep*dy/2.0f));
    // now, create the lower left part
    fillPattern* result=new fillPattern(p1,polygon);
    p2.push_back(pointType(0.0f,yoff+ystep*(dy/2.0f-w)));
    p2.push_back(pointType(dx/2.0f+w,yoff+ystep*dy));
    p2.push_back(pointType(dx/2.0f,yoff+ystep*dy));
    p2.push_back(pointType(0.0f,yoff+ystep*dy/2.0f));
    result->add(p2);
    // now we a a valid vector format, which is unfortunately
    // invalid for pixel-based drawing. Therefore; compute the
    // mask explicitly
    if (yoff > 0) {
      yoff--;
    }
    matrix<ubyte> mask(dy,dx,ubyte(0));
    if (dx >= dy) {
      // delta between lines
      int d=dx/dy;
      int y;
      for (y=0; y<dy/2; ++y) {
        for (int x=dx/2+d*y; x<dx/2+d*y+d*w; ++x) {
          mask.at(static_cast<int>(yoff+ystep*y),x%dx)=1;
        }
      }
      for (y=dy/2; y<dy; ++y) {
        for (int x=d*(y-dy/2); x<d*(y-dy/2)+d*w; ++x) {
          mask.at(static_cast<int>(yoff+ystep*y),x)=1;
        }
      }
    } else {
      // delta between lines
      int d=dy/dx;
      int x;
      for (x=0; x<dx/2; ++x) {
        for (int y=dy/2+d*x; y<dy/2+d*x+d*w; ++y) {
          mask.at(static_cast<int>(yoff+ystep*(y%dy)),x)=1;
        }
      }
      for (x=dx/2; x<dx; ++x) {
        for (int y=d*(x-dx/2); y<d*(x-dx/2)+d*w; ++y) {
          mask.at(static_cast<int>(yoff+ystep*y),x)=1;
        }
      }
    }
    result->setMask(mask);
    return result;
  }


  fillPattern* fillPattern::createCrossHatching(int dx, int dy, int w) {
    fillPattern* fp1=createHatching(abs(dx),abs(dy),w);
    fillPattern* fp2=createHatching(abs(dx),-abs(dy),w);
    channel8 m1,m2;
    m1.castFrom(fp1->mask);
    m2.castFrom(fp2->mask);
    m1.add(m2);
    for (objectList::const_iterator i=fp2->objects.begin();
         i != fp2->objects.end(); ++i) {
      fp1->add(*(*i));
    }
    fp1->setMask(m1);

    delete fp2;

    return fp1;
  }

  // top-level functions

  bool read(ioHandler& handler,graphicsPattern& plst,
            const bool complete) {
    return plst.read(handler,complete);
  }

  bool write(ioHandler& handler, const graphicsPattern& plst,
             const bool complete) {
    return plst.write(handler,complete);
  }

} // namespace lti

namespace std {
  std::ostream& operator<<(std::ostream& s, const lti::graphicsPattern& v) {
    lti::lispStreamHandler lsp(s);

    lti::write(lsp,v);

    return s;
  }

}

/*
// ASCII output of a basicObject to a std stream.

  std::ostream& operator<<(std::ostream& s,const lti::graphicsPattern& v) {

     lti::graphicsPattern::const_iterator it;

     for (it=v.begin();it!=v.end();it++) {
       s << "(" << (*it).x << "," << (*it).y << ") ";
     }

    return s;
  }
*/
