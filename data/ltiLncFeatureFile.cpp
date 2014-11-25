/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiLncFeatureFile.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 19.04.99
 * revisions ..: $Id: ltiLncFeatureFile.cpp,v 1.13 2006/09/05 10:36:21 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiLncFeatureFile.h"
#include "ltiLispStreamHandler.h"
#include <list>
#include <ctime>
#include <fstream>

#ifdef _LTI_GNUC_2
namespace std {
  typedef ios ios_base;
}
#endif

#undef _LTI_DEBUG
//#define _LTI_DEBUG 2
#include "ltiDebug.h"

namespace lti {


  // -----------------------------------------------------------------
  // lncFeatureFile::header
  // -----------------------------------------------------------------

  lncFeatureFile::header::header() {

    numberOfObjects = 0;

    numberOfPatterns    = 0;
    numberOfInputUnits  = 0;
    numberOfOutputUnits  = 1;

    items.clear();

    featureType = "";
  }

  lncFeatureFile::header::header(const lncFeatureFile::header& other)
    : object() {
    copy(other);
  }

  lncFeatureFile::header&
  lncFeatureFile::header::copy(const lncFeatureFile::header& other) {

    numberOfObjects = other.numberOfObjects;

    numberOfPatterns    = other.numberOfPatterns;
    numberOfInputUnits  = other.numberOfInputUnits;
    numberOfOutputUnits = other.numberOfOutputUnits;

    items = other.items;

    featureType = other.featureType;

    return *this;
  }

  bool lncFeatureFile::header::save(std::ostream& dest) {
    if (!dest.good()) {
      return false; // invalid stream!
    }

    // Immer am Anfang schreiben
    dest.seekp(0,std::ios_base::beg);

    struct tm *newtime;
    time_t aclock;
    time( &aclock );                 // Get time in seconds
    newtime = localtime( &aclock );  // Convert time to struct tm form

    dest << "SNNS pattern definition file V3.2"  << std::endl;
    dest << "generated at " << asctime( newtime ) << std::endl << std::endl;
    dest << "# SCAN DESCRIPTION"    << std::endl;
    dest << "# Objects          : " ;
    dest.width(7);
    dest << numberOfObjects << std::endl;

    static const unsigned int minimalNameSize = 17;
    std::map<std::string,int>::const_iterator it;
    for (it=items.begin();it!=items.end();it++) {
      dest << "# " << (*it).first;
      if ((*it).first.size() < minimalNameSize) {
        std::string tmp;
        tmp.assign(minimalNameSize-(*it).first.size(),' ');
        dest << tmp;
      }
      dest << ": ";
      dest.width(7);
      dest << (*it).second << std::endl;
    }

    dest << std::endl;

    dest << "No. of patterns    : ";
    dest.width(7);
    dest << numberOfPatterns << std::endl;

    dest << "No. of input units : ";
    dest.width(7);
    dest << numberOfInputUnits << std::endl;

    dest << "No. of output units: ";
    dest.width(7);
    dest << numberOfOutputUnits	 << std::endl;

    // FeatureID eintragen
    dest << std::endl;
    dest << "# FeatureType : " << featureType << std::endl << std::endl;

    dest.seekp(0,std::ios_base::end); // am Ende der Datei weiter machen...

    return true;
  }

