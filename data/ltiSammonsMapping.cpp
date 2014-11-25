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
 * file .......: ltiSammonsMapping.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 9.10.2002
 * revisions ..: $Id: ltiSammonsMapping.cpp,v 1.10 2006/09/05 10:01:43 ltilib Exp $
 */

#include "ltiSammonsMapping.h"
#include "ltiDistanceFunctor.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"
#include "ltiUniformDist.h"
#include "ltiMeansFunctor.h"
#include "ltiPCA.h"
#include "ltiGenericVector.h"

#include <cstdio>

namespace lti {
  // --------------------------------------------------
  // sammonsMapping::parameters
  // --------------------------------------------------

  // default constructor
  sammonsMapping::parameters::parameters()
    : functor::parameters() {

    dimensions = 2;
    steps = 200;
    errorThresh=0.;
    alpha=0.35;
    initType = PCA;
    initBox.resize(dimensions,1.);
    searchType = Steepest;
    mu = 0.1;
  }

  // copy constructor
  sammonsMapping::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  sammonsMapping::parameters::~parameters() {
  }

  // get type name
  const char* sammonsMapping::parameters::getTypeName() const {
    return "sammonsMapping::parameters";
  }

  // copy member

  sammonsMapping::parameters&
    sammonsMapping::parameters::copy(const parameters& other) {
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


      dimensions = other.dimensions;
      steps = other.steps;
      errorThresh = other.errorThresh;
      alpha = other.alpha;
      initType = other.initType;
      initBox.copy(other.initBox);
      searchType = other.searchType;
      mu = other.mu;


    return *this;
  }

