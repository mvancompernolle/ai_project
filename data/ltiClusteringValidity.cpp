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
 * file .......: ltiClusteringValidity.cpp
 * authors ....: Jens Paustenbach
 * organization: LTI, RWTH Aachen
 * creation ...: 4.4.2002
 * revisions ..: $Id: ltiClusteringValidity.cpp,v 1.14 2006/09/05 09:57:09 ltilib Exp $
 */

#include "ltiClusteringValidity.h"

namespace lti {

  // -------------------------------------------------------------
  // clusteringValidity
  // -------------------------------------------------------------
  //constructor
  clusteringValidity::clusteringValidity() {
  }
  
  // copy constructor
  clusteringValidity::clusteringValidity(const clusteringValidity& other) {
    copy(other);  
  }
  
  //    // destructor
  clusteringValidity::~clusteringValidity() {
  }

  //   functor* dunnIndex::clone() const {
  //     return (new dunnIndex(*this));
  //   }

  // copy member
  clusteringValidity&
  clusteringValidity::copy(const clusteringValidity& other) {
    functor::copy(other);
    return (*this);
  }

  // alias for copy member
  clusteringValidity&
  clusteringValidity::operator=(const clusteringValidity& other) {
    return (copy(other));
  }

  // returns the name of this type
  const char* clusteringValidity::getTypeName() const {
    return "clusteringValidity";
  }
  
  double clusteringValidity::getMinimumDistance(const dmatrix& m1,
                                                const dmatrix& m2) const {
    int i,j;
    dmatrix distances(m1.rows(),m2.rows());
    l2Distance<double> dist;
    for (i=0; i<m1.rows(); i++) {
      for (j=0; j<m2.rows(); j++) {
        distances[i][j]=dist.apply(m1.getRow(i),m2.getRow(j));
      }
    }
    return distances.minimum();
  }
  
  double clusteringValidity::getMaximumDistance(const dmatrix& m1,
                                                const dmatrix& m2) const {
    int i,j;
    dmatrix distances(m1.rows(),m2.rows());
    l2Distance<double> dist;
    for (i=0; i<m1.rows(); i++) {
      for (j=0; j<m2.rows(); j++) {
        distances[i][j]=dist.apply(m1.getRow(i),m2.getRow(j));
      }
    }
    return distances.maximum();
  }
  
  double clusteringValidity::getAverageDistance(const dmatrix& m1,
                                                const dmatrix& m2) const {
    double distance=0.0;
    int i,j;
    l2Distance<double> dist;
    for (i=0; i<m1.rows(); i++) {
      for (j=0; j<m2.rows(); j++) {
        distance+=dist.apply(m1.getRow(i),m2.getRow(j));
      }
    }
    distance=distance/((double)m1.rows()*(double)m2.rows());
    return distance;
  }
  
  double clusteringValidity::getCentroidDistance(const dmatrix& m1,
                                                 const dmatrix& m2) const {
    l2Distance<double> dist;
    int i;
    dvector a(m1.columns());
    dvector b(m2.columns());
    for (i=0; i<m1.rows();i++) {
      a.add(m1.getRow(i));
    }
    a.divide(m1.rows());
    for (i=0; i<m2.rows();i++) {
      b.add(m2.getRow(i));
    }
    b.divide(m2.rows());
    return dist.apply(a,b);
  }
  
  double clusteringValidity::getAverageInterpointDistance(const dmatrix& m1,
                                                          const dmatrix& m2) const {
    l2Distance<double> dist;
    int i;
    dvector a(m1.columns());
    dvector b(m2.columns());
    for (i=0; i<m1.rows();i++) {
      a.add(m1.getRow(i));
    }
    a.divide(m1.rows()); // centroid 1
    for (i=0; i<m2.rows();i++) {
      b.add(m2.getRow(i));
    }
    b.divide(m2.rows()); // centroid 2
    double distance=0.0;
    for (i=0; i<m1.rows(); i++) {
      distance+=dist.apply(m1.getRow(i),a);
    }
    for (i=0; i<m2.rows(); i++) {
      distance+=dist.apply(m2.getRow(i),b);
    }
    return (distance/(m1.rows()+m2.rows()));
  }
  