  bool lncFeatureFile::header::load(std::istream& source) {
    if (!source.good()) {
      return false; // invalid stream!
    }

    source.seekg(0,std::ios_base::beg);

    // Required Lines:
    //  SNNS pattern definition ... (als 1. Zeile!)
    //  Objects
    //  No. of patterns
    //  No. of input units
    //  FeatureType
    // The Rest is Optional

    std::string line,name,value;

    // 1. SNNS pattern definition ...
    std::getline(source,line);

    if (source.eof()) { // File is empty
      return false;
    }

    if (line.find("SNNS pattern definition file") == std::string::npos) {
      // no *.lnc Datei
      return false;
    }
    // read "generated at ..."
    std::getline(source,line);

    int requiredLines = 4;
    std::string::size_type pos;

    while (!source.eof() && requiredLines > 0) {
      std::getline(source,line);

      pos = line.find_first_not_of(" #\t");
      if (pos == std::string::npos) {
        line = "";
      } else {
        line = line.substr(pos);
      }

      pos = line.find(":");

      if (pos != std::string::npos) {

        // get the item name and delete every unneeded spaces
        name = line.substr(0,pos);
        name = name.substr(name.find_first_not_of(" \t"));
        name = name.substr(0,name.find_last_not_of(" \t")+1);

        // get the value delete every unneeded spaces
        value = line.substr(pos+1);
        pos = value.find_first_not_of(" \t");
        if (pos == std::string::npos) {
          value = "";
        } else {
          value = value.substr(pos);
          value = value.substr(0,value.find_last_not_of(" \t")+1);
        }

        // Required Lines:
        if (name == "Objects") {
          numberOfObjects = atoi(value.c_str());
          requiredLines--;
        } else if (name == "No. of patterns") {
          numberOfPatterns = atoi(value.c_str());
          requiredLines--;
        } else if (name == "No. of input units") {
          numberOfInputUnits = atoi(value.c_str());
          requiredLines--;
        } else if (name == "FeatureType") {
          featureType = value;
          requiredLines--;
        }

        // optional Lines:
        else if (name == "No. of output units") {
          numberOfOutputUnits = atoi(value.c_str());
        } else {
          items[name] = atoi(value.c_str());
        }
      }
    }

    return (requiredLines == 0);
  }

  // -----------------------------------------------------------------
  // lncFeatureFile::parameters
  // -----------------------------------------------------------------
  lncFeatureFile::parameters::parameters()
    : ioFunctor::parameters() {
    useCnc = false;
    cncFilename = "";
    idFirst = true;
  }

  lncFeatureFile::parameters&
  lncFeatureFile::parameters::copy(const parameters& other) {
#ifndef _LTI_MSC_6
    // for normal  ANSI C++
    ioFunctor::parameters::copy(other);
#else
    // this doesn't work with MS-VC++ 6.0 (an once again... another bug)
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    ioFunctor::parameters&
      (ioFunctor::parameters::* p_copy)(const ioFunctor::parameters&) =
      ioFunctor::parameters::copy;
    (this->*p_copy)(other);
#endif
    useCnc = other.useCnc;
    cncFilename = other.cncFilename;
    idFirst = other.idFirst;

    return (*this);
  }

  functor::parameters* lncFeatureFile::parameters::clone() const {
    return new parameters(*this);
  }

  const char* lncFeatureFile::parameters::getTypeName() const {
    return "lncFeatureFile::parameters";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool lncFeatureFile::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool lncFeatureFile::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"useCnc",useCnc);
      lti::write(handler,"cncFilename",cncFilename);
      lti::write(handler,"idFirst",idFirst);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioFunctor::parameters::write(handler,false);
# else
    bool (ioFunctor::parameters::* p_writeMS)(ioHandler&,const bool) const =
      ioFunctor::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool lncFeatureFile::parameters::write(ioHandler& handler,
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
  bool lncFeatureFile::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool lncFeatureFile::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"useCnc",useCnc);
      lti::read(handler,"cncFilename",cncFilename);
      lti::read(handler,"idFirst",idFirst);

    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && ioFunctor::parameters::read(handler,false);
# else
    bool (ioFunctor::parameters::* p_readMS)(ioHandler&,const bool) =
      ioFunctor::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool lncFeatureFile::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // ------------------------------------------------------------------
  // Feature File
  // ------------------------------------------------------------------

  lncFeatureFile::lncFeatureFile() {
    initialized = false;

    // default parameters
    parameters tmp;
    setParameters(tmp);
  }

