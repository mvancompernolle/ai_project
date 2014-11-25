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
 * file .......: ltiExternViewer.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 21.07.99
 * revisions ..: $Id: ltiExternViewer.cpp,v 1.12 2010/04/10 01:34:47 alvarado Exp $
 */

#include <string>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>

#include "ltiObject.h"
#include "ltiRGBPixel.h"

#ifndef _LTI_WIN32

# include <dirent.h>
# include <unistd.h>

#else // if _LTI_WIN32

# include <windows.h>
# include <process.h>
# include "ltiThread.h"

#endif //

#include "ltiException.h"
#include "ltiExternViewer.h"
#include "ltiBMPFunctor.h"

namespace lti {

#if defined(_LTI_LINUX)
  // default constructor for the externViewer parameters
  externViewer::parameters::parameters() : viewerBase::parameters() {
    tmpDirectory = "/tmp";
    externViewerApp    = "kview";
  }

#elif defined(_LTI_WIN32)

  // default constructor for the externViewer parameters
  externViewer::parameters::parameters()  : viewerBase::parameters() {
    tmpDirectory = "C:\\TEMP";
    externViewerApp    = "mspaint";
  }

#elif defined(_LTI_MACOSX)

  // default constructor for the externViewer parameters
  externViewer::parameters::parameters()  : viewerBase::parameters() {
    tmpDirectory = "/tmp";
    // should be sufficient to just call "open"
    externViewerApp    = "open -a Preview";
  }


# endif

  // copy constructor
  externViewer::parameters::parameters(const parameters& other)
  : viewerBase::parameters() {
    copy(other);
  }

  // destructor
  externViewer::parameters::~parameters() {
  }

  // get type name
  const char* externViewer::parameters::getTypeName() const {
    return "externViewer::parameters";
  }

  // copy member

  externViewer::parameters&
  externViewer::parameters::copy(const parameters& other) {

    tmpDirectory = other.tmpDirectory;
    externViewerApp  = other.externViewerApp;

    return *this;
  }

  externViewer::parameters&
  externViewer::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* externViewer::parameters::clone() const {
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
  bool externViewer::parameters::write(ioHandler& handler,
                                       const bool complete) const
# else
  bool externViewer::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"tmpDirectory",tmpDirectory);
      lti::write(handler,"externViewerApp",externViewerApp);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && viewerBase::parameters::write(handler,false);
# else
    bool (viewerBase::parameters::* p_writeMS)(ioHandler&,const bool) const =
      viewerBase::parameters::writeMS;
    b = b && (this->*p_writeMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool externViewer::parameters::write(ioHandler& handler,
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
  bool externViewer::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool externViewer::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"tmpDirectory",tmpDirectory);
      lti::read(handler,"externViewerApp",externViewerApp);
    }

# ifndef _LTI_MSC_6
    // This is the standard C++ code, which MS Visual C++ 6 is not able to
    // compile...
    b = b && viewerBase::parameters::read(handler,false);
# else
    bool (viewerBase::parameters::* p_readMS)(ioHandler&,const bool) =
      viewerBase::parameters::readMS;
    b = b && (this->*p_readMS)(handler,false);
# endif

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool externViewer::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif


  // --------------------------------------------------
  // extern viewer
  // --------------------------------------------------

  // static variables initilialization
  int externViewer::numberShows = 0;

  externViewer::externViewer() {
    parameters defParam;
    setParameters(defParam);
  }

  externViewer::externViewer(const externViewer& other)
    : viewerBase() {
    copy(other);
  }

  externViewer::~externViewer() {
  }

  /*
   * returns the name of this type ("externViewer")
   */
  const char* externViewer::getTypeName() const {
    return "externViewer";
  }

  const externViewer::parameters& externViewer::getParameters() const {
    const parameters* par =
      dynamic_cast<const parameters*>(&viewerBase::getParameters());
    if(isNull(par)) {
      throw invalidParametersException(getTypeName());
    }
    return *par;
  }

