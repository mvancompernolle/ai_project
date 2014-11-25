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
 *
 */

/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiGHoughTransform.cpp
 * authors ....: Jens Rietzschel
 * organization: LTI, RWTH Aachen
 * creation ...: 29.01.2001
 * revisions ..: $Id: ltiGHoughTransform.cpp,v 1.16 2006/09/05 10:13:35 ltilib Exp $
 */

#include "ltiGHoughTransform.h"
#include "ltiDraw.h"
#include "ltiOrientationMap.h"
#include "ltiGeometricTransform.h"
#include "ltiBoundingBox.h"
#include "ltiLocation.h"
#include "ltiContour.h"
#include "ltiObjectsFromMask.h"
#include "ltiMath.h"

#include <cstdlib>
#include <cstdio>

#undef _LTI_DEBUG
// #define _LTI_DEBUG 2
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // houghTransform::parameters
  // --------------------------------------------------

  // default constructor
  gHoughTransform::parameters::parameters()
    : modifier::parameters() {



    numberOfObjects=1;
    kernelSizeTranslation=11;
    kernelSizeRotation=21;
    kernelSizeScale=11;
    thresholdEdgePixel=0.9f;
    thresholdLocalMaximum=10;
    findLocalMaximum=false;
    disSize=4;//odd
    disRotation=180;
    extension=2;
    numberOfAngleLists=3;

  }

  // copy constructor
  gHoughTransform::parameters::parameters(const parameters& other)
    : modifier::parameters()  {
    copy(other);
  }

  // destructor
  gHoughTransform::parameters::~parameters() {
  }

  // get type name
  const char* gHoughTransform::parameters::getTypeName() const {
    return "gHoughTransform::parameters";
  }

  // copy member

  gHoughTransform::parameters&
    gHoughTransform::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    modifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    modifier::parameters& (modifier::parameters::* p_copy)
      (const modifier::parameters&) =
      modifier::parameters::copy;
    (this->*p_copy)(other);