  lncFeatureFile::~lncFeatureFile() {
    input.close();
    output.close();
    nameToIdMap.clear();
  }

  lncFeatureFile& lncFeatureFile::copy(const lncFeatureFile& other) {
    ioFunctor::copy(other);

    nameToIdMap = other.nameToIdMap;
    usedObjects = other.usedObjects;
    initialized = false;

    return *this;
  }

  bool lncFeatureFile::initialize(const bool deleteFile) {

    const parameters& param = getParameters();

    // should a cnc file be used?
    if (param.useCnc) { // Read Classfile  (.cnc)
      loadCncFile();
    }

    // if it was already in use, close it...
    if (input.is_open()) {
      input.close();
    }

    input.open(param.filename.c_str(),std::ios_base::in);

    if (input.good() &&
        input.is_open() &&
        !deleteFile) { // file exists!  read header
      input.seekg(0,std::ios_base::beg);
      if (!theHeader.load(input)) {
        initialized = false;
        setStatusString("*.lnc file has an invalid header");
        return false;
      }

      mayCheckConsistence = true;
    } else { // file doesn't exist or we need to delete it => create it!)
      output.open(param.filename.c_str(),std::ios_base::out);
      output.close();
      mayCheckConsistence = false;
    }

    // file remains open, to allow reading the following vectors...
    initialized = true;

    return true;
  }

  bool lncFeatureFile::saveNext(const dvector& theVct,
                                const lncFeatureFile::patternId& origId) {
    const parameters& param = getParameters();

    if (!initialized) {
      initialize();
    }

    lncFeatureFile::patternId id = origId;

    bool wasOpen = true;
    if (!output.is_open()) {
      if (mayCheckConsistence) {
        mayCheckConsistence = false;
        checkConsistency();
      }
      
      output.open(param.filename.c_str(),
                  std::ios_base::in | std::ios_base::out | std::ios_base::ate);
      wasOpen = false;
    }

    // update header
    // 1. was the object id already used?
    if (nameToIdMap.find(id.name) == nameToIdMap.end()) {
      // object id not used yet!

      // 1.1. insert id in the nameToIdMap
      if (id.objectId == -1) { // invalid id?
        // in that case generate a new id
        id.objectId = nameToIdMap.size();
      }
      nameToIdMap[id.name] = id.objectId;

      if (param.useCnc) {
        writeToCncFile(id.name);
      }

      // 1.2. check in usedObjects
      usedObjects.insert(nameToIdMap[id.name]);
    } else {
      if (id.objectId != nameToIdMap[id.name]) {
        if (!param.useCnc) {
          std::string txt;
          txt = "Inconsistency in id of object " + id.name +
                " on the file " + param.filename;
          initialized = false;
          throw exception(txt);
        } else {
          // useCnc = true means, that the data there is valid!
          id.objectId = nameToIdMap[id.name];
        }
      }
    }

    theHeader.numberOfObjects = usedObjects.size();
    theHeader.numberOfInputUnits = theVct.size();
    theHeader.numberOfPatterns++;
    theHeader.save(output);

    // the lti::vector Speichern!
    savePattern(output,theVct,id);

    if (!wasOpen) {
      output.close();
    }

    return true;
  }

  bool lncFeatureFile::loadNext(dvector& theVct,
                                patternId& id) {


    const parameters& param = getParameters();

    if (!initialized) {
      initialize();
    }

    if (theHeader.numberOfInputUnits != theVct.size()) {
      theVct.resize(theHeader.numberOfInputUnits,double(),false,false);
    }

    if (!loadPattern(input,theVct,id))
      return false;

    // Consistency check

    std::map<std::string,int>::iterator it;
    it = nameToIdMap.find(id.name);
    if (it == nameToIdMap.end()) {
      // name not used yet
      nameToIdMap[id.name] = id.objectId;
      usedObjects.insert(id.objectId);
      if (param.useCnc)
        writeToCncFile(id.name);
    } else {
      // if useCnc == true, the used ID will be the one stored on the
      // *.cnc file.  The one in the *.lnc file will be ignored

      if ((*it).second != id.objectId) {
        if (!param.useCnc) {
          std::string txt;

          txt = "Feature file " + param.filename + "is inconsistent";
          initialized = false;
          throw exception(txt);
        } else
          id.objectId = (*it).second;
      }
    }

    return true;
  }