  bool externViewer::show(const image& img) {
    const parameters& param = getParameters();

#  ifndef _LTI_WIN32
    // LINUX/UNIX:

    // this implementation still ignores 'externViewerWnd'!
    numberShows++; // one more...

    // 1. Creates file in the temporary directory

    // 1.1. Check if the temporary directory exists
    int theDir;

    // unix implementation

    struct stat dirStat;

    theDir = stat(param.tmpDirectory.c_str(),&dirStat);

    if ((theDir < 0) || (!S_ISDIR(dirStat.st_mode))) {
      // an error has occured
      setStatusString("Directory for temporary files not found.");
      return false;
    }

    // 1.2. Generate a name for the file
    pid_t pid;
    pid = getpid(); // get process id

    std::string filename;

    filename = param.tmpDirectory;

    if (param.tmpDirectory.rfind('/') < param.tmpDirectory.size()-1)
      filename += "/";

    char buffer[1024];
    sprintf(buffer,"lti_%X_%X.bmp",pid,numberShows);
    filename += buffer;

    // 1.3. Save image

    saveBMP saveImg;
    saveImg.save(filename,img);

    // 2. Starts new process

    if ( (pid = fork()) < 0) {
      // an error has occured
      setStatusString("Problem with fork");
      return false;
    }
    else {
      if (pid == 0) {
        // child process displays image!
        std::string commandLine;

        commandLine = param.externViewerApp + " " + filename;

        if (system(commandLine.c_str()) < 0) {
          // an error has occured
          setStatusString("Problem calling external application");
          return false;
        }

        // delete file!
        commandLine = "rm " + filename;
        int res=system(commandLine.c_str());
        _exit(res);
      }
    }

# else // WINDOWS IMPLEMENTATION

    // this implementation still ignores 'externViewerWnd'!
    numberShows++; // one more...

    // 1. Creates file in the temporary directory

    // 1.1. Check if the temporary directory exists
    int theDir;

    // the same thing for WIN32

    struct _stat dirStat;

    theDir = _stat(param.tmpDirectory.c_str(),&dirStat);
    if ((theDir < 0) || ((_S_IFDIR & dirStat.st_mode) == 0)) {
      // an error has occured
      setStatusString("Directory for temporal files not found");
      return false;
    }

    // 1.2. Generate a name for the file
    int pid;
    pid = _getpid(); // get process id

    std::string filename,batchFile;

    filename = param.tmpDirectory;

    if (param.tmpDirectory.rfind('\\') < param.tmpDirectory.size()-1)
      filename += "\\";

    batchFile = filename;

    char buffer[1024];
    sprintf(buffer,"lti_%X_%X.bmp",pid,numberShows);
    filename += buffer;
    sprintf(buffer,"lti_%X_%X.bat",pid,numberShows);
    batchFile += buffer;

    // 1.3. Save image

    saveBMP saveImg;
    saveImg.save(filename,img);

    // 1.4 Save a batch file
    std::ofstream batch(batchFile.c_str());
    batch << "@echo off" << std::endl;
    batch << "start /w " << param.externViewerApp
          << " " << filename << std::endl;
    batch << "del " << filename << std::endl;
    batch << "del " << batchFile;
    batch.close();

    // 2. Start new process

    _spawnlp(_P_NOWAIT,batchFile.c_str(),
                       batchFile.c_str(),
                       NULL);

# endif
    return true;
  }

  /*
   * shows a 8-bit channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool externViewer::show(const channel8& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool externViewer::show(const channel& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a channel or matrix of float
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool externViewer::show(const matrix<float>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool externViewer::show(const vector<double>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool externViewer::show(const vector<float>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a vector of double
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool externViewer::show(const vector<int>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a matrix of doubles as a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool externViewer::show(const matrix<double>& data) {
    return viewerBase::show(data);
  }

  /*
   * shows a matrix of integers as a channel
   * @param data the object to be shown.
   * @return true if successful, false otherwise.
   */
  bool externViewer::show(const matrix<int>& data) {
    return viewerBase::show(data);
  }

  bool externViewer::hide() {
    setStatusString("hide not implemented for externViewer");
    return false;
  }

  viewerBase* externViewer::clone() const {
    return new externViewer(*this);
  }

  /**
   * copy data of "other" functor.
   */
  externViewer& externViewer::copy(const externViewer& other) {
    viewerBase::copy(other);
    return (*this);
  }

  /**
   * copy data of "other" functor.
   */
  externViewer& externViewer::operator=(const externViewer& other) {
    return copy(other);
  }

} // namespace lti
