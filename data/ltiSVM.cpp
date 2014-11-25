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
 */


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiSvm.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.2001
 * revisions ..: $Id: ltiSVM.cpp,v 1.9 2006/09/05 10:01:33 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiSVM.h"
#include <limits>
#include <cstdio>
#include "ltiLinearKernel.h"
#include "ltiSigmoidKernel.h"
#include "ltiRadialKernel.h"
#include "ltiPolynomialKernel.h"
#include "ltiSort.h"
#include "ltiSTLIoInterface.h"
#include "ltiMeansFunctor.h"
#include "ltiVarianceFunctor.h"

// This stupid so-called C++ compiler from MS sucks!
#ifdef _LTI_MSC_6
#undef min
#undef max
#undef abs
#endif

#undef _LTI_DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  // --------------------------------------------------
  // svm::parameters
  // --------------------------------------------------

  struct instancesSVMKernels {
    static linearKernel     instanceLinearKernel;
    static sigmoidKernel    instanceSigmoidKernel;
    static radialKernel     instanceRadialKernel;
    static polynomialKernel instancePolynomialKernel;
  };

  linearKernel      instancesSVMKernels::instanceLinearKernel;
  sigmoidKernel     instancesSVMKernels::instanceSigmoidKernel;
  radialKernel      instancesSVMKernels::instanceRadialKernel;
  polynomialKernel  instancesSVMKernels::instancePolynomialKernel;

  // initialise list of possible kernels
  const kernelFunctor<double>* svm::parameters::kernelArray[] = {
    &instancesSVMKernels::instanceLinearKernel,
    &instancesSVMKernels::instanceSigmoidKernel,
    &instancesSVMKernels::instanceRadialKernel,
    &instancesSVMKernels::instancePolynomialKernel,
    /*
    new linearKernel,
    new sigmoidKernel,
    new radialKernel,
    new polynomialKernel,
    */
    0
  };

  objectFactory<kernelFunctor<double> >
  svm::parameters::kfa(svm::parameters::kernelArray);

  // default constructor
  svm::parameters::parameters()
    : supervisedInstanceClassifier::parameters() {

    nSupport = 0;
    C=1;
    bias=1;
    tolerance=1e-3;
    epsilon=1e-12;
    sumToOne=false;
    //useKernelVector=false;
    kernel=new linearKernel;
    destroyKernel=true;
    usePairwise=false;
    normalizeData=false;
  }

  // copy constructor
  svm::parameters::parameters(const parameters& other)
    : supervisedInstanceClassifier::parameters(), kernel(0)  {
    copy(other);
  }

  // destructor
  svm::parameters::~parameters() {
    if (kernel != 0 && destroyKernel) {
      delete kernel;
      kernel=0;
    }
  }

  void svm::parameters::setKernel(const kernelFunctor<double>& k) {
    if (kernel != 0 && destroyKernel) {
      delete kernel;
    }
    kernel=dynamic_cast<kernelFunctor<double>*>(k.clone());
    destroyKernel=true;
  }


  void svm::parameters::attachKernel(kernelFunctor<double>* k) {
    if (kernel != 0 && destroyKernel) {
      delete kernel;
    }
    kernel=k;
    destroyKernel=true;
  }

  void svm::parameters::useExternalKernel(kernelFunctor<double>* k) {
    if (kernel != 0 && destroyKernel) {
      delete kernel;
    }
    kernel=k;
    destroyKernel=false;
  }

  kernelFunctor<double>*
  svm::parameters::createKernel(const std::string& name) const {
    return kfa.newInstance(name);
  }

  // get type name
  const char* svm::parameters::getTypeName() const {
    return "svm::parameters";
  }

  // copy member

  svm::parameters&
    svm::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    supervisedInstanceClassifier::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    supervisedInstanceClassifier::parameters& (supervisedInstanceClassifier::parameters::* p_copy)
      (const supervisedInstanceClassifier::parameters&) =
      supervisedInstanceClassifier::parameters::copy;
    (this->*p_copy)(other);