  const lncFeatureFile::parameters& lncFeatureFile::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  bool lncFeatureFile::checkConsistency() {
    const parameters& param = getParameters();

    // repair not implemented yet
    if (input.is_open()) {
      dvector theVct;
      patternId id;

      if (!param.useCnc) {
        nameToIdMap.clear();
      }

      usedObjects.clear();

      // read the header
      input.seekg(0,std::ios_base::beg);
      theHeader.load(input);

      // read all patterns
      int numPatterns = 0;
      while (loadNext(theVct,id)) {
        numPatterns++;
      }

      // go to the begining again and read the header.  This way
      // the first vector can be readed

      input.seekg(0,std::ios_base::beg);
      theHeader.load(input);

      theHeader.numberOfObjects = usedObjects.size();
      theHeader.numberOfPatterns = numPatterns;
    }

    return true;
  }

  bool lncFeatureFile::loadCncFile() {
    const parameters& param = getParameters();
    bool ok;
    lispStreamHandler lsh;

    std::ifstream classStreamIn(param.cncFilename.c_str());
    if (classStreamIn) { // file found
      lsh.use(classStreamIn);

      std::string classname;
      int id = -1;

#ifdef _LTI_MSC_6
      classname = " "; // force some mem allocation
      classname[0] = 0;
#else
      classname="";
#endif      


      do {
        if (param.idFirst) {
          ok = lsh.read(id);
          ok = ok && lsh.readDataSeparator();
          ok = ok && lsh.read(classname);
        } else {
          ok = lsh.read(classname);
          ok = ok && lsh.readDataSeparator();
          ok = ok && lsh.read(id);
        }

        if (!ok || (classname == "" ) || (id<0)) {
          break;  // no more data
        }

        if (nameToIdMap.find(classname) == nameToIdMap.end()) {
          // name not found
          nameToIdMap[classname]=id;
        } else { // Name already used
          // Data consistent?
          int tmp = nameToIdMap[classname];
          if (tmp != id) {
            std::string txt;
            txt = "Inconsistency in cnc file: class ";
            txt += classname;
            initialized = false;
            throw exception(txt);
            return false;
          }
        }

        // Flag to recognize end of file
        id = -1;
        classname[0] = 0;

      } while (!classStreamIn.eof());
      classStreamIn.close();
    } else { // file doesn't exist ! => create a new one
      classStreamIn.close();
      std::ofstream classStreamOut(param.cncFilename.c_str());
      classStreamOut.close();
    }
    return true;
  }

  bool lncFeatureFile::writeToCncFile(const std::string& name) {
    const parameters& param = getParameters();

    int id = nameToIdMap[name];
    std::ofstream classStreamOut(param.cncFilename.c_str(),
                                 std::ios_base::app | 
                                 std::ios_base::out |
                                 std::ios_base::in);
    
    lispStreamHandler lsh;
    lsh.use(classStreamOut);
    
#ifdef _LTI_DEBUG
    classStreamOut.exceptions(std::ios_base::badbit |
                              std::ios_base::failbit |
                              std::ios_base::eofbit);
#endif
    
    if (classStreamOut) {
      if (param.idFirst) {
        lsh.write(id);
        lsh.writeDataSeparator();
        lsh.writeSymbol(name);
      } else {
        lsh.write(name);
        lsh.writeDataSeparator();
        lsh.write(id);
      }
      
      classStreamOut.close();
    } else {
      std::string str = "Class name " + param.cncFilename + 
        " could not be opened.";
      throw exception(str.c_str());
      return false;
    }

    return true;
  }

