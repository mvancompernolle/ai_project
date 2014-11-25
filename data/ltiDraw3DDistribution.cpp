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


/* -------------------------------------------------------------------
 * project ....: LTI-Lib: Image Processing and Computer Vision Library
 * file .......: ltiDraw3DDistribution.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 21.1.2003
 * revisions ..: $Id: ltiDraw3DDistribution.cpp,v 1.9 2006/09/05 10:09:43 ltilib Exp $
 */

#include "ltiDraw3DDistribution.h"
#include "ltiSTLIoInterface.h"
#include "ltiBoundsFunctor.h"
#include <sstream>
#include <iomanip>

namespace lti {
  // --------------------------------------------------
  // draw3DDistribution::parameters
  // --------------------------------------------------

  // default constructor
  draw3DDistribution::parameters::parameters()
    : functor::parameters() {

      // initialize the colorMap
    colorMap.push_back(lti::Red);
    colorMap.push_back(lti::Green);
    colorMap.push_back(lti::Yellow);
    colorMap.push_back(lti::Cyan);
    colorMap.push_back(lti::Magenta);
    colorMap.push_back(lti::Blue);
    colorMap.push_back(lti::White);
    colorMap.push_back(rgbPixel(255,0  ,127)); // Fusia
    colorMap.push_back(rgbPixel(127,255,127)); // Lemon
    colorMap.push_back(rgbPixel(255,127,127)); // Pink
    colorMap.push_back(rgbPixel(255,127,0)  ); // Orange
    colorMap.push_back(rgbPixel(255,255,127)); // BrightYellow
    colorMap.push_back(rgbPixel(255,127,0)  ); // BrightRed
    colorMap.push_back(rgbPixel(0  ,255,127)); // BrightGreen
    colorMap.push_back(rgbPixel(0  ,127,255)); // LightBlue

    markerMap.push_back("o");
    markerMap.push_back("x");
    markerMap.push_back("+");
    markerMap.push_back("*");
    markerMap.push_back("s");
    markerMap.push_back("d");
    markerMap.push_back("^");
    markerMap.push_back("v");
    markerMap.push_back("<");
    markerMap.push_back(">");

    scaleEqual=true;
    nbTicks=5;
    gridColor=rgbPixel(127,127,127);
    grid=true;
    colouredAxes=false;
  }

  // copy constructor
  draw3DDistribution::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  draw3DDistribution::parameters::~parameters() {
  }

  // get type name
  const char* draw3DDistribution::parameters::getTypeName() const {
    return "draw3DDistribution::parameters";
  }

  // copy member

  draw3DDistribution::parameters&
    draw3DDistribution::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    functor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    functor::parameters& (functor::parameters::* p_copy)
      (const functor::parameters&) =
      functor::parameters::copy;
    (this->*p_copy)(other);
# endif


      colorMap = other.colorMap;
      markerMap = other.markerMap;
      scaleEqual = other.scaleEqual;
      nbTicks = other.nbTicks;
      gridColor = other.gridColor;
      grid = other.grid;
      colouredAxes = other.colouredAxes;

    return *this;
  }

  // alias for copy member
  draw3DDistribution::parameters&
    draw3DDistribution::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* draw3DDistribution::parameters::clone() const {
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
  bool draw3DDistribution::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool draw3DDistribution::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"colorMap",colorMap);
      lti::write(handler,"markerMap",markerMap);
      lti::write(handler,"scaleEqual",scaleEqual);
      lti::write(handler,"nbTicks",nbTicks);
      lti::write(handler,"gridColor",gridColor);
      lti::write(handler,"grid",grid);
      lti::write(handler,"colouredAxes",colouredAxes);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::write(handler,false);
