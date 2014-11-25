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
 * file .......: ltiObjectsFromMask.cpp
 * authors ....: Pablo Alvarado, Ulle Canzler
 * organization: LTI, RWTH Aachen
 * creation ...: 5.10.2000
 * revisions ..: $Id: ltiObjectsFromMask.cpp,v 1.11 2006/09/05 10:23:30 ltilib Exp $
 */

#include "ltiObjectsFromMask.h"
#include "ltiSortExpensive.h"

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

#ifdef _LTI_DEBUG
#include "ltiSTLIoInterface.h"
#include "ltiLispStreamHandler.h"
#include "ltiPNGFunctor.h"
#include "ltiColors.h"
#include "ltiDraw.h"
#include "ltiFilledUpsampling.h"
#include <unistd.h>
#include "ltiViewer.h"
#endif


namespace lti {
  // static members and attributes

  const int objectsFromMask::markarray[8][8]=  {{ 1, 3, 3, 7, 7,-1,-1, 1}, // 0
                                                { 1, 3, 3, 7, 7,15,-1, 1}, // 1
                                                {-1, 2, 2, 6, 6,14,14,-1}, // 2
                                                {-1, 2, 2, 6, 6,14,14,15}, // 3
                                                {13,-1,-1, 4, 4,12,12,13}, // 4
                                                {13,15,-1, 4, 4,12,12,13}, // 5
                                                { 9,11,11,-1,-1, 8, 8, 9}, // 6
                                                { 9,11,11,15,-1, 8, 8, 9}};// 7

  const int objectsFromMask::areaarray[8][8]= {{ 0, 0, 0, 2, 2,-1,-1, 0}, // 0
                                               { 0, 0, 0, 2, 2, 2,-1, 0}, // 1
                                               {-1,-1,-1, 1, 1, 1, 1,-1}, // 2
                                               {-1,-1,-1, 1, 1, 1, 1, 2}, // 3
                                               { 2,-1,-1, 1, 1, 1, 1, 2}, // 4
                                               { 2, 2,-1, 1, 1, 1, 1, 2}, // 5
                                               { 0, 0, 0,-1,-1,-1,-1, 0}, // 6
                                               { 0, 0, 0, 2,-1,-1,-1, 0}};// 7
  const int objectsFromMask::xsteparray[8]=  { 1, 1, 0,-1,-1,-1, 0, 1};
  const int objectsFromMask::ysteparray[8]=  { 0, 1, 1, 1, 0,-1,-1,-1};


#ifdef _LTI_DEBUG

  static inline void drawArrow(int x, int y, draw<ubyte> &d, ubyte col,
                               int angle) {
    static const int r=2;
    static const int yp[5]={0, 0, r, -r, 0};
    static const int xp[5]={-r, r, 0, 0, r};
    float a=float(-angle)/180*Pi;
    d.setColor(col);
    int xt[5];
    int yt[5];
    for (int i=0; i<5; i++) {
      xt[i]=iround(xp[i]*cos(a)-yp[i]*sin(a)+x);
      yt[i]=iround(xp[i]*sin(a)+yp[i]*cos(a)+y);
    }
    d.set(xt[0],yt[0]);
    for (int i=1; i<5; i++) {
      d.lineTo(xt[i],yt[i]);
    }
  }

  static inline void drawCanzlerElement(int x, int y, draw<ubyte> &d, ubyte ch) {
    static const int r=3;
    static const int xp[4]={-r, -r, r, r};
    static const int yp[4]={-r, r, -r, r};
    // make sure these are the same as the corresponding members!
    static const int Top=1;
    static const int Right=2;
    static const int Bottom=4;
    static const int Left=8;

    d.setColor(ch);
    if (ch == 0) {
      d.rectangle(x-r+1,y-r+1,x+r-1,y+r-1,true);
    } else {
      if ((ch&Bottom) != 0) {
        d.line(x+xp[1],y+yp[1],x+xp[3],y+yp[3]);
      }
      if ((ch&Top) != 0) {
        d.line(x+xp[0],y+yp[0],x+xp[2],y+yp[2]);
      }
      if ((ch&Left) != 0) {
        d.line(x+xp[0],y+yp[0],x+xp[1],y+yp[1]);
      }
      if ((ch&Right) != 0) {
        d.line(x+xp[2],y+yp[2],x+xp[3],y+yp[3]);
      }
    }
  }


  static void saveDirImage(const channel8& cc, const char* name,
                           int sx, int sy) {
    savePNG saver;
    lti::palette cols(256);
    cols.fill(gray(180));
    // make sure these are the same as the corresponding members!
    static const int E = 0;
    static const int NE = 7;
    static const int N = 6;
    static const int NW = 5;
    static const int W = 4;
    static const int SW = 3;
    static const int S = 2;
    static const int SE = 1;
    cols[E]=Green;
    cols[W]=Grey50;
    cols[N]=Red;
    cols[S]=Blue;
    cols[NE]=Yellow;
    cols[NW]=DarkRed;
    cols[SE]=Cyan;
    cols[SW]=DarkBlue;
    cols[8]=gray(225);
    cols[255]=BrightRed;

    draw<ubyte> drawer;
    static const int size=101;
    static const int fsize=4*size;
    static const int hp1=60;
    static const int hp2=100;
    channel8 tmp(cc,sy-size/2,sy+size/2,sx-size/2,sx+size/2);

    filledUpsampling sampler;
    sampler.apply(tmp);
    drawer.use(tmp);
    drawer.setColor(8);
    drawer.rectangle(5,5,17,103,true);
    drawArrow(12,12,drawer,E,0);
    drawArrow(12,24,drawer,W,180);
    drawArrow(12,36,drawer,N,90);
    drawArrow(12,48,drawer,S,-90);
    drawArrow(12,60,drawer,NE,45);
    drawArrow(12,72,drawer,NW,135);
    drawArrow(12,84,drawer,SE,-45);
    drawArrow(12,96,drawer,SW,-135);

    sampler.apply(tmp);
    drawer.setColor(255);
    drawer.line(fsize/2,hp1,fsize/2,hp2);
    drawer.line(fsize/2,fsize-hp1,fsize/2,fsize-hp2);
    drawer.line(hp1,fsize/2,hp2,fsize/2);
    drawer.line(fsize-hp1,fsize/2,fsize-hp2,fsize/2);

    savePNG::parameters p=saver.getParameters();
    p.filename=name;
    saver.setParameters(p);
    saver.apply(tmp,cols);
  }