  bool lncFeatureFile::writeCncFile(const std::string& filename) {
    const parameters& param = getParameters();
    std::ofstream cncfile(filename.c_str());

    if (cncfile) {
      lispStreamHandler lsh;
      lsh.use(cncfile);
      std::map<std::string,int>::const_iterator it;

      for (it=nameToIdMap.begin();it!=nameToIdMap.end();++it) {
        if (param.idFirst) {
          lsh.write((*it).second);
          lsh.writeDataSeparator();
          lsh.writeSymbol((*it).first);
        } else {
          lsh.write((*it).first);
          lsh.writeDataSeparator();
          lsh.write((*it).second);
        }
        cncfile << std::endl;
      }

      cncfile.close();
      return true;
    }

    return false;
  }


  bool lncFeatureFile::loadPattern(std::istream& source,
                                   dvector& theVector,
                                   patternId& id) {
    std::string tmpstr;

    tmpstr = "";

    // search pattern id
    while ((tmpstr == "") || (tmpstr.at(0) != '#')) {
      if (!std::getline(source, tmpstr)) { // no more data on file
        return false;
      }
    }

    // object name found
    if ((tmpstr != "") && (tmpstr.at(0) == '#')) {
      std::string::size_type pos,pos2;
      pos = tmpstr.find_first_not_of("# \t");
      if (pos != std::string::npos) {
        id.name = tmpstr.substr(pos);
        pos = id.name.find_first_of(" \t");
        if (pos != std::string::npos) {
          pos2 = id.name.find_first_not_of(" \t",pos);
          if (pos2 != std::string::npos) {
            id.attributes = id.name.substr();
          } else {
            id.attributes = "";
          }
        } else {
          id.attributes = "";
        }
        id.name = id.name.substr(0,pos);

      } else {
        id.name = "unknown";
        id.attributes = "";
      }
    }

    // read vector

    if (theVector.size() == 0)  {
      std::vector<double> theElements;

      theElements.clear();

      source >> tmpstr;
      while (!source.eof() && (tmpstr.find('#') == std::string::npos)) {
        double value = atof(tmpstr.c_str());
        theElements.push_back(value);
        source >> tmpstr;
      }

      if (!source.eof() && (theElements.size() > 0)) {
        // vector successfully readed
        theVector.castFrom(theElements);
        theElements.clear();
      } else {
        initialized = false;
        throw exception("unexpected EOF reading the feature file");
        return false;
      }
    } else {
      int featureDimension = theVector.size();
      for (int i1 = 0; i1 < featureDimension; i1++) {
        source >> tmpstr;
        theVector.at(i1) = atof(tmpstr.c_str());
      }
      source >> tmpstr; // this should write a '#' in tmpstr!
    }

    // search class ID (can be german Klassenid or english ClassId)
    while (!source.eof() &&
           (tmpstr.find("Klassen") == std::string::npos) &&
           (tmpstr.find("ClassId") == std::string::npos)) {
      source >> tmpstr;
    }

    if (tmpstr.find("lass") == std::string::npos) {
      initialized = false;
      throw exception("unexpected EOF!");
      return false;
    }

    // classID lesen
    source >> id.objectId;

    return (source.good());
  }

  bool lncFeatureFile::savePattern(std::ostream& dest,
                                   const dvector& theVector,
                                   const patternId& id) const {
    // go to the end of file and write the new pattern

    dest.seekp(0,std::ios_base::end);
    int i;

    dest << "# " << id.name << " " << id.attributes << std::endl;

    for (i = 0; i < theVector.size(); i++) {
      dest << theVector.at(i) << ' ';
    }

    dest << std::endl;
    dest << "# ClassId" << std::endl;
    dest << id.objectId << std::endl << std::endl;

    return (dest.good());
  }