# endif

    _lti_debug("COPY svm::p\n");
    nSupport = other.nSupport;
    C=other.C;
    bias=other.bias;
    tolerance=other.tolerance;
    epsilon=other.epsilon;
    sumToOne=other.sumToOne;
    usePairwise=other.usePairwise;
    normalizeData=other.normalizeData;
    _lti_debug("C1");

    //useKernelVector=other.useKernelVector;

    if (kernel != 0 && destroyKernel) {
      delete kernel;
      kernel=0;
    }
    _lti_debug("C2");
    if (other.kernel != 0) {
      _lti_debug("C2a");
      kernel=dynamic_cast<kernelFunctor<double>*>(other.kernel->clone());
    } else {
      _lti_debug("C2b");
      kernel=0;
    }
    destroyKernel=true;
    _lti_debug("C3\n");

    return *this;
  }

  // alias for copy member
  svm::parameters&
    svm::parameters::operator=(const parameters& other) {
    copy(other);

    return *this;
  }

  // clone member
  classifier::parameters* svm::parameters::clone() const {
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
  bool svm::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool svm::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      if (kernel != 0) {
        std::string kernelName;
        cn.get(kernel,kernelName);
        b=b && lti::write(handler, "haveKernel", true);
        b=b && lti::write(handler, "kernelType", kernelName);
        b=b && lti::write(handler, "kernelParam", kernel->getParameters());
      } else {
        b=b && lti::write(handler, "haveKernel", false);
        b=b && lti::write(handler, "kernelType", "unknown");
      }
      b=b && lti::write(handler,"nSupport",nSupport);
      b=b && lti::write(handler, "C", C);
      b=b && lti::write(handler, "bias", bias);
      b=b && lti::write(handler, "tolerance", tolerance);
      b=b && lti::write(handler, "epsilon", epsilon);
      b=b && lti::write(handler, "sumToOne", sumToOne);
      b=b && lti::write(handler, "usePairwise", usePairwise);
      b=b && lti::write(handler, "normalizeData", normalizeData);
      //lti::write(handler, "useKernelVector",useKernelVector);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && supervisedInstanceClassifier::parameters::write(handler,false);
# else
    bool (supervisedInstanceClassifier::parameters::* p_writeMS)(ioHandler&,const bool) const =
      supervisedInstanceClassifier::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool svm::parameters::write(ioHandler& handler,
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
  bool svm::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool svm::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string tmps;
      if (kernel != 0 && destroyKernel) {
        delete kernel;
      }
      bool ready;
      b=b && lti::read(handler, "haveKernel", ready);
      b=b && lti::read(handler, "kernelType", tmps);
      if (tmps != "unknown") {
        kernel=kfa.newInstance(tmps);
        if (kernel != 0) {
          kernelFunctor<double>::parameters* kp=
            dynamic_cast<kernelFunctor<double>::parameters*>(kernel->getParameters().clone());
          if (ready) {
            b=b && lti::read(handler, "kernelParam", *kp);
            kernel->setParameters(*kp);
          }
          destroyKernel=true;
        } else {
          b=false;
        }
      } else {
        kernel=0;
        destroyKernel=false;
      }
      b=b && lti::read(handler,"nSupport",nSupport);
      b=b && lti::read(handler, "C", C);
      b=b && lti::read(handler, "bias", bias);
      b=b && lti::read(handler, "tolerance", tolerance);
      b=b && lti::read(handler, "epsilon", epsilon);
      b=b && lti::read(handler, "sumToOne", sumToOne);
      b=b && lti::read(handler, "usePairwise", usePairwise);
      b=b && lti::read(handler, "normalizeData", normalizeData);
      //lti::read(handler, "useKernelVector",useKernelVector);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && supervisedInstanceClassifier::parameters::read(handler,false);
# else
    bool (supervisedInstanceClassifier::parameters::* p_readMS)(ioHandler&,const bool) =
      supervisedInstanceClassifier::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool svm::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // svm
  // --------------------------------------------------

  // default constructor
  svm::svm()
    : supervisedInstanceClassifier(),
      syseps(std::numeric_limits<double>::epsilon()) {

    _lti_debug("Creating SVM\n");
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    _lti_debug("Creating SVM 2\n");
    // set the default parameters
    setParameters(defaultParameters);
    _lti_debug("Creating SVM 3\n");

    // set pointers
    kernels.clear();
    _lti_debug("Creating SVM 4\n");
    trainData=0;
    target=0;
    nClasses=0;
    _lti_debug("Creating SVM 5\n");

    // default output object
    outTemplate=outputTemplate();
    _lti_debug("Creating SVM 6\n");
    _lti_debug("Done creating SVM\n");
  }

  // copy constructor
  svm::svm(const svm& other)
    : supervisedInstanceClassifier(), syseps(std::numeric_limits<double>::epsilon())  {
    copy(other);
  }

  // destructor
  svm::~svm() {
    if (target != 0) {
      delete target;
      target=0;
    }
    if (trainData != 0) {
      delete trainData;
      trainData=0;
    }
  }

  // returns the name of this type
  const char* svm::getTypeName() const {
    return "svm";
  }

  // copy member
  svm& svm::copy(const svm& other) {
    supervisedInstanceClassifier::copy(other);


    return (*this);
  }

  // alias for copy member
  svm& svm::operator=(const svm& other) {
    return (copy(other));
  }


  // clone member
  classifier* svm::clone() const {
    return new svm(*this);
  }

  // return parameters
  const svm::parameters&
    svm::getParameters() const {
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

  void svm::buildIdMaps(const ivector& ids) {
    int j=0;
    // create reverse id map
    idMap.clear();
    for (int i=0; i<ids.size(); i++) {
      if (idMap.find(ids.at(i)) == idMap.end()) {
      _lti_debug("Mapping external id " << ids.at(i) << " to " << j << std::endl);
        rIdMap[j]=ids.at(i);
        idMap[ids.at(i)]=j++;
      }
    }

    nClasses=j;
  }


  void
  svm::buildKernelVector(const std::map<int,kernelFunctor<double>* >& ks) {
    // create kernel vector
    kernels.resize(nClasses);
    std::map<int,kernelFunctor<double>*>::const_iterator ki;
    for (ki=ks.begin(); ki != ks.end(); ki++) {
       kernels[idMap[ki->first]]=ki->second;
    }
  }

  void svm::buildKernelVector() {
    // create kernel vector with all the same kernels
    kernels.resize(nClasses);
    kernelFunctor<double>* k=getParameters().kernel;
    for (int i=0; i<nClasses; i++) {
      kernels[i]=k;
    }
  }


  bool svm::train(const dmatrix& input, const ivector& ids,
                  const std::map<int,kernelFunctor<double>* >& ks) {

    buildIdMaps(ids);
    buildKernelVector(ks);

    return genericNormTrain(input,ids);
  }



  // Calls the same method of the superclass.
  bool svm::train(const dmatrix& input, const ivector& ids) {

    buildIdMaps(ids);
    buildKernelVector();

    return genericNormTrain(input,ids);
  }


  bool svm::genericNormTrain(const dmatrix& input, const ivector& ids) {
    if (getParameters().normalizeData) {
      meansFunctor<double> mf;
      mf.meanOfRows(input,offset);
      varianceFunctor<double> vf;
      dvector vscale;
      vf.varianceOfRows(input,vscale);
      /*
      double m1=lti::abs(vscale.minimum());
      std::cerr << "M1 = " << m1 << "\n";
      double m2=lti::abs(vscale.maximum());
      double m3=lti::max(m1,m2);
      */
      scale=sqrt(max(abs(vscale.maximum()),abs(vscale.minimum())));
      _lti_debug("Scale = " << scale << "\n");
      dmatrix* data=new dmatrix(input);
      for (int i=0; i<data->rows(); i++) {
        dvector& item=data->getRow(i);
        item.subtract(offset);
        item.edivide(scale);
      }
      return genericTrain(*data,ids);
    } else {
      offset.resize(input.columns(),0,false,false);
      offset.fill(0.0);
      //scale.resize(input.columns(),0,false,false);
      //scale.fill(1.0);
      scale=1.0;
      return genericTrain(input,ids);
    }
  }




  void svm::makeTargets(const ivector& ids) {
    // expand each class label i to a vector v with v[j]=1 if j == i,
    // and j[j]=-1 if j != i
    srcIds=ids;
    dmatrix* t=new dmatrix(nClasses,ids.size(),-1.0);
    // iterate over training labels
    for (int i=0; i<t->columns(); i++) {
      t->at(idMap[ids.at(i)],i)=1;
    }
    if (target != 0) {
      delete target;
    }
    target=t;
  }

  void svm::rebuildTargets() {
    // expand each class label i to a vector v with v[j]=1 if j == i,
    // and j[j]=-1 if j != i
    dmatrix* t=new dmatrix(nClasses,srcIds.size(),-1.0);
    // iterate over training labels
    for (int i=0; i<t->columns(); i++) {
      t->at(idMap[srcIds.at(i)],i)=1;
    }
    if (target != 0) {
      delete target;
    }
    target=t;
  }



  // Calls the same method of the superclass.
  bool svm::genericTrain(const dmatrix& input, const ivector& ids) {

    char buffer[80];

    if (validProgressObject()) {
      getProgressObject().reset();
      getProgressObject().setTitle("SVM: Training");
      getProgressObject().setMaxSteps(nClasses);
    }

    bias.resize(nClasses,getParameters().bias,false,true);
    trainData=new dmatrix(input);
    alpha.resize(nClasses,input.rows(),0,false,true);
    makeTargets(ids);
    errorCache.resize(input.rows());

    const parameters& param=getParameters();

    C=param.C;
    tolerance=param.tolerance;
    epsilon=param.epsilon;
    bool abort=false;

    // train one SVM for each class
    for (int cid=0; cid<nClasses && !abort; cid++) {
      int numChanged=0;
      bool examineAll=true;

      currentTarget=&target->getRow(cid);
      currentClass=cid;
      currentAlpha=&alpha.getRow(cid);

      _lti_debug("Training class " << cid << "\n");

      fillErrorCache();

      while ((numChanged > 0 || examineAll) && !abort) {
        numChanged=0;
        if (examineAll) {
          // iterate over all alphas
          for (int i=0; i<trainData->rows(); i++) {
            if (examineExample(i)) {
              numChanged++;
            }
          }
          // next turn, look only at non-bound alphas
          examineAll=false;
        } else {
          // iterate over all non-0 and non-C alphas
          int *tmpAlpha=new int[alpha.getRow(cid).size()];
          int j=0,i=0;
          for (i=0; i<alpha.getRow(cid).size(); i++) {
            if (alpha.getRow(cid).at(i) != 0.0 &&
                alpha.getRow(cid).at(i) != C) {
              tmpAlpha[j++]=i;
            }
          }
          delete[] tmpAlpha;
          for (i=0; i<j; i++) {
            if (examineExample(i)) {
              numChanged++;
            }
          }
          // next turn, examine all if we did not succeed this time
          if (numChanged == 0) {
            examineAll=true;
          }
        }
      }
      // update progress info object
      if (validProgressObject()) {
        sprintf(buffer,"numChanged=%d, error=%f",numChanged,errorSum);
        getProgressObject().step(buffer);
        abort=abort || getProgressObject().breakRequested();
      }

     // now limit the number of support vectors
      // does not work yet, so disable it
      if (0) {
        int supnum=0;
        ivector index(currentAlpha->size());
        ivector newindex(currentAlpha->size());
        dvector newkey(currentAlpha->size());
        for (int i=0; i<currentAlpha->size(); i++) {
          if (currentAlpha->at(i) > 0) {
            supnum++;
          }
          index[i]=i;
        }
        if (supnum > param.nSupport && param.nSupport > 0) {
          lti::sort2<double> sorter;
          sorter.apply(*currentAlpha,index,newkey,newindex);

          int i;
          for (i=0; i<newkey.size() &&
                 lti::abs(newkey[i]) > std::numeric_limits<double>::epsilon(); i++) {
          }
          for (int j=i; j<currentAlpha->size()-param.nSupport; j++) {
            currentAlpha->at(newindex[j])=0;
          }
          _lti_debug("Final alpha: " << *currentAlpha << std::endl);
        }
      }
    }

    defineOutputTemplate();

    _lti_debug("alpha:\n" << alpha << "\n");

    // make sure that all lagrange multipliers are larger than
    // zero, otherwise we might get into trouble later
    alpha.apply(rectify);

    if (abort) {
      setStatusString("Training aborted by user!");
    }
    return !abort;
  }

  void svm::defineOutputTemplate() {

    ivector tids(rIdMap.size());
    unsigned int i=0;
    for (i=0; i<rIdMap.size(); i++) {
      tids.at(i)=rIdMap[i];
    }

    outTemplate=outputTemplate(tids);

//      output tmp(nClasses);

//      output::namesMap names;

//      char buf[32];
//      char fmt[32];

//      int dec=0;
//    int i=0;

//      for (int k=1; k<nClasses; k*=10) {
//        dec++;
//      }
//      sprintf(fmt,"%%0%dd",dec);

//      for (i=0; i<nClasses; i++) {
//        sprintf(buf,fmt,rIdMap[i]);
//        names[i]=buf;
//      }

//      tmp.setClassNames(names);

//      std::vector<outputElementProb>& ov=tmp.getOutputVector();
//      for (i=0; i<nClasses; i++) {
//        objectProb objProb;
//        _lti_debug("Assigning element " << i << " to " << rIdMap[i] << std::endl);
//        objProb.id = rIdMap[i];
//        objProb.prob = 1;
//        ov[i].objects.clear();
//        ov[i].objects.push_back(objProb);
//      }
//      setOutputObject(tmp);
  }



  double svm::objectiveFunction() {

    double s=0;
    dvector::const_iterator it=currentAlpha->begin();
    const dvector::const_iterator iend=currentAlpha->end();

    // sum over all lagrange multipliers
    for (; it != iend; it++) {
      s+=*it;
    }

    double t=0;
    for (int i=0; i<trainData->rows(); i++) {
      for (int j=0; j<trainData->rows(); j++) {
        t+=currentTarget->at(i)*currentTarget->at(j)*
          kernels[currentClass]->apply(trainData->getRow(i),
                                       trainData->getRow(j))*currentAlpha->at(i)*currentAlpha->at(j);
      }
    }
    s-=0.5*t;

    return s;
  }





  // custom training procedured
  bool svm::takeStep(const int& i1, const int& i2) {
    if (i1 == i2) {
      return false;
    }

    //debug("Taking step: " << i1 << " and " << i2 << "\n");

    // old alphas
    double alph1=currentAlpha->at(i1);
    // new alphas
    double a1,a2;
    double y1=currentTarget->at(i1);

    double e1=errorCache.at(i1);

    //debug("i1=" << i1 << ", y1=" << y1 << ", alpha1=" << alph1 << ", e1=" << e1 << "\n");

    double s=y1*y2;

    const double C=getParameters().C;


    double L,H;

    if (lti::abs(y1-y2) > syseps) {
      L=lti::max(0.0,alph2-alph1);
      H=lti::min(C,C+alph2-alph1);
    } else {
      L=lti::max(0.0,alph1+alph2-C);
      H=lti::min(C,alph1+alph2);
    }

    //debug("L=" << L << ", H=" << H << "\n");

    if (lti::abs(L-H) < syseps) {
      return false;
    }
    double k11=kernels[currentClass]->apply(trainData->getRow(i1),trainData->getRow(i1));
    double k12=kernels[currentClass]->apply(trainData->getRow(i1),trainData->getRow(i2));
    double k22=kernels[currentClass]->apply(trainData->getRow(i2),trainData->getRow(i2));

    double eta=2*k12-k11-k22;

    if (eta < 0) {
      //debug("eta < 0\n");
      a2=alph2-y2*(e1-e2)/eta;
      if (a2 < L) {
        a2=L;
      } else if (a2 > H) {
        a2=H;
      }
    } else {
      //debug("eta >= 0\n");
      currentAlpha->at(i2)=L;
      double low=objectiveFunction();
      currentAlpha->at(i2)=H;
      double high=objectiveFunction();
      currentAlpha->at(i2)=alph2;
      if (low > high+epsilon) {
        a2=L;
      } else if ( low < high-epsilon) {
        a2=H;
      } else {
        a2=alph2;
      }
    }
    //debug("Checking alpha2[" << i2 << "]: old is " << alph2 << ", new is " << a2 << "\n");
    if (lti::abs(a2-alph2) < epsilon*(a2+alph2+epsilon)) {
      return false;
    }
    a1=alph1+s*(alph2-a2);
    //debug("Checking alpha1[" << i1 << "]: old is " << alph1 << ", new is " << a1 << "\n");
    // update threshold to reflect change in lagrange multipliers
    double w1   = y1*(a1 - alph1);
    double w2   = y2*(a2 - alph2);
    double b1   = e1 + w1*k11 + w2*k12;
    double b2   = e2 + w1*k12 + w2*k22;
    double bold = bias[currentClass];

    bias[currentClass] += 0.5*(b1 + b2);
    updateBiasError(bias[currentClass]-bold);
    setAlpha(i1,a1);
    setAlpha(i2,a2);

    return true;
  }


  void svm::updateBiasError(const double& deltab) {
    for (int i=0; i<trainData->rows(); i++) {
      errorCache[i]-=deltab;
    }
    errorSum-=trainData->rows()*deltab;
  }


  void svm::setAlpha(const int &k, const double& a) {

    //debug("setting alpha[" << k << "]to " << a << "\n");

    double olda=currentAlpha->at(k);

    for (int i=0; i<trainData->rows(); i++) {
      double de=(a-olda)*currentTarget->at(k)*
        kernels[currentClass]->apply(trainData->getRow(k),trainData->getRow(i));
      errorCache[i]+=de;
      errorSum+=de;
    }

    currentAlpha->at(k)=a;
  }


  void svm::fillErrorCache() {
    // error is the result of the SVM minus the target

    errorSum=0;
    for (int i=0; i<errorCache.size(); i++) {
      errorCache[i]=0;
      for (int j=0; j<currentAlpha->size(); j++) {
        errorCache[i]+=
          currentAlpha->at(j)*currentTarget->at(j)*
          kernels[currentClass]->apply(trainData->getRow(j),trainData->getRow(i));
      }
      errorCache[i]-=bias[currentClass]+currentTarget->at(i);
      errorSum+=errorCache[i];
    }
  }



  bool svm::examineExample(const int& i2) {

    //debug("Examining sample " << i2 << ":\n");
    y2=currentTarget->at(i2);
    alph2=currentAlpha->at(i2);
    e2=errorCache.at(i2);

    double r2=e2*y2;

    //debug("i2=" << i2 << ", y2=" << y2 << ", alpha2=" << alph2 << ", e2=" << e2 << ", r2=" << r2 << "\n");
    // check KKT condition for example i2
    if ((r2 < -tolerance && alph2 < C) || (r2 > tolerance && alph2 > 0)) {
      // example violates KKT condition, choose it for optimization
      // init array of non-zero and non-C alphas
      int* tmpAlpha=new int[currentAlpha->size()];
      int j=0,i=0;
      for (i=0; i<currentAlpha->size(); i++) {
        if (currentAlpha->at(i) != 0.0 && currentAlpha->at(i) != C) {
          tmpAlpha[j++]=i;
        }
      }
      // is there more than one non-bound alpha?
      if (j > 1) {
        // yes, then choose one that is expected to
        // maximize the optimization step size
        double emax=std::numeric_limits<double>::min();
        int k=0;
        for (int i=0; i<errorCache.size(); i++) {
          if (errorCache.at(i) > emax) {
            k=i;
            emax=errorCache.at(i);
          }
        }
        if (takeStep(k,i2)) {
          delete[] tmpAlpha;
          return true;
        }
      }
      // iterate over all non-zero and non-C alphas
      // start at random index
      int start;
      if (j > 0) {
        start=rand()%j;
        for (int i=start,count=0; count<j; count++, i=(i+1)%j) {
          if (takeStep(tmpAlpha[i],i2)) {
            delete[] tmpAlpha;
            return true;
          }
        }
      }
      // iterate over all possible i
      start=rand()%currentAlpha->size();
      j=trainData->rows();
      int count;
      for (count=0,i=start; count<currentAlpha->size(); count++, i=(i+1)%currentAlpha->size()) {
        if (takeStep(i,i2)) {
          delete[] tmpAlpha;
          return true;
        }
      }
      delete[] tmpAlpha;
    }
    return false;
  }



  // -------------------------------------------------------------------
  // The classify-method!
  // -------------------------------------------------------------------

  //TODO Usually this method does not need to be implemented use inherited
  // method from direct subclass of classifier (ie centroidClassifier) instead
  // In this case delete. Otherwise comment!

  bool svm::classify(const dvector& feature, outputVector& output) const {

    dvector result(nClasses);

    bool donorm=getParameters().normalizeData;

    dvector tmp;
    const dvector* data;
    if (donorm) {
      tmp.subtract(feature,offset);
      tmp.edivide(scale);
      data=&tmp;
    } else {
      data=&feature;
    }

    bool reject=true;

    assert(static_cast<int>(kernels.size()) == nClasses);

    for (int currentClass=0; currentClass<nClasses; currentClass++) {
      double tmp=outputFunction(*data, currentClass);
      reject=reject && (tmp < 0);
      if (tmp < -1.0) {
        tmp=-1.0;
      } else if (tmp > 1.0) {
        tmp=1.0;
      }
      result[currentClass]=(tmp+1.0)/2.0;
    }

    //debug("result=" << result << "\n");

    if (getParameters().sumToOne) {
      double m1=result.minimum();
      result.add(-m1);
      double s=result.sumOfElements();
      if (lti::abs(s) > std::numeric_limits<double>::epsilon()) {
        result.multiply(1.0/s);
      } else {
        result.fill(0.0);
      }
    }

    return outTemplate.apply(result, output);
  }

  /*
  void svm::classify(const dvector& feature, dvector& result) const {

    result.resize(nClasses);

    bool reject=true;

    assert(static_cast<int>(kernels.size()) == nClasses);

    for (int currentClass=0; currentClass<nClasses; currentClass++) {
      double tmp=outputFunction(feature, currentClass);
      reject=reject && (tmp < 0);
      if (tmp < -1.0) {
        tmp=-1.0;
      } else if (tmp > 1.0) {
        tmp=1.0;
      }
      result[currentClass]=(tmp+1.0)/2.0;
    }

    //debug("result=" << result << "\n");

    if (getParameters().sumToOne) {
      double m1=result.minimum();
      result.add(-m1);
      double s=result.sumOfElements();
      if (lti::abs(s) > std::numeric_limits<double>::epsilon()) {
        result.multiply(1.0/s);
      } else {
        result.fill(0.0);
      }
    }
  }
  */

  // parameter shortcuts

  void svm::attachKernel(kernelFunctor<double>* k) {
    parameters p(getParameters());
    p.attachKernel(k);
    setParameters(p);
  }

  void svm::setKernel(const kernelFunctor<double>& k) {
    parameters p(getParameters());
    p.setKernel(k);
    setParameters(p);
  }

  void svm::useExternalKernel(kernelFunctor<double>* k) {
    parameters p(getParameters());
    p.useExternalKernel(k);
    setParameters(p);
  }

  //
  // IO methods
  //

  /*
   * write the classifier in the given ioHandler
   */
  bool svm::write(ioHandler& handler,const bool complete) const {
    bool b=true;
    if (complete) {
      b=handler.writeBegin();
    }
    b = b && supervisedInstanceClassifier::write(handler,false);
    if (b) {
      b=b && lti::write(handler, "nClasses",nClasses);
      b=b && lti::write(handler, "alpha",alpha);
      b=b && lti::write(handler, "vectors",*trainData);
      b=b && lti::write(handler, "idMap",idMap);
      b=b && lti::write(handler, "rIdMap",rIdMap);
      b=b && lti::write(handler, "srcIds",srcIds);
      b=b && lti::write(handler, "bias", bias);
      b=b && lti::write(handler, "nKernels", kernels.size());
      b=b && handler.writeBegin();
      b=b && lti::write(handler, "kernels");
      b=b && handler.writeDataSeparator();
      className cn;
      std::string kname;
      // always write the complete kernel vector, so we do not need
      // to remember which training methods has been used.
      for (unsigned int i=0; i<kernels.size(); i++) {
        if (kernels[i] != 0) {
          cn.get(kernels[i],kname);
          lti::write(handler,"name",kname);
          b=b && kernels[i]->write(handler);
        } else {
          lti::write(handler,"name","unknown");
        }
      }
      b=b && handler.writeEnd();
      b=b && lti::write(handler, "offset", offset);
      b=b && lti::write(handler, "scale", scale);

    }
    if (complete) {
      b=handler.writeEnd();
    }
    return b;
  }

  /*
   * read the classifier from the given ioHandler
   */
  bool svm::read(ioHandler& handler,const bool complete) {
    bool b=true;
    if (complete) {
      b=handler.readBegin();
    }
    b = b && supervisedInstanceClassifier::read(handler,false);
    if (b) {
      b=b && lti::read(handler, "nClasses",nClasses);
      b=b && lti::read(handler, "alpha",alpha);
      delete trainData;
      dmatrix* t=new dmatrix();
      b=b && lti::read(handler, "vectors",*t);
      trainData=t;
      b=b && lti::read(handler, "idMap",idMap);
      b=b && lti::read(handler, "rIdMap",rIdMap);
      b=b && lti::read(handler, "srcIds",srcIds);
      b=b && lti::read(handler, "bias", bias);
      int n;
      b=b && lti::read(handler, "nKernels", n);
      //kernels.
      b=b && handler.readBegin();
      b=b && handler.trySymbol("kernels");
      className cn;
      std::string kname;
      kernels.resize(n);
      // always read the complete kernel vector, so we have the setup
      // ready to go without the need for any subsequent
      // initializations
      for (int i=0; i<n; i++) {
        b=b && lti::read(handler, "name", kname);
        if (kname != "unknown") {
          kernels[i]=getParameters().createKernel(kname);
          b=b && kernels[i]->read(handler);
        } else {
          kernels[i]=0;
        }
      }
      b=b && handler.readEnd();
      b=b && lti::read(handler, "offset", offset);
      b=b && lti::read(handler, "scale", scale);
      rebuildTargets();
      defineOutputTemplate();
    }
    if (complete) {
      b=b && handler.readEnd();
    }

    return b;
  }



}