  static void saveCanzlerImage(const channel8& cc, const char* name,
                               int sx, int sy) {
    savePNG saver;
    lti::palette cols(256);
    cols.fill(gray(225));
    // make sure these are the same as the corresponding members!
    static const int Nothing=0;
    static const int All=15;
    static const int Top=1;
    static const int Right=2;
    static const int Bottom=4;
    static const int Left=8;
    cols[Nothing]=White;
    cols[Top]=Red;
    cols[Right]=Green;
    cols[Bottom]=Blue;
    cols[Left]=Grey50;
    cols[Top|Right]=Yellow;
    cols[Top|Bottom]=Magenta;
    cols[Top|Left]=DarkRed;
    cols[Right|Bottom]=Cyan;
    cols[Right|Left]=DarkGreen;
    cols[Bottom|Left]=DarkBlue;
    cols[Top|Right|Bottom]=Violet;
    cols[Top|Right|Left]=DarkYellow;
    cols[Top|Bottom|Left]=DarkMagenta;
    cols[Right|Bottom|Left]=DarkCyan;
    cols[All]=Black;
    cols[All+1]=BrightRed;

    draw<ubyte> drawer;
    static const int size=101;
    static const int fsize=4*size;
    static const int hp1=60;
    static const int hp2=100;
    channel8 tmp(cc,sy-size/2,sy+size/2,sx-size/2,sx+size/2);

    filledUpsampling sampler;
    sampler.apply(tmp);
    drawer.use(tmp);
    drawer.setColor(All+2);
    drawer.rectangle(5,5,17,All*12+7,true);
    for (ubyte i=0; i<=All; i++) {
      drawCanzlerElement(12,12+i*12,drawer,i);
    }

    sampler.apply(tmp);
    drawer.setColor(All+1);
    drawer.line(fsize/2,hp1,fsize/2,hp2);
    drawer.line(fsize/2,fsize-hp1,fsize/2,fsize-hp2);
    drawer.line(hp1,fsize/2,hp2,fsize/2);
    drawer.line(fsize-hp1,fsize/2,fsize-hp2,fsize/2);

    savePNG::parameters p=saver.getParameters();
    p.filename=name;
    saver.setParameters(p);
    saver.apply(tmp,cols);
  }

#endif

  // --------------------------------------------------
  // objectsFromMask::parameters
  // --------------------------------------------------

  // default constructor
  objectsFromMask::parameters::parameters()
    : segmentation::parameters() {

    assumeLabeledMask = false;
    threshold = int(1);
    minSize = int(1);
    level = int (-1);
    meltHoles=bool (false);
    sortObjects=bool (false);
    sortByArea = false;
  }

  // copy constructor
  objectsFromMask::parameters::parameters(const parameters& other)
    : segmentation::parameters()  {
    copy(other);
  }

  // destructor
  objectsFromMask::parameters::~parameters() {
  }

  // get type name
  const char* objectsFromMask::parameters::getTypeName() const {
    return "objectsFromMask::parameters";
  }

  // copy member

  objectsFromMask::parameters&
    objectsFromMask::parameters::copy(const parameters& other) {
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

    assumeLabeledMask = other.assumeLabeledMask;
    threshold = other.threshold;
    minSize = other.minSize;
    level = other.level;
    meltHoles = other.meltHoles;
    sortObjects=other.sortObjects;
    sortByArea=other.sortByArea;
    ioSearchAreaList=other.ioSearchAreaList;

    return *this;
  }

