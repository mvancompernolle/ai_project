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
 * file .......: ltiLoadImageList.cpp
 * authors ....: Peter Doerfler
 * organization: LTI, RWTH Aachen
 * creation ...: 22.11.2002
 * revisions ..: $Id: ltiLoadImageList.cpp,v 1.17 2006/09/08 10:01:54 ltilib Exp $
 */

#include "ltiRGBPixel.h"
#include "ltiLoadImageList.h"
#include <fstream>
#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
#include <dirent.h>
#endif
#ifdef _LTI_WIN32
#include <io.h>
#endif

// #define _LTI_DEBUG

#ifdef _LTI_DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

#include "ltiSTLIoInterface.h"

namespace lti {
  // --------------------------------------------------
  // loadImageList::parameters
  // --------------------------------------------------

  // default constructor
  loadImageList::parameters::parameters()
    : ioFunctor::parameters() {

    mode = parameters::ListFileName;
    dirName = "";
    listFileName = "";
    nameList = std::list<std::string>();
    skipErrors = false;
  }

  // copy constructor
  loadImageList::parameters::parameters(const parameters& other)
    : ioFunctor::parameters()  {
    copy(other);
  }

  // destructor
  loadImageList::parameters::~parameters() {
  }

  // get type name
  const char* loadImageList::parameters::getTypeName() const {
    return "loadImageList::parameters";
  }

  // copy member

  loadImageList::parameters&
    loadImageList::parameters::copy(const parameters& other) {
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


      dirName = other.dirName;
      listFileName = other.listFileName;
      mode = other.mode;
      nameList = other.nameList;
      skipErrors = other.skipErrors;

    return *this;
  }

