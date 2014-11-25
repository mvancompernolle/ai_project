/*
 * Copyright (C) 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiSOFM2D.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 20.8.2002
 * revisions ..: $Id: ltiSOFM2D.cpp,v 1.9 2006/09/05 10:01:10 ltilib Exp $
 */

#include "ltiSOFM2D.h"
#include "ltiVarianceFunctor.h"
#include "ltiEigenSystem.h"
#include "ltiDistanceFunctor.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"
#include "ltiScramble.h"
#include "ltiLinearKernels.h"

#include <cstdio>

//  #include "ltiImage.h"
//  #include "ltiDraw.h"
//  #include "ltiViewer.h"

namespace lti {
  // --------------------------------------------------
  // SOFM2D::parameters
  // --------------------------------------------------

  // default constructor
  SOFM2D::parameters::parameters()
    : SOFM::parameters() {

    area = int();
    sizeX = int();
    sizeY = int();
    calculateSize = false;
  }

  // copy constructor
  SOFM2D::parameters::parameters(const parameters& other)
    : SOFM::parameters()  {
    copy(other);
  }

  // destructor
  SOFM2D::parameters::~parameters() {
  }

  // get type name
  const char* SOFM2D::parameters::getTypeName() const {
    return "SOFM2D::parameters";
  }

  // copy member

  SOFM2D::parameters&
    SOFM2D::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    SOFM::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    SOFM::parameters& (SOFM::parameters::* p_copy)
      (const SOFM::parameters&) =
      SOFM::parameters::copy;
    (this->*p_copy)(other);
# endif


      area = other.area;
      sizeX = other.sizeX;
      sizeY = other.sizeY;
      calculateSize = other.calculateSize;