  // clone member
  functor::parameters* objectsFromMask::parameters::clone() const {
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
  bool objectsFromMask::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool objectsFromMask::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"assumeLabeledMask",assumeLabeledMask);
      lti::write(handler,"threshold",threshold);
      lti::write(handler,"minSize",minSize);
      lti::write(handler,"level",level);
      lti::write(handler,"meltHoles",meltHoles);
      lti::write(handler,"sortObjects",sortObjects);
      lti::write(handler,"sortByArea",sortByArea);
      lti::write(handler,"ioSearchAreaList",ioSearchAreaList);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && segmentation::parameters::write(handler,false);
# else
    bool (segmentation::parameters::* p_writeMS)(ioHandler&,
                                                 const bool) const =
      segmentation::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool objectsFromMask::parameters::write(ioHandler& handler,
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
  bool objectsFromMask::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool objectsFromMask::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      b=b && lti::read(handler,"assumeLabeledMask",assumeLabeledMask);
      b=b && lti::read(handler,"threshold",threshold);
      b=b && lti::read(handler,"minSize",minSize);
      b=b && lti::read(handler,"level",level);
      b=b && lti::read(handler,"meltHoles",meltHoles);
      b=b && lti::read(handler,"sortObjects",sortObjects);
      b=b && lti::read(handler,"sortByArea",sortByArea);
      b=b && lti::read(handler,"ioSearchAreaList",ioSearchAreaList);
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
  bool objectsFromMask::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // objectsFromMask
  // --------------------------------------------------

  // default constructor
  objectsFromMask::objectsFromMask()
    : segmentation(){
    parameters theParams;
    setParameters(theParams);
    maxThresh = 2147483647; //2^31-1 max of "int"
    geometricFeatures::parameters geomFeatParams;
    geomFeatParams.calcFeatureGroup1 = false;
    geomFeatParams.calcFeatureGroup2 = false;
    geomFeatParams.calcFeatureGroup3 = false;
    geomFeatParams.calcFeatureGroup4 = false;
    featureExtractor.setParameters(geomFeatParams);
  }

  // copy constructor
  objectsFromMask::objectsFromMask(const objectsFromMask& other)
    : segmentation()  {
    copy(other);
  }

  // destructor
  objectsFromMask::~objectsFromMask() {
  }

  // returns the name of this type
  const char* objectsFromMask::getTypeName() const {
    return "objectsFromMask";
  }

  // copy member
  objectsFromMask&
  objectsFromMask::copy(const objectsFromMask& other) {
    segmentation::copy(other);
    featureExtractor = other.featureExtractor;
    return (*this);
  }

  // clone member
  functor* objectsFromMask::clone() const {
    return new objectsFromMask(*this);
  }

  // return parameters
  const objectsFromMask::parameters&
    objectsFromMask::getParameters() const {
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
  bool objectsFromMask::buildTree(const imatrix& src,
                                  ioPoints& ioPointList,
                                  tree<objectStruct>::node& mNode,
                                  const int& level) {

    int result;
    if ((level % 2) == 0) {
      result=detectObjects(src,gParams.threshold,ioPointList,mNode);
    } else {
      result=detectHoles(src,gParams.threshold,ioPointList,mNode);
    }
    if (result < 0) {
      // ERROR!
      return false;
    }

    bool allresult=true;

    if (level < gParams.level) {
      int size=mNode.degree();

      for (int a=0;a < size;a++) {
        objectStruct& objTemp = mNode.child(a).getData();
        if (objTemp.ioPointList.size() > 0) {
          allresult=allresult &&
                    buildTree(src,objTemp.ioPointList,mNode.child(a),level+1);
        }
      }
    }
    return allresult;
  }

  //---------------------------------------------------------------------------

  // Apply Nr 0
  bool objectsFromMask::apply(const channel8& src8,
                              std::list<ioPoints>& lstIOPointLists) {
    maxThresh = 255;
    matrix<int> src;
    src.castFrom(src8);
    return apply(src,lstIOPointLists);
  }

  bool objectsFromMask::apply(const imatrix& src,
                              std::list<ioPoints>& lstIOPointLists) {

    if (src.empty()) {
      setStatusString("Input channel empty");
      return false;
    }

    bCalcIoPoints=true;
    bCalcBorderPoints=false;
    bCalcAreaPoints=false;

    vres=src.rows();                   // y-size of the channel
    hres=src.columns();                // x-size of the channel
    vresm1=vres-1;
    hresm1=hres-1;
    if (!checkParameters()) {
      return false;
    }

    lstIOPointLists.clear();
    objectTree.clear();
    objectTree.pushRoot(m_object);
    tree<objectStruct>::node& pNodeRoot=objectTree.root();

    plstIOPointLists=&lstIOPointLists;

    bool result=buildTree(src,gParams.ioSearchAreaList,pNodeRoot,0);

    int a,numOfLists;
    std::list<ioPoints>::iterator iterLstIoPointLists;
    numOfLists=plstIOPointLists->size();

    iterLstIoPointLists=plstIOPointLists->begin();

    // now remove the small objects
    for (a=0;a<numOfLists;a++) {
      if ((*iterLstIoPointLists).size() < gParams.minSize)
        iterLstIoPointLists=plstIOPointLists->erase(iterLstIoPointLists);
      else iterLstIoPointLists++;
    }

    // and if the user wishes, sort the objects by their size
    if (gParams.sortObjects) {
      if (gParams.sortByArea) {
        lti::sortExpensive<ioPoints,double> aSorter;
        aSorter.apply(lstIOPointLists,this,getArea);
      } else {
        lstIOPointLists.sort();
      }
      lstIOPointLists.reverse();
    }

    return result;
  }

  // Apply Nr 1
  bool objectsFromMask::apply(const channel8& src8,
                              std::list<borderPoints>& lstBorderPointLists) {
    if (src8.empty()) {
      setStatusString("Input channel empty");
      return false;
    }

    maxThresh = 255;
    matrix<int> src;
    src.castFrom(src8);
    return apply(src,lstBorderPointLists);
  }

  bool objectsFromMask::apply(const matrix<int>& src,
                              std::list<borderPoints>& lstBorderPointLists) {

    if (src.empty()) {
      setStatusString("Input channel empty");
      return false;
    }

    bCalcIoPoints=false;
    bCalcBorderPoints=true;
    bCalcAreaPoints=false;

    vres=src.rows();                   // y-size of the channel
    hres=src.columns();                // x-size of the channel
    vresm1=vres-1;
    hresm1=hres-1;

    if (!checkParameters()) {
      return false;
    }

    lstBorderPointLists.clear();
    objectTree.clear();
    objectTree.pushRoot(m_object);
    tree<objectStruct>::node& pNodeRoot=objectTree.root();

    plstBorderPointLists=&lstBorderPointLists;

    bool result=buildTree(src,gParams.ioSearchAreaList,pNodeRoot,0);

    if (gParams.sortObjects) {
      if (gParams.sortByArea) {
        lti::sortExpensive<borderPoints,double> aSorter;
        aSorter.apply(lstBorderPointLists,this,getArea);
      } else {
        lstBorderPointLists.sort();
      }
      lstBorderPointLists.reverse();
    }

    return result;
  };

  // Apply Nr 2
  bool objectsFromMask::apply(const channel8& src8,
                              std::list<areaPoints>& lstAreaPointLists) {
    maxThresh = 255;
    matrix<int> src;
    src.castFrom(src8);
    return apply(src,lstAreaPointLists);
  }

  bool objectsFromMask::apply(const matrix<int>& src,
                              std::list<areaPoints>& lstAreaPointLists) {

    if (src.empty()) {
      setStatusString("Input channel empty");
      return false;
    }

    bCalcIoPoints=true;
    bCalcBorderPoints=false;
    bCalcAreaPoints=false;

    vres=src.rows();                   // y-size of the channel
    hres=src.columns();                // x-size of the channel
    vresm1=vres-1;
    hresm1=hres-1;

    if (!checkParameters()) {
      return false;
    }

    // temporary ioPoints-list
    std::list<ioPoints> lstIOPointLists;
    // temporary area points
    areaPoints areaPointList;

    std::list<ioPoints>::iterator iterIOList;
    lstAreaPointLists.clear();

    plstAreaPointLists=&lstAreaPointLists;
    plstIOPointLists=&lstIOPointLists;

    objectTree.clear();
    objectTree.pushRoot(m_object);
    tree<objectStruct>::node& pNodeRoot=objectTree.root();

    bool result=buildTree(src,gParams.ioSearchAreaList,pNodeRoot,0);


    for (iterIOList = lstIOPointLists.begin();
         iterIOList != lstIOPointLists.end();
         ++iterIOList) {
      areaPointList.castFrom(*iterIOList);
      if (areaPointList.size()>=gParams.minSize) {
        lstAreaPointLists.push_back(areaPointList);
      }
    }

    if (gParams.sortObjects) {
      if (gParams.sortByArea) {
        lti::sortExpensive<areaPoints,double> aSorter;
        aSorter.apply(lstAreaPointLists,this,getArea);
      } else {
        lstAreaPointLists.sort();
      }
      lstAreaPointLists.reverse();
    }

    return result;
  };

  // Apply Nr 3
  bool objectsFromMask::apply(const channel8& src8,
                              std::list<areaPoints>& lstAreaPointLists,
                              matrix<int>& labeledMask) {
    maxThresh = 255;
    matrix<int> src;
    src.castFrom(src8);
    return apply(src,lstAreaPointLists,labeledMask);
  }

  bool objectsFromMask::apply(const matrix<int>& src,
                              std::list<areaPoints>& lstAreaPointLists,
                              matrix<int>& labeledMask) {

    if (!apply(src,lstAreaPointLists)) {
      return false;
    }

    labeledMask.resize(vres,hres,0,false,true);

    std::list<areaPoints>::iterator iterLstAreaPointLists;
    areaPoints::iterator iterAreaPoints,endIterAreaPoints;

    int label = 1;

    for (iterLstAreaPointLists = lstAreaPointLists.begin();
         iterLstAreaPointLists != lstAreaPointLists.end();
         ++iterLstAreaPointLists) {

      for (iterAreaPoints = (*iterLstAreaPointLists).begin(),
             endIterAreaPoints = (*iterLstAreaPointLists).end();
           iterAreaPoints != endIterAreaPoints;
           ++iterAreaPoints) {
        labeledMask.at(*iterAreaPoints) = label;
      }

      label++;
    }

    return true;
  };

  // Apply Nr 4
  bool objectsFromMask::apply(const channel8& src8,
                              tree<objectStruct>& objectTree) {
    maxThresh = 255;
    matrix<int> src;
    src.castFrom(src8);
    return apply(src,objectTree);
  }

  bool objectsFromMask::apply(const matrix<int>& src,
                              tree<objectStruct>& objectTree) {

    if (src.empty()) {
      setStatusString("Input channel empty");
      return false;
    }

    bCalcIoPoints=false;
    bCalcBorderPoints=false;
    bCalcAreaPoints=false;

    vres=src.rows();                   // y-size of the channel
    hres=src.columns();                // x-size of the channel
    vresm1=vres-1;
    hresm1=hres-1;

    if (!checkParameters()) {
      return false;
    }

    objectTree.clear();
    objectTree.pushRoot(m_object);
    tree<objectStruct>::node& pNodeRoot=objectTree.root();

    bool result=buildTree(src,gParams.ioSearchAreaList,pNodeRoot,0);

    // area points need to be casted
    tree<objectStruct>::iterator it;
    for (it = objectTree.begin(); it != objectTree.end(); ++it) {
      (*it).getData().areaPointList.castFrom((*it).getData().ioPointList);
    }

    return result;
  }

  // --------------------------------------------------
  //  Detect Objects -> Extracting Objects from Images
  // --------------------------------------------------
  int objectsFromMask::detectObjects(const imatrix& pImg,
                                     const int& thresh,
                                     lti::ioPoints& lstIOSearchArea,
                                     tree<objectStruct>::node& mNode) {

    if (gParams.assumeLabeledMask) {
      return detectObjectsLabeled(pImg,thresh,lstIOSearchArea,mNode);
    } else {
      return detectObjectsSimple(pImg,thresh,lstIOSearchArea,mNode);
    }
  }


  // --------------------------------------------------
  //  Detect Objects -> Extracting Objects from Images
  // --------------------------------------------------
  int objectsFromMask::detectObjectsSimple(const imatrix& pImg,
                                           const int& thresh,
                                           lti::ioPoints& lstIOSearchArea,
                                           tree<objectStruct>::node& mNode) {

    _lti_debug("--- OBJECT DETECTION LEVEL " << mNode.level() << " ---" <<
               std::endl);

    int x=0, y=0, col, row;   // temp values
    int xa,ya,xe;
    int objnr=0;     // Number of detected objects
    int xstartpos, ystartpos;    // start position
    int  olddir,dir,dirstart;     // directions of walk through the chaincode

    bool found;                  // Cancel flag
    bool ready = false;
    point pntTemp;

    chain.fill(channel8::value_type(0));

    iterIOPoint=lstIOSearchArea.begin();
    iterIOPointEnd=lstIOSearchArea.end();

    xa=(*iterIOPoint).x;
    y=(*iterIOPoint).y;
    iterIOPoint++;
    xe=(*iterIOPoint).x;
    iterIOPoint++;
    x=xa;

    // find first object by scanning the channel row by row and column
    // by column
    do {
      // case 1: the scanned pixel is white
      if (chain.at(y,x) == Nothing) {
        if (pImg.at(y,x) >= thresh) {
          objectStruct tmpObject;
          mNode.appendChild(tmpObject);
          pObject=&mNode.child(mNode.degree()-1).getData();

          // memorize first pixel of the object (left upper corner)
          xstartpos = x;
          ystartpos = y;

          _lti_debug("  object at (" << x << "," << y << ")" << std::endl);

          // calculate start direction for end argue
          dirstart=Invalid;

          // check from which direction the chain comes
          if (y<vresm1) {
            if ((x>0) && (pImg.at(y+1,x-1)>=thresh)) {
              dirstart=NE;
            } else if (pImg.at(y+1,x)>=thresh) {
              dirstart=N;
            } else if (x<hresm1) {
              if (pImg.at(y+1,x+1)>=thresh) {
                dirstart=NW;
              } else if (pImg.at(y,x+1)>=thresh) {
                dirstart=W;
              }
            }
          } else if ((x<hresm1) && (pImg.at(y,x+1)>=thresh)) {
            dirstart=W;
          }

          // object consists of only one pixel? Yes => store and exit
          if (dirstart == Invalid) {
            chain.at(ystartpos,xstartpos)=All;
            pntTemp.x=xstartpos;
            pntTemp.y=ystartpos;
            pObject->borderPointList.push_back(pntTemp);
            pObject->ioPointList.push_back(pntTemp); // in point
            pObject->ioPointList.push_back(pntTemp); // out point
          } else {

            // object consists of more than one point
            dir=dirstart;
            do { // repeat until the procedure reaches the start point
              // look left pixel
              olddir=dir;
              dir = (dir+NW)%8;
              found=false;
              // find next direction
              do {
                dir = (dir+SE)%8;

                col=xsteparray[dir]+x;
                row=ysteparray[dir]+y;
                // is pixel inRegion pixel (defined by threshold)
                if ( (col>=0)&&(row>=0)&&(col<hres)&&(row<vres) ) {
                  if ( (pImg.at(row,col) >= thresh) ) {
                    found=true;
                  }
                }

              } while(!found);

              // mark detected edges, store border and io points
              pntTemp.x=x;
              pntTemp.y=y;

              pObject->borderPointList.push_back(pntTemp);

              if ((markarray[olddir][dir] & Right) == Right) {
                // now just insert the point, later on we will sort them...
                pObject->ioPointList.push_back(pntTemp);
              }

              if ((markarray[olddir][dir] & Left) == Left) {
                // now just insert the point, later on we will sort them...
                pObject->ioPointList.push_back(pntTemp);
              }

              chain.at(y,x)|=markarray[olddir][dir];

              x=col;  // use new x-value
              y=row;  // use new y-value

            } while (x!=xstartpos || y!=ystartpos || dir!=dirstart);

            x=xstartpos;
            y=ystartpos;

            // io points must be sorted here, because even if they are
            // not required for the plstIOPointLists, they are used to
            // indicate the search area in the next level, and they MUST
            // be consistent
            pObject->ioPointList.sort();

          }//else

          // we cannot check the size of the ioPoints, because the user
          // may want to apply the minSize condition to area points
          if (bCalcIoPoints) {
            plstIOPointLists->push_back(pObject->ioPointList);
          }

          if (bCalcBorderPoints &&
              pObject->borderPointList.size() >= gParams.minSize) {
            plstBorderPointLists->push_back(pObject->borderPointList);
          }

          objnr++;
        }
      }

      // case 2: point is left border to an detected object->search
      // for right border
      else if ((chain.at(y,x) & Left) == Left) {
        _lti_debug("  in-point at (" << x << "," << y << ")" << std::endl);
        while((chain.at(y,x) & Right) != Right) x++;
        _lti_debug("  out-point at (" << x << "," << y << ")" << std::endl);
      }

      x++;
      if (x > xe) {
        ready = (iterIOPoint == lstIOSearchArea.end());
        if (!ready) {
          // the io points could already be processed.  In that case,
          // we need to get all already checked regions out of the io
          // points list.
          do {
            ya=(*iterIOPoint).y;
            if (y==ya) {
              // it is not allowed that we get back!
              xa=max(x,(*iterIOPoint).x);
            } else {
              xa=(*iterIOPoint).x;
            }

            iterIOPoint++;
            xe=(*iterIOPoint).x;
            iterIOPoint++;
          } while ((y==ya) && (xe < x));
          x=xa;
          y=ya;

          _lti_debug("Next y = " << y << std::endl);
        }
      }
    } while (!ready);

    // returning the number of detected objects
    return objnr;
  }

  //--------------------------------------------------------------------------

  // --------------------------------------------------
  //  Detect Objects -> Extracting Objects from Images
  // --------------------------------------------------
  int objectsFromMask::detectObjectsLabeled(const imatrix& pImg,
                                            const int& thresh,
                                            lti::ioPoints& lstIOSearchArea,
                                            tree<objectStruct>::node& mNode) {

#ifdef _LTI_DEBUG
    bool saveChain = true;

    _lti_debug("--- OBJECT(L) DETECTION LEVEL " << mNode.level() << " ---\n");

    if (saveChain) {
      char buf[80];
      int pid=getpid();
      savePNG saver;
      channel tmp;
      tmp.castFrom(pImg);
      tmp.divide(pImg.maximum());
      sprintf(buf,"errorimg%05d.png",pid);
      saver.save(buf,tmp);

      sprintf(buf,"iopoints%05d.txt",pid);
      std::ofstream out(buf);
      lispStreamHandler lsh(out);
      lti::write(lsh,"IoPoints",lstIOSearchArea);
      out.close();
    }
#endif

    int x=0, y=0, col, row;      // temp values
    int xa,ya,xe;
    int objnr=0;                 // Number of detected objects
    int xstartpos, ystartpos;    // start position
    int olddir,dir,dirstart;     // directions of walk through the chaincode
    int label;                   // label of a region
    int checkersLabel;           // consistency label
    bool ready = false;
    lti::ubyte canzler;          // canzler code
    lti::ubyte *xptr,*xeptr;     // pointer to a point in mask
    static const ubyte OuterSpace = 0xF0;

    point pntTemp;

    // mark everything as outer space.
    chain.fill(channel8::value_type(OuterSpace));

    // fill the OuterSpace with "Nothing" there, where objects can be expected
    iterIOPoint=lstIOSearchArea.begin();
    iterIOPointEnd=lstIOSearchArea.end();

    _lti_debug("    io area: " << lstIOSearchArea.size());
    while (iterIOPoint != iterIOPointEnd) {
      // the "in"-point
      y = (*iterIOPoint).y;
      xptr = &chain.at(y,(*iterIOPoint).x);
      // the "out"-point
      ++iterIOPoint;
      xeptr = &chain.at(y,(*iterIOPoint).x);
      ++iterIOPoint;
      // fill between in- and out-points
      while (xptr<=xeptr) {
        *xptr++ = Nothing;
      }
    }
    _lti_debug("."  << std::endl);

#ifdef _LTI_DEBUG
    channel8 olddirimg(chain);
    channel8 dirimg(chain);

    static viewer view("1st Chain Mask");
    view.show(chain);

    olddirimg.fill(8);
    dirimg.fill(8);
#endif

    // find first object by scanning the channel row by row and column
    // by column

    iterIOPoint=lstIOSearchArea.begin();
    iterIOPointEnd=lstIOSearchArea.end();

    xa=(*iterIOPoint).x;
    y=(*iterIOPoint).y;
    iterIOPoint++;
    xe=(*iterIOPoint).x;
    iterIOPoint++;
    x=xa;

    // default label uninitialized!
    label = -1;

    _lti_debug("Entering main loop"<< std::endl);

    do {
      // case 1: the scanned pixel is white
      if (chain.at(y,x) == Nothing) {
        if (pImg.at(y,x) >= thresh) {

          objectStruct tmpObject;
          mNode.appendChild(tmpObject);
          pObject=&mNode.child(mNode.degree()-1).getData();

          // memorize first pixel of the object (left upper corner)
          xstartpos = x;
          ystartpos = y;

          // memorize label of region
          label = pImg.at(ystartpos,xstartpos);

          _lti_debug("  label (c1) <- " << static_cast<int>(label));
          _lti_debug(" at (" << x << "," << y << ")" << std::endl);

          // calculate start direction for end argue
          dirstart=Invalid;

          // check from which direction the chain comes
          if (y<vresm1) {
            if ((x>0) && (pImg.at(y+1,x-1)==label) && 
                (chain.at(y+1,x-1) < OuterSpace)) {
              dirstart=NE;
            } else if ((pImg.at(y+1,x)==label) &&  
                       (chain.at(y+1,x) < OuterSpace)) {
              dirstart=N;
            } else if (x<hresm1) {
              if ((pImg.at(y+1,x+1)==label) &&
                  (chain.at(y+1,x+1) < OuterSpace)) {
                dirstart=NW;
              } else if ((pImg.at(y,x+1)==label) &&
                         (chain.at(y,x+1) < OuterSpace)) {
                dirstart=W;
              }
            }
          } else if ((x<hresm1) && (pImg.at(y,x+1)==label) &&
                     (chain.at(y,x+1) < OuterSpace)) {
            dirstart=W;
          }

          // object consists of only one pixel->store and exit
          if (dirstart == Invalid) {
            chain.at(ystartpos,xstartpos)=All;
            pntTemp.x=xstartpos;
            pntTemp.y=ystartpos;
            pObject->borderPointList.push_back(pntTemp);
            pObject->ioPointList.push_back(pntTemp); // in point
            pObject->ioPointList.push_back(pntTemp); // out point
          } else {
            // object consists of more than one point
            dir=dirstart;

            _lti_debug("START at x=" << xstartpos << ", y=" << ystartpos);
            _lti_debug(", dir=" << dirstart << "\n");

            _lti_debug("c");

            // iteration counter (in the worst case, all pixels in the
            // image will be checked, but having always a neighbor with
            // a different label.
            int ic=chain.rows()*chain.columns()/2;

            do { // repeat until the procedure reaches the start point

              // look left pixel
              olddir=dir;
              dir = (dir+NW)%8;

              // find next direction
              do {
                dir = (dir+SE)%8;
                col=xsteparray[dir]+x;
                row=ysteparray[dir]+y;
                // is pixel inRegion pixel (defined by threshold)
              } while ( (row<ystartpos) || (row>=vres) ||
                        (static_cast<unsigned int>(col) >=
                         static_cast<unsigned int>(hres)) ||
                        (pImg.at(row,col) != label) ||
                        (chain.at(row,col) > All) );

              // mark detected edges, store border and io points
              pntTemp.x=x;
              pntTemp.y=y;

              pObject->borderPointList.push_back(pntTemp);

              canzler = markarray[olddir][dir];

              if ((canzler & Right) == Right) {
                // now just insert the point, later on we will sort them...
                pObject->ioPointList.push_back(pntTemp);
              }

              if ((canzler & Left) == Left) {
                // now just insert the point, later on we will sort them...
                pObject->ioPointList.push_back(pntTemp);
              }

              chain.at(y,x)|=canzler;

              x=col;  // use new x-value
              y=row;  // use new y-value

              // try to detect infinite loops, which can happen sometimes
              if (--ic < 0) {
                setStatusString("Infinite loop in detectObjectsLabeled");
                return -1;
              }

#ifdef _LTI_DEBUG
              dirimg[y][x]=dir;
              olddirimg[y][x]=olddir;

              if (ic++ > 10000 && saveChain) {
                _lti_debug("Infinite Loop detected!\n");
                saveChain = false;

                std::cerr << ic << " " << xstartpos << " " << ystartpos << "\n";
                std::cerr << "Level: " << mNode.level() << "\n";
                char buf[80];
                int pid=getpid();
                sprintf(buf,"dirimage%05d.png",pid);
                saveDirImage(dirimg,buf,xstartpos,ystartpos);
                sprintf(buf,"errorchain%05d.png",pid);
                saveCanzlerImage(chain,buf,xstartpos,ystartpos);
                sprintf(buf,"olddirimage%05d.png",pid);
                saveDirImage(olddirimg,buf, xstartpos,ystartpos);

                // no hope for us: all's messed up
                exit(1);
              }
#endif
            } while (x!=xstartpos || y!=ystartpos || dir!=dirstart);
            _lti_debug("C");

            x=xstartpos;
            y=ystartpos;

            // io points must be sorted here, because even if they are
            // not required for the plstIOPointLists, they are used to
            // indicate the search area in the next level, and they MUST
            // be consistent
            pObject->ioPointList.sort();

          } //end of else (more than one point)

          // we cannot check the size of the ioPoints, because the user
          // may want to apply the minSize condition to area points
          if (bCalcIoPoints) {
            plstIOPointLists->push_back(pObject->ioPointList);
          }

          if (bCalcBorderPoints &&
              pObject->borderPointList.size() >= gParams.minSize) {
            plstBorderPointLists->push_back(pObject->borderPointList);
          }

          objnr++;
        }
      }
      // Case 2:
      // point is left border to a detected object->search
      // for right border or for another object within
      else if ((chain.at(y,x) & Left) != 0) {
        _lti_debug("  in-point at (" << x << "," << y << ")" << std::endl);

        checkersLabel = 0;
        xptr = &chain.at(y,x);
        do {
          if ((*xptr & Left) != 0) {
            ++checkersLabel;
          }

          if ((*xptr & Right) != 0) {
            --checkersLabel;
          }

          ++xptr;
        } while (checkersLabel > 0);

        x+=((xptr-&chain.at(y,x))-1);

        _lti_debug("  out-point at (" << x << "," << y << ")" << std::endl);
      }

      x++;

      if (x > xe) {
        ready = (iterIOPoint == lstIOSearchArea.end());
        if (!ready) {
          // the io points could already be processed.  In that case,
          // we need to get all already checked regions out of the io
          // points list.
          do {
            // input point
            ya=(*iterIOPoint).y;

            if (y==ya) {
              // it is not allowed that we get back!
              xa=max(x,(*iterIOPoint).x);
            } else {
              xa=(*iterIOPoint).x;
            }

            // output point
            iterIOPoint++;
            xe=(*iterIOPoint).x;

            iterIOPoint++;
          } while ((y==ya) && (xe < x));

          x=xa;
          y=ya;

          _lti_debug("Next y = " << y << std::endl);
        }
      }
    } while (!ready); //while
    _lti_debug("  End of Main Loop." << std::endl);

    // returning the number of detected objects
    return objnr;
  }

  //--------------------------------------------------------------------------

  // --------------------------------------------------
  //  Detect Objects -> Extracting Objects from Images
  // --------------------------------------------------
  int objectsFromMask::detectHoles(const imatrix& pImg,
                                   const int& thresh,
                                   lti::ioPoints& lstIOSearchArea,
                                   tree<objectStruct>::node& mNode) {

    if (gParams.assumeLabeledMask) {
      return detectHolesLabeled(pImg,thresh,lstIOSearchArea,mNode);
    } else {
      return detectHolesSimple(pImg,thresh,lstIOSearchArea,mNode);
    }
  }

  // --------------------------------------------------
  //  Detect Objects -> Extracting Objects from Images
  // --------------------------------------------------
  int objectsFromMask::detectHolesSimple(const imatrix& pImg,
                                         const int& thresh,
                                         lti::ioPoints& lstIOSearchArea,
                                         tree<objectStruct>::node& mNode) {

    _lti_debug("--- HOLE (S) DETECTION LEVEL " << mNode.level() << " ---" << 
               std::endl);

    int x=0, y=0, col, row,col2,row2; // temp values
    int xa,ya,xe;
    int objnr=0;                // Number of detected objects
    int xstartpos, ystartpos;   // start position
    int olddir,dir,dirstart,newdir; // directions of walk through the chaincode
    bool found;                 // Cancel flag
    bool ready = false;
    lti::point pntTemp;
    lti::ubyte *xptr;     // pointer to a point in mask

    chain.fill(channel8::value_type(0));

    iterIOPoint=lstIOSearchArea.begin();
    iterIOPointEnd=lstIOSearchArea.end();
    xa=(*iterIOPoint).x;
    y=(*iterIOPoint).y;
    iterIOPoint++;
    xe=(*iterIOPoint).x;
    iterIOPoint++;
    x=xa;xe=xe;

    // find first object by scanning the channel row by row and column
    // by column
    do {

      // case 1: the scanned pixel is white
      if (chain.at(y,x) == Nothing) {
        if (pImg.at(y,x) < thresh) {

          objectStruct tmpObject;
          mNode.appendChild(tmpObject);
          pObject=&mNode.child(mNode.degree()-1).getData();

          // memorize first pixel of the object (left upper corner)
          xstartpos = x;
          ystartpos = y;

          _lti_debug("  hole <- at (" << x << "," << y << ")" << std::endl);

          // calculate start direction for end argue
          dirstart=Invalid;

          if (y<vresm1) {
            if ((x>0) && (pImg.at(y+1,x-1) < thresh)) {
              dirstart=NE;
            } else if (pImg.at(y+1,x) < thresh) {
              dirstart=N;
            } else if (x<hresm1) {
              if (pImg.at(y+1,x+1) < thresh) {
                dirstart=NW;
              } else if (pImg.at(y,x+1) < thresh) {
                dirstart=W;
              }
            }
          } else if ((x<hresm1) && (pImg.at(y,x+1) < thresh)) {
            dirstart=W;
          }

          if (pImg.at(y+1,x) >= thresh && pImg.at(y,x+1) >= thresh)
            dir=dirstart=Invalid;
          else if (dirstart==NE && pImg.at(y+1,x) >= thresh) {
            if (pImg.at(y+1,x+1) < thresh) dir=dirstart=NW;
            else dir=dirstart=W;
          }

          // object consists of only one pixel->store and exit
          if (dirstart == Invalid) {
            chain.at(ystartpos,xstartpos)=All;
            pntTemp.x=xstartpos;
            pntTemp.y=ystartpos;
            pObject->borderPointList.push_back(pntTemp);
            pObject->ioPointList.push_back(pntTemp);
            pObject->ioPointList.push_back(pntTemp);
          }
          else { // object consists of more than one point
            dir=dirstart;
            do {// repeat until the procedure reaches the start point
              // look left pixel
              olddir=dir;
              dir = (dir+N)%8;

              found=false;
              // find next direction
              do {
                col=xsteparray[dir]+x;
                row=ysteparray[dir]+y;
                newdir = (dir+SE)%8;
                // is pixel inRegion pixel (defined by threshold)
                if (pImg.at(row,col) < thresh) {
                  col2=xsteparray[newdir]+x;
                  row2=ysteparray[newdir]+y;
                  if ((pImg.at(row2,col2) < thresh) || ((dir%2) == 0))
                    found=true;
                  else
                    dir = newdir;
                } else {
                  dir = newdir;
                }
              } while (!found); //while

              // mark detected edges, store border and io points
              pntTemp.x=x;
              pntTemp.y=y;

              pObject->borderPointList.push_back(pntTemp);

              if ((markarray[olddir][dir] & Right) == Right) {
                // now just insert the point, later on we will sort them...
                pObject->ioPointList.push_back(pntTemp);
              }

              if ((markarray[olddir][dir] & Left) == Left) {
                // now just insert the point, later on we will sort them...
                pObject->ioPointList.push_back(pntTemp);
              }

              chain.at(y,x)|=markarray[olddir][dir];

              x=col;  // use new x-value
              y=row;  // use new y-value

            } while (x!=xstartpos || y!=ystartpos || dir!=dirstart);

            x=xstartpos;
            y=ystartpos;

            // ioPoints need to be sorted now!
            // io points must be sorted here, because even if they are
            // not required for the plstIOPointLists, they are used to
            // indicate the search area in the next level, and they MUST
            // be consistent
            pObject->ioPointList.sort();

          } // else (more than one point)

          if (!gParams.meltHoles) {


            // insert sort used to "insert" to the holes in the corresponding
            // object
            pObject2=&mNode.getData();

            ioPoints::iterator iterObj=pObject2->ioPointList.begin();
            ioPoints::iterator iterHole=pObject->ioPointList.begin();

            while (iterHole != pObject->ioPointList.end()) {
              if ((*iterObj) < (*iterHole)) {
                ++iterObj;
              } else {
                pObject2->ioPointList.insert(iterObj,
                                             point((*iterHole).x-1,
                                                   (*iterHole).y));
                iterHole++;
                pObject2->ioPointList.insert(iterObj,
                                             point((*iterHole).x+1,
                                                   (*iterHole).y));
                iterHole++;
              }
            }

            // we cannot check the size of the ioPoints, because the user
            // may want to apply the minSize condition to area points
            if (bCalcIoPoints) {
              std::list<ioPoints>::iterator iter;
              // search for this object in the list
              iter=plstIOPointLists->begin();
              while ((*(*iter).begin()) != (*(pObject2->ioPointList.begin())))
                iter++;

              // and delete it in order to insert the new one with holes.
              iter=plstIOPointLists->erase(iter);
              plstIOPointLists->insert(iter,pObject2->ioPointList);
            }
          }
          objnr++;
        }
      }

      // case 2: point is left border to an detected object->search
      // for right border
      else if ((chain.at(y,x) & Left) != 0) {
        _lti_debug( "  in point at (" << x << "," << y << ")" << std::endl);

        // search for next right
        xptr=&chain.at(y,x);
        while((*xptr & Right) == 0) xptr++;

        x+=(xptr-&chain.at(y,x));

        _lti_debug("  out point at (" << x << "," << y << ")" << std::endl);
      }

      x++;
      if (x > xe) {
        ready = (iterIOPoint == lstIOSearchArea.end());
        if (!ready) {
          // the io points could already be processed.  In that case,
          // we need to get all already checked regions out of the io
          // points list.
          do {
            ya=(*iterIOPoint).y;

            if (y==ya) {
              // it is not allowed that we get back!
              xa=max(x,(*iterIOPoint).x);
            } else {
              xa=(*iterIOPoint).x;
            }

            iterIOPoint++;
            xe=(*iterIOPoint).x;
            iterIOPoint++;
          } while ((y==ya) && (xe < x));

          x=xa;
          y=ya;
        }
      }
    } while (!ready); //while

    // returning the number of detected objects
    return objnr;
  }

  // --------------------------------------------------
  //  Detect Objects -> Extracting Objects from Images
  // --------------------------------------------------
  int objectsFromMask::detectHolesLabeled(const imatrix& pImg,
                                          const int& thresh,
                                          lti::ioPoints& lstIOSearchArea,
                                          tree<objectStruct>::node& mNode) {

    _lti_debug("--- HOLE (L) DETECTION LEVEL " << mNode.level() << " ---" <<
               std::endl);

    int x=0, y=0, col, row,col2,row2; // temp values
    int xa,ya,xe;
    int objnr=0;                // Number of detected objects
    int xstartpos, ystartpos;   // start position
    int olddir,dir,dirstart,newdir; // directions of walk through the chaincode
    bool found;                 // Cancel flag
    bool ready = false;
    lti::point pntTemp;
    int label;                // label of region
    lti::ubyte *xptr,*xeptr;     // pointer to a point in mask

    static const ubyte OuterSpace = 0xF0;

    chain.fill(channel8::value_type(OuterSpace));

    // fill the OuterSpace with Nothing there, where holes can be expected
    iterIOPoint=lstIOSearchArea.begin();
    iterIOPointEnd=lstIOSearchArea.end();

    while (iterIOPoint != iterIOPointEnd) {
      // the "in"-point
      y = (*iterIOPoint).y;
      xptr = &chain.at(y,(*iterIOPoint).x);
      // the "out"-point
      ++iterIOPoint;
      xeptr = &chain.at(y,(*iterIOPoint).x);
      ++iterIOPoint;
      // fill between in- and out-points
      while (xptr<=xeptr) {
        *xptr++ = Nothing;
      }
    }

    iterIOPoint=lstIOSearchArea.begin();
    iterIOPointEnd=lstIOSearchArea.end();
    xa=(*iterIOPoint).x;
    y=(*iterIOPoint).y;
    iterIOPoint++;
    xe=(*iterIOPoint).x;
    iterIOPoint++;
    x=xa;xe=xe;

    // memorize label of the object
    label = pImg.at(y,x);

    // find first object by scanning the channel row by row and column
    // by column
    do {

      // case 1: the scanned pixel is white
      if (chain.at(y,x) == Nothing) {
        if (pImg.at(y,x)!=label) {
          objectStruct tmpObject;
          mNode.appendChild(tmpObject);
          pObject=&mNode.child(mNode.degree()-1).getData();

          // memorize first pixel of the object (left upper corner)
          xstartpos = x;
          ystartpos = y;

          _lti_debug("  hole <- " << static_cast<int>(label) << " at ("
                << x << "," << y << ")" << std::endl);

          // calculate start direction for end argue
          dirstart=Invalid;

          if (y<vresm1) {
            if ((x>0) && (pImg.at(y+1,x-1) != label) &&
                (chain.at(y+1,x-1) <= OuterSpace)) {
              dirstart=NE;
            } else if ((pImg.at(y+1,x) != label) &&
                       (chain.at(y+1,x) <= OuterSpace)) {
              dirstart=N;
            } else if (x<hresm1) {
              if ((pImg.at(y+1,x+1) != label) &&
                  (chain.at(y+1,x+1) <= OuterSpace)) {
                dirstart=NW;
              } else if ((pImg.at(y,x+1) != label) &&
                         (chain.at(y,x+1) <= OuterSpace)) {
                dirstart=W;
              }
            }
          } else if ((x<hresm1) && (pImg.at(y,x+1) != label) &&
                     (chain.at(y,x+1) <= OuterSpace)) {
            dirstart=W;
          }

          if (pImg.at(y+1,x) == label && pImg.at(y,x+1) == label)
            dir=dirstart=Invalid;
          else if (dirstart==NE && pImg.at(y+1,x) == label) {
            if (pImg.at(y+1,x+1) != label) dir=dirstart=NW;
            else dir=dirstart=W;
          }

          // object consists of only one pixel->store and exit
          if (dirstart == Invalid) {
            chain.at(ystartpos,xstartpos)=All;
            pntTemp.x=xstartpos;
            pntTemp.y=ystartpos;
            pObject->borderPointList.push_back(pntTemp);
            pObject->ioPointList.push_back(pntTemp);
            pObject->ioPointList.push_back(pntTemp);
          }
          else { // object consists of more than one point
            dir=dirstart;
            do {// repeat until the procedure reaches the start point
              // look left pixel
              olddir=dir;
              dir = (dir+N)%8;
              found=false;

              // find next direction
              do {
                col=xsteparray[dir]+x;
                row=ysteparray[dir]+y;
                newdir = (dir+SE)%8;
                // is pixel inRegion pixel (defined by threshold)
                if ((pImg.at(row,col) != label) &&
                    (chain.at(row,col) <= All)) {
                  col2=xsteparray[newdir]+x;
                  row2=ysteparray[newdir]+y;
                  if ((chain.at(row2,col2) <= All) &&
                      ((pImg.at(row2,col2) != label) || ((dir%2) == 0)))
                    found=true;
                  else
                    dir = newdir;
                } else {
                  dir = newdir;
                }
              } while (!found);

              // mark detected edges, store border and io points
              pntTemp.x=x;
              pntTemp.y=y;

              pObject->borderPointList.push_back(pntTemp);

              if ((markarray[olddir][dir] & Right) == Right) {
                // now just insert the point, later on we will sort them...
                pObject->ioPointList.push_back(pntTemp);
              }

              if ((markarray[olddir][dir] & Left) == Left) {
                // now just insert the point, later on we will sort them...
                pObject->ioPointList.push_back(pntTemp);
              }

              chain.at(y,x)|=markarray[olddir][dir];

              x=col;  // use new x-value
              y=row;  // use new y-value

            } while (x!=xstartpos || y!=ystartpos || dir!=dirstart);

            x=xstartpos;
            y=ystartpos;

            // ioPoints need to be sorted now!
            // io points must be sorted here, because even if they are
            // not required for the plstIOPointLists, they are used to
            // indicate the search area in the next level, and they MUST
            // be consistent
            pObject->ioPointList.sort();

          } // else (more than one point)

          if (!gParams.meltHoles) {

            // insert sort used to "insert" to the holes in the corresponding
            // object
            pObject2=&mNode.getData();

            ioPoints::iterator iterObj=pObject2->ioPointList.begin();
            ioPoints::iterator iterHole=pObject->ioPointList.begin();

            while (iterHole != pObject->ioPointList.end()) {
              if ((*iterObj) < (*iterHole)) {
                ++iterObj;
              } else {
                pObject2->ioPointList.insert(iterObj,
                                             point((*iterHole).x-1,
                                                   (*iterHole).y));
                iterHole++;
                pObject2->ioPointList.insert(iterObj,
                                             point((*iterHole).x+1,
                                                   (*iterHole).y));
                iterHole++;
              }
            }

            // we cannot check the size of the ioPoints, because the user
            // may want to apply the minSize condition to area points
            if (bCalcIoPoints) {
              std::list<ioPoints>::iterator iter;
              // search for this object in the list
              iter=plstIOPointLists->begin();
              while ((*(*iter).begin()) != (*(pObject2->ioPointList.begin())))
                iter++;

              // and delete it in order to insert the new one with holes.
              iter=plstIOPointLists->erase(iter);
              plstIOPointLists->insert(iter,pObject2->ioPointList);
            }
          }
          objnr++;
        }
      }

      // case 2: point is left border to an detected object->search
      // for right border
      else if ((chain.at(y,x) & Left) != 0) {
        _lti_debug( "  in point at (" << x << "," << y << ")" << std::endl);

        // search for next right
        xptr=&chain.at(y,x);
        while((*xptr & Right) == 0) xptr++;

        x+=(xptr-&chain.at(y,x));

        _lti_debug("  out point at (" << x << "," << y << ")" << std::endl);
      }

      x++;
      if (x > xe) {
        ready = (iterIOPoint == lstIOSearchArea.end());
        if (!ready) {
          // the io points could already be processed.  In that case,
          // we need to get all already checked regions out of the io
          // points list.
          do {
            ya=(*iterIOPoint).y;

            if (y==ya) {
              // it is not allowed that we get back!
              xa=max(x,(*iterIOPoint).x);
            } else {
              xa=(*iterIOPoint).x;
            }

            iterIOPoint++;
            xe=(*iterIOPoint).x;
            iterIOPoint++;
          } while ((y==ya) && (xe < x));

          x=xa;
          y=ya;

          // memorize label of the object
          label = pImg.at(y,x);
        }
      }
    } while (!ready); //while

    // returning the number of detected objects
    return objnr;
  }

  bool objectsFromMask::checkParameters() {

    const parameters&   theParams = getParameters();

    gParams.copy(theParams);

    if (gParams.level < -1) {
      setStatusString("parameters::level must be greater or equal -1");
      return false;
    }

    if ((gParams.threshold < 0) || (gParams.threshold > maxThresh)) {
      setStatusString("parameters::threshold must be between 0 and "+maxThresh);
      maxThresh = 2147483647; //back to 2^31-1 max of "int"
      return false;
    }
    maxThresh = 2147483647; //back to 2^31-1 max of "int"

    if (gParams.level == -1) {
      // this magic number (max number of 16 bits) was chosen, because
      // at this time, images of at least 65535 rows/columns are any way
      // not possible...
      //gParams.level=65535; todo think about
      gParams.level = max(vres,hres);
    }

    if (gParams.minSize < 1) {
      setStatusString("parameters::minSize must be at least 1");
      return false;
    }

    if (gParams.ioSearchAreaList.empty()) {
      for (int y=0;y<vres;y++) {
        gParams.ioSearchAreaList.push_back(point(0,y));
        gParams.ioSearchAreaList.push_back(point(hresm1,y));
      }
    }

    chain.resize(vres,hres,0,false,false); // set temp chain-channel
                                           // to channel size
    return true;
  }

  double objectsFromMask::getArea(void* thisPtr, const areaPoints& ap) {
    lti::dvector features;
    ((objectsFromMask*)(thisPtr))->featureExtractor.apply(ap,features);
    return features[0];
  }
  double objectsFromMask::getArea(void* thisPtr, const borderPoints& bp) {
    lti::dvector features;
    ((objectsFromMask*)(thisPtr))->featureExtractor.apply(bp,features);
    return features[0];
  }
  double objectsFromMask::getArea(void* thisPtr, const ioPoints& iop) {
    lti::dvector features;
    ((objectsFromMask*)(thisPtr))->featureExtractor.apply(iop,features);
    return features[0];
  }

  // storable interface
  /*
   * Storable interface for object struct
   */

  /*
   * write the functor in the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not.
   * The default implementation is to write only the parameters object,
   * since most functors do not have a state.
   */
  bool objectsFromMask::objectStruct::write(ioHandler& handler,
                                            const bool complete) const {
    bool b = true;

    if (complete) {
      b = handler.writeBegin();
    }
    
    lti::write(handler,ioPointList);
    lti::write(handler,areaPointList);
    lti::write(handler,borderPointList);

    if (complete) {
      b = handler.writeEnd();
    }

    return b;
  }

  /*
   * read the functor from the given ioHandler.
   * The complete flag indicates
   * if the enclosing begin and end should be also be written or not.
   * The default implementation is to write only the parameters object,
   * since most functors do not have a state.
   */
  bool objectsFromMask::objectStruct::read(ioHandler& handler,
                                           const bool complete) {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }
    
    lti::read(handler,ioPointList);
    lti::read(handler,areaPointList);
    lti::read(handler,borderPointList);

    if (complete) {
      b = handler.readEnd();
    }

    return b;
  }
}
