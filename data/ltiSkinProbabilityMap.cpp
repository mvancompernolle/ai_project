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
 * file .......: ltiSkinProbabilityMap.cpp
 * authors ....: Benjamin Winkler
 * organization: LTI, RWTH Aachen
 * creation ...: 30.1.2001
 * revisions ..: $Id: ltiSkinProbabilityMap.cpp,v 1.7 2006/09/05 10:31:35 ltilib Exp $
 */

#include "ltiSkinProbabilityMap.h"

namespace lti {
  // --------------------------------------------------
  // skinProbabilityMap static members
  // --------------------------------------------------

  /**
   * skin model
   */
  const thistogram<double> skinProbabilityMap::theSkinModel;

  /**
   * non-skin model
   */
  const thistogram<double> skinProbabilityMap::theNonSkinModel;

  /**
   * initialization flag
   */
  bool skinProbabilityMap::initialized = false;


  // --------------------------------------------------
  // skinProbabilityMap::parameters
  // --------------------------------------------------

  // default constructor
  skinProbabilityMap::parameters::parameters()
    : probabilityMap::parameters(),
      skinProbability(objectProbability) {

    // If you add more parameters manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new parameters
    // 3. make sure that the read and write members also read and
    //    write your parameters

    // indicate the parent class not to take care of the memory!
    ownModels = false;

    // set the color models
    setObjectColorModel(skinProbabilityMap::getSkinHistogram());
    setNonObjectColorModel(skinProbabilityMap::getNonSkinHistogram());

    skinProbability = 0.5; // overwrite the value of the parent class
    path = ""; // actual directory
  }

  // copy constructor
  skinProbabilityMap::parameters::parameters(const parameters& other)
    : probabilityMap::parameters(),
      skinProbability(objectProbability) {

    // indicate the parent class not to take care of the memory!
    ownModels = false;

    setObjectColorModel(skinProbabilityMap::getSkinHistogram());
    setNonObjectColorModel(skinProbabilityMap::getNonSkinHistogram());

    copy(other);
  }

  // destructor
  skinProbabilityMap::parameters::~parameters() {
  }

  // get type name
  const char* skinProbabilityMap::parameters::getTypeName() const {
    return "skinProbabilityMap::parameters";
  }

  // copy member

  skinProbabilityMap::parameters&
    skinProbabilityMap::parameters::copy(const parameters& other) {
# ifndef _LTI_MSC_6
    // MS Visual C++ 6 is not able to compile this...
    probabilityMap::parameters::copy(other);
# else
    // ...so we have to use this workaround.
    // Conditional on that, copy may not be virtual.
    probabilityMap::parameters& (probabilityMap::parameters::* p_copy)
      (const probabilityMap::parameters&) =
      probabilityMap::parameters::copy;
    (this->*p_copy)(other);
# endif

    // not necessary to copy the skinProbability, because it is just an
    // alias

    // skinProbability = other.skinProbability;
    path = other.path;

    return *this;
  }

  // alias for copy member
  skinProbabilityMap::parameters&
    skinProbabilityMap::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* skinProbabilityMap::parameters::clone() const {
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
  bool skinProbabilityMap::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool skinProbabilityMap::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      // not necessary to save (alias for objectProbability)
      // lti::write(handler,"skinProbability",skinProbability);
      lti::write(handler,"path",path);
    }

    // it is not necessary to save the histograms...
    // just write an equivalent parent instance, with no histograms
    probabilityMap::parameters tmpParent(*this);
    const thistogram<double>* dummy = 0;
    tmpParent.setNonObjectColorModel(*dummy);
    tmpParent.setObjectColorModel(*dummy);