    return *this;
  }

  // alias for copy member
  SOFM2D::parameters&
    SOFM2D::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  classifier::parameters* SOFM2D::parameters::clone() const {
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
  bool SOFM2D::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool SOFM2D::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"area",area);
      lti::write(handler,"sizeX",sizeX);
      lti::write(handler,"sizeY",sizeY);
      lti::write(handler,"calculateSize",calculateSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && SOFM::parameters::write(handler,false);
# else
    bool (SOFM::parameters::* p_writeMS)(ioHandler&,const bool) const =
      SOFM::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool SOFM2D::parameters::write(ioHandler& handler,
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
  bool SOFM2D::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool SOFM2D::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"area",area);
      lti::read(handler,"sizeX",sizeX);
      lti::read(handler,"sizeY",sizeY);
      lti::read(handler,"calculateSize",calculateSize);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && SOFM::parameters::read(handler,false);
# else
    bool (SOFM::parameters::* p_readMS)(ioHandler&,const bool) =
      SOFM::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool SOFM2D::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // SOFM2D
  // --------------------------------------------------

  // default constructor
  SOFM2D::SOFM2D()
    : SOFM(), eva1(0), eva2(0), eve1(0), eve2(0) {


    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // copy constructor
  SOFM2D::SOFM2D(const SOFM2D& other)
    : SOFM()  {
    copy(other);
  }

  // destructor
  SOFM2D::~SOFM2D() {
  }

  // returns the name of this type
  const char* SOFM2D::getTypeName() const {
    return "SOFM2D";
  }

  // copy member
  SOFM2D&
    SOFM2D::copy(const SOFM2D& other) {
      SOFM::copy(other);

    return (*this);
  }

  // alias for copy member
  SOFM2D&
    SOFM2D::operator=(const SOFM2D& other) {
    return (copy(other));
  }


  // clone member
  classifier* SOFM2D::clone() const {
    return new SOFM2D(*this);
  }

  // return parameters
  const SOFM2D::parameters&
    SOFM2D::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&classifier::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // -------------------------------------------------------------------
  // The train-methods!
  // -------------------------------------------------------------------

  // Calls the same method of the superclass.
  bool SOFM2D::train(const dmatrix& input, ivector& ids) {

    return SOFM::train(input, ids);
  }

  bool SOFM2D::train(const dmatrix& data) {

    // tracks the status of the training process.
    // if an error occurs set to false and use setStatusString()
    // however, training should continue, fixing the error as well as possible
    bool b=true;

    int i;

    const parameters& param=getParameters();

    // find the actual size of the grid
    if (param.calculateSize) {
      b = calcSize(data);
    } else {
      sizeX=param.sizeX;
      sizeY=param.sizeY;
    }

    // check whether one of the dimensions has negative or zero size
    // and try to fix by using alternate way of setting sizes.
    if (sizeX<=0 || sizeY<=0) {
      b=false;
      std::string err="Negative or zero size of one dimension";
      if (param.calculateSize) {
        if (param.area<=0) {
          err += "\narea is <= 0";
          if (param.sizeX>0 && param.sizeY>0) {
            sizeX=param.sizeX;
            sizeY=param.sizeY;
            err += "\nusing sizeX and sizeY instead";
          }
        }
      } else {
        if (param.sizeX<=0) {
          err += "\nsizeX <= 0";
        }
        if (param.sizeY<=0) {
          err += "\nsizeY <= 0";
        }
        if (param.area>0) {
          err += "\ncalculating size from area instead";
          calcSize(data);
          err += getStatusString();
        }
      }
      setStatusString(err.c_str());
    }

    // set grid to size
    grid.resize(sizeY*sizeX, data.columns());

    //set learn rates
    setLearnRates(data.rows());

    if (validProgressObject()) {
      getProgressObject().reset();
      std::string str("SOFM2D: Training using ");
      switch(param.metricType) {
        case parameters::L1:
          str += "L1 distance";
          break;
        case parameters::L2:
          str += "L2 distance";
          break;
        case parameters::Dot:
          str += "dot product";
          break;
        default:
          str += "unnamed method";
      }
      char buffer[256];
      sprintf(buffer," size of map %i x %i", sizeY, sizeX);
      str += std::string(buffer);
      getProgressObject().setTitle(str);
      getProgressObject().setMaxSteps(param.stepsOrdering+param.stepsConvergence+2);
    }



    //initialize grid
    if (validProgressObject()) {
      getProgressObject().step("initializing map");
    }
    b = initGrid(data);

    //training
    if (param.metricType == parameters::Dot) {
      trainDot(data);
    } else {
      trainDist(data);
    }

    if (validProgressObject()) {
      getProgressObject().step("training finished");
    }


    int nbOutputs = sizeX*sizeY;

    //Put the id information into the result object
    //Each output value has the id of its position in the matrix
    ivector tids(nbOutputs);
    for (i=0; i<nbOutputs; i++) {
      tids.at(i)=i;
    }
    outTemplate=outputTemplate(tids);

    return b;
  }


  // -------------------------------------------------------------------
  // The classify-method!
  // -------------------------------------------------------------------

  //TODO Usually this method does not need to be implemented use inherited
  // method from direct subclass of classifier (ie centroidClassifier) instead
  // In this case delete. Otherwise comment!

  bool
  SOFM2D::classify(const dvector& feature,
                   outputVector& result) const {

    dvector tResult;
    const parameters& param=getParameters();

    if (param.metricType==parameters::Dot) {
      l2Distance<double> dist;
      double normFeat=dist.apply(feature);
      dvector tFeat(feature);
      tFeat.divide(normFeat);
      grid.multiply(tFeat,tResult);
      tResult.add(1.);
      tResult.divide(2.);
    } else {
      double sig=1.;
      if (param.metricType==parameters::L1) {
        l1Distance<double> dist;
        dist.apply(grid,feature,tResult);
      } else if (param.metricType==parameters::L2) {
        l2Distance<double> dist;
        dist.apply(grid,feature,tResult);
      }
      int i;
      for (i=0; i<tResult.size(); i++) {
        tResult[i]=exp(-1*tResult[i]*tResult[i]/(sig*sig));
      }
    }

    bool b=outTemplate.apply(tResult, result);
    result.setWinnerAtMax();
    return b;
  }


  bool SOFM2D::write(ioHandler& handler,const bool complete) const {
    bool b(true);

    if (complete) {
      b = handler.writeBegin();
    }

    // write the standard data (output and parameters)
    SOFM::write(handler,false);

    if (b) {
      lti::write(handler,"sizeX",sizeX);
      lti::write(handler,"sizeY",sizeY);
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

  bool SOFM2D::read(ioHandler& handler,const bool complete) {
    bool b(true);

    if (complete) {
      b = handler.readBegin();
    }

    // read the standard data (output and parameters)
    SOFM::read(handler,false);

    if (b) {
      lti::read(handler,"sizeX",sizeX);
      lti::read(handler,"sizeY",sizeY);
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }



  bool SOFM2D::calcSize(const dmatrix& data) {

    bool b=true;

    varianceFunctor<double> varFunc;
    dmatrix cov;
    varFunc.covarianceMatrixOfRows(data, cov);
    jacobi<double> eigenFunc;
    jacobi<double>::parameters jp;
    jp.sort=true;
    eigenFunc.setParameters(jp);
    dvector eva;
    dmatrix eve;
    b = eigenFunc.apply(cov, eva, eve);

    if (b) {
      eva1=eva.at(0);
      eva2=eva.at(1);
      eve1=eve.getColumnCopy(0);
      eve2=eve.getColumnCopy(1);
    } else {
      setStatusString("could not find eigenvalues\n");
      b = false;
      eva1=1;
      eva2=1;
    }

    int area=getParameters().area;
    if (area<=0) {
      setStatusString("negative or zero area\n");
      return false;
    }

    double c=sqrt(double(area)/(eva1*eva2));

    int x1=int(eva1*c);
    int x2=int(ceil(eva1*c));
    int y1=int(eva2*c);
    int y2=int(ceil(eva2*c));

    sizeX=x1;
    if (abs(area-x1*y1)<abs(area-x1*y2)) {
      sizeY=y1;
    } else {
      sizeY=y2;
    }
    if (abs(area-x2*y1)<abs(area-sizeX*sizeY)) {
      sizeX=x2;
      sizeY=y1;
    }
    if (abs(area-x2*y2)<abs(area-sizeX*sizeY)) {
      sizeX=x2;
      sizeY=y2;
    }

    return b;

  }

  bool SOFM2D::initGrid(const dmatrix& data) {

    bool b=true;
    int i,j;

    const parameters& param=getParameters();


    if (param.initType == parameters::Linear) {
      //eigenvalues already calculated?
      if (eva1==0.) {
        varianceFunctor<double> varFunc;
        dmatrix cov;
        varFunc.covarianceMatrixOfRows(data, cov);
        jacobi<double> eigenFunc;
        jacobi<double>::parameters jp;
        jp.sort=true;
        eigenFunc.setParameters(jp);
        dvector eva;
        dmatrix eve;
        b = eigenFunc.apply(cov, eva, eve);

        if (b) {
          eva1=eva.at(0);
          eva2=eva.at(1);
          eve1=eve.getColumnCopy(0);
          eve2=eve.getColumnCopy(1);
        } else {
          setStatusString("could not find eigenvalues using random points\n");
          selectRandomPoints(data, sizeX*sizeY, grid);
          return false;
        }
      }
      meansFunctor<double> meanFunc;
      dvector mean;
      meanFunc.meanOfRows(data, mean);

      double x,y;
      dvector deltaX(eve1);
      deltaX.multiply(eva1/sizeX);
      dvector deltaY(eve2);
      deltaY.multiply(eva2/sizeY);
      dvector delta;

      for (i=0, y=-(double(sizeY-1)); i<sizeY; i++, y++) {
        for (j=0, x=-(double(sizeX-1)); j<sizeX; j++, x++) {
          delta.addScaled(x,deltaX,y,deltaY);
          grid.getRow(i*sizeX+j).add(mean,delta);
        }
      }
    } else {
      selectRandomPoints(data, sizeX*sizeY, grid);
    }
    return b;
  }

  bool SOFM2D::trainDist(const dmatrix& data) {

    bool b=true;
    int i,j,k,maxN;
    int startx, starty, stopx, stopy;
    kernel2D<double> facN;

    const parameters& param=getParameters();

    distanceFunctor<double>* dist = 0;
    if (param.metricType == parameters::L1) {
      dist = new l1Distance<double>();
    } else {
      dist = new l2Distance<double>();
    }
    distanceFunctor<double>::parameters dfp;
    dfp.rowWise=true;
    dist->setParameters(dfp);

    int step=0;
    int epoch;
    scramble<int> mix;
    ivector idx(data.rows());
    for (i=0; i<data.rows(); i++) {
      idx[i]=i;
    }

    dvector distances;
    dvector delta;
    int winner;

    char buffer[256];
    bool abort=false;

    // temp value needed for kernel init
    const double tfac=sqrt(-2*log(param.orderNeighborThresh));

    //ordering
    for (epoch=0; epoch<param.stepsOrdering; epoch++) {

      if (validProgressObject()) {
        sprintf(buffer,"ordering step %i",epoch);
        getProgressObject().step(buffer);
        abort = getProgressObject().breakRequested();
        }
      if (abort) return b;

      mix.apply(idx);
      for (i=0; i<idx.size(); i++, step++) {
        const dvector& curr = data.getRow(idx[i]);
        dist->apply(grid, curr, distances);
        winner=distances.getIndexOfMinimum();

        maxN=static_cast<int>(sigma*tfac);
        getNeighborhoodKernel(maxN, facN);

        //find bounds
        if (winner%sizeX-maxN < 0) {
          startx=-winner%sizeX;
        } else {
          startx=-maxN;
        }
        if (winner%sizeX+maxN > sizeX) {
          stopx=sizeX-winner%sizeX;
        } else {
          stopx=maxN;
        }
        if (winner/sizeX-maxN < 0) {
          starty=-winner/sizeX;
        } else {
          starty=-maxN;
        }
        if (winner/sizeX+maxN > sizeY) {
          stopy=sizeY-winner/sizeX;
        } else {
          stopy=maxN;
        }
        for (j=starty; j<stopy; j++) {
          for (k=startx; k<stopx; k++) {
            if (facN.at(j,k)==0.) {
              continue;
            }
            delta.subtract(curr, grid[winner+j*sizeX+k]);
            grid[winner+j*sizeX+k].addScaled(lrOrder*facN.at(j,k),delta);
          }
        }


        lrOrder-=lrOrderDelta;
        sigma-=sigmaDelta;
      }
    }

    // convergence training

    // neighborhood is fixed: calc matrix of factors.
    maxN=static_cast<int>(sigma*tfac);
    getNeighborhoodKernel(maxN, facN);

    double lrC=lrConvergeA/lrConvergeB;
    step=0;
    for (epoch=0; epoch<param.stepsConvergence; epoch++) {

      if (validProgressObject()) {
        sprintf(buffer,"convergence step %i",epoch);
        getProgressObject().step(buffer);
        abort = getProgressObject().breakRequested();
        }
      if (abort) return b;

      mix.apply(idx);
      for (i=0; i<idx.size(); i++, step++) {
        const dvector& curr = data.getRow(idx[i]);
        //find winner
        dist->apply(grid, curr, distances);
        winner=distances.getIndexOfMinimum();
        //find bounds
        if (winner%sizeX-maxN < 0) {
          startx=-winner%sizeX;
        } else {
          startx=-maxN;
        }
        if (winner%sizeX+maxN > sizeX) {
          stopx=sizeX-winner%sizeX;
        } else {
          stopx=maxN;
        }
        if (winner/sizeX-maxN < 0) {
          starty=-winner/sizeX;
        } else {
          starty=-maxN;
        }
        if (winner/sizeX+maxN > sizeY) {
          stopy=sizeY-winner/sizeX;
        } else {
          stopy=maxN;
        }
        for (j=starty; j<stopy; j++) {
          for (k=startx; k<stopx; k++) {
            if (facN.at(j,k)==0.) {
              continue;
            }
            delta.subtract(curr, grid[winner+j*sizeX+k]);
            grid[winner+j*sizeX+k].addScaled(lrC*facN.at(j,k),delta);
          }
        }
        lrC=lrConvergeA/(step+lrConvergeB);
      }
    }
    delete dist;
    return b;
  }

  bool SOFM2D::trainDot(const dmatrix& data) {

    bool b=true;

    int i,j,k,maxN;
    int startx, starty, stopx, stopy;
    kernel2D<double> facN;

    l2Distance<double> dist;
    l2Distance<double>::parameters dfp;
    dfp.rowWise=true;
    dist.setParameters(dfp);

    const parameters& param=getParameters();

    int step=0;
    int epoch;
    scramble<int> mix;
    ivector idx(data.rows());
    for (i=0; i<data.rows(); i++) {
      idx[i]=i;
    }

    dvector prod;
    dvector sum;
    int winner;

    //normalize grid
    dvector norms;
    b = b && dist.apply(grid,norms);
    for (i=0; i<grid.rows(); i++) {
      grid.getRow(i).divide(norms[i]);
    }

    // temp value needed for kernel init
    const double tfac=sqrt(-2*log(param.orderNeighborThresh));

    char buffer[256];
    bool abort=false;
    //ordering
    for (epoch=0; epoch<param.stepsOrdering; epoch++) {

      if (validProgressObject()) {
        sprintf(buffer,"ordering step %i",epoch);
        getProgressObject().step(buffer);
        abort = getProgressObject().breakRequested();
        }
      if (abort) return b;

      mix.apply(idx);
      for (i=0; i<idx.size(); i++, step++) {
        const dvector& curr = data.getRow(idx[i]);
        //find winner
        grid.multiply(curr, prod);
        winner=prod.getIndexOfMaximum();

        //find size and init neighborhood function
        maxN=static_cast<int>(sigma*tfac);
        getNeighborhoodKernel(maxN, facN);

        //find bounds
        if (winner%sizeX-maxN < 0) {
          startx=-winner%sizeX;
        } else {
          startx=-maxN;
        }
        if (winner%sizeX+maxN > sizeX) {
          stopx=sizeX-winner%sizeX;
        } else {
          stopx=maxN;
        }
        if (winner/sizeX-maxN < 0) {
          starty=-winner/sizeX;
        } else {
          starty=-maxN;
        }
        if (winner/sizeX+maxN > sizeY) {
          stopy=sizeY-winner/sizeX;
        } else {
          stopy=maxN;
        }
        for (j=starty; j<stopy; j++) {
          for (k=startx; k<stopx; k++) {
            if (facN.at(j,k)==0.) {
              continue;
            }
            dvector& winnerRow=grid[winner+j*sizeX+k];
            winnerRow.addScaled(lrOrder*facN.at(j,k), curr);
            winnerRow.divide(dist.apply(winnerRow));
          }
        }


        lrOrder-=lrOrderDelta;
        sigma-=sigmaDelta;
      }
    }

    // convergence training

    // neighborhood is fixed: calc matrix of factors.
    maxN=static_cast<int>(sigma*tfac);
    getNeighborhoodKernel(maxN, facN);

    double lrC=lrConvergeA/lrConvergeB;
    step=0;
    for (epoch=0; epoch<param.stepsConvergence; epoch++) {

      if (validProgressObject()) {
        sprintf(buffer,"convergence step %i",epoch);
        getProgressObject().step(buffer);
        abort = getProgressObject().breakRequested();
        }
      if (abort) return b;

      mix.apply(idx);
      for (i=0; i<idx.size(); i++, step++) {
        const dvector& curr = data.getRow(idx[i]);
        //find winner
        grid.multiply(curr, prod);
        winner=prod.getIndexOfMaximum();
        //find bounds
        if (winner%sizeX-maxN < 0) {
          startx=-winner%sizeX;
        } else {
          startx=-maxN;
        }
        if (winner%sizeX+maxN > sizeX) {
          stopx=sizeX-winner%sizeX;
        } else {
          stopx=maxN;
        }
        if (winner/sizeX-maxN < 0) {
          starty=-winner/sizeX;
        } else {
          starty=-maxN;
        }
        if (winner/sizeX+maxN > sizeY) {
          stopy=sizeY-winner/sizeX;
        } else {
          stopy=maxN;
        }
        for (j=starty; j<stopy; j++) {
          for (k=startx; k<stopx; k++) {
            if (facN.at(j,k)==0.) {
              continue;
            }
            dvector& winnerRow=grid[winner+j*sizeX+k];
            winnerRow.addScaled(lrC*facN.at(j,k), curr);
            winnerRow.divide(dist.apply(winnerRow));
          }
        }
        lrC=lrConvergeA/(step+lrConvergeB);
      }
    }

    return b;

  }


  void SOFM2D::getNeighborhoodKernel(const int& maxN, kernel2D<double>& facN) {
    int i,j;
    double dx,dy;
    const parameters& param=getParameters();
    facN.resize(-maxN, -maxN, maxN, maxN);
    facN.at(0,0)=1.;
    for (i=1, dy=1.; i<maxN; i++, dy++) {
      facN.at(0,i) =exp(-(dy*dy)/(2*sigma*sigma));
      facN.at(0,-i)=facN.at(0,i);
      facN.at(i,0) =facN.at(0,i);
      facN.at(-i,0)=facN.at(0,i);
    }
    for (i=1, dy=1.; i<maxN; i++, dy++) {
      facN.at(i,i) =exp(-(dy*dy)/(sigma*sigma));
      if (facN.at(i,i)<param.orderNeighborThresh) {
        facN.at(i,i)=0.;
        break;
      } else {
        facN.at(i,-i) =facN.at(i,i);
        facN.at(-i,-i)=facN.at(i,i);
        facN.at(-i,i) =facN.at(i,i);
      }
    }
    for (i=1, dy=1.; i<maxN; i++, dy++) {
      for (j=1, dx=1.; j<i; j++, dx++) {
        facN.at(i,j) =exp(-(dx*dx+dy*dy)/(2*sigma*sigma));
        if (facN.at(i,j)<param.orderNeighborThresh) {
          facN.at(i,j)=0.;
          break;
        } else {
          facN.at(i,-j) =facN.at(i,j);
          facN.at(-i,-j)=facN.at(i,j);
          facN.at(-i,j) =facN.at(i,j);
          facN.at(j,i)  =facN.at(i,j);
          facN.at(j,-i) =facN.at(i,j);
          facN.at(-j,-i)=facN.at(i,j);
          facN.at(-j,i) =facN.at(i,j);
        }
      }
    }
  }



}