  // alias for copy member
  loadImageList::parameters&
    loadImageList::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* loadImageList::parameters::clone() const {
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
  bool loadImageList::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool loadImageList::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      switch (mode) {
          case ListFileName:
            lti::write(handler,"mode","ListFileName");
            break;
          case DirName:
            lti::write(handler,"mode","DirName");
            break;
          case NameList:
            lti::write(handler,"mode","NameList");
            break;
          default:
            lti::write(handler,"mode","ListFileName");
      }
      lti::write(handler,"dirName",dirName);
      lti::write(handler,"listFileName",listFileName);
      lti::write(handler,"nameList",nameList);
      lti::write(handler,"skipErrors",skipErrors);
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
  bool loadImageList::parameters::write(ioHandler& handler,
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
  bool loadImageList::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool loadImageList::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      std::string tmp;
      lti::read(handler,"mode",tmp);
      if (tmp=="ListFileName") {
        mode=ListFileName;
      } else if (tmp=="DirName") {
        mode=DirName;
      } else if (tmp=="NameList") {
        mode=NameList;
      } else {
        mode=ListFileName;
      }

      lti::read(handler,"dirName",dirName);
      lti::read(handler,"listFileName",listFileName);
      lti::read(handler,"nameList",nameList);
      lti::read(handler,"skipErrors",skipErrors);
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
  bool loadImageList::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // loadImageList
  // --------------------------------------------------

  // default constructor
  loadImageList::loadImageList()
    : ioFunctor(), fileNames(0), iter(), emptyDummy(""), iLoader() {

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

  }

  // default constructor
  loadImageList::loadImageList(const parameters& par)
    : ioFunctor(), fileNames(0), iter(), emptyDummy(""), iLoader() {

    //TODO: comment the attributes of your functor
    // If you add more attributes manually, do not forget to do following:
    // 1. indicate in the default constructor the default values
    // 2. make sure that the copy member also copy your new attributes, or
    //    to ensure there, that these attributes are properly initialized.

    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  loadImageList::loadImageList(const loadImageList& other)
    : ioFunctor()  {
    copy(other);
  }

  // constructor
  loadImageList::loadImageList(const std::string& name,
                               parameters::eOperationMode mode)
  : ioFunctor(), fileNames(0), iter(), emptyDummy(""), iLoader() {
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    defaultParameters.mode=mode;
    if (mode==parameters::ListFileName) {
      defaultParameters.listFileName=name;
    } else if (mode==parameters::DirName) {
      defaultParameters.dirName=name;
    }
    // set the default parameters
    setParameters(defaultParameters);
  }

  loadImageList::loadImageList(const std::list<std::string>& names)
    : ioFunctor(), fileNames(0), iter(), emptyDummy(""), iLoader() {
    // create an instance of the parameters with the default values
    parameters defaultParameters;
    defaultParameters.mode=parameters::NameList;
    defaultParameters.nameList=names;
    // set the default parameters
    setParameters(defaultParameters);  }

  // destructor
  loadImageList::~loadImageList() {
  }

  // returns the name of this type
  const char* loadImageList::getTypeName() const {
    return "loadImageList";
  }

  // copy member
  loadImageList& loadImageList::copy(const loadImageList& other) {
    ioFunctor::copy(other);
    
    fileNames=other.fileNames;
    iter=fileNames.begin();
    
    return (*this);
  }

  // alias for copy member
  loadImageList&
  loadImageList::operator=(const loadImageList& other) {
    return (copy(other));
  }


  // clone member
  functor* loadImageList::clone() const {
    return new loadImageList(*this);
  }

  // return parameters
  const loadImageList::parameters&
    loadImageList::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&functor::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool loadImageList::updateParameters() {
    bool b=true;
    fileNames.clear();
    b=getImageNames();
    iter=fileNames.begin();
    return b;
  }

  bool loadImageList::hasNext() const {
    return ((!fileNames.empty()) && iter!=fileNames.end());
  }

  const std::string& loadImageList::getNextFileName() const {
    if (fileNames.empty() || iter==fileNames.end()) {
      return emptyDummy;
    } else {
      return *iter;
    }
  }

  void loadImageList::rewind() {
    iter=fileNames.begin();
  }

  int loadImageList::size() const {
    return fileNames.size();
  }

  // -------------------------------------------------------------------
  // The apply-methods!
  // -------------------------------------------------------------------

  bool loadImageList::use(const std::string& name,
                          parameters::eOperationMode mode) {

    parameters p=getParameters();
    p.mode=mode;
    if (mode==parameters::ListFileName) {
      p.listFileName=name;
    } else if (mode==parameters::DirName) {
      p.dirName=name;
    } else {
      setStatusString("Illegal eOperationMode in use()");
      return false;
    }
    return setParameters(p);
  }

  bool loadImageList::use(const std::list<std::string>& names) {

    parameters p=getParameters();
    p.mode=parameters::NameList;
    p.nameList=names;
    return setParameters(p);
  }

  bool loadImageList::apply(image& img) {

    bool b=true;

    if (fileNames.empty()) {
      setStatusString("no source for images available");
      img=image();
      return false;
    }

    if (iter!=fileNames.end()) {
      const std::string& tmp=*iter;
      b=iLoader.load(tmp,img);
      iter++;
      if (!b) {
        setStatusString(iLoader.getStatusString());
        if (getParameters().skipErrors) {
          if (iter!=fileNames.end()) {
            setStatusString("Loading file: ");
            appendStatusString((*iter).c_str());
          } else {
            setStatusString("No more image files in list");
          }
          apply(img);
          appendStatusString("\nCould not open file: ");
          appendStatusString(tmp.c_str());
          return false;
        }
      }
    } else {
      img=image();
      setStatusString("no more images to load");
      return false;
    }

    return b;
  }

  bool loadImageList::apply(channel& ch) {

    bool b=true;

    if (fileNames.empty()) {
      setStatusString("no source for images available");
      ch=channel();
      return false;
    }

    if (iter!=fileNames.end()) {
      const std::string& tmp=*iter;
      b=iLoader.load(tmp,ch);
      iter++;
      if (!b) {
        setStatusString(iLoader.getStatusString());
        if (getParameters().skipErrors) {
          if (iter!=fileNames.end()) {
            setStatusString("Loading file: ");
            appendStatusString((*iter).c_str());
          } else {
            setStatusString("No more image files in list");
          }
          apply(ch);
          appendStatusString("\nCould not open file: ");
          appendStatusString(tmp.c_str());
          return false;
        }
      }
    } else {
      ch=channel();
      setStatusString("no more images to load");
      return false;
    }

    return b;
  }

  bool loadImageList::apply(channel8& ch, lti::palette& pal) {

    bool b=true;

    if (fileNames.empty()) {
      setStatusString("no source for images available");
      ch=channel8();
      pal=palette();
      return false;
    }

    if (iter!=fileNames.end()) {
      const std::string& tmp=*iter;
      b=iLoader.load(tmp,ch,pal);
      iter++;
      if (!b) {
        setStatusString(iLoader.getStatusString());
        if (getParameters().skipErrors) {
          if (iter!=fileNames.end()) {
            setStatusString("Loading file: ");
            appendStatusString((*iter).c_str());
          } else {
            setStatusString("No more image files in list");
          }
          apply(ch,pal);
          appendStatusString("\nCould not open file: ");
          appendStatusString(tmp.c_str());
          return false;
        }
      }
    } else {
      ch=channel8();
      pal=palette();
      setStatusString("no more images to load");
      return false;
    }


    return b;
  }

  bool loadImageList::apply(std::vector<image>& images) {

    bool b=true;
    images.clear();
    images.resize(size());
    int i;
    for (i=0; i<size(); i++) {
      b = b && apply(images[i]);
    }
   
    if (!b) {
      images.clear();
    }

    return b;
  }

  bool loadImageList::apply(std::vector<channel>& channels) {

    bool b=true;
    channels.clear();
    channels.resize(size());
    int i;
    for (i=0; i<size(); i++) {
      b = b && apply(channels[i]);
    }
   
    if (!b) {
      channels.clear();
    }

    return b;
  }

  bool loadImageList::apply(std::vector<channel8>& channels,
                            std::vector<palette>& colors) {

    bool b=true;
    channels.clear();
    colors.clear();
    channels.resize(size());
    colors.resize(size());
    int i;
    for (i=0; i<size(); i++) {
      b = b && apply(channels[i],colors[i]);
    }
   
    if (!b) {
      channels.clear();
      colors.clear();
    }

    return b;
  }

  bool loadImageList::checkHeader(point& imageSize) {
    return iLoader.checkHeader(*iter, imageSize);
  }

  bool loadImageList::checkHeader(point& imageSize,
                                  bool& trueColor) {
    return iLoader.checkHeader(*iter, imageSize, trueColor);
  }

// protected methods

  bool loadImageList::getImageNames() {

    const parameters& param=getParameters();
    bool b=true;

    switch (param.mode) {

        case parameters::ListFileName: {
          if (param.listFileName=="") {
            setStatusString("no listFileName specified");
            return false;
          }
          b=readListFile(param.listFileName);
          break;
        }
        case parameters::DirName: {
          if (param.dirName=="") {
            setStatusString("no dirName specified");
            return false;
          }
          b=readDirectory(param.dirName);
          break;
        }
        case parameters::NameList: {
          if (param.nameList.empty()) {
            setStatusString("nameList is empty");
            return false;
          }
          fileNames=param.nameList;
          break;
        }
        default:
          setStatusString("unknown eOperationMode in parameters");
          return false;
    }
    return b;
  }

  bool loadImageList::readListFile(const std::string& filename) {

    bool b=true;
    std::ifstream is;
    is.open(filename.c_str());
    b = b && is.good();
    b = b && !is.eof();
    if (!b) {
      setStatusString("could not open file or empty file");
      return false;
    }
    do {
      std::string tmp;
      std::string::size_type pos;
	  std::getline(is, tmp);
      pos = tmp.find(';');
      if (pos != std::string::npos) {
        tmp.erase(pos);
      }
      if (tmp=="") continue;
      fileNames.push_back(tmp);
    } while (!is.eof());

    std::list<std::string>::iterator it=fileNames.begin();
    while (it!=fileNames.end()) {
#ifdef _LTI_DEBUG
      std::cout << *it << std::endl;
#endif
      it++;
    }
#ifdef _LTI_DEBUG
    std::cout <<  std::endl;
#endif

    return b;
  }

  bool loadImageList::readDirectory(const std::string& dirName) {

#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
    DIR *dir;
    dirent* entry;
#endif
#ifdef _LTI_WIN32
    struct _finddata_t entry;
    long handle;
#endif
    std::string ndir,file;

    ndir = dirName;
    if (ndir[ndir.length()-1] != '/') {
      ndir += "/";
    }
#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
    // check if the path directory exists
    dir = opendir(dirName.c_str());
    if (dir==NULL) {
      setStatusString("Could not open directory");
      return false;
    }

    // while still objects in directory
    while ((entry=readdir(dir)) != 0) {
      file = entry->d_name;
      if ((file == ".") || (file == "..")) {
        continue;
      }
      if (!hasImageExtension(file)) {
        continue;
      }
      file = ndir + file;
      fileNames.push_back(file);
    }

    closedir(dir);
#endif
#ifdef _LTI_WIN32
    const std::string base = ndir+"*";
    handle=_findfirst(base.c_str(),&entry);
    if (handle != -1) {
      while (_findnext(handle,&entry) == 0) {
        file = entry.name;
        if ((file == ".") || (file == "..")) {
          continue;
        }
        if (!hasImageExtension(file)) {
          continue;
        }
        file = ndir + file;
        fileNames.push_back(file);
      }
      _findclose(handle);
    }
#endif

    fileNames.sort();

    return true;
  }

  bool loadImageList::hasImageExtension(std::string& filename) {

    const unsigned int pointPos = filename.rfind('.',filename.size());
    std::string ext = filename.substr(pointPos+1,filename.size());

    //format extension name
    std::string::iterator it;
    for(it=ext.begin();it!=ext.end();++it) {
      *it = tolower(*it);
    }

    //decision of file type
    return ((ext == "bmp")  ||
            (ext == "jpeg") ||
            (ext == "jpg")  ||
            (ext == "png"));
  }


}
