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
 * file .......: ltiChiSquareFunctor.cpp
 * authors ....: Birgit Gehrke
 * organization: LTI, RWTH Aachen
 * creation ...: 3.6.2002
 * revisions ..: $Id: ltiChiSquareFunctor.cpp,v 1.8 2006/09/05 10:38:48 ltilib Exp $
 */

// include files
#include "ltiChiSquareFunctor.h"
#include <iostream>
#include <list>
#include "ltiSerialStatsFunctor.h"
#include <vector>
#include <limits>
#include <fstream>
#include <string>
#include <iomanip>

//using namespace std;

namespace lti {
  // --------------------------------------------------
  // chiSquareFunctor::parameters
  // --------------------------------------------------

  // default constructor
  chiSquareFunctor::parameters::parameters()
    : functor::parameters() {
    // Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    accuracy            = 1.0e-5;
    discrete            = true;
    equidistant         = false;
    maxNumOfIntervalls  = 100;
    maxSteps            = 10;
    mean                = std::numeric_limits<double>::min();
    minimalWidth        = -1.0;
    saveInfoFile        = true;
    nameOfInfoFile      = "chiSquareInfo.txt";
    useBetterMeanAndVar = true;
    variance            = std::numeric_limits<double>::min();
  }

  // copy constructor
  chiSquareFunctor::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  chiSquareFunctor::parameters::~parameters() {
  }

  // get type name
  const char* chiSquareFunctor::parameters::getTypeName() const {
    return "chiSquareFunctor::parameters";
  }

  // copy member

  chiSquareFunctor::parameters&
    chiSquareFunctor::parameters::copy(const parameters& other) {
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

      accuracy =    other.accuracy;
      discrete =    other.discrete;
      equidistant = other.equidistant;
      maxNumOfIntervalls = other.maxNumOfIntervalls;
      maxSteps =    other.maxSteps;
      mean =        other.mean;
      minimalWidth   = other.minimalWidth;
      nameOfInfoFile = other.nameOfInfoFile;
      saveInfoFile = other.saveInfoFile;
      useBetterMeanAndVar = other.useBetterMeanAndVar;
      variance =    other.variance;

    return *this;
  }

  // alias for copy member
  chiSquareFunctor::parameters&
    chiSquareFunctor::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* chiSquareFunctor::parameters::clone() const {
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
  bool chiSquareFunctor::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool chiSquareFunctor::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler, "accuracy", accuracy);
      lti::write(handler, "discrete", discrete);
      lti::write(handler, "equidistant", equidistant);
      lti::write(handler, "maxNumOfIntervalls", maxNumOfIntervalls);
      lti::write(handler, "maxSteps", maxSteps);
      lti::write(handler, "mean", mean);
      lti::write(handler, "minimalWidth", minimalWidth);
      lti::write(handler, "nameOfInfoFile", nameOfInfoFile);
      lti::write(handler, "saveInfoFile", saveInfoFile);
      lti::write(handler, "useBetterMeanAndVar", useBetterMeanAndVar);
      lti::write(handler, "variance", variance);
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
  bool chiSquareFunctor::parameters::write(ioHandler& handler,
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
  bool chiSquareFunctor::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool chiSquareFunctor::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"accuracy",accuracy);
      lti::read(handler,"discrete",discrete);
      lti::read(handler, "equidistant", equidistant);
      lti::read(handler, "maxNumOfIntervalls", maxNumOfIntervalls);
      lti::read(handler,"maxSteps",maxSteps);
      lti::read(handler,"mean", mean);
      lti::read(handler, "minimalWidth", minimalWidth);
      lti::read(handler, "nameOfInfoFile", nameOfInfoFile);
      lti::read(handler, "saveInfoFile", saveInfoFile);
      lti::read(handler, "useBetterMeanAndVar", useBetterMeanAndVar);
      lti::read(handler,"variance", variance);
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
  bool chiSquareFunctor::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // chiSquareFunctor
  // --------------------------------------------------