  double clusteringValidity::getStandardDiameter(const dmatrix& m1) const {
    dmatrix distances(m1.rows(),m1.rows());
    int j,k;
    l2Distance<double> dist;
    for (j=0; j<m1.rows(); j++) {
      for (k=j+1; k<m1.rows(); k++) {
        distances[j][k]=dist.apply(m1.getRow(j),
                                   m1.getRow(k));
      }
    }
    return distances.maximum();  
  }
  
  double clusteringValidity::getAverageDiameter(const dmatrix& m1) const {
    double distance=0.0;
    int j,k;
    l2Distance<double> dist;
    for (j=0; j<m1.rows(); j++) {
      for (k=0; k<m1.rows(); k++) {
        distance+=dist.apply(m1.getRow(j),
                             m1.getRow(k));
      }
    }
    if (m1.rows()>1) {
      return (distance/((double)m1.rows()*
                        (double)(m1.rows()-1)));
    } else {
      return distance;
    }
  }
  
  double clusteringValidity::getAverageToCentroidDiameter(const dmatrix& m1) const {
    dvector a(m1.columns());
    int i,j;
    l2Distance<double> dist;
    double distance=0.0;
    for (i=0; i<m1.rows(); i++) {
      a.add(m1.getRow(i));
    }
    a.divide(m1.rows());
    for (j=0; j< m1.rows(); j++) {
      distance+=dist.apply(a,m1.getRow(j));
    }
    if (m1.rows()>0) {
      return (2*distance/(double)m1.rows());
    } else {
      return 2*distance;
    }
    
  }
  
  
  ///////////////////////////////////////////////////////////////////////
  //////////////////////// dunnIndex ////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////  
  
  // constructor
  dunnIndex::dunnIndex() : clusteringValidity() {
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }
  
  // copy constructor
  dunnIndex::dunnIndex(const dunnIndex& other) : clusteringValidity() {
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  
    copy(other);  
  }
  
  // destructor
  dunnIndex::~dunnIndex() {
  }

  const char* dunnIndex::getTypeName() const {
    return "dunnIndex";
  }
  
  functor* dunnIndex::clone() const {
    return (new dunnIndex(*this));
  }

  // copy member
  dunnIndex& dunnIndex::copy(const dunnIndex& other) {
    clusteringValidity::copy(other);
    return (*this);
  }

  // alias for copy member
  dunnIndex& dunnIndex::operator=(const dunnIndex& other) {
    return (copy(other));
  }

  // return parameters
  const dunnIndex::parameters&
  dunnIndex::getParameters() const {
    const parameters* par = 
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException();
    }
    return *par;
  }

  bool dunnIndex::apply(const std::vector<dmatrix>& clusteredData,
                        double& index, const dmatrix& centroids) const {
    int nbClusters=clusteredData.size();
    double denominator=0.0;
    int i,j;
    l2Distance<double> dist;
    dvector diameters(nbClusters);
    parameters param;
    param=getParameters();
    // pointer to the function which implements the measure according to the 
    // parameters
    double (lti::clusteringValidity::*diamFunc)(const dmatrix&) const;
    
#ifdef _LTI_MSC_DOT_NET_2003
    // nasty bug in this version of the .NET compiler
#define QUALIFIER
#else 
#define QUALIFIER &lti::dunnIndex::
#endif

    switch (param.diameterMeasure) {
      case parameters::Standard:
        diamFunc=QUALIFIER getStandardDiameter;
        break;
      case parameters::Average:
        diamFunc=QUALIFIER getAverageDiameter;
        break;
      case parameters::Centroid:
        diamFunc=QUALIFIER getAverageToCentroidDiameter;
        break;
      default:
        diamFunc=QUALIFIER getStandardDiameter;
        setStatusString("Unknown diameterMeasure in clusteringValidity\n");
        return false;
    }
    // compute all diameters of all clusters
    for (i=0; i<nbClusters; i++) {
      diameters[i]=(this->*diamFunc)(clusteredData[i]);
    }
    denominator=diameters.maximum();
    // pointer to the function which calculates the distance of the functions
    // a pointer to a function is used, because the function will be called 
    // many times later
    double (lti::clusteringValidity::*distFunc)
      (const dmatrix&,const dmatrix&) const ;
    // set pointer to function which is set by the parameter distanceMeasure
    switch (param.distanceMeasure) {
      case parameters::Minimum:
        distFunc=QUALIFIER getMinimumDistance;
        break;
      case parameters::Maximum:
        distFunc=QUALIFIER getMaximumDistance;
        break;
      case parameters::Mean:
        distFunc=QUALIFIER getAverageDistance;
        break;
      case parameters::Centroids:
        distFunc=QUALIFIER getCentroidDistance;
        break;
      case parameters::Interpoint:
        distFunc=QUALIFIER getAverageInterpointDistance;
        break;
      default:
        distFunc=QUALIFIER getAverageDistance;
        setStatusString("Unknown distanceMeasure in clusteringValidity\n");
        return false;
    }
    // compute the distances of all clusters to each other
    int counter=0;
    dvector distanceVector(static_cast<int>(.5*(nbClusters*(nbClusters-1))));
    for (i=0; i<nbClusters; i++) {
      for (j=i+1; j<nbClusters; j++) {
        if (distFunc==QUALIFIER getCentroidDistance) {
          distanceVector[counter]=dist.apply(centroids.getRow(i),
                                             centroids.getRow(j));
        } else {
          distanceVector[counter]=(this->*distFunc)(clusteredData[i],
                                                    clusteredData[j]);
        }
        counter++;
      }
    }
    distanceVector.divide(denominator);
    index=distanceVector.minimum();

    return true;
  }

  // --------------------------------------------------
  // dunnIndex::parameters
  // --------------------------------------------------

  // default constructor
  dunnIndex::parameters::parameters() 
    : clusteringValidity::parameters() {    
    distanceMeasure=Minimum;
    diameterMeasure=Standard;
  }
  // copy constructor
  dunnIndex::parameters::parameters(const parameters& other) {
    copy(other);
  }
  
  // destructor 
  dunnIndex::parameters::~parameters() {    
  }

  // get type name
  const char* dunnIndex::parameters::getTypeName() const {
    return "dunnIndex::parameters";
  }
  
  // copy member
  dunnIndex::parameters& 
  dunnIndex::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    clusteringValidity::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    clusteringValidity::parameters& (clusteringValidity::parameters::* p_copy)
      (const clusteringValidity::parameters&) = 
      clusteringValidity::parameters::copy;
    (this->*p_copy)(other);