# else
    bool (functor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      functor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool draw3DDistribution::parameters::write(ioHandler& handler,
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
  bool draw3DDistribution::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool draw3DDistribution::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"colorMap",colorMap);
      lti::read(handler,"markerMap",markerMap);
      lti::read(handler,"scaleEqual",scaleEqual);
      lti::read(handler,"nbTicks",nbTicks);
      lti::read(handler,"gridColor",gridColor);
      lti::read(handler,"grid",grid);
      lti::read(handler,"colouredAxes",colouredAxes);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && functor::parameters::read(handler,false);
# else
    bool (functor::parameters::* p_readMS)(ioHandler&,const bool) =
      functor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool draw3DDistribution::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // draw3DDistribution
  // --------------------------------------------------

  // default constructor
  draw3DDistribution::draw3DDistribution()
    : functor() {

    idsToColor = new std::map<int,rgbPixel>;
    idsToMarker = new std::map<int,std::string>;
    
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // default constructor
  draw3DDistribution::draw3DDistribution(const parameters& par)
    : functor() {

    idsToColor = new std::map<int,rgbPixel>;
    idsToMarker = new std::map<int,std::string>;

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  draw3DDistribution::draw3DDistribution(const draw3DDistribution& other)
    : functor()  {
    copy(other);
  }

  // destructor
  draw3DDistribution::~draw3DDistribution() {
  }

  // returns the name of this type
  const char* draw3DDistribution::getTypeName() const {
    return "draw3DDistribution";
  }

  // copy member
  draw3DDistribution&
  draw3DDistribution::copy(const draw3DDistribution& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  draw3DDistribution&
    draw3DDistribution::operator=(const draw3DDistribution& other) {
    return (copy(other));
  }


  // clone member
  functor* draw3DDistribution::clone() const {
    return new draw3DDistribution(*this);
  }

  // return parameters
  const draw3DDistribution::parameters&
    draw3DDistribution::getParameters() const {
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


  bool draw3DDistribution::apply(scene3D<rgbPixel>& drawer,const dmatrix& dist,
                                 const bool& hold) const {
      bool ok=true;
      // find first free negativ id. That is done, because this distribution
      // is connected with an id, to save the color that is used for this
      // distribution.
      int i=-1;
      while ( idsToColor->find(i)!=idsToColor->end() ) i--;
      ok=apply(drawer,dist,ivector(dist.rows(),i),hold);
      return ok;
  }

  bool draw3DDistribution::apply(scene3D<rgbPixel>& drawer,const dmatrix& dist,
                                 int first, int second, int third,
                                 const bool& hold) const {
      bool ok;
      dmatrix temp(dist.rows(),3);
      temp.setColumn(0,dist.getColumnCopy(first) );
      temp.setColumn(1,dist.getColumnCopy(second) );
      temp.setColumn(2,dist.getColumnCopy(third) );
      ok=apply(drawer,temp,hold);
      return ok;
  }

  bool draw3DDistribution::apply(scene3D<rgbPixel>& drawer,const dvector& x,
                                 const dvector& y, const dvector& z,
                                 const bool& hold) const {
      if ( !( (x.size()==y.size()) && (x.size()==z.size()) )  ) {
          setStatusString("both vectors must have the same size");
          return false;
      }
      dmatrix temp(x.size(),3);
      temp.setColumn(0,x);
      temp.setColumn(1,y);
      temp.setColumn(2,z);
      return apply(drawer,temp,hold);
  }

  bool draw3DDistribution::apply(scene3D<rgbPixel>& drawer,const dmatrix& dist,
                                 const ivector& ids, const bool& hold) const {
    bool ok=true;
    drawer.resetDeepCtrl(true);
    parameters p=getParameters();
    int i;

    boundsFunctor<double> bF;      
    dvector minV(3);
    dvector maxV(3);
    dvector ranges(3);

    bF.boundsOfRows(dist,minV,maxV);
    dvector scale(3,1.0);

    if (dist.columns()!=3) {
      ok=false;
      setStatusString("This functor only shows 3 dimensional distributions");
    }
    if (dist.rows()!=ids.size() ) {
      ok=false;
      setStatusString("the distrubtion and the id-vector must have the same size");
    }
    // if hold is false, reset the scene, else the new points are appended
    // to the saved commands in scene3D
    if (!hold) {
      drawer.reset();
      idsToColor->clear();
    }

    if (!p.scaleEqual) {    
      ranges.subtract(maxV,minV);
      int maxDim=ranges.getIndexOfMaximum();
      for (i=0; i<3; i++) {
          scale.at(i)=ranges.at(maxDim)/ranges.at(i);
      }
    }

    double min=dist.minimum();
    double max=dist.maximum();
    double steps=(max-min)/double(p.nbTicks);
    dmatrix range(3,2);
    range.setColumn(0,minV);
    range.setColumn(1,maxV);

    if (p.scaleEqual) {
      minV.fill(min);
      maxV.fill(max);
      range.setColumn(0,minV);
      range.setColumn(1,maxV);
    }
    else {
      minV.emultiply(scale);
      maxV.emultiply(scale);
    }
    drawer.setRange(range);

    if (p.grid) {
      drawer.setColor(p.gridColor);
      for (i=0; i<=p.nbTicks; i++) {
        drawer.line3D(minV.at(0),minV.at(1)+i*steps,minV.at(2),
                      maxV.at(0),minV.at(1)+i*steps,minV.at(2));
        drawer.line3D(minV.at(0),minV.at(1),minV.at(2)+i*steps,
                      maxV.at(0),minV.at(1),minV.at(2)+i*steps);
        drawer.line3D(minV.at(0)+i*steps,minV.at(1),minV.at(2),
                      minV.at(0)+i*steps,maxV.at(1),minV.at(2));
        drawer.line3D(minV.at(0),minV.at(1),minV.at(2)+i*steps,
                      minV.at(0),maxV.at(1),minV.at(2)+i*steps);
        drawer.line3D(minV.at(0)+i*steps,minV.at(1),minV.at(2),
                      minV.at(0)+i*steps,minV.at(1),maxV.at(2));
        drawer.line3D(minV.at(0),minV.at(1)+i*steps,minV.at(2),
                      minV.at(0),minV.at(1)+i*steps,maxV.at(2));
      }
    }

    if (p.colouredAxes) {
      drawer.setColor(lti::Red);
      drawer.line3D(minV.at(0),minV.at(1),minV.at(2),
                    maxV.at(0),minV.at(1),minV.at(2));
      drawer.setColor(lti::Green);
      drawer.line3D(minV.at(0),minV.at(1),minV.at(2),
                    minV.at(0),maxV.at(1),minV.at(2));
      drawer.setColor(lti::Yellow);
      drawer.line3D(minV.at(0),minV.at(1),minV.at(2),
                    minV.at(0),minV.at(1),maxV.at(2));
    }

      // initialize the idToColor and idToMarker maps.
    int oldId;
    if (idsToColor->size()==0) {
      oldId=ids.at(0);
      drawer.setColor(p.colorMap[0]);
      (*idsToColor)[oldId]=p.colorMap[0];
      (*idsToMarker)[oldId]=p.markerMap[0];
      drawer.setStyle((*idsToMarker)[oldId].c_str());
    } 
    else {
      oldId=(*idsToColor->begin()).first;
      drawer.setColor(p.colorMap[0]);
      drawer.setStyle(p.markerMap[0].c_str());
    }
    std::map<int,rgbPixel>::iterator it;
    for (i=0; i<dist.rows(); i++) {
        // test if id has changed. If so set new color and marker type.
      if (oldId!=ids.at(i) ) {
        // first test if that id has already an assigned color and marker type
        it = idsToColor->find(ids.at(i));
        oldId=ids.at(i);
        if (it==idsToColor->end() ) {           
          (*idsToColor)[ids.at(i)] = 
              p.colorMap[idsToColor->size()%p.colorMap.size()];
          (*idsToMarker)[ids.at(i)] = p.markerMap[idsToMarker->size()/
                                         p.colorMap.size()%p.markerMap.size()];
        }
        // then set the color and the marker style for the actual id
        drawer.setColor( (*idsToColor)[ids.at(i)] );
        drawer.setStyle( (*idsToMarker)[ids.at(i)].c_str() );
      }
      drawer.marker3D(dist.at(i,0)*scale.at(0),dist.at(i,1)*scale.at(1),dist.at(i,2)*scale.at(2));
    }
    return ok;
  };




}
