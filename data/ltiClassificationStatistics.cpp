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
 * file .......: ltiClassificationStatistics.h
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 08.10.2000
 * revisions ..: $Id: ltiClassificationStatistics.cpp,v 1.9 2006/09/05 09:56:27 ltilib Exp $
 */

#include "ltiClassificationStatistics.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio> // for sprintf(...)

#include "ltiLispStreamHandler.h"
#include "ltiSTLIoInterface.h"

namespace lti {

  // -------------------------------------------------------------
  // classificationStatistics::parameters
  // -------------------------------------------------------------

  // default constructor
  classificationStatistics::parameters::parameters() : ioObject() {
    // default values
    useNames             = false;
    namesFile            = "objectNames.nms";
    namesInFileFirst     = false;

    writeConfusionMatrix = true;
    confusionMatrix      = "confusion.mat";
    suppressZeros        = true;

    saveStep             = 100;

    useLogFile           = true;
    logFile              = "logfile.txt";
    logAllPatterns       = false;

    path                 = "";
  }

  // copy constructor
  classificationStatistics::parameters::parameters(const parameters& other) 
    : ioObject() {
    copy(other);
  }

  // destructor
  classificationStatistics::parameters::~parameters() {
  }

  // get type name
  const char* classificationStatistics::parameters::getTypeName() const {
    return "classificationStatistics::parameters";
  }

  // copy member

  classificationStatistics::parameters&
  classificationStatistics::parameters::copy(const parameters& other) {

    useNames = other.useNames;
    namesFile = other.namesFile;
    namesInFileFirst = other.namesInFileFirst;

    writeConfusionMatrix = other.writeConfusionMatrix;
    confusionMatrix = other.confusionMatrix;
    suppressZeros = other.suppressZeros;

    saveStep = other.saveStep;

    useLogFile = other.useLogFile;
    logFile    = other.logFile;
    logAllPatterns = other.logAllPatterns;

    path = other.path;

    return (*this);
  }

  // alias for copy
  classificationStatistics::parameters&
  classificationStatistics::parameters::operator=(const parameters& other) {

    return copy(other);
  }

