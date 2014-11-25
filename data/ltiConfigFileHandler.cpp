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
 * file .......: ltiConfigFile.cpp
 * authors ....: Ruediger Weiler
 * organization: LTI, RWTH Aachen
 * creation ...: 23.10.2001
 * revisions ..: $Id: ltiConfigFileHandler.cpp,v 1.12 2006/09/05 10:34:43 ltilib Exp $
 */

#include "ltiConfigFileHandler.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

namespace lti {
  // --------------------------------------------------
  // configFileHandler::parameters
  // --------------------------------------------------

  // default constructor
  configFileHandler::parameters::parameters()
    : ioFunctor::parameters() {
    //TODO: Initialize your parameter values!
    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters
    filename = "";
    formatOutput = true;;
  }

  // copy constructor
  configFileHandler::parameters::parameters(const parameters& other)
    : ioFunctor::parameters()  {
    copy(other);
  }

  // destructor
  configFileHandler::parameters::~parameters() {
  }

  // get type name
  const char* configFileHandler::parameters::getTypeName() const {
    return "configFileHandler::parameters";
  }

  // copy member

  configFileHandler::parameters&
    configFileHandler::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    ioFunctor::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    ioFunctor::parameters& (ioFunctor::parameters::* p_copy)
      (const ioFunctor::parameters&) =
      ioFunctor::parameters::copy;
    (this->*p_copy)(other);
# endif

    formatOutput = other.formatOutput;

