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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiStdLayer.cpp
 * authors ....: Peter Doerfler, Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 11.8.2000
 * revisions ..: $Id: ltiStdLayer.cpp,v 1.4 2006/02/07 18:26:28 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiStdLayer.h"
#include "ltiL1Distance.h"
#include "ltiL2Distance.h"
#include "ltiLispStreamHandler.h"
#include "ltiMath.h"
#include <set>   // hash_set changed to set due to its absence in MSVC++...


using std::getline;

namespace lti {

  // ----------------------------------------------
  // initialization functor
  // ----------------------------------------------

  ivector stdLayer::initFunctor::uniqueIDs
  (const ivector& allIDs) const {

    // hash_set changed to set due to its absence in MSVC++...
    std::set<int> hset;
    int len=allIDs.size();
    int i;

    for (i=0; i<len; i++) {
      hset.insert(allIDs.at(i));
    }

    ivector unique(hset.size());

    std::set<int>::iterator it=hset.begin();

    i=0;
    while (it!=hset.end()) {
      unique.at(i)=*it++;
    }

    return unique;
  }

  dmatrix stdLayer::initFunctor::classVectors(const dmatrix& allVectors,
                                              const ivector& allIDs,
                                              int nbViews,
                                              int id) const {


    dmatrix cv(nbViews, allVectors.columns());
    
    int j=0;
    for (int i=0; i<allIDs.size(); i++) {
      if (allIDs.at(i) == id) {
        cv.setRow(j++, allVectors.getRow(i));
      }
    }

    return cv;
  }

  // ----------------------------------------------
  // activation functor
  // ----------------------------------------------
  bool stdLayer::gaussActFunctor::operator()(dvector& output) const {
    static const double fac = sqrt(2*Pi);
    if (sigma==-1.) {
      dvector::iterator it,e;
      dvector::const_iterator vit;
      for(it=output.begin(),vit=sigmaVec->begin(),e=output.end();
          it!=e;
          it++,vit++) {
        double tmp = *it-mu;
        *it = ::exp(-(tmp*tmp)/(2*(*vit)*(*vit)))/(fac*(*vit));
      }
    } else {
      dvector::iterator it,e;
      for(it=output.begin(),e=output.end();
          it!=e;
          it++) {
        double tmp = *it - mu;
        *it =  ::exp(-(tmp*tmp)/(2*sigma*sigma))/(fac*sigma);
      }
    }
    return true;
  }


  bool stdLayer::sigmoidActFunctor::operator()(dvector& output) const {
    dvector::iterator it,e;
    if (slope!=1) {
      for(it=output.begin(),e=output.end();
          it!=e;
          it++) {
        *it = 1.0/(1.0+::exp(-slope*(*it)));
      }
    } else {
      for(it=output.begin(),e=output.end();
          it!=e;
          it++) {
        *it = 1.0/(1.0+::exp(-*it));
      }
    }
    return true;
  }

  // ----------------------------------------------
  // stdLayer::trainFunctor
  // ----------------------------------------------

  /**
   * set the learn rate factor
   */
  void stdLayer::trainFunctor::setLearnRate(const double & lr) {
    learnRate = lr;
  };


  // ----------------------------------------------
  // stdLayer::distancePropFunctor
  // ----------------------------------------------


  bool stdLayer::distancePropFunctor::operator()(const dvector& input,
                                                 const dmatrix& weights,
                                                 dvector& outLayer) const {

    switch(norm) {
      case L1distance: {
        l1Distance<double> dist;
        dist.apply(weights,input,outLayer);
      }
      break;
      case L2distance: {
        l2Distance<double> dist;
        dist.apply(weights,input,outLayer);
      }
      break;
      default:
        // NORMally impossibly to get here, but it's C++ after all ;-)
        return false; // throw exception("Undefined norm for lvqPropFunctor");
    }
    return act(outLayer);
  }


  // ----------------------------------------------
  // stdLayer::distancePropFunctor
  // ----------------------------------------------

  bool stdLayer::dotPropFunctor::operator()(const dvector& input,
                                            const dmatrix& weights,
                                            dvector& outLayer) const {

    weights.multiply(input,outLayer);
    return act(outLayer);
  }

  // ----------------------------------------------
  // stdLayer
  // ----------------------------------------------

  // default empty string for the functor
  const char* const stdLayer::emptyString = "";

  stdLayer::stdLayer(const std::string& theName)
    : object() {
    layerName=theName;
    sizeIn = 0;
    sizeOut = 0;
    learnRate = 0.0;
    statusString = 0;
  }

  stdLayer::stdLayer(const stdLayer& other) : object(),statusString(0) {
    copy(other);
  }

  stdLayer::~stdLayer() {

  }

  stdLayer& stdLayer::copy(const stdLayer& other) {
    weights.copy(other.weights);
//      outLayer.copy(other.outLayer);
    outID.copy(outID);
    sizeIn = other.sizeIn;
    sizeOut = other.sizeOut;
    layerName = other.layerName;
    learnRate = other.learnRate;
    return *this;
  }