  // ------------------------------------------------------------------
  // loadLnc
  // ------------------------------------------------------------------

  loadLnc::loadLnc() : lncFeatureFile() {
  }

  loadLnc::loadLnc(const parameters& par) : lncFeatureFile() {
    setParameters(par);
  }

  /*
   * load the next vector in the file.
   * @param vct the vector object, where the readed data will be written
   * @param id the id of the class the vector belongs to.
   * @return true if everything is ok, false if there
   *         is no more data left on the file. If the file is
   *         corrupted, a lti::exception will be thrown.
   */
  bool loadLnc::apply(dvector& vct,patternId& id) {
    return loadNext(vct,id);
  }

  /*
   * load all the vectors in the file and leave the results in the given
   * matrix.  The i-th row of the matrix will be the i-th vector on the file.
   * @param vcts the matrix where the vectors will be written
   * @param ids a vector the class ids for each vector in the matrix.  The
   *            i-th element of the vector contains the id of the i-th row
   *            of the matrix.
   * @param names a table with the corresponding names for each object id
   * @return true if everything is ok or false if the file is corrupted or
   *              not found.
   */
  bool loadLnc::apply(dmatrix& vcts,
                      ivector& ids,
                      std::map<int,std::string>& names) {

    if (apply(vcts,ids)) {
      getClassNames(names);
      return true;
    }

    return false;
  }

  /*
   * load all the vectors in the file and leave the results in the given
   * matrix.  The i-th row of the matrix will be the i-th vector on the file.
   * @param vcts the matrix where the vectors will be written
   * @param ids a vector the class ids for each vector in the matrix.  The
   *            i-th element of the vector contains the id of the i-th row
   *            of the matrix.
   * @param names a table with the corresponding names for each object id
   * @return true if everything is ok or false if the file is corrupted or
   *              not found.
   */
  bool loadLnc::apply(dmatrix& vcts,
                      ivector& ids,
                      std::vector<std::string>& attribs,
                      std::map<int,std::string>& names) {

    if (apply(vcts,ids,attribs)) {
      getClassNames(names);
      return true;
    }

    return false;
  }


  /*
   * load all the vectors in the file and leave the results in the given
   * matrix.  The i-th row of the matrix will be the i-th vector on the file.
   * @param vcts the matrix where the vectors will be written
   * @param ids a vector the class ids for each vector in the matrix.  The
   *            i-th element of the vector contains the id of the i-th row
   *            of the matrix.
   * @return true if everything is ok or false if the file is corrupted or
   *              not found.
   */
  bool loadLnc::apply(dmatrix& vcts,ivector& ids) {

    if (initialize()) {

      vcts.resize(getHeader().numberOfPatterns,
                  getHeader().numberOfInputUnits,
                  0.0,false,false);
      ids.resize(vcts.rows(),int(0),false,false);

      int i = 0;
      patternId id;
      while ((i<vcts.rows()) && loadNext(vcts.getRow(i),id)) {
        ids.at(i) = id.objectId;
        i++;
      };

      if (i != getHeader().numberOfPatterns) {
        if (vcts.empty()) {
          setStatusString("No feature vectors could be found!");
        } else {
          setStatusString("Inconsistent number of feature vectors in file");
        }
        return false;
      }

      return true;
    }

    return false;
  }