    return *this;
  }

  // alias for copy member
  configFileHandler::parameters&
    configFileHandler::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* configFileHandler::parameters::clone() const {
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
  bool configFileHandler::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool configFileHandler::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"formatOutput",formatOutput);
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
  bool configFileHandler::parameters::write(ioHandler& handler,
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
  bool configFileHandler::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool configFileHandler::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"formatOutput",formatOutput);
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
  bool configFileHandler::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // configFileHandler
  // --------------------------------------------------

  inline bool isLineStartComment(unsigned char c) {
    //check ih c is a comment
    return ((c=='*') || (c=='#') || (c==';') || (c=='/'));
  }

  inline bool isLineMidComment(unsigned char c) {
    //is c a comment in the line
    return (c==';');
  }

  inline bool isWhiteSpace(unsigned char c) {
    //is c a tab a space or a whitespace
    return ((c==' ')||(c=='\t')||(c==160)); //160 is whitespace
  }

  inline bool isLegalChar(unsigned char c) {
    return ( c >= 32 );
  }

  inline void killSpaces(std::string &str) {
    unsigned int start =0;
    unsigned int end =0;
    for(start=0; start < static_cast<unsigned int>(str.size()); start++)
      if(!isWhiteSpace(str[start])) {
        break;
      }
    for(end=str.size(); end>0; end--) {
      if( !( (str[end-1]==' ') || (str[end-1]=='\t') || (str[end-1]==char(0xa0)) ) ) {
        break;
      }
    }
    str = str.substr(start,end-start);
  }

  inline void lowerString(std::string &str) {
    //transform a string to lower case
    for(unsigned int x=0; x < static_cast<unsigned int>(str.size()); x++)
      str[x] = tolower(str[x]);
  }

  // default constructor
  configFileHandler::configFileHandler()
    : ioFunctor(){

    clear();

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // copy constructor
  configFileHandler::configFileHandler(const configFileHandler& other)
    : ioFunctor()  {
    copy(other);
  }

  // destructor
  configFileHandler::~configFileHandler() {
  }

  // returns the name of this type
  const char* configFileHandler::getTypeName() const {
    return "configFileHandler";
  }

  // copy member
  configFileHandler& configFileHandler::copy(const configFileHandler& other) {
    ioFunctor::copy(other);

    dictionary = other.dictionary;
    orderKeys = other.orderKeys;
    orderSections = other.orderSections;
    sectionComments = other.sectionComments;
    sectionOrig = other.sectionOrig;
    keyOrig = other.keyOrig;

    return (*this);
  }

  // alias for copy member
  configFileHandler&
  configFileHandler::operator=(const configFileHandler& other) {
    return (copy(other));
  }

  // clone member
  functor* configFileHandler::clone() const {
    return new configFileHandler(*this);
  }

  // return parameters
  const configFileHandler::parameters&
    configFileHandler::getParameters() const {
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

  bool configFileHandler::apply(){
     // Get parameters
    const parameters& param = getParameters();
    return loadFile(param.filename);
  }


  bool configFileHandler::loadFile(const std::string &aFileName){

    std::ifstream file(aFileName.c_str());
    std::string input;
    int lineNr = 0;
    std::string currentSection = "";
    std::string prolog = "";
    std::string section = "";
    std::string comment = "";
    std::string key = "";
    std::string value = "";
    std::string errorS = "";

    // check if file is open
    if(!file.is_open()){
      std::string error = "Unable to open file: " + aFileName;
      setStatusString(error.c_str());
      return false;
    }

    // clear memory
    clear();
    
    while(std::getline(file,input)){ //go line by line through the file
      killSpaces(input);
      if (input.empty()) {
        ++lineNr;
        continue;
      }

      errorS = splitLine(input,prolog,section,key,value,comment);

      if (section != "")
        currentSection = section;

      if( errorS != "") {
         //generatin the ostream is the securest way for converting
        std::ostringstream os;
        os << "Line " << lineNr << ": " << errorS;
        setStatusString(os.str().c_str());
        file.close();
        return false;
      }

      // check for dupe (which should not happen when loading)
      std::string key2 = key;
      std::string section2 = currentSection;
      lowerString(key2);
      lowerString(section2);
      if (key2 != "" && dictionary[section2].find(key2) != dictionary[section2].end()) {
        std::ostringstream os;
        os << "Line " << lineNr << ": Duplicate key: " << key;
        setStatusString(os.str().c_str());
        file.close();
        return false;
      }

      setValue(currentSection, key, value, comment);

      ++lineNr;
    }
    file.close();
    return true;
  }

  bool configFileHandler::loadFile() {
    return apply();
  }

  bool configFileHandler::saveFile(const std::string &aFileName) const {
    std::ofstream file(aFileName.c_str());

    if( !file.is_open() ){
      std::string error = "Unable to open file: " + aFileName;
      setStatusString(error.c_str());
      return false;
    }

    std::map<std::string,int> keyLenMap;
    if (getParameters().formatOutput) {
      getMaxKeyLengths(keyLenMap); 
    }

    std::list<std::string>::const_iterator itSec, itKey, itCom;
    std::map<std::string, configEntry>::const_iterator entry;
    std::map<std::string, std::map<std::string, configEntry> >::const_iterator it2;
    
    int keyLen, i;
    bool firstLine = true;
    std::string section = ""; // write section "" first (this entry is removed later)
    itSec = orderSections.begin();
    do {      
      if (section != "") { // write [section] unless empty (which indicates prologue)
        file << (firstLine ? "[" : "\n[") << sectionOrig.find(section)->second << "]";
        firstLine = false;
        // check for section comment
        it2 = dictionary.find(section);
        if (it2 != dictionary.end() && it2->second.find("") != it2->second.end()) // comment present
          file << " ; " << it2->second.find("")->second.comment;
        file << "\n";
      }

      if (sectionComments.find(section) != sectionComments.end()) {
        for (itCom = sectionComments.find(section)->second.begin(); itCom != sectionComments.find(section)->second.end(); ++itCom) {
          file << "# " << *itCom << "\n";
        }
        firstLine = false;
      }

      if (orderKeys.find(section) != orderKeys.end()) {
        for (itKey = orderKeys.find(section)->second.begin(); itKey != orderKeys.find(section)->second.end(); ++itKey) {
          entry = dictionary.find(section)->second.find(*itKey);
          if (getParameters().formatOutput && (keyLenMap.find(section)!=keyLenMap.end()) ) {
            keyLen = keyLenMap.find(section)->second;
          } else {
            keyLen = 0;
          }
          file << keyOrig.find(section)->second.find(*itKey)->second;
          for (i = itKey->size(); i < keyLen; ++i)
            file << ' ';
          file << " = " << entry->second.value;
          if (entry->second.comment != "")
            file << " ; " + entry->second.comment;
          file << "\n";
        }
      }

      if (section != "")
        ++itSec;
      if (itSec == orderSections.end())
        break;
      section = *itSec;
    } while (true);

    file.close();
    return true;
  }

  bool configFileHandler::saveFile() const {
    return saveFile(getParameters().filename);
  }


  // setValue members ---------------------------------------------------------

  bool configFileHandler::getValue(const std::string &section,
                                   const std::string &key,
                                   std::string &value,
                                   std::string &comment) const {
    // get a string at the point section and key
    // value = ""; leave unaltered
    comment = "";
    //allways lower case section and key names
    std::string sec = section; killSpaces(sec);
    lowerString(sec); // for case insensitive search
    std::string k = key; killSpaces(k);
    lowerString(k); // for case insensitive search

    std::map<std::string, std::map<std::string, configEntry> >::const_iterator itSection;
    std::map<std::string, configEntry>::const_iterator itEntry;
    if ((itSection = dictionary.find(sec)) == dictionary.end())
      return false; // no such section
    if ((itEntry = itSection->second.find(k)) == itSection->second.end())
      return false; // no such key
    value = itEntry->second.value;
    comment = itEntry->second.comment;
    return true;
  }

  bool configFileHandler::getValue(const std::string &section,
                                   const std::string &key,
                                   bool &value,
                                   std::string &comment) const {
    //get the string at the desired point and convert it to a boolean
    std::string erg;
    if( getValue(section,key,erg,comment) ){
      //convert the string erg to a boolean
      if(erg[0]=='y'|| erg[0]=='Y'|| erg[0]=='1'|| erg[0]=='t'|| erg[0]=='T') {
        value = true;
        return true;
      }
      else if(erg[0]=='n'|| erg[0]=='N'|| erg[0]=='0'|| erg[0]=='f'|| erg[0]=='F') {
        value = false;
        return true;
      }
      //if the string is not recognized return false
    }
    //value = false; //if nothing found leave unaltered
    return false; //and return false
  }

  bool configFileHandler::getValue(const std::string &section,
                                   const std::string &key,
                                   double &value,
                                   std::string &comment) const {
    //get the string at the desired point and convert it to a double
    std::string erg;
    if( getValue(section,key,erg,comment) ){
      //convert the string erg to double
      value = atof(erg.c_str());
      return true;
    }
    //value = 0; //if nothing found leave unaltered
    return false; //and return false
  }

  bool configFileHandler::getValue(const std::string &section,
                                   const std::string &key,
                                   float &value,
                                   std::string &comment) const {
    //get the string at the desired point and convert it to a double
    std::string erg;
    if( getValue(section,key,erg,comment) ){
      //convert the string erg to double
      value = static_cast<float>(atof(erg.c_str()));
      return true;
    }
    //value = 0; //if nothing found leave unaltered
    return false; //and return false
  }

  bool configFileHandler::getValue(const std::string &section,
                                   const std::string &key,
                                   int &value,
                                   std::string &comment) const {
    //get the string at the desired point and convert it to an int
    std::string erg;
    if( getValue(section,key,erg,comment) ){
      //convert the string erg to int
      value = atoi(erg.c_str());
      return true;
    }
    //value = 0; //if nothing found leave unaltered
    return false; //and return false
  }

  bool configFileHandler::getValue(const std::string &section,
                                   const std::string &key,
                                   unsigned int &value,
                                   std::string &comment) const {
    //get the string at the desired point and convert it to an int
    std::string erg;
    if( getValue(section,key,erg,comment) ){
      //convert the string erg to int
      value = static_cast<unsigned int>(atoi(erg.c_str()));
      return true;
    }
    //value = 0; //if nothing found leave unaltered
    return false; //and return false
  }



  // setValue members ---------------------------------------------------------

  bool configFileHandler::setValue(const std::string& s,
                                   const std::string& k,
                                   const std::string& v,
                                   const std::string& c) {
    std::string section = s;
    std::string key = k;
    std::string value = v;
    std::string comment = c;
    std::string tmp;
    killSpaces(section);
    killSpaces(key);
    tmp = section;
    lowerString(section); // for case insensitive search
    sectionOrig[section] = tmp;
    tmp = key;
    lowerString(key);   // for case insensitive search
    keyOrig[section][key] = tmp;

    killSpaces(value);
    killSpaces(comment);

    // no empty lines!
    if ((section=="") && (key=="") && (value=="") && (comment=="")) {
      setStatusString("Error: no empty lines are allowed.");
      return false;
    }

    // no values without keys!
    if ((key=="") && (value!="")) {
      setStatusString("Error: no values without keys.");
      return false;
    }

    // remember order of sections (except "", which is always first)
    if (section != "") {
      if (std::find(orderSections.begin(), orderSections.end(), section) == orderSections.end())
        orderSections.push_back(section);
    }
    if (orderKeys.find(section) == orderKeys.end())
      orderKeys[section]; // create list of keys for every section
    if (key != "" && std::find(orderKeys[section].begin(), orderKeys[section].end(), key) == orderKeys[section].end())
      orderKeys[section].push_back(key);
    
    if (key == "") { // no key -> may be section comment
      // if this is the first occurence of section, then comment is to be attached to it (else part)
      if (comment != "") {
        if (section == "" || dictionary.find(section) != dictionary.end())
          sectionComments[section].push_back(comment);
        else
          dictionary[section][""] = configEntry("", comment); // [section] ; comment
      }
    } else {
      // store data
      dictionary[section][key] = configEntry(value, comment);
    }

    return true;
  }


  bool configFileHandler::setValue(const std::string &section,
                                   const std::string &key,
                                   const bool &value,
                                   const std::string &comment) {
    std::string wert="false";
    if(value == true) wert = "true";
    return setValue(section,key,wert,comment);
  }

  bool configFileHandler::setValue(const std::string &section,
                                   const std::string &key,
                                   const double &value,
                                   const std::string &comment) {
    //generating the ostream is the securest way for converting
    std::ostringstream os;
    os << value;
    return setValue(section,key,os.str(),comment);
  }

  bool configFileHandler::setValue(const std::string &section,
                                   const std::string &key,
                                   const float &value,
                                   const std::string &comment) {
    //generating the ostream is the securest way for converting
    std::ostringstream os;
    os << value;
    return setValue(section,key,os.str(),comment);
  }

  bool configFileHandler::setValue(const std::string &section,
                                   const std::string &key,
                                   const int &value,
                                   const std::string &comment) {
    //generating the ostream is the securest way for converting
    std::ostringstream os;
    os << value;
    return setValue(section,key,os.str(),comment);
  }

  bool configFileHandler::setValue(const std::string &section,
                                   const std::string &key,
                                   const unsigned int &value,
                                   const std::string &comment) {

    //generating the ostream is the securest way for converting
    std::ostringstream os;
    os << value;
    return setValue(section,key,os.str(),comment);
  }



  // manipulation -------------------------------------------------------------

  void configFileHandler::sortSections() {
    orderSections.sort();
  }

  bool configFileHandler::deleteSection(const std::string &section) {
    //std::list<configEntry>::iterator it;
    std::string sec = section;
    killSpaces(sec);
    lowerString(sec);
    
    if (dictionary.find(sec) == dictionary.end())
      return false;

    dictionary.erase(dictionary.find(sec));
    return true;
  }

  bool configFileHandler::deleteKey(const std::string &section,
                                    const std::string &key) {
    //std::list<configEntry>::iterator it;
    std::string sec = section; 
    killSpaces(sec);
    lowerString(sec);
    std::string ky = key; 
    killSpaces(ky);
    lowerString(ky);

    std::map<std::string, std::map<std::string, configEntry> >::iterator it2
      = dictionary.find(sec);
    
    if (it2 == dictionary.end())
      return false;    
    
    std::map<std::string, configEntry>::iterator it = it2->second.find(ky);
    if (it == it2->second.end())
      return false;

    it2->second.erase(it2->second.find(ky));
    return true;
  }

  void configFileHandler::clear() {
    dictionary.clear();
    orderKeys.clear();
    orderSections.clear();
    sectionComments.clear();
    sectionOrig.clear();
    keyOrig.clear();
  }


  // other --------------------------------------------------------------------

  std::string configFileHandler::splitLine(const std::string& inputLine,
                                           std::string& prolog,
                                           std::string& section,
                                           std::string& key,
                                           std::string& value,
                                           std::string& comment) const {

    enum { PROLOG, SECTION, SECTIONEPILOG, KEY, VALUE, COMMENT } state;

    // initialize empty
    prolog = section = key = value = comment = "";

    //initial status
    state = PROLOG;

    // scan characters
    for(unsigned int x=0; x<inputLine.size(); x++){

      unsigned char c = inputLine[x];

      switch(state) {
      case PROLOG:
        if( isLineStartComment(c) ){
          prolog = "";
          comment = inputLine.substr(1);
          killSpaces(comment);
          x=inputLine.size();
        }
        else if( c == '[' )
          state = SECTION;
        else if( isWhiteSpace(c) )
          prolog += c;
        else if( c == '=' )
          return "no key name specified";
        else if( c== ']' )
          return "the '[' and a name is missing";
        else if( !isLegalChar(c) )
          return "illegal character found";
        else {
          state = KEY;
          key += c;
        }
        break;
      case SECTION:
        if( c == ']' )
          state = SECTIONEPILOG;
        else if( !isLegalChar(c) )
          return "illegeal character found";
        else if( isLineStartComment(c) )
          return "comment character in a section definition";
        else if( c== '[' )
          return "expect the ']' not the '[' character";
        else
          section += c;
        break;
      case SECTIONEPILOG:
        if( isWhiteSpace(c) )
          break; //do nothing
        else if( isLineMidComment(c) )
          state = COMMENT;
        else
          return "only comments are allowed after a section";
        break;
      case KEY:
        if( c == '=' )
          state = VALUE;
        else if( isLegalChar(c) )
          key += c;
        else
          return "wrong character in key expression";
        break;
      case VALUE:
        if( isLineMidComment(c) )
          state = COMMENT;
        else if( isLegalChar(c)||isWhiteSpace(c) )
          value += c;
        else
          return "wrong character in value expression";
        break;
      case COMMENT:
        comment += c;
        break;
      default: //default is a comment line
        prolog = "";
        comment = inputLine;
        x=inputLine.size();
        break;
      }
    }

    if( state == KEY ) //no = is found in this line
      return "the '=' is missing";

    killSpaces(section);
    //lowerString(section);
    killSpaces(key);
    //lowerString(key);
    killSpaces(value);

    return "";
  }


  void configFileHandler::composeLine(const std::string& prolog,
                                      const std::string& section,
                                      const std::string& key,
                                      const std::string& value,
                                      const std::string& comment,
                                      bool secDef,
                                      int keyLength,
                                      std::string& outputLine) const {

    outputLine = "";

    if(secDef) { //a section definition

      if (getParameters().formatOutput) {
        outputLine = "\n";
      }

      if( (section!="")&&(comment=="") ) {
        outputLine = outputLine + "[" + section +"]";
      }
      else if( (section!="")&&(comment!="") ) {
        outputLine = outputLine + "[" + section +"] ;" + comment;
      }
      else {
        outputLine = "";
      }

    }
    else { // a normal comment or key-value line

      std::string fillStr="";
      if (keyLength > static_cast<int>(key.size())) {
        fillStr = std::string(keyLength-key.size(),' ');
      }

      if( (key=="") && (value=="") && (comment!="") ) {
        outputLine = comment;
      }
      else if( (key!="")&&(comment=="") ) {
        outputLine = prolog + key + fillStr + " = " + value;
      }
      else if( (key!="")&&(comment!="") ) {
        outputLine = prolog + key + fillStr + " = " + value + " ;" + comment;
      }
      else {
        outputLine = "";
      }
    }

  }

  int configFileHandler::getMaxKeyLengths(std::map<std::string,int>& maxKeyLengths) const {
    maxKeyLengths.clear();
    int maxLength = 0;

    std::map<std::string, std::map<std::string, configEntry> >::const_iterator itSection;
    std::map<std::string, configEntry>::const_iterator itEntry;

    for (itSection = dictionary.begin(); itSection != dictionary.end(); ++itSection) {
      for (itEntry = itSection->second.begin(); itEntry != itSection->second.end(); ++itEntry) {
        maxKeyLengths[itSection->first] = lti::max<unsigned int>(
          maxKeyLengths[itSection->first], itEntry->first.size());
        maxLength = lti::max<unsigned int>(maxLength, itEntry->first.size());
      }
    }
    return maxLength;
  }

  std::list<std::string> configFileHandler::getKeys(std::string section) const {
    killSpaces(section);
    lowerString(section);
    std::list<std::string> keys;
    std::map<std::string, std::map<std::string, configEntry> >::const_iterator it;
    std::map<std::string, configEntry>::const_iterator it2;
    it = dictionary.find(section);
    if (it == dictionary.end())
      return keys;
    for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
      keys.push_back(it2->first);
    return keys;
  }

}
