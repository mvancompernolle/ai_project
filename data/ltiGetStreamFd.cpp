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
 * file .......: ltiGetStreamFd.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 11.11.2002
 * revisions ..: $Id: ltiGetStreamFd.cpp,v 1.15 2008/08/17 22:20:13 alvarado Exp $
 */

#include "ltiGetStreamFd.h"
#include "ltiMacroSymbols.h"

#include <cstring>
#ifdef HAVE_STREAMBUF_FD
#  include <iostream>
#  include <fstream>
#else
#  include <istream>
#  include <ostream>
#  include <fstream>
#  include <locale>       // For codecvt
#  ifdef _LTI_GNUC
//   for accessing the basic_filebuf object
#    include <bits/c++io.h>
#    include <bits/basic_file.h>
#    include <bits/gthr.h>
#  endif
// Changes in 3.4 seem to apply to following versions as well
#  if defined(_LTI_GNUC_3_4) || defined(_LTI_GNUC_4)
#    define _LTI_NEWER_GCC
#  endif
#endif

#include <errno.h>
#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
#  include <unistd.h>
#else
#  include <io.h>
#  define dup _dup
#endif

namespace lti {

/*
 * Depending on the OS and compiler define which way should
 * be used to get the file descriptor from a stream.
 */ 
#if !defined(HAVE_STREAMBUF_FD) && defined(_LTI_GNUC)
  /*
   * Used for GCC 3.2 
   */
#  define GCC32_STREAMBUF
#elif defined(_LTI_MSC_VER)
  /*
   * Used for MS Visual C++
   */
#  define MSVC_STREAMBUF

  /*
   * Otherwise, if the HAVE_STREAMBUF is defined we use the 
   * proper fd() member.  If this is not the case, we cannot
   * return the file descriptor and all methods will return
   * error.
   */
#endif

  

#ifdef GCC32_STREAMBUF
  /*
   * Now, this is a very ugly hack to get the filedescriptor
   * of a fstream.
   * Each fstream has an associated std::filebuf object. This can
   * be accessed legally. However, sometimes we need access to the
   * underlying native file descriptor. This can be accessed via
   * the _M_file member of the filebuf object, which is
   * unfortunately protected. So, we use the following dummy class
   * which is an exact duplicate of the std::basic_filebuf class,
   * except that it has a public _M_type object.
   */
  template<typename _CharT, typename _Traits>
  class basic_filebuf : public std::basic_streambuf<_CharT, _Traits>
  {
  public:
    typedef _CharT                                    char_type;
    typedef _Traits                                   traits_type;
    typedef typename std::__basic_file<char>     __file_type;

    // the next variables are protected in the original class.

    // MT lock inherited from libio or other low-level io library.
    std::__c_lock                  _M_lock;

    // External buffer.
    __file_type               _M_file;

    // rest of class does not matter to us.
  };

  // this is the type that is used in std::ifstream and
  // std::ofstream
  typedef basic_filebuf<char, std::char_traits<char> > standard_filebuf;

#endif

  // --------------------------------------------------
  // getStreamFd::parameters
  // --------------------------------------------------

  // default constructor
  getStreamFd::parameters::parameters()
    : functor::parameters() {

    duplicateFd = false;
  }

  // copy constructor
  getStreamFd::parameters::parameters(const parameters& other)
    : functor::parameters()  {
    copy(other);
  }

  // destructor
  getStreamFd::parameters::~parameters() {
  }

  // get type name
  const char* getStreamFd::parameters::getTypeName() const {
    return "getStreamFd::parameters";
  }

  // copy member

  getStreamFd::parameters&
    getStreamFd::parameters::copy(const parameters& other) {
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
  duplicateFd = other.duplicateFd;

    return *this;
  }