  // default constructor
  chiSquareFunctor::chiSquareFunctor()
    : functor() {

    // comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // create an instance of the parameters with the default values
    // parameters defaultParameters;
    // set the default parameters
    // setParameters(param);

    m_NumberOfIntervalls = 100;
    m_ChiSquareValue     = -1;
    m_Norm               = 0;
    m_Biggest            = 0;
    m_Smallest           = 0;
    m_Mean               = 0;
    m_Variance           = 0;
    m_GaussIntegral      = 0;
    m_NumberOfElements   = 0;
    m_NameOfInfoFile     = "chiSquareInfo.txt";
    m_MeanIntervalls     = 0;
    m_VarianceIntervalls = 0;
    m_UseBetterMeanAndVar= false;

  }

  // copy constructor
  chiSquareFunctor::chiSquareFunctor(const chiSquareFunctor& other)
    : functor()  {
    copy(other);
  }

  // destructor
  chiSquareFunctor::~chiSquareFunctor() {
  }

  // returns the name of this type
  const char* chiSquareFunctor::getTypeName() const {
    return "chiSquareFunctor";
  }

  // copy member
  chiSquareFunctor&
    chiSquareFunctor::copy(const chiSquareFunctor& other) {
      functor::copy(other);

    // comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    m_NumberOfIntervalls = other.m_NumberOfIntervalls;
    m_ChiSquareValue     = other.m_ChiSquareValue;
    m_Norm               = other.m_Norm;
    m_Biggest            = other.m_Biggest;
    m_Smallest           = other.m_Smallest;
    m_Mean               = other.m_Mean;
    m_Variance           = other.m_Variance;
    m_GaussIntegral      = other.m_GaussIntegral;
    m_NumberOfElements   = other.m_NumberOfElements;
    m_MeanIntervalls     = other.m_MeanIntervalls;
    m_VarianceIntervalls = other.m_VarianceIntervalls;
    m_UseBetterMeanAndVar= other.m_UseBetterMeanAndVar;

    return (*this);
  }

  // alias for copy member
  chiSquareFunctor&
    chiSquareFunctor::operator=(const chiSquareFunctor& other) {
    return (copy(other));
  }


  // clone member
  functor* chiSquareFunctor::clone() const {
    return new chiSquareFunctor(*this);
  }