  /*
   * return the last message set with setStatusString().  This will
   * never return 0.  If no status-string has been set yet an empty string
   * (pointer to a string with only the char(0)) will be returned.
   */
  const char* stdLayer::getStatusString() const {
    if (isNull(statusString)) {
      return emptyString;
    } else {
      return statusString;
    }
  }

  /*
   * set a status string.
   *
   * @param msg the const string to be reported next time by
   * getStatusString()
   * This message will be usually set within the apply methods to indicate
   * an error cause.
   */
  void stdLayer::setStatusString(const char* msg) const {
    delete[] statusString;
    statusString = 0;

    statusString = new char[strlen(msg)+1];
    strcpy(statusString,msg);
  }

  void stdLayer::initWeights(initFunctor& initFunc) {
    initFunc(weights, outID);
  }

  bool stdLayer::propagate(const dvector& input,
                           propagationFunctor& prop,
                           dvector& output) const {
    return prop(input,weights,output);
  }

  bool stdLayer::write(ioHandler& handler,const bool complete) const {
    bool b(true);
    if (complete) {
      b = handler.writeBegin();
    }

    lti::write(handler,"layerName",layerName);
    lti::write(handler,"sizeIn",sizeIn);
    lti::write(handler,"sizeOut",sizeOut);
    lti::write(handler,"learnRate",learnRate);
    lti::write(handler,"weights",weights);
    lti::write(handler,"outID",outID);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  };

  bool stdLayer::read(ioHandler& handler,const bool complete) {
    bool b(true);
    if (complete) {
      b = handler.readBegin();
    }

    lti::read(handler,"layerName",layerName);
    lti::read(handler,"sizeIn",sizeIn);
    lti::read(handler,"sizeOut",sizeOut);
    lti::read(handler,"learnRate",learnRate);
    lti::read(handler,"weights",weights);
    lti::read(handler,"outID",outID);

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  };

  void stdLayer::setWeights(const dmatrix& w) {
    weights=w;
  }

  void stdLayer::setIDs(const ivector& v) {
    outID=v;
  }


#ifdef _INCLUDE_DEPRECATED
  bool stdLayer::save(std::ostream& out) const {
    lispStreamHandler h(out);
    return write(h,false);
  }

  bool stdLayer::load(std::istream& in) {

    if (!in.good()) {
      return false;
    }

    lispStreamHandler h(in);
    return read(h,false);
  }

  bool stdLayer::saveBinary(std::ostream& out) const {
    int laenge=layerName.length();
    out.write((char*)&(laenge), sizeof(int));
    out.write(layerName.c_str(), laenge);

    out.write((char*)&(sizeIn), sizeof(int));
    out.write((char*)&(sizeOut), sizeof(int));

    for(int i=0;i<weights.rows();i++) {
      out.write((char*)&(weights[i][0]),
                sizeof(double)*weights.columns());
    }
    out.write((char*)&outID[0],sizeof(int)*outID.size());

    return true;
  }

  bool stdLayer::loadBinary(std::istream& in) {
    if (!in.good()) {
      return false;
    }

    int laenge;
    in.read((char*)&(laenge), sizeof(int));
    char* buffer=new char[laenge+1];
    in.read(buffer,laenge);
    buffer[laenge]='\0';
    layerName=buffer;
    delete []buffer;

    in.read((char*)&(sizeIn), sizeof(int));
    in.read((char*)&(sizeOut), sizeof(int));

    setSize(sizeIn,sizeOut);

    for(int i=0;i<weights.rows();i++) {
      in.read((char*)&(weights[i][0]),sizeof(double)*weights.columns());
    }
    in.read((char*)&outID[0],sizeof(int)*outID.size());
    learnRate=0;

    return true;
  }
#endif

  void stdLayer::setSize(const int &in, const int& out) {
    sizeIn=in;
    sizeOut=out;

    weights.resize(sizeOut,sizeIn,0.);
    outID.resize(sizeOut,0);
  }


  /**
   * train the network (supervised) with the given input vector
   */
  bool stdLayer::train(const dvector& input,
                       const int& trainID,
                       trainFunctor& trainFunc) {
    bool modified;
    dvector outLayer;
    trainFunc(input, weights, outLayer, outID, trainID, modified);

    return true;
  }

  /**
   * train the network (unsupervised) with the given input vector
   */
  bool stdLayer::train(const dvector& input,trainFunctor& trainFunc) {
    bool modified;
    dvector outLayer;
    trainFunc(input, weights, outLayer, modified);

    return true;
  }

  /*
   * read the objectProb from the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be readed
   */
  bool read(ioHandler& handler,
            stdLayer& p,
            const bool complete) {
    return p.read(handler,complete);
  }

  /*
   * write the vector in the given ioHandler.  The complete flag indicates
   * if the enclosing begin and end should be also be written or not
   */
  bool write(ioHandler& handler,
             const stdLayer& p,
             const bool complete) {
    return p.write(handler,complete);
  }


}