  // clone member
  classificationStatistics::parameters*
  classificationStatistics::parameters::clone() const {
    return new parameters(*this);
  }

# ifndef _LTI_MSC_6
  bool classificationStatistics::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool classificationStatistics::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"useNames",useNames);
      lti::write(handler,"namesFile",namesFile);
      lti::write(handler,"namesInFileFirst",namesInFileFirst);
      lti::write(handler,"suppressZeros",suppressZeros);
      lti::write(handler,"writeConfusionMatrix",writeConfusionMatrix);
      lti::write(handler,"confusionMatrix",confusionMatrix);
      lti::write(handler,"saveStep",saveStep);
      lti::write(handler,"useLogFile",useLogFile);
      lti::write(handler,"logFile",logFile);
      lti::write(handler,"logAllPatterns",logAllPatterns);
      lti::write(handler,"path",path);
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classificationStatistics::parameters::write(ioHandler& handler,
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
  bool classificationStatistics::parameters::read(ioHandler& handler,
						  const bool complete)
# else
  bool classificationStatistics::parameters::readMS(ioHandler& handler,
						    const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"useNames",useNames);
      lti::read(handler,"namesFile",namesFile);
      lti::read(handler,"namesInFileFirst",namesInFileFirst);
      lti::read(handler,"suppressZeros",suppressZeros);
      lti::read(handler,"writeConfusionMatrix",writeConfusionMatrix);
      lti::read(handler,"confusionMatrix",confusionMatrix);
      lti::read(handler,"saveStep",saveStep);
      lti::read(handler,"useLogFile",useLogFile);
      lti::read(handler,"logFile",logFile);
      lti::read(handler,"logAllPatterns",logAllPatterns);
      lti::read(handler,"path",path);     
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool classificationStatistics::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // -------------------------------------------------------------
  // classificationStatistics
  // -------------------------------------------------------------

  // log-file format widths
  const int classificationStatistics::logWidths[] = {5,30,15,14};
  // confusion-matrix format widths
  const int classificationStatistics::confWidths[] = {20,14,20};


  classificationStatistics::classificationStatistics() {
    reset();
  }

  classificationStatistics::classificationStatistics(const parameters& par) {
    setParameters(par);
  }

  classificationStatistics::~classificationStatistics() {
    flush();
  }

  void classificationStatistics::flush() {
    // everything which has not been saved yet, must be now flushed!
    if ((param.writeConfusionMatrix) && !saved) {
      writeData();
    }
  }

  void classificationStatistics::reset() {

    // initialize the confusion matrices

    confMatrix.clear();
    confMatrix.setErrorValue(-1.0);

    idToNamesMap.clear();

    idToNamesMap[CorrectSingle]  = "correctSingle";
    idToNamesMap[CorrectMultiple]= "correctMultiple";
    idToNamesMap[CorrectReject]  = "correctReject";

    idToNamesMap[WrongSingle]    = "wrongSingle";
    idToNamesMap[WrongMultiple]  = "wrongMultiple";
    idToNamesMap[WrongReject]    = "wrongReject";

    if ( param.useNames ) {
      const std::string filename = param.path + param.namesFile;
      loadNames(filename,idToNamesMap,param.namesInFileFirst);
    }

    confMatrix.addColumn(CorrectSingle,0.0);
    confMatrix.addColumn(CorrectMultiple,0.0);
    confMatrix.addColumn(CorrectReject,0.0);

    confMatrix.addColumn(WrongSingle,0.0);
    confMatrix.addColumn(WrongMultiple,0.0);
    confMatrix.addColumn(WrongReject,0.0);

    // nBest matrix
    nBest.clear();
    nBestMultiple.clear();
    minPlaces.clear();

    numberOfResults = 0;
    correctSingle = 0;
    correctMultiple = 0;
    correctReject = 0;
    wrongSingle = 0;
    wrongMultiple = 0;
    wrongReject = 0;
    saved = true;
  }

  bool classificationStatistics::setParameters(const parameters& par) {
    param.copy(par);
    reset();
    return true;
  }

  bool 
  classificationStatistics::loadNames(const std::string& filename,
                                      std::map<int,std::string>& idToNamesMap,
                                      bool namesInFileFirst) {
    std::ifstream input;
    std::string name;
    int id;

    input.open(filename.c_str(),std::ios::in);

    if (input.is_open()) {
      lispStreamHandler lsh(input);
      bool b = true;
      if (namesInFileFirst) {
        while (b && !input.eof()) {
          b = b && lsh.read(name);
          b = b && lsh.readKeyValueSeparator();
          b = b && lsh.read(id);
          b = b && lsh.readKeyValueSeparator();
          if (b && (idToNamesMap.find(id) == idToNamesMap.end())) {
            idToNamesMap[id] = name;
          }
        }
      } else {
        while (b && !input.eof()) {
          b = b && lsh.read(id);
          b = b && lsh.readKeyValueSeparator();
          b = b && lsh.read(name);
          b = b && lsh.readKeyValueSeparator();
          if (b && (idToNamesMap.find(id) == idToNamesMap.end())) {
            idToNamesMap[id] = name;
          }
        }
      }
      input.close();
    } else {
      return false;
    }

    return true;
  }

  bool classificationStatistics::setNames(const std::map<int,std::string>& idMap) {

    idToNamesMap.clear();

    idToNamesMap[CorrectSingle]  = "correctSingle";
    idToNamesMap[CorrectMultiple]= "correctMultiple";
    idToNamesMap[CorrectReject]  = "correctReject";

    idToNamesMap[WrongSingle]    = "wrongSingle";
    idToNamesMap[WrongMultiple]  = "wrongMultiple";
    idToNamesMap[WrongReject]    = "wrongReject";

    std::map<int,std::string>::const_iterator it;

    it = idMap.begin();

    while (it != idMap.end()) {

      idToNamesMap.insert((*it));
      it++;
    }

    return true;
  }

  bool classificationStatistics::inscribeObject(const std::string& name,
                                                const int& id) {


    /*
    std::ofstream debg("debug.txt");
    lispStreamHandler lsh(debg);
    write(lsh, idToNamesMap);
    debg.flush();
    */

    // id number will be used to check if the object has been already
    // inscribed and NOT the name, which could be different
    if (idToNamesMap.find(id) == idToNamesMap.end()) {
      idToNamesMap[id] = name;
    }

    if (!confMatrix.isRowValid(id)) {
      // this object hat not beed used yet!
      // make entry in the matrices:
      confMatrix.addRow(id,0.0);
      confMatrix.addColumn(id,0.0);
    }

    if (!nBest.isRowValid(id)) {
      nBest.addRow(id,0.0);
      nBestMultiple.addRow(id,0.0);
      minPlaces.addRow(id,0.0);
    }

    return true;
  }

  classificationStatistics::eStatistics
  classificationStatistics::consider(const classifier::outputVector& result,
                                     const int& realObjectID,
                                     const std::string& patternInfo) {
    // (empty classifier::outputVector?)
    if ( (result.size() == 0) || (realObjectID<0) ) { 
      return Error;
    }

    int numClasses;
    eStatistics recognitionState;

    numberOfResults++;

    std::ostringstream oss;
    oss << realObjectID;
    if (idToNamesMap.find(realObjectID) == idToNamesMap.end()) {
      idToNamesMap[realObjectID] = oss.str();
    }
    inscribeObject(idToNamesMap[realObjectID], realObjectID);

    bool lCorrectSingle	  = false;
    bool lCorrectMultiple = false;
    bool lCorrectReject   = false;

    bool lWrongSingle	  = false;
    bool lWrongMultiple   = false;
    bool lWrongReject	  = false;

    // calculate the position of the object in the sorted list, and generate
    // a list of the number of object-groups with same probability

    // probGroup[n] = m means that the n-th position belongs to group m.
    // It is always valid m<=n;
    // probGroup[0] highest probability group
    // probGroup[1] second highest probability group
    // and so on...

    int i,pos,n,id;
    double prob, highestProb;

    classifier::outputVector sorted(result);
    sorted.sortDescending();

    numClasses = sorted.size();

    std::vector<int> probGroup(numClasses,0);
    std::map<int,int> elemInGroup;

    double actualProb;
    sorted.getValueByPosition(0, actualProb);
    int actualGroup = 0;

    pos = -1; // Flag for "unknown object"
    sorted.getValueByPosition(0, highestProb); //highest probablility in sorted

    for (i = 0; i<numClasses; i++) {

      sorted.getPair(i, id, prob);

      if (id == realObjectID) {// has i-th pos. the realObj?
        pos = i;
      }

      // similar to (actualProb == sorted.objProbs[idx]) but with less
      // quantisation errors
      if (actualProb - prob > classifier::nullProbability) {
        actualGroup++;
        actualProb = prob;
      }
      probGroup[i] = actualGroup;
      elemInGroup[actualGroup]++;
    }

    // calculate the six basic-values
    int winnerPos=sorted.getWinnerUnit();
    double winnerProb=0.;
    if (winnerPos>=0) {
      sorted.getValueByPosition(winnerPos, winnerProb);
    }

    lCorrectSingle = (probGroup[1] != probGroup[0]) && (pos == 0);
    if (lCorrectSingle) {
      correctSingle++;
      recognitionState = CorrectSingle;
      confMatrix.at(realObjectID,CorrectSingle)++;
    }

    lCorrectMultiple = !lCorrectSingle && (probGroup[pos] == 0) &&
      (highestProb > classifier::nullProbability);

    if (lCorrectMultiple) {
      correctMultiple++;
      recognitionState = CorrectMultiple;
      confMatrix.at(realObjectID,CorrectMultiple)++;
    }

    lCorrectReject   =  (pos == -1) &&
      (sorted.isRejected() ||
       highestProb < classifier::nullProbability   ||
       (((winnerPos >= 0) &&
        (winnerProb < classifier::nullProbability))));

    if (lCorrectReject) {
      correctReject++;
      recognitionState = CorrectReject;
      confMatrix.at(realObjectID,CorrectReject)++;
    }

    lWrongSingle =  (probGroup[1] != probGroup[0]) && (pos != 0);

    if (lWrongSingle) {
      wrongSingle++;
      recognitionState = WrongSingle;
      confMatrix.at(realObjectID,WrongSingle)++;
    }

    lWrongMultiple = !lWrongSingle && (probGroup[pos] != 0);
    if (lWrongMultiple) {
      wrongMultiple++;
      recognitionState = WrongMultiple;
      confMatrix.at(realObjectID,WrongMultiple)++;
    }

    lWrongReject = (pos >= 0) &&
      (sorted.isRejected() ||
       highestProb < classifier::nullProbability   ||
       (((winnerPos >= 0) &&
        (winnerProb < classifier::nullProbability))));

    if (lWrongReject) {
      wrongReject++;
      recognitionState = WrongReject;
      confMatrix.at(realObjectID,WrongReject)++;
    }

    // ...with this values make the corresponding entries in the confusion
    // matrix and the n-best matrix

    // calculation of the n-best can be done only if a classification was done!
    sorted.getValueByPosition(pos, prob);
    if (prob > classifier::nullProbability) {
      int minPlacesLocal;

      i = probGroup[pos];

      for (n = pos;(n>=0) && (probGroup[n] == i);n--);

      minPlacesLocal = n + 1 + elemInGroup[i];

      n++; // to compensate the last n--!
      n++; // to be in a 1 to n range.

      if (!nBest.isColumnValid(n)) {
        nBest.addColumn(n,0);
        nBestMultiple.addColumn(n,0);
      }

      nBest.at(realObjectID,n)++;

      if (elemInGroup[i]>1) {
        nBestMultiple.at(realObjectID,n)++;
      }

      if (!minPlaces.isColumnValid(minPlacesLocal)) {
        minPlaces.addColumn(minPlacesLocal,0);
      }

      minPlaces.at(realObjectID,minPlacesLocal)++;
    }

    // confusion matrix
    double addValue;

    if (lCorrectSingle || lWrongSingle) {
      int colId;
      sorted.getId(0, colId);
      if (!confMatrix.isRowValid(colId)) {
        if (idToNamesMap.find(colId)==idToNamesMap.end()) {
          char t[50];
          sprintf(t,"%i",colId);
          idToNamesMap[colId]=std::string(t);
        }
        inscribeObject(idToNamesMap[colId],colId);
      }

      confMatrix.at(realObjectID,colId)++;
    }

    if (lCorrectMultiple || lWrongMultiple) {
      addValue = 1.0/double(elemInGroup[probGroup[0]]);

      // make an entry for all elements in the "probability Group"

      int colId;
      for (i=0;i<numClasses;i++) {
        if (probGroup[i] == probGroup[0]) {
           sorted.getId(i, colId);
          if (!confMatrix.isRowValid(colId))	{
            if (idToNamesMap.find(colId)==idToNamesMap.end()) {
              char t[50];
              sprintf(t,"%i",colId);
              idToNamesMap[colId]=std::string(t);
            }
            inscribeObject(idToNamesMap[colId],colId);
          }
          confMatrix.at(realObjectID,colId)+=addValue;
        }
      }
    }

    // write the result on the stream!
    if (param.writeConfusionMatrix &&
        (param.saveStep > 0) &&
        ((numberOfResults % param.saveStep) == 0)) {
      writeData();
      saved = true;
    } else {
      saved = false;
    }

    if (param.useLogFile)
      writeLogFile(sorted,recognitionState,realObjectID,patternInfo);

    return recognitionState;
  }


  bool classificationStatistics::writeData(std::ostream& ostr) {

    // write date and type of statistics
    struct tm *newtime;
    time_t aclock;
    time( &aclock );                 // Get time in seconds
    newtime = localtime( &aclock );  // Convert time to struct tm form

    ostr << "Confusion-Matrix LTI-Lib V1.0" << std::endl;
    ostr << "generated at " << asctime( newtime ) << std::endl;
    ostr << std::endl;

    ostr << std::endl << std::endl;

    bool success = true;
    success &= writeConfusionMatrix(ostr, confMatrix);
    success &= writeNBestMatrix(ostr,nBest,
                     "n-Best Statistics (not normed)",false);
    success &= writeNBestMatrix(ostr,nBestMultiple,
                     "shared n-Best positions (not normed)",false);
    success &= writeNBestMatrix(ostr,  minPlaces,
                     "Minimal number of positions to considered an object",
                     false,"Stelle","Stellen");

    return success;
  }

  bool classificationStatistics::writeData() {
    // write the results on the file/stream

    std::string filename;
    filename = param.path + param.confusionMatrix;
    std::ofstream out(filename.c_str(), std::ios::out);

    bool success = false;

    if (out.is_open()) {

      success = writeData(out);
      out.close();
    }

    return success;
  }

  bool
  classificationStatistics::writeConfusionMatrix(std::ostream& out,
                                      const dynamicMatrix<double>& aMatrix) {
    if( out.good() ) {
      out.precision(6);
      out.flags( out.flags() | std::ios::fixed|std::ios::showpoint);

      int numberOfClasses,numberOfColumns;
      int i,realObject;
      std::string name;

      numberOfClasses = aMatrix.rows();
      numberOfColumns = aMatrix.columns();

      std::vector<int> seqToRealId(numberOfColumns,0);
      // const dynamicMatrix<double>::stdIndexList& theRows =
      // aMatrix.getUsedRows();

      sortObjNames(aMatrix,seqToRealId);

      for (i=numberOfClasses; i<numberOfColumns; i++) {
        seqToRealId[i] = numberOfClasses-i-1; // -1 = singleCorrect,
                                              // -2 multipleCorrect, etc.
      }

      out << numberOfClasses << " Classes" << std::endl << std::endl;

      // Header

      out.width(confWidths[0]);
      out << "---    ";

      for (i=0;i<numberOfColumns;i++) {

        name = idToNamesMap[seqToRealId[i]];
        // cut long names!
        name = name.substr(0,confWidths[1]-1);
        out.width(confWidths[1]);
        out << name.c_str();
      }

      out << std::endl << std::endl;

      // write confusion aMatrix (line by line)

      for (realObject = 0;realObject < numberOfClasses;realObject++) {
	      // 1. RealNameId!

        name = idToNamesMap[seqToRealId[realObject]];
        name = name.substr(0,confWidths[0]-1);
        out.width(confWidths[0]);
        out << name.c_str();

        double totalProb = 0;

        // Normalisieren!
        for (i=0;i<numberOfClasses;i++)  //all classes and unknown objects
          totalProb+=aMatrix.at(seqToRealId[realObject],seqToRealId[i]);

        totalProb+=aMatrix.at(seqToRealId[realObject],CorrectReject);
        totalProb+=aMatrix.at(seqToRealId[realObject],WrongReject);

        for (i=0;i<numberOfColumns;i++) {
          double prob;
          prob = aMatrix.at(seqToRealId[realObject],seqToRealId[i])/totalProb;
          if (prob > classifier::nullProbability) {
            format(out,confWidths[1],prob);
          } else {
            format(out,confWidths[1],0.0);
          }
        }
        out << std::endl;
      }
      out << std::endl;

      out.width(confWidths[2]);
      out << "Classifications";
      format(out,confWidths[1],numberOfResults);
      out << std::endl;

      out.width(confWidths[2]);
      out << "correctSingle";
      format(out,confWidths[1],(double)correctSingle/(double)numberOfResults);
      out << std::endl;

      out.width(confWidths[2]);
      out << "correctMultiple";
      format(out,confWidths[1],(double)correctMultiple/
                               (double)numberOfResults);
      out << std::endl;

      out.width(confWidths[2]);
      out << "correctReject";
      format(out,confWidths[1],(double)correctReject/(double)numberOfResults);
      out << std::endl;

      out.width(confWidths[2]);
      out << "wrongSingle";
      format(out,confWidths[1],(double)wrongSingle/(double)numberOfResults);
      out << std::endl;

      out.width(confWidths[2]);
      out << "wrongMultiple";
      format(out,confWidths[1],(double)wrongMultiple/(double)numberOfResults);
      out << std::endl;

      out.width(confWidths[2]);
      out << "wrongReject";
      format(out,confWidths[1],(double)wrongReject/(double)numberOfResults);
      out << std::endl;

      out << std::endl;

      return true;
    } else {
      return false;
    }
  }

  void
  classificationStatistics::sortObjNames(const dynamicMatrix<double>& aMatrix,
                                         std::vector<int>& seqToRealId) {
    int numberOfClasses;
    int i;
    std::string name;

    numberOfClasses = aMatrix.rows();

    const dynamicMatrix<double>::stdIndexList& theRows = aMatrix.getUsedRows();
    std::map<std::string,int> namesToIdMap;
    std::list<std::string> theNames;

    dynamicMatrix<double>::stdIndexList::const_iterator it;
    std::list<std::string>::const_iterator its;

    for (i=0,it = theRows.begin();i<numberOfClasses;i++,it++) {
      name = idToNamesMap[*it];
      theNames.push_back(name);
      namesToIdMap[name] = *it;
    }

    theNames.sort();

    for (i=0,its = theNames.begin();its != theNames.end();its++,i++) {
      seqToRealId[i] = namesToIdMap[*its];
    }

    namesToIdMap.clear();
    theNames.clear();
  }

  bool classificationStatistics::writeNBestMatrix(std::ostream& out,
                                       const dynamicMatrix<double>& aMatrix,
                                       const std::string& title,
                                       const bool& normalize,
                                       const std::string& labelSing,
                                       const std::string& labelPlur) {
    if( out.good() ) {

      if (normalize) {
        out.precision(6);
        out.flags( out.flags() | std::ios::fixed|std::ios::showpoint);
      }	else {
        out.precision(0);
        out.flags( out.flags() | std::ios::fixed);
      }

      int numberOfClasses,numberOfColumns;
      int i,realObject;
      std::string name;
      char buffer[1024];

      numberOfClasses = aMatrix.rows();
      numberOfColumns = aMatrix.columns();

      std::vector<int> seqToRealId(numberOfClasses,0);

      const dynamicMatrix<double>::stdIndexList& theCols =
        aMatrix.getUsedColumns();
      dynamicMatrix<double>::stdIndexList::const_iterator it;

      dynamicMatrix<double>::stdIndexList theSortedCols = theCols;
      theSortedCols.sort();

      sortObjNames(aMatrix,seqToRealId);

      // Header

      out << title << std::endl << std::endl;

      out.width(confWidths[0]);
      out << "---    ";

      for (it = theSortedCols.begin(),i=0;i<numberOfColumns;i++,it++) {
        if (i==0)
          sprintf(buffer,"%i-%s",(*it),labelSing.c_str());
        else
          sprintf(buffer,"%i-%s",(*it),labelPlur.c_str());

        out.width(confWidths[1]);
        out << buffer;
      }

      out << std::endl;

      // write confusion aMatrix (line by line)

      std::vector<double> totalNBest(numberOfColumns,0.0);

      for (realObject = 0;realObject < numberOfClasses;realObject++) {
	// 1. RealNameId!

        name = idToNamesMap[seqToRealId[realObject]];
        name = name.substr(0,confWidths[0]-1);
        out.width(confWidths[0]);
        out << name.c_str();

        double totalProb = 1.0;

        if (normalize) {
          totalProb = 0.0;
          // Normalize
          //all nBest-classes
          for (it = theSortedCols.begin(),i=0;i<numberOfColumns;i++,it++)
            totalProb+=aMatrix.at(seqToRealId[realObject],(*it));
        }

        for (it = theSortedCols.begin(),i=0;i<numberOfColumns;i++,it++) {
          double prob,value;
          value = aMatrix.at(seqToRealId[realObject],(*it));
          prob = value/totalProb;
          totalNBest[i] += value;
          if (prob > classifier::nullProbability)
            format(out,confWidths[1],prob);
          else
            format(out,confWidths[1],0.0);
        }
        out << std::endl;
      }
      out << std::endl;

      // totalNBest accumulative

      double accu = 0;

      out << "Total 'n-Best' classification:" << std::endl << std::endl;
      out.width(confWidths[0]);
      out << " ";

      out.precision(6);

      for (i=0;i<numberOfColumns;i++) {
        accu += totalNBest[i];

        format(out,confWidths[1],accu/double(numberOfResults));
      }

      out << std::endl << std::endl;

      return true;
    } else {
      return false;
    }
  }

  bool classificationStatistics::writeLogFileHeader(const std::string& filename) {

    std::ofstream output;
    output.open(filename.c_str(),std::ios::out);

    if (output.is_open()) {
      int col = 0;
      // Header
      struct tm *newtime;
      time_t aclock;
      time( &aclock );                 // Get time in seconds
      newtime = localtime( &aclock );  // Convert time to struct tm form

      output << "Classification Log File, LTI-Lib V1.0"  << std::endl;
      output << "generated at " << asctime( newtime )
             << std::endl << std::endl;

      output << std::endl;

  	  output << std::endl << std::endl;

  	  output.width(logWidths[col++]);
  	  output << "ID#";

  	  output.width(logWidths[col++]);
  	  output << "Object Real Name";

  	  output.width(logWidths[col++]);
  	  output << "Recognition";

  	  output << "  Probabilities" << std::endl;

  	  output.close();
  	}
  	else {
  	  return false;
  	}

  	return true;
  }

  bool classificationStatistics::writeLogFile(
                    const classifier::outputVector& theOutput,
                    const eStatistics& rState,
                    const int &realObjectID,
                    const std::string& patternInfo) {
    bool show = param.logAllPatterns;

    show = show || (rState<-2);

    if (show) {
      std::string name;
      std::string filename;

      // Object ID
      filename = param.path + param.logFile;

      if (numberOfResults<=1) {
        if (!writeLogFileHeader(filename.c_str())) {
          return false;
        }
      }
      std::ofstream out(filename.c_str(), std::ios::app );
      if (!out.is_open()){
        return false;
      }

      // Object-real name
      format(out,logWidths[0],realObjectID);

      if (idToNamesMap.find(realObjectID) != idToNamesMap.end()) {
        name = idToNamesMap[realObjectID];
      } else {
        name = "unknown";
      }
      name += " ";
      name += patternInfo;

      name = name.substr(0,logWidths[1]-1);

      out.width(logWidths[1]);
      out << name.c_str();

      // Recognition state
      name = "  " + idToNamesMap[rState];
      if (static_cast<unsigned int>(logWidths[2]) > name.length()) {
        name.append(logWidths[2]-name.length(),' ');
      }
      out << name.c_str();

      bool stop = false;
      bool elip = false;
      int numClasses = theOutput.size();
      int i,id;
      double prob;
      classifier::outputVector sorted(theOutput);
      sorted.sortDescending();
      i=0;
      do {
        sorted.getPair(i, id, prob);
        if (idToNamesMap.find(id) != idToNamesMap.end()) {
          name = idToNamesMap[id];
        } else {
          name = "unknown";
        }
//          idx = theOutput.objIndex[i];
//          name = theOutput.objNames.find(idx)->second;
//          prob = theOutput.objProbs.find(idx)->second;
        out << "("
            << name.c_str()
            << ":"
            << prob
            << ") ";
        elip = (i>=4); // Nur 4 Elementen zeigen
        stop = (prob < classifier::nullProbability) || elip;
        i++;
      } while ((!stop) && (i<numClasses));

      if (elip)
        out << "...";

      out << std::endl;
    }

    return true;
  }

  /**
   * workaround for std::ios bug in gcc when writing double numbers
   */
  void classificationStatistics::format(std::ostream& out,
					const int& width,
					const double& val) const {
    char buffer[32];

    if ((val != 0) || !param.suppressZeros) {
      sprintf(buffer,"%*.6f",width,val);
      out.width(width);
      out << buffer;
    }
    else {
      out << std::setw(confWidths[1]) << "-";
    }
  }

  /**
   * workaround for std::ios bug in gcc when writing double numbers
   */
  void classificationStatistics::format(std::ostream& out,
                                                 const int& width,
                                                 const int& val) const {
    char buffer[32];
    sprintf(buffer,"%*i",width,val);
    out.width(width);
    out << buffer;
  }



  const bool&
    classificationStatistics::getSaved() const {

    return saved;
  }

  const classificationStatistics::parameters&
    classificationStatistics::getParameters() const {

    return param;
  }

  const dynamicMatrix<double>&
    classificationStatistics::getConfusionMatrix() const {

    return confMatrix;
  }

  const std::map<int,std::string>&
    classificationStatistics::getIdToNamesMap() const {

    return idToNamesMap;
  }

  const int&
    classificationStatistics::getNumberOfResults() const {

    return numberOfResults;
  }

  const int&
    classificationStatistics::getCorrectSingle() const {

    return correctSingle;
  }

  const int&
    classificationStatistics::getCorrectMultiple() const {

    return correctMultiple;
  }

  const int&
    classificationStatistics::getCorrectReject() const {

    return correctReject;
  }

  const int&
    classificationStatistics::getWrongSingle() const {

    return wrongSingle;
  }

  const int&
    classificationStatistics::getWrongMultiple() const {

    return wrongMultiple;
  }

  const int&
    classificationStatistics::getWrongReject() const {

    return wrongReject;
  }

  const dynamicMatrix<double>&
    classificationStatistics::getNBestMatrix() const {

    return nBest;
  }

  const dynamicMatrix<double>&
    classificationStatistics::getNBestMultipleMatrix() const {

    return nBestMultiple;
  }

  const dynamicMatrix<double>&
    classificationStatistics::getMinPlacesMatrix() const {

      return minPlaces;
  }


}