  // alias for copy member
  getStreamFd::parameters&
    getStreamFd::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  functor::parameters* getStreamFd::parameters::clone() const {
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
  bool getStreamFd::parameters::write(ioHandler& handler,
                                         const bool complete) const
# else
  bool getStreamFd::parameters::writeMS(ioHandler& handler,
                                           const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {

      lti::write(handler,"duplicateFd",duplicateFd);
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
  bool getStreamFd::parameters::write(ioHandler& handler,
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
  bool getStreamFd::parameters::read(ioHandler& handler,
                                        const bool complete)
# else
  bool getStreamFd::parameters::readMS(ioHandler& handler,
                                          const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {

      lti::read(handler,"duplicateFd",duplicateFd);
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
  bool getStreamFd::parameters::read(ioHandler& handler,
                                        const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif

  // --------------------------------------------------
  // getStreamFd
  // --------------------------------------------------

  // default constructor
  getStreamFd::getStreamFd()
    : functor(){

    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);
  }

  // default constructor
  getStreamFd::getStreamFd(const parameters& par)
    : functor() {
    // set the given parameters
    setParameters(par);
  }


  // copy constructor
  getStreamFd::getStreamFd(const getStreamFd& other)
    : functor()  {
    copy(other);
  }

  // destructor
  getStreamFd::~getStreamFd() {
  }

  // returns the name of this type
  const char* getStreamFd::getTypeName() const {
    return "getStreamFd";
  }

  // copy member
  getStreamFd& getStreamFd::copy(const getStreamFd& other) {
    functor::copy(other);

    return (*this);
  }

  // alias for copy member
  getStreamFd& getStreamFd::operator=(const getStreamFd& other) {
    return (copy(other));
  }


  // clone member
  functor* getStreamFd::clone() const {
    return new getStreamFd(*this);
  }

  // return parameters
  const getStreamFd::parameters& getStreamFd::getParameters() const {
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

  bool getStreamFd::apply(std::ifstream& ins, int& fd) const {
    fd=getfd(ins);
    if (fd >= 0 && getParameters().duplicateFd) {
      fd=dup(fd);
      if (fd < 0) {
        setStatusString(strerror(errno));
        return false;
      } else {
        return true;
      }
    }
    return (fd >= 0);
  }

  bool getStreamFd::apply(std::ofstream& ins, int& fd) const {
    fd=getfd(ins);
    if (fd >= 0 && getParameters().duplicateFd) {
      fd=dup(fd);
      if (fd < 0) {
        setStatusString(strerror(errno));
        return false;
      } else {
        return true;
      }
    }
    return (fd >= 0);
  }

  bool getStreamFd::apply(std::fstream& ins, int& fd) const {
    fd=getfd(ins);
    if (fd >= 0 && getParameters().duplicateFd) {
      fd=dup(fd);
      if (fd < 0) {
        setStatusString(strerror(errno));
        return false;
      } else {
        return true;
      }
    }
    return (fd >= 0);
  }


  bool getStreamFd::apply(std::ifstream& ins, FILE*& f) const {
    int fd;
    if (apply(ins,fd)) {
      f=fdopen(fd,"r");
      if (notNull(f)) {
        return true;
      } else {
        setStatusString(strerror(errno));
        return false;
      }
    } else {
      return false;
    }
  }

  bool getStreamFd::apply(std::ofstream& ins, FILE*& f) const {
    int fd;
    if (apply(ins,fd)) {
      f=fdopen(fd,"w");
      if (notNull(f)) {
        return true;
      } else {
        setStatusString(strerror(errno));
        return false;
      }
    } else {
      return false;
    }
  }


  std::ifstream* getStreamFd::newInputStream(int fd, bool buffered) const {
#ifdef GCC32_STREAMBUF
    bool duplicate=getParameters().duplicateFd;
    std::ifstream* ins=new std::ifstream();
    standard_filebuf* dummy=reinterpret_cast<standard_filebuf*>(ins->rdbuf());
    if (duplicate) {
      fd=dup(fd);
    }
#ifdef _LTI_NEWER_GCC
    dummy->_M_file.sys_open(fd, std::ios_base::in);
#else
    dummy->_M_file.sys_open(fd, std::ios_base::in, duplicate);
#endif
    return ins;
#elif defined(MSVC_STREAMBUF)
    // TODO:  Create a stream using the given file descritor
    return 0;
#elif defined(HAVE_STREAMBUF_FD)
    std::ifstream* f=new std::ifstream(fd);
    if (*f && !buffered) {
      f->setbuf(0,0);
    }
    return f;
#endif

  }

  std::ofstream* getStreamFd::newOutputStream(int fd, bool buffered) const {
#ifdef GCC32_STREAMBUF
    bool duplicate=getParameters().duplicateFd;
    std::ofstream* ins=new std::ofstream();
    standard_filebuf* dummy=reinterpret_cast<standard_filebuf*>(ins->rdbuf());
    if (duplicate) {
      fd=dup(fd);
    }
#ifdef _LTI_NEWER_GCC
    dummy->_M_file.sys_open(fd, std::ios_base::out);
#else 
    dummy->_M_file.sys_open(fd, std::ios_base::out, duplicate);
#endif
    return ins;
#elif defined(MSVC_STREAMBUF)
    // TODO: create a new stream with the given file descriptor
    return 0;
#elif defined(HAVE_STREAMBUF_FD)
    std::ofstream* f=new std::ofstream(fd);
    if (*f && !buffered) {
      f->setbuf(0,0);
    }
    return f;
#endif
  }


  int getStreamFd::getfd(std::ifstream& ins) const {
    int fd(-1);

#ifdef GCC32_STREAMBUF
    standard_filebuf* dummy=reinterpret_cast<standard_filebuf*>(ins.rdbuf());
    fd=dummy->_M_file.fd();
#elif defined(MSVC_STREAMBUF)
    // TODO: return the file descritor of the given ifstream
#elif defined(HAVE_STREAMBUF_FD)
    fd=ins.rdbuf()->fd();
#endif

    if (fd < 0) {
      setStatusString("Could not get file descriptor");
    }
    return fd;
  }

  int getStreamFd::getfd(std::ofstream& ins) const {
    int fd(-1);

#ifdef GCC32_STREAMBUF
    standard_filebuf* dummy=reinterpret_cast<standard_filebuf*>(ins.rdbuf());
    fd=dummy->_M_file.fd();
#elif defined(MSVC_STREAMBUF)
    // TODO: return the file descritor of the given ifstream
#elif defined(HAVE_STREAMBUF_FD)
    fd=ins.rdbuf()->fd();
#endif

    if (fd < 0) {
      setStatusString("Could not get file descriptor");
    }
    return fd;
  }

  int getStreamFd::getfd(std::fstream& ins) const {
    int fd(-1);

#ifdef GCC32_STREAMBUF
    standard_filebuf* dummy=reinterpret_cast<standard_filebuf*>(ins.rdbuf());
    fd=dummy->_M_file.fd();
#elif defined(MSVC_STREAMBUF)
    // TODO: return the file descritor of the given ifstream
#elif defined(HAVE_STREAMBUF_FD)
    fd=ins.rdbuf()->fd();
#endif
    if (fd < 0) {
      setStatusString("Could not get file descriptor");
    }
    return fd;
  }

  void getStreamFd::setDuplicate(bool d) {
    parameters p=getParameters();
    p.duplicateFd=d;
    setParameters(p);
  }


}