  // alias for copy member
  sammonsMapping::parameters&
    sammonsMapping::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* sammonsMapping::parameters::clone() const {
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
  bool sammonsMapping::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool sammonsMapping::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      switch(initType) {
          case Random:
            lti::write(handler,"initType","Random");
            break;
          case PCA:
            lti::write(handler,"initType","PCA");
            break;
          default:
            lti::write(handler,"initType","PCA");
      }

      switch(searchType) {
          case Steepest:
            lti::write(handler,"searchType","Steepest");
            break;
          case Gradient:
            lti::write(handler,"searchType","Gradient");
            break;
          case Momentum:
            lti::write(handler,"searchType","Momentum");
            break;
          default:
            lti::write(handler,"searchType","Steepest");
      }

      lti::write(handler,"dimensions",dimensions);
      lti::write(handler,"steps",steps);
      lti::write(handler,"errorThresh",errorThresh);
      lti::write(handler,"alpha",alpha);
      lti::write(handler,"mu",mu);
      lti::write(handler,"initBox",initBox);
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
  bool sammonsMapping::parameters::write(ioHandler& handler,
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
  bool sammonsMapping::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool sammonsMapping::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      std::string str;

      lti::read(handler,"initType",str);
      if (str=="Random") {
        initType = Random;
      } else if (str=="PCA") {
        initType = PCA;
      } else {
        initType = PCA;
      }

      lti::read(handler,"searchType",str);
      if (str=="Steepest") {
        searchType = Steepest;
      } else if (str=="Gradient") {
        searchType = Gradient;
      } else if (str=="Momentum") {
        searchType = Momentum;
      } else {
        searchType = Steepest;
      }

      lti::read(handler,"dimensions",dimensions);
      lti::read(handler,"steps",steps);
      lti::read(handler,"errorThresh",errorThresh);
      lti::read(handler,"alpha",alpha);
      lti::read(handler,"mu",mu);
      lti::read(handler,"initBox",initBox);
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
  bool sammonsMapping::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // sammonsMapping
  // --------------------------------------------------

  // default constructor
  sammonsMapping::sammonsMapping()
    : functor(),progressBox(0) {


    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);


  }

  // copy constructor
  sammonsMapping::sammonsMapping(const sammonsMapping& other)
    : functor()  {
    copy(other);
  }

  // destructor
  sammonsMapping::~sammonsMapping() {
    delete progressBox;
    progressBox = 0;
  }

  // returns the name of this type
  const char* sammonsMapping::getTypeName() const {
    return "sammonsMapping";
  }

  // copy member
  sammonsMapping&
  sammonsMapping::copy(const sammonsMapping& other) {
    functor::copy(other);

    if (other.validProgressObject()) {
      setProgressObject(other.getProgressObject());
    } else {
      progressBox = 0;
    }

    return (*this);
  }

  // alias for copy member
  sammonsMapping&
    sammonsMapping::operator=(const sammonsMapping& other) {
    return (copy(other));
  }


  // clone member
  functor* sammonsMapping::clone() const {
    return new sammonsMapping(*this);
  }

  // return parameters
  const sammonsMapping::parameters&
    sammonsMapping::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // set the progress object
  void sammonsMapping::setProgressObject(const progressInfo& progBox) {
    removeProgressObject();
    progressBox = progBox.clone();
  }

  // remove the active progress object
  void sammonsMapping::removeProgressObject() {
    delete progressBox;
    progressBox = 0;
  }

  // valid progress object
  bool sammonsMapping::validProgressObject() const {
    return notNull(progressBox);
  }

  progressInfo& sammonsMapping::getProgressObject() {
    return *progressBox;
  }

  const progressInfo& sammonsMapping::getProgressObject() const {
    return *progressBox;
  }


  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  // On copy apply for type dmatrix!
  bool sammonsMapping::apply(const dmatrix& src,dmatrix& dest) const {
    double error;
    return apply(src,dest,error);
  }


  // On copy apply for type dmatrix!
  bool sammonsMapping::apply(const dmatrix& src,dmatrix& dest,
                             double& error) const {

    bool b=true;
    const parameters& param=getParameters();
    int dim=param.dimensions;
    int maxSteps=param.steps;
    double maxError=param.errorThresh;
    int i,j,k;
    int r=src.rows();

    l2Distance<double> distFunc;
    l2Distance<double>::parameters dp;
    dp.rowWise=true;
    distFunc.setParameters(dp);

    if (validProgressObject()) {
      progressBox->reset();
      std::string str("Sammon's mapping using ");
      switch (param.searchType) {
          case parameters::Gradient:
            str += "gradient descent";
            break;
          case parameters::Momentum:
            str += "gradient descent with momentum";
            break;
          case parameters::Steepest:
            str += "steepest descent";
            break;
          default:
            str += "unknown method";
      }
      progressBox->setTitle(str);
      progressBox->setMaxSteps(maxSteps+3);
      progressBox->step("calculating distance matrix");
    }

    // contains distances between all points in src space
    genericVector<bool> leaveMe(r,false);
    dmatrix distances(r,r,0.);
    for (i=0; i<r; i++) {
      if (leaveMe.at(i)) continue;
      for (j=i+1; j<r; j++) {
        if (leaveMe.at(j)) continue;
        distances.at(i,j)=distFunc.apply(src.getRow(i),src.getRow(j));
        distances.at(j,i)=distances.at(i,j);
        if (distances[i][j]==0) {
          std::cerr << "rows " << i << " and " << j
                    << " are equal. Ignoring row " << j << std::endl;
          leaveMe.at(j)=true;
        }
      }
    }

    double errorC=distances.sumOfElements()/2.;

    //set size of dest
    dest.resize(r, dim, 0.);

    if (validProgressObject()) {
      progressBox->step("initializing mapping");
    }

    //initialize dest
    switch (param.initType) {
        case parameters::Random:
          initRandom(dest);
          break;
        case parameters::PCA:
          initPca(src,dest);
          break;
        default:
          initPca(src,dest);
    }

    int step=0;
    double dImg;
    double fac;
    dvector diffE(dim);
    char buffer[256];

    error=0.;
    for (i=0; i<r; i++) {
      if (leaveMe.at(i)) continue;
      for (j=i+1; j<r; j++) {
        if (leaveMe.at(j)) continue;
        error+=pow(distances[i][j]-distFunc.apply(dest[i],dest[j]),2)
          /distances[i][j];
      }
    }
    error/=errorC;

    switch (param.searchType) {

        case parameters::Gradient: {

          while (error>maxError && step < maxSteps) {
            if (validProgressObject()) {
              sprintf(buffer,"error = %f",error);
              progressBox->step(buffer);
            }
            for (i=0; i<r; i++) {
              if (leaveMe.at(i)) continue;
              dvector& curr=dest.getRow(i);
              for (j=0; j<r; j++) {
                if (leaveMe.at(j)) continue;
                if (j==i) continue;
                dImg=distFunc.apply(curr, dest.getRow(j));
                double& dStar=distances[i][j];
                fac=(dStar-dImg)/(dStar*dImg);
                for (k=0; k<dim; k++) {
                  diffE[k]+=fac*(dest[i][k]-dest[j][k]);
                }
              }
              curr.addScaled(2.*param.alpha/errorC,diffE);
              diffE.fill(0.);
            }
            error=0.;
            for (i=0; i<r; i++) {
              if (leaveMe.at(i)) continue;
              for (j=i+1; j<r; j++) {
                if (leaveMe.at(j)) continue;
                error+=pow(distances[i][j]-distFunc.apply(dest[i],dest[j]),2)
                  /distances[i][j];
              }
            }
            error/=errorC;
            step++;
          }
        }

        case parameters::Momentum: {

          dmatrix delta(r,dim,0.);
          while (error>maxError && step < maxSteps) {
            if (validProgressObject()) {
              sprintf(buffer,"error = %f",error);
              progressBox->step(buffer);
            }
            for (i=0; i<r; i++) {
              if (leaveMe.at(i)) continue;
              dvector& curr=dest.getRow(i);
              for (j=0; j<r; j++) {
                if (leaveMe.at(j)) continue;
                if (j==i) continue;
                dImg=distFunc.apply(curr, dest.getRow(j));
                double& dStar=distances[i][j];
                fac=(dStar-dImg)/(dStar*dImg);
                for (k=0; k<dim; k++) {
                  diffE[k]+=fac*(dest[i][k]-dest[j][k]);
                }
              }
              diffE.multiply(-2./errorC);
              delta[i].multiply(param.mu);
              delta[i].addScaled(param.alpha,diffE);
              curr.subtract(delta[i]);
              diffE.fill(0.);
            }
            error=0.;
            for (i=0; i<r; i++) {
              if (leaveMe.at(i)) continue;
              for (j=i+1; j<r; j++) {
                if (leaveMe.at(j)) continue;
                error+=pow(distances[i][j]-distFunc.apply(dest[i],dest[j]),2)
                  /distances[i][j];
              }
            }
            error/=errorC;
            step++;
          }
        }

        case parameters::Steepest:
        default: {

          double diffDim,diffE2norm;
          double dSub,dProd;
          dvector diffE2(dim);
          while (error>maxError && step < maxSteps) {
            if (validProgressObject()) {
              sprintf(buffer,"error = %f",error);
              progressBox->step(buffer);
            }
            for (i=0; i<r; i++) {
              if (leaveMe.at(i)) continue;
              dvector& curr=dest.getRow(i);
              for (j=0; j<r; j++) {
                if (leaveMe.at(j)) continue;
                if (j==i) continue;
                dImg=distFunc.apply(curr, dest.getRow(j));
                double& dStar=distances[i][j];
                dSub=dStar-dImg;
                dProd=dStar*dImg;
                fac=dSub/dProd;
                for (k=0; k<dim; k++) {
                  diffDim=dest[i][k]-dest[j][k];
                  diffE[k]+=fac*diffDim;
                  diffE2[k]+=(dSub-diffDim*diffDim/dImg*(1.+dSub/dImg))/dProd;
                }
              }
              diffE2norm=distFunc.apply(diffE2);
              if (diffE2norm==0) {
                std::cerr << "second derivative is zero\n";
                diffE2norm=1.E-4;
              }
              curr.addScaled(param.alpha/diffE2norm, diffE);
              diffE.fill(0.);
              diffE2.fill(0.);
            }
            error=0.;
            for (i=0; i<r; i++) {
              if (leaveMe.at(i)) continue;
              for (j=i+1; j<r; j++) {
                if (leaveMe.at(j)) continue;
                error+=pow(distances[i][j]-distFunc.apply(dest[i],dest[j]),2)
                  /distances[i][j];
              }
            }
            error/=errorC;
            step++;
          }
        }
    }

    meansFunctor<double> meanFunc;
    dvector destMean;
    meanFunc.meanOfRows(dest,destMean);
    for (i=0;i<r;i++) {
      dest[i].subtract(destMean);
    }

    if (validProgressObject()) {
      sprintf(buffer,"done. error = %f",error);
      progressBox->step(buffer);
    }


    return b;
  }


  void sammonsMapping::initRandom(dmatrix& dest) const {
    int i,j;
    uniformDistribution rand;
    uniformDistribution::parameters randP;
    randP.lowerLimit=0.;
    const dvector& box=getParameters().initBox;
    for (j=0; j<dest.columns(); j++) {
      randP.upperLimit=box[j];
      for (i=0; i<dest.rows(); i++) {
        rand.setParameters(randP);
        dest.at(i,j)=rand.draw();
      }
    }
  }

  void sammonsMapping::initPca(const dmatrix& data, dmatrix& dest) const {

    principalComponents<double> pca;
    pca.setDimension(dest.columns());

    pca.apply(data,dest);

  }


}