    b = b && tmpParent.write(handler,false);

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool skinProbabilityMap::parameters::write(ioHandler& handler,
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
  bool skinProbabilityMap::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool skinProbabilityMap::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      // skinProbability already read in the parent class (it is just an
      // alias for the object probability
      // lti::read(handler,"skinProbability",skinProbability);

      lti::read(handler,"path",path);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && probabilityMap::parameters::read(handler,false);
# else
    bool (probabilityMap::parameters::* p_readMS)(ioHandler&,const bool) =
      probabilityMap::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    // so, the parent created two histograms in the heap to load the two
    // dummy histograms we tell it to, which are not realy needed!.
    // So we need now to delete them in order to avoid an awful memory leak!
    // and to set the real ones here...
    delete nonObjectColorModel;
    delete objectColorModel;

    ownModels = false;

    // set the color models
    setObjectColorModel(skinProbabilityMap::getSkinHistogram());
    setNonObjectColorModel(skinProbabilityMap::getNonSkinHistogram());

    // now every one is happy!! (even if this wasn't very elegant)

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool skinProbabilityMap::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // skinProbabilityMap
  // --------------------------------------------------

  // default constructor
  skinProbabilityMap::skinProbabilityMap(const bool& useDefaultParams)
    : probabilityMap() {
    if (useDefaultParams) {
      parameters defParam;
      setParameters(defParam);
    }
  }

  // default constructor
  skinProbabilityMap::skinProbabilityMap(const std::string path)
    : probabilityMap() {

    parameters myParameters;
    myParameters.path = path;

    setParameters(myParameters);

    // create parameters and generate skin probability model
    generate();
  }

  // default constructor
  skinProbabilityMap::skinProbabilityMap(const char* path)
    : probabilityMap() {

    parameters myParameters;
    myParameters.path = path;

    setParameters(myParameters);

    // create parameters and generate skin probability model
    generate();
  }

  // default constructor
  skinProbabilityMap::skinProbabilityMap(const parameters& param)
    : probabilityMap(){

    setParameters(param);

    // create parameters and generate skin probability model
    generate();
  }



  // read the color models from the path specified in the parameters
  void skinProbabilityMap::initialize() {

    if (!initialized) {

      initialized = true;

      const parameters& param = getParameters();

      std::string thePath;
      std::string pathfile;
      const std::string skinFile = "skin-32-32-32.hist";
      const std::string noSkinFile = "nonskin-32-32-32.hist";

      const std::string& path = param.path;

      thePath=path;
      if ((path.length()>0) &&
          (path[path.length()-1] != '/') &&
          (path[path.length()-1] != '\\')) {

        thePath = path;
#       ifdef _LTI_WIN32
	      thePath+='\\';
#       else
	      thePath+='/';
#       endif
      }

      lispStreamHandler fileHandler;

      // load skin model
      pathfile = thePath + skinFile;
      std::ifstream histoFileSkin(pathfile.c_str());

      if (!histoFileSkin) {
        std::string str;
        str = "skin histogram file <";
        str += pathfile;
        str += "> not found.";
        initialized = false;
        throw exception(str);
      }

      fileHandler.use(histoFileSkin);
      const_cast<thistogram<double>*>(&theSkinModel)->read(fileHandler);
      histoFileSkin.close();

      // load non-skin model
      pathfile = thePath + noSkinFile;
      std::ifstream histoFileNonSkin(pathfile.c_str());

      if (!histoFileNonSkin) {
        std::string str;
        str = "non-skin histogram file <";
        str += pathfile;
        str += "> not found.";
        initialized = false;
        throw exception(str);
      }

      fileHandler.use(histoFileNonSkin);
      const_cast<thistogram<double>*>(&theNonSkinModel)->read(fileHandler);
      histoFileNonSkin.close();
    }
  }

  // copy constructor
  skinProbabilityMap::skinProbabilityMap(const skinProbabilityMap& other)
    : probabilityMap() {
    copy(other);
  }

  // destructor
  skinProbabilityMap::~skinProbabilityMap() {
  }

  // returns the name of this type
  const char* skinProbabilityMap::getTypeName() const {
    return "skinProbabilityMap";
  }

  // copy member
  skinProbabilityMap&
    skinProbabilityMap::copy(const skinProbabilityMap& other) {
      probabilityMap::copy(other);
    return (*this);
  }

  // clone member
  functor* skinProbabilityMap::clone() const {
    return new skinProbabilityMap(*this);
  }

  // return parameters
  const skinProbabilityMap::parameters&
    skinProbabilityMap::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  // temporarily sets modified parameters and calls probabilityMap::generate()
  bool skinProbabilityMap::generate() {

    // read the color models from disk
    initialize();

    // generate skin model
    return probabilityMap::generate();
  }

  // get a copy of the skin color model
  const thistogram<double>& skinProbabilityMap::getSkinHistogram() {
    return theSkinModel;
  }

  // get a copy of the non-skin color model
  const thistogram<double>& skinProbabilityMap::getNonSkinHistogram() {
    return theNonSkinModel;
  }

}
