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
 * file .......: ltiSHClassifier.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 30.10.2001
 * revisions ..: $Id: ltiSHClassifier.cpp,v 1.7 2006/02/07 18:23:23 ltilib Exp $
 */

#include "ltiObject.h"
#include <limits>
#include <cstdio>
#include "ltiBoundsFunctor.h"
#include "ltiSTLIoInterface.h"
#include "ltiSHClassifier.h"

// This stupid so-called C++ compiler from MS sucks!
#ifdef _LTI_MSC_6
#undef min
#undef max
#endif

#undef _DEBUG
//#define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  // --------------------------------------------------
  // shClassifier::parameters
  // --------------------------------------------------

  // default constructor
  shClassifier::parameters::parameters()
    : supervisedInstanceClassifier::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters
    numberOfBins=32;
    autoBounds=true;
  }

  // copy constructor
  shClassifier::parameters::parameters(const parameters& other)
    : supervisedInstanceClassifier::parameters()  {
    copy(other);
  }

  // destructor
  shClassifier::parameters::~parameters() {
  }


  // get type name
  const char* shClassifier::parameters::getTypeName() const {
    return "shClassifier::parameters";
  }

  // copy member

  shClassifier::parameters&
    shClassifier::parameters::copy(const parameters& other) {
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

    numberOfBins=other.numberOfBins;
    binVector=other.binVector;
    minimum=other.minimum;
    maximum=other.maximum;
    autoBounds=other.autoBounds;

    return *this;
  }

  // alias for copy member
  shClassifier::parameters&
    shClassifier::parameters::operator=(const parameters& other) {
    copy(other);

    return *this;
  }

  // clone member
  classifier::parameters* shClassifier::parameters::clone() const {
    return new parameters(*this);
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was succeful
   */
# ifndef _LTI_MSC_6
  bool shClassifier::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool shClassifier::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      b=b && lti::write(handler, "numberOfBins",numberOfBins);
      b=b && lti::write(handler, "binVector",binVector);
      b=b && lti::write(handler, "minimum",minimum);
      b=b && lti::write(handler, "maximum",maximum);
      b=b && lti::write(handler, "autoBounds", autoBounds);
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
  bool shClassifier::parameters::write(ioHandler& handler,
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
   * @return true if write was succeful
   */
# ifndef _LTI_MSC_6
  bool shClassifier::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool shClassifier::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    _lti_debug("b="<<b<<",");

    if (b) {
      b=b && lti::read(handler, "numberOfBins",numberOfBins);
    _lti_debug("b="<<b<<",");
      b=b && lti::read(handler, "binVector",binVector);
    _lti_debug("b="<<b<<",");
      b=b && lti::read(handler, "minimum",minimum);
    _lti_debug("b="<<b<<",");
      b=b && lti::read(handler, "maximum",maximum);
    _lti_debug("b="<<b<<",");
      b=b && lti::read(handler, "autoBounds", autoBounds);
    }
    _lti_debug("b="<<b<<",");

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
    _lti_debug("b="<<b<<",");

    return b;
  }

# ifdef _LTI_MSC_6
  bool shClassifier::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // shClassifier
  // --------------------------------------------------

  // default constructor
  shClassifier::shClassifier()
    : supervisedInstanceClassifier() {

    //TODO: comment the attributes of your clustering
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

    models.clear();
    nClasses=0;
    idMap.clear();
    rIdMap.clear();
  }

  // copy constructor
  shClassifier::shClassifier(const shClassifier& other)
    : supervisedInstanceClassifier()  {
    copy(other);
  }

  // destructor
  shClassifier::~shClassifier() {
    for (unsigned int i=0; i<models.size(); i++) {
      delete models[i];
      models[i]=0;
    }
    models.clear();
  }

  // returns the name of this type
  const char* shClassifier::getTypeName() const {
    return "shClassifier";
  }

  // copy member
  shClassifier& shClassifier::copy(const shClassifier& other) {
      supervisedInstanceClassifier::copy(other);

    //TODO: comment the attributes of your clustering
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.
      models=other.models;
      nClasses=other.nClasses;
      idMap=other.idMap;
      rIdMap=other.rIdMap;

    return (*this);
  }

  // alias for copy member
  shClassifier& shClassifier::operator=(const shClassifier& other) {
    return (copy(other));
  }


  // clone member
  classifier* shClassifier::clone() const {
    return new shClassifier(*this);
  }

  // return parameters
  const shClassifier::parameters&
    shClassifier::getParameters() const {
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

  // TODO: Implement objective function
  // TODO: Implement SHCLASSIFIER output function



  // Calls the same method of the superclass.
  bool shClassifier::train(const dmatrix& input, const ivector& ids) {

    buildIdMaps(ids);

    boundsFunctor<double> bounds;
    const parameters& par=getParameters();

    dvector min,max;

    if (par.autoBounds) {
      bounds.boundsOfRows(input,min,max);
    } else {
      min=par.minimum;
      max=par.maximum;
    }

    _lti_debug("Binvector.size = " << par.binVector.size() << "\n");

    int i;

    // build one histogram per object
    models.resize(nClasses);
    for (i=0; i<nClasses; i++) {
      if (par.binVector.size() == min.size()) {
        models[i]=new sparseHistogram(par.binVector,min,max);
      } else {
        models[i]=new sparseHistogram(par.numberOfBins,min,max);
      }
    }

    ivector sum(nClasses);

    // fill histograms
    for (i=0; i<input.rows(); i++) {
      int id=idMap[ids.at(i)];
      models[id]->add(input.getRow(i));
      sum[id]++;
    }

    // normalize histograms
    for (i=0; i<nClasses; i++) {
      _lti_debug("Sum of " << i << " is " << sum.at(i) << "\n");
      if (sum.at(i) == 0) {
        delete models[i];
        models[i]=0;
      } else {
        models[i]->divide(static_cast<float>(sum.at(i)));
      }
    }
    defineOutputTemplate();
    return true;
  }


  /**
   * Adds an object to this classifier. The id is determined automatically
   * and returned in the parameter.
   */
  bool shClassifier::trainObject(const dmatrix& input, int& id) {
    id=0;
    for (std::map<int,int>::const_iterator i=rIdMap.begin(); i != rIdMap.end(); i++) {
      if (i->second >= id) {
        id=i->second+1;
      }
    }
    idMap[id]=nClasses;
    rIdMap[nClasses]=id;
    nClasses++;

    const parameters& par=getParameters();

    // do not touch min and max
    if (getParameters().binVector.size() > 0) {
      models.push_back(new sparseHistogram(getParameters().binVector,
                                           par.minimum,par.maximum));
    } else {
      models.push_back(new sparseHistogram(getParameters().numberOfBins,
                                           par.minimum,par.maximum));
    }
    // fill histograms
    int sum=0;
    for (int j=0; j<input.rows(); j++) {
      models[nClasses-1]->add(input.getRow(j));
      sum++;
    }
    models[nClasses-1]->divide(static_cast<float>(sum));

    defineOutputTemplate();

    return true;
  }



  void shClassifier::defineOutputTemplate() {

    ivector tids(rIdMap.size());
    unsigned int i=0;
    for (i=0; i<rIdMap.size(); i++) {
      tids.at(i)=rIdMap[i];
    }

    outTemplate=outputTemplate(tids);
  }

  void shClassifier::buildIdMaps(const ivector& ids) {
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



  // -------------------------------------------------------------------
  // The classify-method!
  // -------------------------------------------------------------------

  //TODO Usually this method does not need to be implemented use inherited
  // method from direct subclass of classifier (ie centroidClassifier) instead
  // In this case delete. Otherwise comment!

  bool shClassifier::classify(const dvector& feature, outputVector& output) const {
    _lti_debug("Classifying: " << feature << "\n");
    dvector result(nClasses);
    _lti_debug("nclasses is " << nClasses << " in " << this << "\n");

    for (int i=0; i<nClasses; i++) {
      result[i]=models[i]->get(feature);
      _lti_debug("result[" << i << "] is " << result[i] << "\n");
    }

    return outTemplate.apply(result,output);
  }


  // parameter shortcuts

  //
  // IO methods
  //

  /*
   * write the classifier in the given ioHandler
   */
  bool shClassifier::write(ioHandler& handler,const bool complete) const {
    bool b=true;
    if (complete) {
      b=handler.writeBegin();
    }
    b = b && supervisedInstanceClassifier::write(handler,false);
    if (b) {

      // TODO: Write data
      b=b && lti::write(handler, "nClasses",nClasses);
      b=b && lti::write(handler, "idMap",idMap);
      b=b && lti::write(handler, "rIdMap",rIdMap);

      for (int i=0; i<nClasses; i++) {
        b=b && models[i]->write(handler);
      }

      //b=b && handler.writeEnd();
    }
    if (complete) {
      b=handler.writeEnd();
    }
    return b;
  }

  /*
   * read the classifier from the given ioHandler
   */
  bool shClassifier::read(ioHandler& handler,const bool complete) {
    bool b=true;
    if (complete) {
      b=handler.readBegin();
    }
    b = b && supervisedInstanceClassifier::read(handler,false);
    if (b) {
      b=b && lti::read(handler, "nClasses",nClasses);
      b=b && lti::read(handler, "idMap",idMap);
      b=b && lti::read(handler, "rIdMap",rIdMap);

      _lti_debug("B1 = " << b << " in " << this << "\n");

      models.resize(nClasses);

      if (getParameters().binVector.size() > 0) {
        const ivector& bvect=getParameters().binVector;
        for (int i=0; i<nClasses; i++) {
          models[i]=new sparseHistogram(bvect);
          b=b && models[i]->read(handler);
        }
        _lti_debug("B2 = " << b << "\n");
      } else {
        int bins=getParameters().numberOfBins;
        for (int i=0; i<nClasses; i++) {
          models[i]=new sparseHistogram(bins,0);
          b=b && models[i]->read(handler);
        }
        _lti_debug("B3 = " << b << "\n");
      }
      //b=b && handler.readEnd();
    }
    if (complete) {
      b=b && handler.readEnd();
    }

    return b;
  }



}