# endif


    numberOfObjects=other.numberOfObjects;
    kernelSizeTranslation=other.kernelSizeTranslation;
    kernelSizeRotation=other.kernelSizeRotation;
    kernelSizeScale=other.kernelSizeScale;
    thresholdEdgePixel=other.thresholdEdgePixel;
    thresholdLocalMaximum=other.thresholdLocalMaximum;
    findLocalMaximum=other.findLocalMaximum;
    disSize=other.disSize;
    disRotation=other.disRotation;
    extension=other.extension;
    numberOfAngleLists=other.numberOfAngleLists;


    return *this;
  }

  // alias for copy member
  gHoughTransform::parameters&
    gHoughTransform::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* gHoughTransform::parameters::clone() const {
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
  bool gHoughTransform::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool gHoughTransform::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"numberOfObjects",numberOfObjects);
      lti::write(handler,"kernelSizeTranslation",kernelSizeTranslation);
      lti::write(handler,"thresholdEdgePixel",thresholdEdgePixel);
      lti::write(handler,"thresholdLocalMaximum",thresholdLocalMaximum);
      lti::write(handler,"findLocalMaximum",findLocalMaximum);
      lti::write(handler,"disSize",disSize);
      lti::write(handler,"disRotation",disRotation);
      lti::write(handler,"extension",extension);
      lti::write(handler,"numberOfAngleLists",numberOfAngleLists);
      lti::write(handler,"kernelSizeRotation",kernelSizeRotation);
      lti::write(handler,"kernelSizeScale",kernelSizeScale);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::write(handler,false);
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
  bool gHoughTransform::parameters::write(ioHandler& handler,
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
  bool gHoughTransform::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool gHoughTransform::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"numberOfObjects",numberOfObjects);
      lti::read(handler,"kernelSizeTranslation",kernelSizeTranslation);
      lti::read(handler,"thresholdEdgePixel",thresholdEdgePixel);
      lti::read(handler,"thresholdLocalMaximum",thresholdLocalMaximum);
      lti::read(handler,"findLocalMaximum",findLocalMaximum);
      lti::read(handler,"disSize",disSize);
      lti::read(handler,"disRotation",disRotation);
      lti::read(handler,"extension",extension);
      lti::read(handler,"numberOfAngleLists",numberOfAngleLists);
      lti::read(handler,"kernelSizeRotation",kernelSizeRotation);
      lti::read(handler,"kernelSizeScale",kernelSizeScale);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && modifier::parameters::read(handler,false);
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
  bool gHoughTransform::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // houghTransform
  // --------------------------------------------------

  // default constructor
  gHoughTransform::gHoughTransform()
    : modifier() {
    parameters defaultParam;
    setParameters(defaultParam);
    isSet=false;
  }

  // copy constructor
  gHoughTransform::gHoughTransform(const gHoughTransform& other)
    : modifier()  {
    copy(other);
  }

  // destructor
  gHoughTransform::~gHoughTransform() {
  }

  // returns the name of this type
  const char* gHoughTransform::getTypeName() const {
    return "gHoughTransform";
  }

  // copy member
  gHoughTransform&
    gHoughTransform::copy(const gHoughTransform& other) {
      modifier::copy(other);
    return (*this);
  }

  // clone member
  functor* gHoughTransform::clone() const {
    return new gHoughTransform(*this);
  }

  // return parameters
  const gHoughTransform::parameters&
    gHoughTransform::getParameters() const {
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



  bool gHoughTransform::apply(const channel& src,
                                    std::list<vector<float> >& dest) {

    if (!isSet)
      return false;
    else {

      lti::gHoughTransform::parameters param=getParameters();
      int numberOfObjects=param.numberOfObjects;
      int disSize=param.disSize;
      int disRotation = param.disRotation;
      double extension=param.extension;
      float threshold=param.thresholdEdgePixel;

      if(disRotation<1)disRotation=1;
      if(disSize<1)disSize=1;
      if(!(disSize%2))disSize++;
      if(extension<=1){
        extension=1;
        disSize=1;
      }
      double stepSize((extension-1)/((disSize-1)/2));
      int originSize=int((disSize-1)/2);
      double stepRotation=2*Pi/disRotation;
      orientationMap myMap;
      channel direction,relevance;
      myMap.apply(src,direction,relevance);
      ivector vec(4);
      ivector vec2(4);
      vec.at(0)=src.rows();
      vec.at(1)=src.columns();
      vec.at(2)=disRotation;
      vec.at(3)=disSize;
      tensor<float> accu(4,vec);
      accu.fill(0);
      tpointList<float>::const_iterator it;
      double phi;
      int rowIndex1,rowIndex2,colIndex1,colIndex2;
      float valueRow1,valueRow2,valueCol1,valueCol2;
      float row,col;
      listEntry entry;
      std::list<listEntry> indexList;
      std::list<listEntry>::iterator it2;
      vector<float> result(5);
      tpointList<float> myAlphaRList;
      double size;
      float sina, cosa;

      _lti_debug2("transforming");
      for(int i=0;i<src.rows();i++){//transform
        _lti_debug2(".");
        for(int j=0;j<src.columns();j++){
          if(src.at(i,j)>=threshold){//edgePixel
            phi=direction.at(i,j);
            for(int k=0;k<disRotation;k++){
              const double rotation = k*stepRotation;
              if(myReferenceTable.getAlphaRList(phi,rotation,myAlphaRList)){
                for(it=myAlphaRList.begin();it!=myAlphaRList.end();it++){
                  const float alpha = it->x;
                  const float r = it->y;
                  sincos(static_cast<float>(alpha + rotation), sina, cosa);
                  for(int l=0;l<disSize;l++){
                    if(l>originSize) {
                      size=(1+(l-originSize)*stepSize);
                    } else if(l==originSize) {
                      size=1;
                    } else {
                      size=(1/((originSize-l)*stepSize+1));
                    }

                    row=static_cast<float>(i-(size*r*cosa));
                    col=static_cast<float>(j-(size*r*sina));

                    colIndex1=int(col);colIndex2=int(col)+1;
                    rowIndex1=int(row);rowIndex2=int(row)+1;
                    valueRow1=rowIndex2-row;valueRow2=row-rowIndex1;
                    valueCol1=colIndex2-col;valueCol2=col-colIndex1;

                    if(((rowIndex1>=0)&&(rowIndex1<src.rows()))&&
                       ((colIndex1>=0)&&(colIndex1<src.columns()))){
                      vec[0]=rowIndex1;vec[1]=colIndex1;vec[2]=k;vec[3]=l;
                      accu.at(vec)+=(valueRow1+valueCol1);
                    }
                    if(((rowIndex1>=0)&&(rowIndex1<src.rows()))&&
                       ((colIndex2>=0)&&(colIndex2<src.columns()))){
                      vec[0]=rowIndex1;vec[1]=colIndex2;vec[2]=k;vec[3]=l;
                      accu.at(vec)+=(valueRow1+valueCol2);
                    }
                    if(((rowIndex2>=0)&&(rowIndex2<src.rows()))&&
                       ((colIndex1>=0)&&(colIndex1<src.columns()))){
                      vec[0]=rowIndex2;vec[1]=colIndex1;vec[2]=k;vec[3]=l;
                      accu.at(vec)+=(valueRow2+valueCol1);
                    }
                    if(((rowIndex2>=0)&&(rowIndex2<src.rows()))&&
                       ((colIndex2>=0)&&(colIndex2<src.columns()))){
                      vec[0]=rowIndex2;vec[1]=colIndex2;vec[2]=k;vec[3]=l;
                      accu.at(vec)+=(valueRow2+valueCol2);
                    }
                  }
                }
              }
            }
          }
        }
      }

      findMaximum4D(accu,src.rows(),src.columns(),
                    disSize,disRotation,indexList);
      it2=indexList.begin();
      for(int n=1;((n<=numberOfObjects)&&(it2!=indexList.end()));n++,it2++){
        entry=(*it2);
        double size=1;
        if (entry.sizeIndex>originSize) {
          size=(1+(entry.sizeIndex-originSize)*stepSize);
        } else if (entry.sizeIndex==originSize) {
          size=1;
        } else if (entry.sizeIndex<originSize) {
          size=(1/((originSize-entry.sizeIndex)*stepSize+1));
        }

        const float rotation=static_cast<float>(entry.rotationIndex*stepRotation);
        result[0]=static_cast<float>(entry.column-myReferenceTable.anchor.x);
        result[1]=static_cast<float>(entry.row-myReferenceTable.anchor.y);
        result[2]=static_cast<float>(size);
        result[3]=static_cast<float>(rotation);
        result[4]=static_cast<float>(entry.value);
        dest.push_back(result);

        _lti_debug2("\nrow: "<<entry.row<<"  column: "<<entry.column
                    <<"    size: "<<size
                    <<"    rotation: "<<((rotation/(2*Pi))*360)
                    <<"\n"<<"value: "<<entry.value<<"\n\n");
      }
    }
    return true;
  }


  bool gHoughTransform::apply(const channel& sample,
                              const channel& src,
                                    std::list<vector<float> > & dest) {

    lti::gHoughTransform::parameters param=getParameters();
    float threshold=param.thresholdEdgePixel;
    int numberOfAngleLists=param.numberOfAngleLists;
    myReferenceTable.buildReferenceTable(sample,threshold,numberOfAngleLists);
    isSet=true;
    apply(src,dest);
    return true;
  }


  bool gHoughTransform::apply(const tpointList<int>& contour,
                              const channel& src,
                                    std::list<vector<float> >& dest) {

    lti::gHoughTransform::parameters param=getParameters();
    int numberOfAngleLists=param.numberOfAngleLists;
    myReferenceTable.buildReferenceTable(contour,numberOfAngleLists);
    isSet=true;
    apply(src,dest);

    return true;
  }


  bool gHoughTransform::draw(const std::list<vector<float> >& parameter,
                                   image& dest,
                             const rgbPixel& color) {

    lti::draw<rgbPixel> drawer;
    drawer.setColor(color);
    drawer.use(dest);
    std::list<vector<float> >::const_iterator it1;
    geometricTransform transformer;
    geometricTransform::parameters param;
    vector<double> center(3);
    vector<double> axis(3);
    axis[0]=0;axis[1]=0;axis[2]=1;
    int n=1;
    tpoint<double> shiftPoint;
    tpoint<double> shiftTemp;
    shiftTemp.x=-myReferenceTable.anchor.x;
    shiftTemp.y=-myReferenceTable.anchor.y;

    for(it1=parameter.begin();it1!=parameter.end();it1++){
      tpointList<int> all=myReferenceTable.allPoints;
      n++;
      shiftPoint.x=(it1->at(0));
      shiftPoint.y=(it1->at(1));
      double scale=(it1->at(2));
      float rotation=(it1->at(3));

      center[0]=(double)myReferenceTable.anchor.x;
      center[1]=(double)myReferenceTable.anchor.y;
      center[2]=0;

      param.clear();
      param.rotate(center,axis,double(rotation*(-1)));
      param.shift(shiftTemp);
      transformer.setParameters(param);
      transformer.apply(all);

      param.clear();
      param.scale(scale);
      transformer.setParameters(param);
      transformer.apply(all);

      param.clear();
      param.shift((shiftPoint-shiftTemp));
      transformer.setParameters(param);
      transformer.apply(all);

      drawer.set(all);

    }
    _lti_debug2("\n"<<n<<" objects drawn\n");
    return true;
  }

  bool gHoughTransform::referenceTable::push(float alpha,float r,double phi) {

    tpoint<float> alphaR;
    alphaR.x=alpha;alphaR.y=r;
    int phiGrad=int(round(phi*360/(2*Pi)));
    std::map<int,tpointList<float> >::iterator it;
    it=refTable.find(phiGrad);
    if(it==refTable.end()){
      tpointList<float> myList;
      myList.push_back(alphaR);
      refTable[phiGrad]=myList;
    } else {
      (it->second).push_back(alphaR);
    }
    return true;
  }

  bool
  gHoughTransform::referenceTable::getAlphaRList(double phi,
                                                 double rotation,
                                                 tpointList<float>& dest) {

    int rotationGrad=int(round(rotation/(2*Pi)*360));
    int phiGrad=(int(round(phi/(2*Pi)*360)));
    int phiEntry=(phiGrad+rotationGrad)%180;
    std::map<int,tpointList<float> >::iterator it;
    std::map<int,tpointList<float> >::iterator it2;
    tpointList<float> tempList;
    it=refTable.find(phiEntry);
    bool found=false;
    if(it==refTable.end()){
      it2=refTable.find(phiEntry-1);
      if(it2!=refTable.end()){
        dest=it2->second;
        found=true;
      }
      it2=refTable.find(phiEntry+1);
      if(it2!=refTable.end()){
        if(found){
          tempList=it2->second;
          dest.insert(dest.end(),tempList.begin(),tempList.end());
        }
        else{
          dest=it2->second;
          found=true;
        }
      }
      if(!found) return false;
      else return true;
    }
    else{

      dest=(it->second);

      int i;

      for(i=numberOfAngleLists;i>0;i--){
        it2=refTable.find(phiEntry-i);
        if(it2!=refTable.end()){
          tempList=it2->second;
          dest.insert(dest.end(),tempList.begin(),tempList.end());
        }
      }
      for(i=numberOfAngleLists;i>0;i--){
        it2=refTable.find(phiEntry+i);
        if(it2!=refTable.end()){
          tempList=it2->second;
          dest.insert(dest.end(),tempList.begin(),tempList.end());
        }
      }
      return true;
    }
  }


  bool
  gHoughTransform::referenceTable::buildReferenceTable(const channel& src,
                                                     float threshold,
                                                     int numberOfAngleListsP) {

    numberOfAngleLists=numberOfAngleListsP;
    lti::objectsFromMask myObjectsFromMask;
    lti::objectsFromMask::parameters paramOFM;
    paramOFM.sortObjects=true;
    myObjectsFromMask.setParameters(paramOFM);
    lti::ioPoints myIoPoints;
    std::list<ioPoints> myListOfObjects;
    std::list<ioPoints>::iterator it;
    channel8 mask;
    mask.castFrom(src);
    myObjectsFromMask.apply(mask,myListOfObjects);
    it=myListOfObjects.begin();
    myIoPoints=(*it);
    lti::orientationMap myOrientationMap;
    channel direction,relevance;
    myOrientationMap.apply(src,direction,relevance);
    boundingBox<float> myBox;
    rectLocation myRectLocation;
    int deltaX,deltaY;
    float alpha,r;
    double phi;
    tpoint<int> p;

    myBox.apply(myIoPoints,myRectLocation);
    anchor.x=(int)myRectLocation.position.x;
    anchor.y=(int)myRectLocation.position.y;

    _lti_debug2("\n\nanchor.x="<<anchor.x<<"   anchor.y"<<anchor.y<<"\n");

    for(int i=0;i<src.rows();i++){
      for(int j=0;j<src.columns();j++){
        if(src.at(i,j)>=threshold){
          p.x=j;p.y=i;
          allPoints.push_back(p);
          deltaX=i-anchor.y;
          deltaY=j-anchor.x;
          alpha=atan2(float(deltaY),float(deltaX));
          if (alpha<0) {
            alpha=static_cast<float>((2.0*Pi)+alpha);
          }
          r=sqrt(static_cast<float>(sqr(deltaX)+sqr(deltaY)));
          phi=direction.at(i,j);
          push(alpha,r,phi);
        }
      }
    }

    return true;
  }


  bool
  gHoughTransform::referenceTable::buildReferenceTable(
                                           const tpointList<int>& src,
                                           int numberOfAngleListsP) {
    allPoints=src;
    numberOfAngleLists=numberOfAngleListsP;
    boundingBox<float> myBox;
    rectLocation myRectLocation;
    myBox.apply(src,myRectLocation);
    int maxSize=(int)myRectLocation.maxLength+2;
    lti::orientationMap myOrientationMap;
    channel direction,relevance;
    channel chnl(iround(maxSize+myRectLocation.position.y),
                 iround(maxSize+myRectLocation.position.x),0.0f);
    tpointList<int>::const_iterator it;
    for(it=src.begin();it!=src.end();it++){
      chnl.at(it->y,it->x)=1;
    }
    myOrientationMap.apply(chnl,direction,relevance);
    int deltaX,deltaY;
    float alpha,r;
    double phi;

    anchor.x=(int)myRectLocation.position.x;
    anchor.y=(int)myRectLocation.position.y;

    _lti_debug2("\n\nanchor.x="<<anchor.x<<"   anchor.y"<<anchor.y<<"\n");

    static const float pi2=static_cast<float>(2.0*Pi);
    for(it=src.begin();it!=src.end();it++){
          deltaX=(it->y)-anchor.y;
          deltaY=(it->x)-anchor.x;

          alpha=atan2(static_cast<float>(deltaY),
                      static_cast<float>(deltaX));
          if(alpha<0)alpha=pi2+alpha;
          r=sqrt(static_cast<float>(sqr(deltaX)+sqr(deltaY)));
          phi=direction.at(it->y,it->x);
          push(alpha,r,phi);
          push(static_cast<float>(deltaX),
               static_cast<float>(deltaY),phi);
    }

    return true;
  }


  bool gHoughTransform::findMaximum4D(tensor<float>& accu,
                                      int rows,
                                      int columns,
                                      int disSize,
                                      int disRotation,
                                      std::list<listEntry>& dest) {

    lti::gHoughTransform::parameters param=getParameters();
    int kernelSizeTranslation=int((param.kernelSizeTranslation-1)/2);
    int kernelSizeRotation=int((param.kernelSizeRotation-1)/2);
    int kernelSizeScale=int((param.kernelSizeScale-1)/2);
    int numberOfObjects=param.numberOfObjects;
    float thresholdLocalMaximum=param.thresholdLocalMaximum;
    bool findLocalMaximum=param.findLocalMaximum;
    ivector vec(4),vec2(4);
    listEntry entry;
    bool found=true;
    std::list<listEntry>::iterator it;

    if(findLocalMaximum){ //find local maxima in parameter-space
      _lti_debug2("\nsearching local maxima");
      for(int i=0;i<rows;i++){
         _lti_debug2(".");
        for(int j=0;j<columns;j++){
          for(int k=0;k<disRotation;k++){
            for(int l=0;l<disSize;l++){
              for(int I=i-kernelSizeTranslation;
                  I<=i+kernelSizeTranslation;I++){
                if((I>=0)&&(I<rows)){
                  for(int J=j-kernelSizeTranslation;
                      J<=j+kernelSizeTranslation;J++){
                    if((J>=0)&&(J<columns)){
                      for(int K=k-kernelSizeRotation;
                          K<=k+kernelSizeRotation;K++){
                        if((K>=0)&&(K<disRotation)){
                          for(int L=l-kernelSizeScale;
                              L<=l+kernelSizeScale;L++){
                            if((L>=0)&&(L<disSize)){
                              vec[0]=i;vec[1]=j;vec[2]=k;vec[3]=l;
                              vec2[0]=I;vec2[1]=J;vec2[2]=K;vec2[3]=L;
                              if((accu.at(vec2)>=accu.at(vec))&&
                                 ((I!=i)||(J!=j)||(L!=l)||(K!=k))){
                                found=false;
                              }
                            }
                            if(!found)break;
                          }
                        }
                        if(!found)break;
                      }
                    }
                    if(!found)break;
                  }
                }
                if(!found)break;
              }
              if(found){
                entry.row=i;
                entry.column=j;
                entry.sizeIndex=l;
                entry.rotationIndex=k;

                vec[0]=i;vec[1]=j;vec[2]=k;vec[3]=l;
                entry.value=accu.at(vec);
                if(entry.value>=thresholdLocalMaximum)dest.push_back(entry);
              }
              found=true;
            }
          }
        }
      }
    }

    else {
      int n=0;
      vec2[0]=0;vec2[1]=0;vec2[2]=0;vec[3]=0;
      for(int i=0;i<rows;i++){
        _lti_debug2(".");
        for(int j=0;j<columns;j++) {
          for(int k=0;k<disRotation;k++) {
            for(int l=0;l<disSize;l++) {
              vec[0]=i;vec[1]=j;vec[2]=k;vec[3]=l;
              if (accu.at(vec)>=thresholdLocalMaximum) {
                if (n<numberOfObjects) {
                  entry.row=vec[0];
                  entry.column=vec[1];
                  entry.rotationIndex=vec[2];
                  entry.sizeIndex=vec[3];
                  entry.value=accu.at(vec);
                  n++;
                  dest.push_back(entry);
                  distance_remove(dest,n);
                } else {
                  it=dest.begin();
                  if(accu.at(vec)>it->value){
                    entry.row=vec[0];
                    entry.column=vec[1];
                    entry.rotationIndex=vec[2];
                    entry.sizeIndex=vec[3];
                    entry.value=accu.at(vec);
                    dest.push_back(entry);
                    n++;
                    distance_remove(dest,n);
                  }
                }
              }
            }
          }
        }
      }
    }

    dest.sort();
    dest.reverse();

    return true;
  }

  bool gHoughTransform::distance_remove(std::list<listEntry>& src,int& n) {


    lti::gHoughTransform::parameters param=getParameters();
    int kernelSizeTranslation=param.kernelSizeTranslation;
    int kernelSizeScale=param.kernelSizeScale;
    int kernelSizeRotation=param.kernelSizeRotation;
    int number=param.numberOfObjects;

    std::list<listEntry>::iterator it;
    std::list<listEntry>::iterator it2;
    src.sort();
    src.reverse();
    bool erased=false;

    for(int k=0;k<number;k++){
      for(it=src.begin();it!=src.end();it++){
        for(it2=it;it2!=src.end();it2++){
          if(it!=it2){
            if(((abs(it->row-it2->row)<kernelSizeTranslation)&&
                (abs(it->column-it2->column)<kernelSizeTranslation))&&
               ((abs(it->sizeIndex-it2->sizeIndex)<kernelSizeScale)&&
                (((abs(it->rotationIndex-it2->rotationIndex) <
                   kernelSizeRotation)||
                  (((abs(360-it->rotationIndex)+it2->rotationIndex)) <
                   kernelSizeRotation))||
                 (((abs(360-it2->rotationIndex)+it->rotationIndex)) <
                  kernelSizeRotation)))){
              src.erase(it2);
              n--;
              erased=true;
            }
          }
          if(erased) break;
        }
        if(erased) break;
      }
    }

    src.sort();
    if(n>number){
      for(int i=n;i>number;i--){
        it=src.begin();
        src.erase(it);
        n--;
      }
    }
    src.sort();
    return true;
  }

  bool gHoughTransform::use (const channel& src) {
    lti::gHoughTransform::parameters param=getParameters();
    float threshold=param.thresholdEdgePixel;
    int numberOfAngleLists=param.numberOfAngleLists;
    myReferenceTable.buildReferenceTable(src,threshold,numberOfAngleLists);
    isSet=true;
    return true;
  }

  bool gHoughTransform::use (const tpointList<int>& src) {
    lti::gHoughTransform::parameters param=getParameters();
    int numberOfAngleLists=param.numberOfAngleLists;
    myReferenceTable.buildReferenceTable(src,numberOfAngleLists);
    isSet=true;
    return true;
  }

  gHoughTransform::referenceTable::referenceTable() {

  }


  gHoughTransform::referenceTable::referenceTable(const channel& src,
                                                  float threshold,
                                                  int disRTable){

    buildReferenceTable(src,threshold,disRTable);
  }

  gHoughTransform::referenceTable::referenceTable(const tpointList<int>& src,
                                                  int disRTable){

    buildReferenceTable(src,disRTable);
  }


  bool gHoughTransform::listEntry::operator<(const listEntry& a){
    return(value<a.value);
  }

} // end namespace