  /*
   * load all the vectors in the file and leave the results in the given
   * matrix.  The i-th row of the matrix will be the i-th vector on the file.
   * @param vcts the matrix where the vectors will be written
   * @param ids a vector the class ids for each vector in the matrix.  The
   *            i-th element of the vector contains the id of the i-th row
   *            of the matrix.
   * @param attribs a vector of std::string with the attributes for each
   *                pattern in the file
   * @return true if everything is ok or false if the file is corrupted or
   *              not found.
   */
  bool loadLnc::apply(dmatrix& vcts,
                      ivector& ids,
                      std::vector<std::string>& attribs) {

    if (initialize()) {

      vcts.resize(getHeader().numberOfPatterns,
                  getHeader().numberOfInputUnits,
                  0.0,false,false);
      ids.resize(vcts.rows(),int(0),false,false);
      attribs.resize(vcts.rows());

      int i = 0;
      patternId id;
      while ((i<vcts.rows()) && loadNext(vcts.getRow(i),id)) {
        ids.at(i) = id.objectId;
        attribs[i] = id.attributes;
        i++;
      };

      if (i != getHeader().numberOfPatterns) {
        if (vcts.empty()) {
          setStatusString("No feature vectors could be found!");
        } else {
          setStatusString("Inconsistent number of feature vectors in file");
        }
        return false;
      }

      return true;
    }

    return false;
  }


  /*
   * shortcut to read the given lnc file, ignoring the filenames
   * @param vcts the matrix where the vectors will be written
   * @param ids a vector the class ids for each vector in the matrix.  The
   *            i-th element of the vector contains the id of the i-th row
   *            of the matrix.
   * return true  if everything is ok or false if the file is corrupted or
   *              not found.
   */
  bool loadLnc::load(const std::string& filename,
                     dmatrix& vcts,
                     ivector& ids,
                     const bool useCnc) {
    parameters param(getParameters());

    param.filename = filename;
    if (useCnc && (param.filename == "")) {
      std::string::size_type pos;
      pos = filename.rfind(".");
      if (pos == std::string::npos) {
        param.cncFilename = filename + ".cnc";
      } else {
        param.cncFilename = filename.substr(0,pos) + ".cnc";
      }
    }

    param.useCnc = useCnc;

    setParameters(param);

    return apply(vcts,ids);
  }


  /*
   * shortcut to read the given lnc file, ignoring the filenames
   * @param vcts the matrix where the vectors will be written
   * @param ids a vector the class ids for each vector in the matrix.  The
   *            i-th element of the vector contains the id of the i-th row
   *            of the matrix.
   * return true  if everything is ok or false if the file is corrupted or
   *              not found.
   */
  bool loadLnc::load(const std::string& filename,
                     dmatrix& vcts,
                     ivector& ids,
                     std::vector<std::string>& attribs,
                     const bool useCnc) {
    parameters param(getParameters());

    param.filename = filename;
    if (useCnc && (param.filename == "")) {
      std::string::size_type pos;
      pos = filename.rfind(".");
      if (pos == std::string::npos) {
        param.cncFilename = filename + ".cnc";
      } else {
        param.cncFilename = filename.substr(0,pos) + ".cnc";
      }
    }

    param.useCnc = useCnc;

    setParameters(param);

    return apply(vcts,ids,attribs);
  }

  /*
   * shortcut to read the given lnc file, ignoring the filenames
   * @param vcts the matrix where the vectors will be written
   * @param ids a vector the class ids for each vector in the matrix.  The
   *            i-th element of the vector contains the id of the i-th row
   *            of the matrix.
   * return true  if everything is ok or false if the file is corrupted or
   *              not found.
   */
  bool loadLnc::load(const std::string& filename,
                     dmatrix& vcts,
                     ivector& ids,
                     std::map<int,std::string>& names,
                     const bool useCnc) {

    if (load(filename,vcts,ids,useCnc)) {
      getClassNames(names);
      return true;
    }

    getClassNames(names);
    return false;
  }


  /*
   * shortcut to read the given lnc file, ignoring the filenames
   * @param vcts the matrix where the vectors will be written
   * @param ids a vector the class ids for each vector in the matrix.  The
   *            i-th element of the vector contains the id of the i-th row
   *            of the matrix.
   * return true  if everything is ok or false if the file is corrupted or
   *              not found.
   */
  bool loadLnc::load(const std::string& filename,
                     dmatrix& vcts,
                     ivector& ids,
                     std::vector<std::string>& attribs,
                     std::map<int,std::string>& names,
                     const bool useCnc) {

    if (load(filename,vcts,ids,attribs,useCnc)) {
      getClassNames(names);
      return true;
    }

    getClassNames(names);
    return false;
  }