  // return parameters
  const chiSquareFunctor::parameters&
    chiSquareFunctor::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      // throw invalidParametersException(getTypeName());
      // test
      static parameters defaultParams = parameters();
      return defaultParams;

    }
    return *par;
  }


  //----------------------------------------------------------
  // own Methods
  //----------------------------------------------------------

  // considers one element
  void chiSquareFunctor::consider(const double& element)  {
    m_ElementList.push_back(element);
    // and considers elements for calculation of mean and variance
    m_Stats.consider(element);
  }


  // considers one element freq times
  bool chiSquareFunctor::consider(const double&  element, const int freq) {
    if (freq <= 0)  {
      return false;
    }
    for( int i=0; i<freq; i++) {
      m_ElementList.push_back(element);
    }
    // and considers elements for calculation of mean and variance
    m_Stats.consider(element, freq);
    return true;
  }


  // consider all elements of the vector
  bool chiSquareFunctor::consider(const lti::vector<double> element)  {
    if (element.size() == 0)  {
      return false;
    }
    for ( int i=0; i<element.size(); i++) {
      m_ElementList.push_back(element[i]);
      // and considers elements for calculation of mean and variance
      m_Stats.consider(element[i]);
    }
    return true;
  }


  // consider each element of the vector freq times
  bool chiSquareFunctor::consider(const lti::vector<double> element, const int freq)  {
    if (element.size() == 0)  {
      return false;
    }
    if (freq <= 0)  {
      return false;
    }
    for ( int i=0; i<element.size(); i++)  {
      for ( int j=0; j<freq; j++) {
        m_ElementList.push_back(element[i]);
      }
      // and considers elements for calculation of mean and variance
      m_Stats.consider(element[i], freq);
    }
    return true;
  }


  // return number of intervalls
  int chiSquareFunctor::getNumberOfIntervalls() const {
    return m_NumberOfIntervalls;
  }


  // prints elements to standard outpt
  void chiSquareFunctor::showElementList()  {
    std::list<double>::iterator it;
    for(it = m_ElementList.begin(); it != m_ElementList.end(); it++) {
      std::cout << *it << std::endl;
    }
  }


  // deletes all elements from m_ElementList, m_IntervallVec
  // and m_BordersVec and sets member variables to initial values
  void chiSquareFunctor::resetElements()  {

    m_NumberOfIntervalls = 100;
    m_ChiSquareValue     = -1;
    m_Norm               = 0;
    m_Biggest            = 0;
    m_Smallest           = 0;
    m_Mean               = 0;
    m_Variance           = 0;
    m_GaussIntegral      = 0;
    m_NumberOfElements   = 0;
    m_MeanIntervalls     = 0;
    m_VarianceIntervalls = 0;
    m_UseBetterMeanAndVar= false;
    m_NameOfInfoFile     = "chiSquareInfo.txt";


    int z =  m_ElementList.size();
    int a;
    for(a=0; a<z; a++)  {
      m_ElementList.pop_back();
    }
    z = m_IntervallVec.size();
    for(a=0; a<z; a++)  {
      m_IntervallVec.pop_back();
    }
    z = m_BordersVec.size();
    for(a=0; a<z; a++)  {
      m_BordersVec.pop_back();
    }
  }


  // calculate mean and variance of all considered elements
  // and return variance
  double chiSquareFunctor::getVarianceOfData() {
    double mean;
    double variance;
    m_Stats.apply(mean, variance);
    return variance;
  }


  // calculate mean and variance of all considered elements
  // and return mean
  double chiSquareFunctor::getMeanOfData()  {
    double mean;
    double variance;
    m_Stats.apply(mean, variance);
    return mean;
  }


  // returns chiSquareValue or if apply has not been called jet
  // StatusString is set
  double chiSquareFunctor::getChiSquareValue()  {
    if(m_ChiSquareValue < 0) {
      setStatusString("apply must be called to calculate the chiSquareValue!");
      statusPushback(getStatusString());
    }
    return m_ChiSquareValue;
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool chiSquareFunctor::apply(double& chiSquare)  {

    // get the parameters the user has specified
    const parameters& param = getParameters();

    // clear output file if the new InfoFile should be saved
    m_NameOfInfoFile = param.nameOfInfoFile;
    if(param.saveInfoFile)  {
      std::ofstream infoFile(m_NameOfInfoFile.c_str(), std::ios::trunc);
    }

    // reset StatusString
    setStatusString("");

    // set m_NumberOfIntervalls,
    // use default if maxNumberOfIntervalls is smaller or euqal zero
    int maxNumberOfIntervalls = param.maxNumOfIntervalls;
    if(maxNumberOfIntervalls <= 0)  {
      // if maxNumberOfIntervalls is smaller zero, stick to default value (100)
      setStatusString("Using default maxNumberOfIntervall = 100");
      statusPushback(getStatusString());
    }
    // m_NumberOfIntervalls must not be smaller than 3;
    else if (maxNumberOfIntervalls<=3)  {
      m_NumberOfIntervalls = 3;
      setStatusString("Using smallest possible NumberOfIntervalls = 3");
      statusPushback(getStatusString());
    }
    else  {
      m_NumberOfIntervalls = maxNumberOfIntervalls;
    }

    // if there are not enough elements in the elementList
    if(m_ElementList.size() <= 6) {
      setStatusString("Not enough data" );
      // write infoFile now, because programm will stop here
      statusPushback(getStatusString());
      if(param.saveInfoFile)  {
        std::ofstream infoFile(m_NameOfInfoFile.c_str(), std::ios::app);
        infoFile << "chiSquareTest was not possible" << std::endl;
        infoFile << "STATUS" << std::endl;
        infoFile << "-----------------" << std::endl;
        std::list<std::string>::iterator stringIt;
        for(stringIt = m_StatusList.begin();
            stringIt != m_StatusList.end(); stringIt++)  {
          infoFile << *stringIt << std::endl;
        }
        infoFile.close();
      }
      return false;
    }

    // find Mean and Variance
    // test if variance and mean have been set by user
    // if not use mean and variance of given data (calculated by
    // serialStatsFunctor)
    if(param.mean > std::numeric_limits<double>::min()) {
      m_Mean = param.mean;
    }
    else  {
      m_Mean = getMeanOfData();
    }
    if(param.variance> std::numeric_limits<double>::min()) {
      m_Variance = param.variance;
    }
    else if(param.variance < 0) {
      setStatusString("variance must be positive! Set to absolute value!");
      statusPushback(getStatusString());
      m_Variance = fabs(param.variance);
    }
    else  {
      m_Variance = getVarianceOfData();
    }

    // find m_Biggest and m_Smallest element
    m_Biggest=  *m_ElementList.begin();
    m_Smallest= *m_ElementList.begin();
    std::list<double>::iterator it;
    for(it = m_ElementList.begin(); it != m_ElementList.end(); it++) {
      if(*it>m_Biggest) {
        m_Biggest=*it;
      }
      if(*it<m_Smallest)  {
        m_Smallest=*it;
      }
    }

    // find minimal data distance
    double distance;
    distance = m_Biggest-m_Smallest;
    double smallestDistance = distance;
    std::list<double>::iterator dataIt;
    double help1, help2;
    for(it = m_ElementList.begin(); it != m_ElementList.end(); it++) {
      dataIt = it;
      dataIt++;
      if(dataIt == m_ElementList.end()) {
        break;
      }
      help1 = *it;
      help2 = *dataIt;
      if(help1 != help2)  {
        if(smallestDistance > fabs(help2-help1)) {
          smallestDistance = fabs(help2-help1);
        }
      }
    }

    // find intervallWidth and m_NumberOfIntervalls
    // if the parameter minimalWidth is not explicitly set,
    // than minimalWidth is set to smallestDistance
    double intervallWidth;
    double minWidth = param.minimalWidth;
    if(minWidth < 0)  {
      minWidth = smallestDistance;
    }
    intervallWidth = (distance/(m_NumberOfIntervalls-1));
    // if intervalls are too small,
    // m_NumberOfIntervalls is set to a smaller value
    if(intervallWidth < minWidth) {
      intervallWidth = minWidth;
      m_NumberOfIntervalls = iround(distance/intervallWidth+1);
    }
    if(m_NumberOfIntervalls <= 2) {
      setStatusString("to few intervalls, because minimalWidth is to large");
      statusPushback(getStatusString());
      // write infoFile now, because programm will stop here
      if(param.saveInfoFile)  {
        std::ofstream infoFile(m_NameOfInfoFile.c_str(), std::ios::app);
        infoFile << "chiSquareTest was not possible" << std::endl;
        infoFile << "STATUS" << std::endl;
        infoFile << "-----------------" << std::endl;
        std::list<std::string>::iterator stringIt;
        for(stringIt = m_StatusList.begin();
            stringIt != m_StatusList.end(); stringIt++)  {
          infoFile << *stringIt << std::endl;
        }
        infoFile.close();
      }
      return false;
    }
    // if discrete is true, only multiple of distance are allowed
    // intervallWidth is set to next m_Biggest whole-numbered multiple
    if(param.discrete)  {
      double help;
      help = (int)(intervallWidth/smallestDistance );
      help +=1;
      intervallWidth = help*smallestDistance;
      // set new number of intervalls
      m_NumberOfIntervalls = iround(distance/intervallWidth+1);
    }
    if(m_NumberOfIntervalls <= 2) {
      setStatusString("to few intervalls");
      statusPushback(getStatusString());
      // write infoFile now, because programm will stop here
      if(param.saveInfoFile)  {
        std::ofstream infoFile(m_NameOfInfoFile.c_str(), std::ios::app);
        infoFile << "chiSquareTest was not possible" << std::endl;
        infoFile << "STATUS" << std::endl;
        infoFile << "-----------------" << std::endl;
        std::list<std::string>::iterator stringIt;
        for(stringIt = m_StatusList.begin();
            stringIt != m_StatusList.end(); stringIt++)  {
          infoFile << *stringIt << std::endl;
        }
      }
      return false;
    }

    // first set of IntervallBorders
    // (intervalls will later be combined in order to meet the constraints)
    // vector must contain m_NumberOfIntervalls+1 Borders
    for(int i=0; i<=m_NumberOfIntervalls; i++)  {
      m_BordersVec.push_back(((m_Smallest-(intervallWidth/2)) +
                              i*intervallWidth));
    }

    // find Number Of Elements
    double counter=0;
    for(it = m_ElementList.begin(); it != m_ElementList.end(); it++) {
      counter++;
    }
    m_NumberOfElements = counter;

    // try to set intervallBorders which meet constraints
    // returns false if there is not enough data
    if(!setIntervalls(param.equidistant)) {
      std::string status =getStatusString() ;
      if(status == "Using smallest possible m_NumberOfIntervalls = 3") {
        setStatusString("Couldn't set intervalls properly: maxNumberOfIntervalls too small" );
        statusPushback(getStatusString());
      }
      else  {
        setStatusString("Couldn't set intervalls properly!");
        statusPushback(getStatusString());
      }
      // write infoFile now, because programm will stop here
      if(param.saveInfoFile)  {
        std::ofstream infoFile(m_NameOfInfoFile.c_str(), std::ios::app);
        infoFile << "chiSquareTest was not possible" << std::endl;
        infoFile << "STATUS" << std::endl;
        infoFile << "-----------------" << std::endl;
        std::list<std::string>::iterator stringIt;
        for(stringIt = m_StatusList.begin();
            stringIt != m_StatusList.end(); stringIt++)  {
          infoFile << *stringIt << std::endl;
        }
        infoFile.close();
      }
      return false;
    }

    // counts number of elements in each intervall and store in m_IntervallVec
    setIntervallVec();

    // calculate mean and variance of data stored in intervalls
    // and check if these values can be used
    lti::serialStatsFunctor<double> stats;
    double middleOfIntervall;
    for(int k=0; k<static_cast<int>(m_IntervallVec.size()); k++) {
      middleOfIntervall=(m_BordersVec[k+1]-m_BordersVec[k])/2+m_BordersVec[k];
      stats.consider(middleOfIntervall, m_IntervallVec[k]);
    }
    stats.apply(m_MeanIntervalls, m_VarianceIntervalls);
    // check if all constraints are still met
    // if they are and the parameter useBetterMeanAndVar is true
    // m_UseBetterMeanAndVar is set to true and those values will be used
    if(param.useBetterMeanAndVar) {
      int x = m_BordersVec.size();
      if (integration(m_BordersVec[0],
                      m_BordersVec[1], 5)*m_NumberOfElements >= 1    &&
         integration(m_BordersVec[x-2],
                     m_BordersVec[x-1], 5)*m_NumberOfElements >=1    &&
         integration(m_BordersVec[1],
                     m_BordersVec[2], 5)*m_NumberOfElements >=5      &&
         integration(m_BordersVec[x-3],
                     m_BordersVec[x-2], 5)*m_NumberOfElements >=5) {
        m_UseBetterMeanAndVar = true;
      }
      else {
        m_UseBetterMeanAndVar = false;
        setStatusString("Could not use better Mean and Variance");
        statusPushback(getStatusString());
      }
    }

    // calculates area under gauss function and stores result in
    // m_Norm for normalization
    std::vector<double>::iterator vecIt;
    vecIt = m_BordersVec.end();
    vecIt--;
    if(!integrationAccuracy(m_BordersVec[0], *vecIt))  {
      setStatusString("Integration could not reach given accuracy");
      statusPushback(getStatusString());
    }
    m_Norm = m_GaussIntegral;

    // call calculation function for chi square
    chiSquareCalc();
    chiSquare = m_ChiSquareValue;

    // write all information about the results and errors in chiSquareInfo.txt
    // or to the file which name the user has specified
    if(param.saveInfoFile) {
      writeOutputFile();
    }

    return true;
}


  //--------------------------------------------------------------------------
  // private methods
  //--------------------------------------------------------------------------

  // returns false if there is not enough data
  bool chiSquareFunctor::setIntervalls(bool equidistant)  {

    double low;     // Borders
    double high;    // Borders
    bool success = true;

    // test if there is at least 1 in in the first intervall
    while(true) {
      low  = m_BordersVec[0];
      high = m_BordersVec[1];
      integrationAccuracy(low,high);
      if((m_GaussIntegral*m_NumberOfElements) < 1) {
        // first parameter is index of lower border of the intervall
        success = combineIntervalls(0, equidistant);
        if(!success)  {
          return false;
        }
      }
      else  {
        break;
      }
    }

    // test if there is at least 1 in the last intervall
    while(true) {
      low  = m_BordersVec[m_BordersVec.size()-2];
      high = m_BordersVec[m_BordersVec.size()-1];
      integrationAccuracy(low,high);
      if(m_GaussIntegral*m_NumberOfElements < 1) {
        // first parameter is index of lower border of the intervall
        success = combineIntervalls((m_BordersVec.size()-2), equidistant);
        if(!success)  {
          return false;
        }
      }
      else  {
        break;
      }
    }

    // test if there are at least 5 in inner intervalls
    while(true) {
      int counter = 0;
      for(int z=1; z<static_cast<int>(m_BordersVec.size()-2);z++)  {
        low  = m_BordersVec[z];
        high = m_BordersVec[z+1];
        integrationAccuracy(low,high);
        if(m_GaussIntegral*m_NumberOfElements < 5) {
          // first parameter is index of lower border of the intervall
          success = combineIntervalls(z, equidistant);
          if(!success)  {
            return false;
          }
        }
        else  {
          counter++;
        }
      }
      if(counter == static_cast<int>(m_BordersVec.size()-3)) {
        break;
      }
    }
    return true;
  }


  // calculates the chiSquare value and stores it in private variable
  // m_ChiSquareValue
  void chiSquareFunctor::chiSquareCalc()  {

    double temp;
    double numOfElements = m_ElementList.size();

    m_ChiSquareValue = 0;
    for (int j=0; j < (m_NumberOfIntervalls); j++)  {
      integrationAccuracy(m_BordersVec[j], m_BordersVec[j+1]);

      double normalizedGauss = numOfElements*m_GaussIntegral/m_Norm;
      m_ExpectedVec.push_back(normalizedGauss);

      temp = (m_IntervallVec[j]- normalizedGauss);
      double temp2 = (temp*temp/normalizedGauss);

      m_ChiSquareValue += temp2;
    }
  }


  // numerical integration of the gauss Func between the given borders
  double chiSquareFunctor::integration(double lowBorder,
                                       double highBorder, int depth) {

    double width;
    double sum =0;
    double x;
    int    j;
    double steps = pow(2.0, depth);

    width = ((highBorder - lowBorder)/steps);
    x = lowBorder + 0.5*width;

    for(j=1; j<=steps; j++, x+=width) {
      sum += gaussFunc(x);
    }
    return ((highBorder-lowBorder)*sum/steps);

  }


  // returns the result of the gauss function
  double chiSquareFunctor::gaussFunc(double x)  {
    double mean = m_Mean;
    double variance = m_Variance;
    if(m_UseBetterMeanAndVar == true) {
      mean = m_MeanIntervalls;
      variance = m_VarianceIntervalls;
    }
    return (1/(sqrt(2*lti::Pi*variance))*exp(-0.5*sqr(x-mean)/variance));
  }


  // calls integration function untill accuracy or maxSteps is reached
  // returns false if the given accuracy could not be reached in maxSteps
  // result of integration is stored in m_GaussIntegral
  bool chiSquareFunctor::integrationAccuracy(double lowBorder,
                                             double highBorder) {
    double olds;
    double result(0.);
    const parameters& param = getParameters();
    double acc      = param.accuracy;
    double maxSteps = param.maxSteps;

    // check if acc has a reasonable value
    if(acc < 1.0e-10) {
      acc = 1.0e-10;
      setStatusString("accuracy was too small, set to minimum value 1.0e-10");
      statusPushback(getStatusString());
    }

    // check if maxSteps has a reasonable value
    if(maxSteps > 15) {
      maxSteps = 15;
      setStatusString("maxSteps was too large, set to maximum value 15");
      statusPushback(getStatusString());
    }
    else if(maxSteps < 5) {
      maxSteps = 5;
      setStatusString("maxSteps was too small; set to minmum value 5");
      statusPushback(getStatusString());
    }

    // set olds to smallest possible value of double
    olds = std::numeric_limits<double>::min();

    for(int j=4; j<maxSteps; j++) {
      result = integration(lowBorder, highBorder, j);
      if(fabs(result-olds) < acc*fabs(olds)) {
        m_GaussIntegral = result;
        return true;
      }
      olds = result;
    }
    m_GaussIntegral = result;
    return false;
  }


  // builds a list, which stores how many elements are in each intervall
  void chiSquareFunctor::setIntervallVec() {
    std::list<double>::iterator it;
    for(int i=0; i<static_cast<int>(m_BordersVec.size()-1); i++) {
      double low   = m_BordersVec[i];
      double high  = m_BordersVec[i+1];
      int counter =0;

      for(it = m_ElementList.begin(); it != m_ElementList.end(); it++) {
        // count if element is in intervall
        if(*it<=high && *it>low )  {
          counter ++;
        }
      }
      m_IntervallVec.push_back(counter);
    }
  }


  // combines intervalls untill constraints are met
  bool chiSquareFunctor::combineIntervalls(int index, bool equidistant) {

    while(m_BordersVec.size() > 4) {

      // equidistant
      if(equidistant) {
        double distance = m_Biggest-m_Smallest;
        double intervallWidth = (distance/(m_NumberOfIntervalls-1));
        // m_NumberOfIntervalls--;
        for(int i=0; i<=m_NumberOfIntervalls; i++)  {
          m_BordersVec[i] = (((m_Smallest-(intervallWidth/2)) + i*intervallWidth));
        }
        m_BordersVec.pop_back();
        m_NumberOfIntervalls--;
        return true;
      }

      // not equidistant
      else  {
        if(index < static_cast<int>(m_BordersVec.size()-2))  {
          // than combine with left neighbour
          int j = (index+2);
          for(int i = (index+1); i<=static_cast<int>(m_BordersVec.size()-1); i++) {
            m_BordersVec[i]=m_BordersVec[j];
            j++;
          }
          m_BordersVec.pop_back();
          m_NumberOfIntervalls--;
          return true;
        }
        // if it is the index of the last intervall
        else  {
          // combine with right neighbour
          m_BordersVec[index] = m_BordersVec[index+1];
          m_BordersVec.pop_back();
          m_NumberOfIntervalls--;
          return true;
        }
      }
    }
    return false;
  }

  // checks if string is allready in m_StringList, if not string is added
  void chiSquareFunctor::statusPushback(std::string status)  {

    std::list<std::string>::iterator it;
    for(it = m_StatusList.begin(); it != m_StatusList.end(); it++)  {
      if(status == *it) {
        return;
      }
    }
    m_StatusList.push_back(status);

  }

  // writes all information to file
  void chiSquareFunctor::writeOutputFile()  {
    // write content in chiSquareInfo.txt
    const parameters& param = getParameters();
    std::ofstream infoFile(m_NameOfInfoFile.c_str(), std::ios::app);

    // general information
    infoFile << "GENERAL INFORMATION" << std::endl;
    infoFile << "-----------------" << std::endl;
    infoFile << std::setw(22) << "Chi-Square Value: " << std::setw(8)
             << m_ChiSquareValue << std::endl;
    infoFile << std::setw(22) << "Number of Intervalls: " << std::setw(8)
             << getNumberOfIntervalls() << std::endl;
    infoFile << std::setw(22) << "Number of Elements: " << std::setw(8)
             << m_NumberOfElements << std::endl;
    infoFile << std::setw(22) << "Biggest Element: " << std::setw(8)
             << m_Biggest << std::endl;
    infoFile << std::setw(22) << "Smallest Element: " << std::setw(8)
             << m_Smallest << std::endl;
    infoFile << std::endl;

    // parameters the user has specified
    infoFile << "PARAMETERS" << std::endl;
    infoFile << "-----------------" << std::endl;
    infoFile << "If the user has set wrong parameters, default values will be used (See 'Status')." << std::endl;
    infoFile << "If the user has not specified a parameter the default value is given" << std::endl<< std::endl;
    infoFile << std::setw(22) << "accuracy: "  << std::setw(8)
             << param.accuracy << std::endl;
    infoFile << std::setw(22) << "discrete: "  << std::setw(8)
             << param.discrete << std::endl;
    infoFile << std::setw(22) << "equidistant: "  << std::setw(8)
             << param.equidistant<< std::endl;
    infoFile << std::setw(22) << "maxNumOfIntervalls: "  << std::setw(8)
             << param.maxNumOfIntervalls << std::endl;
    infoFile << std::setw(22) << "maxSteps: "  << std::setw(8)
             << param.maxSteps<< std::endl;
    infoFile << std::setw(22) << "mean: "  << std::setw(8) << param.mean
             << std::endl;
    infoFile << std::setw(22) << "variance: "  << std::setw(8)
             << param.variance<< std::endl;
    infoFile << std::setw(22) << "minimalWidth: "  << std::setw(8)
             << param.minimalWidth<< std::endl;
    infoFile << std::setw(22) << "useBetterMeanAndVar: " << std::setw(8)
             << param.useBetterMeanAndVar << std::endl;
    infoFile << std::setw(22) << "nameOfInfoFile: " << std::setw(8)
             << param.nameOfInfoFile << std::endl;
    infoFile << std::endl;

    // information about mean and variance
    infoFile << "MEAN AND VARIANCE" << std::endl;
    infoFile << "-----------------" << std::endl;
    if(param.mean > std::numeric_limits<double>::min())  {
      infoFile << std::setw(22) <<  "Mean (user): " << std::setw(10)
               << param.mean << std::endl;
    }
    else  {
      infoFile << std::setw(22) << "Mean (user): " << std::setw(10)
               << "none" << std::endl;
    }
    if(param.variance > std::numeric_limits<double>::min())  {
      infoFile << std::setw(22) << "Variance (user): " << std::setw(10)
               << param.variance << std::endl;
    }
    else  {
      infoFile << std::setw(22) << "Variance (user): " << std::setw(10)
               << "none" << std::endl;
    }
    infoFile << std::setw(22) << "Mean of Data: " << std::setw(10)
             << m_Mean << std::endl;
    infoFile << std::setw(22) << "Variance of Data: " << std::setw(10)
             << m_Variance << std::endl;
    infoFile << std::setw(22) << "After data was put into intervalls"
             << std::endl;
    infoFile << std::setw(22) << "Mean of Data: " << std::setw(10)
             << m_MeanIntervalls << std::endl;
    infoFile << std::setw(22) << "Variance of Data: " << std::setw(10)
             << m_VarianceIntervalls << std::endl;
    infoFile << std::endl;

    // information about borders, intervalls and expected elements
    infoFile << "INTERVALLS" << std::endl;
    infoFile << "-----------------" << std::endl;
    infoFile << std::setw(10) << "border" << std::setw(10) << "elements"
             << std::setw(10) << "expected" << std::endl;
    for(int i=0; i<static_cast<int>(m_IntervallVec.size()); i++) {
      infoFile << std::setw(10) << m_BordersVec[i] << std::endl;
      infoFile << std::setw(20) << m_IntervallVec[i] << std::setw(10)
               << m_ExpectedVec[i] << std::endl;
    }
    std::vector<double>::iterator vecIt;
    vecIt = m_BordersVec.end();
    vecIt--;
    infoFile << std::setw(10) << *vecIt << std::endl;
    infoFile << std::endl;

    // writing m_StatusList in chiSquareInfo.txt
    infoFile << "STATUS" << std::endl;
    infoFile << "-----------------" << std::endl;
    std::list<std::string>::iterator stringIt;
    for(stringIt = m_StatusList.begin();
        stringIt != m_StatusList.end(); stringIt++)  {
      infoFile << *stringIt << std::endl;
    }
    infoFile.close();

  }

} // end of namespace lti