# endif

    distanceMeasure = other.distanceMeasure;
    diameterMeasure = other.diameterMeasure;
    return *this;
  }

  // alias for copy member
  dunnIndex::parameters& 
  dunnIndex::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* dunnIndex::parameters::clone() const {
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
  bool dunnIndex::parameters::write(ioHandler& handler,
                                    const bool complete) const 
# else
    bool dunnIndex::parameters::writeMS(ioHandler& handler,
                                        const bool complete) const 
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      switch (distanceMeasure) {
        case Minimum:
          lti::write(handler,"distanceMeasure", "Minimum");
          break;
        case Maximum:
          lti::write(handler,"distanceMeasure", "Maximum");
          break;
        case Mean:
          lti::write(handler,"distanceMeasure", "Mean");
          break;
        case Centroids:
          lti::write(handler,"distanceMeasure", "Centroids");
          break;
        case Interpoint:
          lti::write(handler,"distanceMeasure", "Interpoint");
          break;
        default:
          lti::write(handler,"distanceMeasure", "Minimum");          
      }

      switch (diameterMeasure) {
        case Standard:
          lti::write(handler,"diameterMeasure", "Standard");
          break;
        case Average:
          lti::write(handler,"diameterMeasure", "Average");
          break;
        case Centroid:
          lti::write(handler,"diameterMeasure", "Centroid");
          break;
        default:
          lti::write(handler,"diameterMeasure", "Standard");
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && clusteringValidity::parameters::write(handler,false);
# else
    bool (clusteringValidity::parameters::* p_writeMS)(ioHandler&,const bool) const = 
      clusteringValidity::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool dunnIndex::parameters::write(ioHandler& handler,
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
  bool dunnIndex::parameters::read(ioHandler& handler,
                                   const bool complete) 
# else
    bool dunnIndex::parameters::readMS(ioHandler& handler,
                                       const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      std::string str1,str2;
      lti::read(handler,"diameterMeasure",str1);
      lti::read(handler,"distanceMeasure",str2);
      if (str1.find("tand") != std::string::npos) {
        diameterMeasure = Standard;
      } else if (str1.find("verag") != std::string::npos) {
        diameterMeasure = Average;
      } else if (str1.find("entr")!= std::string::npos) {
        diameterMeasure = Centroid;
      }

      if (str2.find("inim") != std::string::npos) {
        distanceMeasure = Minimum;
      } else if (str2.find("axim") != std::string::npos) {
        distanceMeasure = Maximum;
      } else if (str2.find("ean") != std::string::npos) {
        distanceMeasure = Mean;
      } else if (str2.find("nterp") != std::string::npos) {
        distanceMeasure = Interpoint;
      } else if (str2.find("entroid") != std::string::npos) {
        distanceMeasure = Centroids;
      }
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to 
    // compile...
    b = b && clusteringValidity::parameters::read(handler,false);
# else
    bool (clusteringValidity::parameters::* p_readMS)(ioHandler&,const bool) = 
      clusteringValidity::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool dunnIndex::parameters::read(ioHandler& handler,
                                   const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif



  ///////////////////////////////////////////////////////////////////////
  //////////////////////// modified Hubert Statistic ////////////////////
  ///////////////////////////////////////////////////////////////////////  

  // constructor
  modHubertStat::modHubertStat() : clusteringValidity() {
  }
  
  // copy constructor
  modHubertStat::modHubertStat(const modHubertStat& other) 
    : clusteringValidity() {
    copy(other);  
  }
  
  // destructor
  modHubertStat::~modHubertStat() {
  }

  // copy member
  modHubertStat&
  modHubertStat::copy(const modHubertStat& other) {
    clusteringValidity::copy(other);
    return (*this);
  }

  // alias for copy member
  modHubertStat&
  modHubertStat::operator=(const modHubertStat& other) {
    return (copy(other));
  }

  const char* modHubertStat::getTypeName() const {
    return "modHubertStat";
  }

  functor* modHubertStat::clone() const {
    return (new modHubertStat(*this));
  }

  bool modHubertStat::apply(const std::vector<dmatrix>& clusteredData,
                            double& index, const dmatrix& centroids) const {
    index =0.0;
    int nbClusters=clusteredData.size();
    l2Distance<double> dist;
    int nbPoints=0;
    int i,j,k,l;
    for (i=0; i<nbClusters; i++) {
      for (j=0; j<clusteredData[i].rows(); j++) { 
        // count the number of points
        nbPoints++;
        // for all other clusters and all points in these clusters
        for (k=i+1; k<nbClusters; k++) {
          for (l=0; l<clusteredData[k].rows(); l++) {
            index+=dist.apply(clusteredData[i].getRow(j),
                              clusteredData[k].getRow(l))*
              dist.apply(centroids.getRow(i),centroids.getRow(k));
          }
        }
      }
    }
    double m=0.5*(nbPoints*(nbPoints-1));
    index=index/m;

    return true;
  }
    
  ///////////////////////////////////////////////////////////////////////
  ///////////// normalized modified Hubert Statistic ////////////////////
  ///////////////////////////////////////////////////////////////////////  


  // constructor
  normModHubertStat::normModHubertStat() : clusteringValidity() {
  }
  
  // copy constructor
  normModHubertStat::normModHubertStat(const normModHubertStat& other) 
    : clusteringValidity() {
    copy(other);  
  }
  
  // destructor
  normModHubertStat::~normModHubertStat() {
  }

  // copy member
  normModHubertStat&
  normModHubertStat::copy(const normModHubertStat& other) {
    clusteringValidity::copy(other);
    return (*this);
  }

  // alias for copy member
  normModHubertStat&
  normModHubertStat::operator=(const normModHubertStat& other) {
    return (copy(other));
  }

  const char* normModHubertStat::getTypeName() const {
    return "normModifiedHubertStatistic";
  }

  functor* normModHubertStat::clone() const {
    return (new normModHubertStat(*this));
  }

  bool normModHubertStat::apply(const std::vector<dmatrix>& 
                                clusteredData, double& index, 
                                const dmatrix& centroids) const {
    index =0.0;
    int nbClusters=clusteredData.size();
    l2Distance<double> dist;
    int nbPoints=0;
    int i,j,k,l;
    // count the points that are in clusteredData
    for (i=0; i<nbClusters; i++) {
      nbPoints+=clusteredData[i].rows();
    }
    // x is the distance matrix. It has in the i-th rows and j-th column
    // the distance between the i-th and j-th point
    // y is an other distance matrix. It has in the i-th row and j-th column
    // the distance of the centroids of the clusters they belong to.
    dmatrix x(nbPoints,nbPoints);
    dmatrix y(nbPoints,nbPoints);
    int row=0;
    int col=0;
    for (i=0; i<nbClusters; i++) {
      for (j=0; j<clusteredData[i].rows(); j++) {
        for (k=0; k<nbClusters; k++) {
          for (l=0; l<clusteredData[k].rows(); l++) {
            if (col>row) {
              y.at(row,col)=dist.apply(centroids.getRow(i),
                                       centroids.getRow(k));
              x.at(row,col)=dist.apply(clusteredData[i].getRow(j),
                                       clusteredData[k].getRow(l));
            }
            if (col<nbPoints-1) col++;
            else { 
              col=0;
              row++;
            }  
          }
        }
      }
    }
    double m=0.5*(nbPoints*(nbPoints-1));
    double meanX=x.sumOfElements()/m;
    double meanY=y.sumOfElements()/m;
    double tmp1=meanX*meanX;
    double tmp2=meanY*meanY;
    double varianzX=0.0;
    double varianzY=0.0;
    for (i=0; i<nbPoints; i++) {
      for (j=i+1; j<nbPoints; j++) {
        varianzX+=x[i][j]*x[i][j]-tmp1;
        varianzY+=y[i][j]*y[i][j]-tmp2;
      }
    }
    varianzX=varianzX/m;
    varianzY=varianzY/m;
    varianzX=sqrt(varianzX);
    varianzY=sqrt(varianzY);
    double varianz=varianzX*varianzY;
    for (i=0; i<nbPoints; i++) {
      for (j=i+1; j<nbPoints; j++) {
        index+=(x[i][j]-meanX)*(y[i][j]-meanY);
      }
    }
    index=index/(m*varianz); 
  
    return true;
  }
    
  ///////////////////////////////////////////////////////////////////////
  //////////////////////// Davies Bouldin Index /////////////////////////
  ///////////////////////////////////////////////////////////////////////  


  // constructor
  daviesBouldinIndex::daviesBouldinIndex() : clusteringValidity() {
  }
  
  // copy constructor
  daviesBouldinIndex::daviesBouldinIndex(const daviesBouldinIndex& other) 
    : clusteringValidity() {
    copy(other);  
  }
  
  // destructor
  daviesBouldinIndex::~daviesBouldinIndex() {
  }

  // copy member
  daviesBouldinIndex&
  daviesBouldinIndex::copy(const daviesBouldinIndex& other) {
    clusteringValidity::copy(other);
    return (*this);
  }

  // alias for copy member
  daviesBouldinIndex&
  daviesBouldinIndex::operator=(const daviesBouldinIndex& other) {
    return (copy(other));
  }

  const char*  daviesBouldinIndex::getTypeName() const {
    return "daviesBouldinIndex";
  }

  functor* daviesBouldinIndex::clone() const {
    return (new  daviesBouldinIndex(*this));
  }

  bool daviesBouldinIndex::apply(const std::vector<dmatrix>& clusteredData,
                                 double& index, 
                                 const dmatrix& centroids) const {
    int nbClusters=clusteredData.size();
    int i,j;
    dvector interClusterDistance(nbClusters);
    l2Distance<double> dist;
    // compute the average inter cluster distance
    double distance;
    for (i=0; i<nbClusters; i++) {
      for (j=0; j<clusteredData[i].rows(); j++) {
        distance=dist.apply(clusteredData[i].getRow(j),
                            centroids.getRow(i));
        distance=distance*distance;
        interClusterDistance[i]+=distance;
      }
      if (clusteredData[i].rows()!=0) {
        interClusterDistance[i]=interClusterDistance[i]/
          (double)clusteredData[i].rows();}
      interClusterDistance[i]=sqrt(interClusterDistance[i]);
    }
    dmatrix indices(nbClusters,nbClusters);
    for (i=0; i<nbClusters; i++) {
      for (j=i+1; j<nbClusters; j++) {
        double distance=dist.apply(centroids.getRow(i),
                                   centroids.getRow(j));
        indices[i][j]=(interClusterDistance[i]+
                       interClusterDistance[j])/distance;
        indices[j][i]=indices[i][j];
      }
    }
    index=0.0;
    for (i=0; i<nbClusters; i++) {
      index+=indices.getRow(i).maximum();
    }
    index=index/(double)nbClusters;
   
    return true;
  }

} //namespace lti