  /**
   * returns a map with the names of the classes for each object id.
   */
  void loadLnc::getClassNames(std::map<int,std::string>& names) const {
    names.clear();

    std::map<std::string,int>::const_iterator cit;

    for (cit=nameToIdMap.begin();cit!=nameToIdMap.end();cit++) {
      names[(*cit).second] = (*cit).first;
    }
  }

  /**
   * restart reading the file (use only when a file need to be readed again)
   */
  void loadLnc::reset() {
    initialized = false;
  }

  // ------------------------------------------------------------------
  // saveLnc
  // ------------------------------------------------------------------

  // -----------------------------------------------------------------
  // saveLnc::parameters
  // -----------------------------------------------------------------
  saveLnc::parameters::parameters()
    : lncFeatureFile::parameters() {
    deleteIfExists = true;
  }

  saveLnc::parameters&
  saveLnc::parameters::copy(const parameters& other) {
#ifndef _LTI_MSC_6
    // for normal  ANSI C++
    lncFeatureFile::parameters::copy(other);
#else
    // this doesn't work with MS-VC++ 6.0 (an once again... another bug)
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    lncFeatureFile::parameters&
      (lncFeatureFile::parameters::* p_copy)(const lncFeatureFile::parameters&) =
      lncFeatureFile::parameters::copy;
    (this->*p_copy)(other);
#endif
    deleteIfExists = other.deleteIfExists;

    return (*this);
  }

  functor::parameters* saveLnc::parameters::clone() const {
    return new parameters(*this);
  }

  const char* saveLnc::parameters::getTypeName() const {
    return "saveLnc::parameters";
  }

  /*
   * write the parameters in the given ioHandler
   * @param handler the ioHandler to be used
   * @param complete if true (the default) the enclosing begin/end will
   *        be also written, otherwise only the data block will be written.
   * @return true if write was successful
   */
# ifndef _LTI_MSC_6
  bool saveLnc::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool saveLnc::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"deleteIfExists",deleteIfExists);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && lncFeatureFile::parameters::write(handler,false);
# else
    bool
      (lncFeatureFile::parameters::* p_writeMS)(ioHandler&,const bool) const=
      lncFeatureFile::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool saveLnc::parameters::write(ioHandler& handler,
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
  bool saveLnc::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool saveLnc::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"deleteIfExists",deleteIfExists);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && lncFeatureFile::parameters::read(handler,false);
# else
    bool (lncFeatureFile::parameters::* p_readMS)(ioHandler&,const bool) =
      lncFeatureFile::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool saveLnc::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // ---------------------------------------------------------------------
  // saveLnc
  // ---------------------------------------------------------------------

  saveLnc::saveLnc() : lncFeatureFile() {
  }

  saveLnc::saveLnc(const parameters& par) : lncFeatureFile() {
    setParameters(par);
  }

  const saveLnc::parameters& saveLnc::getParameters() const {
    const parameters* params =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(params==0) throw invalidParametersException(getTypeName());
    return *params;
  }

  bool saveLnc::apply(const dvector& vct, const patternId& id) {
    if (!initialized) {
      reset();
    }
    return saveNext(vct,id);
  }

  /**
   * restart reading the file (use only when a file need to be readed again)
   */
  void saveLnc::reset() {
    const parameters& param = getParameters();
    initialize(param.deleteIfExists);
  }

  /**
   * set the current name to object id table
   */
  void saveLnc::setNameToIdMap(const std::map<std::string,int>& newMap) {
    nameToIdMap = newMap;
  }

  bool saveLnc::writeCncFile(const std::string& filename) {
    return lncFeatureFile::writeCncFile(filename);
  }

}
