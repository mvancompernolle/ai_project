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



/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiURL.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 08.07.02
 * revisions ..: $Id: ltiURL.cpp,v 1.11 2006/09/05 10:42:47 ltilib Exp $
 */

#include "ltiRGBPixel.h"
#include "ltiURL.h"
#include "ltiPNGFunctor.h"
#include "ltiBMPFunctor.h"
#include "ltiJPEGFunctor.h"
#include "ltiGetStreamFd.h"

#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <sys/types.h>


#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
extern "C" {
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
}
#endif

#ifdef _LTI_MACOSX
#include <strings.h>
#endif

#ifdef _LTI_WIN32
#include <winsock.h>
#include <process.h>
#include <sys/stat.h>
#define strcasecmp stricmp
#endif

#undef _LTI_DEBUG
// #define _LTI_DEBUG 1
#include "ltiDebug.h"

namespace lti {

  inline bool strEqual(const char *a, const char* b) {
    return strcmp(a,b) == 0;
  }

  inline bool strEqualI(const char *a, const char* b) {
    return strcasecmp(a,b) == 0;
  }

  inline bool strEqualI(const std::string& a, const char* b) {
    return strcasecmp(a.c_str(),b) == 0;
  }

  inline bool strStartsWith(const char *a, const char* b) {
    return strncmp(a,b,strlen(b)) == 0;
  }

  inline bool strStartsWith(const std::string& a, const char* b) {
    return strncmp(a.c_str(),b,strlen(b)) == 0;
  }



  // default constructor for the url parameters
  url::parameters::parameters() {
    tmpDirectory = "C:\\TEMP";
  }

  // copy constructor
  url::parameters::parameters(const parameters& other) {
    copy(other);
  }

  // destructor
  url::parameters::~parameters() {
  }

  // get type name
  const char* url::parameters::getTypeName() const {
    return "url::parameters";
  }

  // copy member

  url::parameters&
  url::parameters::copy(const parameters& other) {

    tmpDirectory = other.tmpDirectory;

    return *this;
  }

  url::parameters&
  url::parameters::operator=(const parameters& other) {
    return copy(other);
  }

  // clone member
  url::parameters* url::parameters::clone() const {
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
  bool url::parameters::write(ioHandler& handler,
                              const bool complete) const
# else
    bool url::parameters::writeMS(ioHandler& handler,
                                  const bool complete) const
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.writeBegin();
    }

    if (b) {
      lti::write(handler,"tmpDirectory",tmpDirectory);
    }

    if (complete) {
      b = b && handler.writeEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool url::parameters::write(ioHandler& handler,
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
  bool url::parameters::read(ioHandler& handler,
                             const bool complete)
# else
    bool url::parameters::readMS(ioHandler& handler,
                                 const bool complete)
# endif
  {
    bool b = true;
    if (complete) {
      b = handler.readBegin();
    }

    if (b) {
      lti::read(handler,"tmpDirectory",tmpDirectory);
    }

    if (complete) {
      b = b && handler.readEnd();
    }

    return b;
  }

# ifdef _LTI_MSC_6
  bool url::parameters::read(ioHandler& handler,
                             const bool complete) {
    // ...we need this workaround to cope with another really awful MSVC
    // bug.
    return readMS(handler,complete);
  }
# endif




  /*
   * class url::urlFields
   */

  url::urlFields::urlFields() {
    hostname="localhost";
    port=80;
    service=HTTP;
    path="/index.html";
  }

  url::urlFields::urlFields(const std::string& u) {

    const char *cu=u.c_str();
    const char *c=strchr(cu,':');

    char* buffer=new char[u.length()];

    // first, determine the service type
    strncpy(buffer,cu,c-cu);
    buffer[c-cu]='\0';
    if (strEqualI(buffer, "http")) {
      service=HTTP;
    } else if (strEqualI(buffer, "file")) {
      service=FILE;
    } else if (strEqualI(buffer, "ftp")) {
      service=FTP;
    } else {
      service=UNKNOWN;
    }

    // now, figure out the hostname
    if (service == HTTP || service == FTP) {
      // skip ://
      cu=c+3;
      c=strchr(cu,'/');
      if (notNull(c)) {
        strncpy(buffer,cu,c-cu);
        buffer[c-cu]='\0';
        hostname=buffer;
        path=c;
      } else {
        hostname=cu;
        path="/index.html";
      }
      c=strrchr(hostname.c_str(),':');
      if (notNull(c)) {
        port=atoi(c+1);
      } else {
        port=(service == HTTP) ? 80 : 21;
      }
    } else if (service == FILE) {
      hostname="localhost";
      cu=c+3;
      path=cu;
      port=0;
    } else {
      hostname="";
      port=0;
      path="";
    }
    delete[] buffer;
  }


  /*
   * class url::serviceHandler
   */

  url::serviceHandler::serviceHandler(url& u) : doc(u) {
    // do nothing
  }


  void url::serviceHandler::setStatusString(const char* msg, const char *msg2) {
    status=msg;
    if (notNull(msg2)) {
      status+=msg2;
    }
  }

  const char* url::serviceHandler::getStatusString() const {
    return status.c_str();
  }


  /*
   * class httpHandler
   */

  class httpHandler: public url::serviceHandler {

  public:
    enum replyCodes {
      OK = 200
    };

    httpHandler(url& u) : serviceHandler(u) {
      ok=readHTTPHeader(u.getPath(), ctype,clen,cdate);
    }

    virtual bool getDate(std::string& date) { date=cdate; return ok; };

    virtual bool getContentType(std::string& type) { type=ctype; return ok; };

    virtual bool getContentLength(int& size) { size=clen; return ok; };

    virtual bool getDataStream(std::ifstream* &i) { i=getIn(); return ok; };

  private:

    bool mygetline(std::string &hbuf) {
#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
      std::getline(*getIn(), hbuf);
      if (hbuf.length() > 0 && hbuf.at(hbuf.length()-1) == '\r') {
        hbuf.erase(hbuf.length()-1,1);
      }
      return true;
#endif

#ifdef _LTI_WIN32

      hbuf.erase(0,hbuf.length());//clear hbuf
      char buf;
      int rv;
      while (true){
        rv=recv(getSocket(),&buf,1,0);//receive one byte

        if(rv==SOCKET_ERROR) {    //check error
          char buf[50];
          std::string status = "SOCKET_ERROR recv() error-code: ";
          status.append(itoa(WSAGetLastError(),buf,10));
          setStatusString(status.c_str());
          return false;
        }
        else if(rv==1){
          hbuf += buf; //append one byte to hbuf
          if(buf=='\n'){  //end of line reached
            break;
          }
        }
        else {break;}
      };

      //delete '\n' and '\r' at end of string
      if((hbuf.at(hbuf.length()-1)=='\n')||(hbuf.at(hbuf.length()-1)=='\r')){
        hbuf.erase(hbuf.length()-1,1);
      }
      if((hbuf.at(hbuf.length()-1)=='\n')||(hbuf.at(hbuf.length()-1)=='\r')){
        hbuf.erase(hbuf.length()-1,1);
      }
      return true;
#endif
    }


    bool readHTTPHeader(const std::string& path,
                        std::string& contentType, int& contentLength,
                        std::string& date) {

      _lti_debug("PATH: " << path << "\n");

#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
      *getOut() << "GET " << path << " HTTP/1.0\r\n"
                << "User-Agent: LTIlib/0.8\r\n\r\n";
      getOut()->flush();
#endif

#ifdef _LTI_WIN32
      std::string httpHeader("GET "+path+" HTTP/1.0\nUser-Agent: LTIlib/0.8\n\n");
      send(getSocket(),httpHeader.c_str(),httpHeader.length(),0);
#endif

      std::string hbuf;
      if(!mygetline(hbuf)){return false;}
      _lti_debug("# " << hbuf << "\n");
      int ver;
      int rev;
      int rcode;
      static const int bufSize=256;
      char buffer[bufSize];
      int k=sscanf(hbuf.c_str(),"HTTP/%d.%d %d",&ver,&rev,&rcode);
      if (k != 3) {
        setStatusString("Misformed reply: ", hbuf.c_str());
        //while (hbuf.length() > 0) {
        //  std::getline(*getIn(), hbuf);
        //  _lti_debug("> " << hbuf);
        //}
        return false;
      }
      sprintf(buffer,"%d",rcode);
      const char *c = strstr(hbuf.c_str(),buffer)+strlen(buffer)+1;
      strncpy(buffer,c,bufSize-1);
      buffer[bufSize-1]='\0';
      if (rcode != OK) {
        char buffer2[bufSize+16];
        sprintf(buffer2,"Error %d: %s", rcode, buffer);
        setStatusString(buffer2);
        return false;
      }
      contentType="unknown";
      contentLength=-1;
      date="unknown";
      do {
        if(!mygetline(hbuf)){return false;}
        _lti_debug("# " << hbuf << "\n");
        if (hbuf.length() == 0 || hbuf == "\r") {
          break;
        }
        const char *hval;
        const char *cu = hbuf.c_str();
        const char *c = strchr(cu,':');
        if (c != 0) {
          strncpy(buffer,cu,c-cu);
          buffer[c-cu]='\0';
          hval=c+2;
          if (strEqualI(buffer,"Content-Type")) {
            contentType=hval;
          } else if (strEqualI(buffer,"Content-Length")) {
            contentLength=atoi(hval);
          } else if (strEqualI(buffer, "Date")) {
            date=hval;
          }
          _lti_debug("Key = " << buffer << "; Value = " << hval << "\n");
        } else {
          setStatusString("Malformed header: ",hbuf.c_str());
          return false;
        }
      } while (hbuf.length() > 0 && c != 0);
      return true;
    }

    std::string cdate;
    std::string ctype;
    int clen;

  };


  /* --------------------------------------------------------
   * class url
   * --------------------------------------------------------
   */

  url::url(const std::string &n) :params(0){


    // create an instance of the parameters with the default values
    parameters defaultParameters;
    // set the default parameters
    setParameters(defaultParameters);

#ifdef _LTI_WIN32
    instanceNumber++;
    WSADATA info;
    if (WSAStartup(MAKEWORD(1,1), &info) != 0){
      char buf[50];
      std::string status = "SOCKET_ERROR WSAStartup() error-code: ";
      status.append(itoa(WSAGetLastError(),buf,10));
      setStatusString(status.c_str());
    }
#endif

    dtype=RAW;
    defaultType="text/html";
    status="OK";
    sock=-1;
    in=0;
    out=0;
    fields=urlFields(n);
    if (initConnect() && openStreams()) {
      switch (fields.getService()) {
        case HTTP: handler=new httpHandler(*this); break;
        case FTP: handler=0; break;
        case FILE: handler=0; break;
        default: handler=0; break;
      }
      if (!handler->isOk()) {
        setStatusString("Service error: ",handler->getStatusString());
      }
    }
  }

  url::~url() {

    delete params;
    params = 0;
#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
    if (sock >= 0) {
      delete in;
      delete out;
      close(sock);
    }

#endif

#ifdef _LTI_WIN32

    closesocket(winsock);
    if (WSACleanup() != 0){
      char buf[50];
      std::string status = "SOCKET_ERROR WSACleanup() error-code: ";
      status.append(itoa(WSAGetLastError(),buf,10));
      setStatusString(status.c_str());
    }

#endif
  }

  bool url::isOk() {
    return (status == "OK");
  }


  bool url::setParameters(const url::parameters& theParams) {
    delete params;
    params = theParams.clone();

    return (params != 0);
  }


  const url::parameters& url::getParameters() const {
    return *params;
  }



  bool url::get(std::string& data) {
#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
    std::ifstream* in;
    if (!handler->getDataStream(in)) {
      return false;
    }

    data="";

    std::string t;
    handler->getContentType(t);

    if (strStartsWith(t.c_str(),"text")) {
      while (!in->eof()) {
        std::string tmp;
        std::getline(*in,tmp);
        data+=tmp;
        data+="\n";
      }
    } else {
      setStatusString("URL does not describe a text document, " \
                      "but one of type ", t.c_str());
    }
    return true;
#endif
#ifdef _LTI_WIN32

    char buf[1024];
    int rv;  //# of received bytes

    while (true){
      rv=recv(winsock, buf, 1024,0);        //receive 1024 bytes
      if(rv==SOCKET_ERROR) {                  //check for error
        char buf[50];
        std::string status = "SOCKET_ERROR: recv(): error-code: ";
        status.append(itoa(WSAGetLastError(),buf,10));
        setStatusString(status.c_str());
        return false;
      }
      else if(rv>0){                //rv bytes received
        std::string t;
        t.assign(buf,rv);
        data += t;                //append rv bytes to data
      }
      else if(rv==0){                //connection has been gracefully closed
        return true;
      }
      else return false;
    }
    return true;
#endif
  }

  bool url::get(vector<ubyte>& data) {

    std::ifstream* in;
    if (!handler->getDataStream(in)) {
      return false;
    }
    int len;
    handler->getContentLength(len);

    data.clear();

    if (len < 0) {
      setStatusString("Size of document is unknown");
      return false;
    }

    if (len > 0) {

#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
      // in->flags(std::ios::in|std::ios::binary);
      data.resize(len);
      char* buffer=reinterpret_cast<char*>(&data.at(0));
      in->read(buffer,len);
#endif

#ifdef _LTI_WIN32
      data.resize(len);
      vector<ubyte>::iterator it=data.begin();
      char buf[1024];
      int rv;  //# of received bytes

      while (true){
        rv=recv(winsock, buf, 1024,0);        //receive 1024 bytes
        if(rv==SOCKET_ERROR) {                  //check for error
          char buf[50];
          std::string status = "SOCKET_ERROR: recv(): error-code: ";
          status.append(itoa(WSAGetLastError(),buf,10));
          setStatusString(status.c_str());
          return false;
        }
        else if(rv>0){                //rv bytes received
          for(int i=0;i<rv;i++){
            (*it)=buf[i];//append rv bytes to data
            it++;
          }
        }
        else if(rv==0){                //connection has been gracefully closed
          return true;
        }
        else return false;
      }
#endif
    }
    else {
      setStatusString("URL contains empty document.");
      return false;
    }

    return true;
  }


  bool url::put(const vector<ubyte>& data) {
    setStatusString("\"put\" not implemented yet");
    return false;
  }


  bool url::put(const std::string& data) {
    setStatusString("\"put\" not implemented yet");
    return false;
  }

  bool url::get(ioObject& data, dataType type) {
    setStatusString("\"get\" not implemented for ioObjects yet");
    return false;
  }

  bool url::put(const ioObject& data, dataType type) {
    setStatusString("\"put\" not implemented yet");
    return false;
  }

#if defined(HAVE_LIBPNG) || defined(HAVE_LOCALPNG)

  bool url::getPNGImage(image& data) {

#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
    std::ifstream* in;
    std::string t;
    if (handler->getDataStream(in) && handler->getContentType(t)) {
      if (strEqualI(t,"image/png")) {
        loadPNG loader;
        if (loader.load(*in,data)) {
          return true;
        } else {
          setStatusString("loadPNG failed: ", loader.getStatusString());
          return false;
        }
      } else {
        setStatusString("URL does not describe a PNG image, " \
                        "but a document of type ", t.c_str());
        return false;
      }
    } else {
      return false;
    }
#endif

#ifdef _LTI_WIN32

    std::string s_fileName;
    if(!getTempFilePath(s_fileName)){ return false;}
    const char * fileName = s_fileName.c_str();
    std::ofstream out(fileName,std::ios_base::out | std::ios_base::binary);
    if (!out) {
      setStatusString("ofstream bad");
      return false;
    }
    std::string t;
    loadPNG loader;

    if (!handler->getContentType(t)) {return false;}

    if (!strEqualI(t,"image/png")) {
      setStatusString("URL does not describe a PNG image, " \
                      "but a document of type ", t.c_str());
      return false;
    }
    if(!receiveAndWriteFile(out)){        return false;}

    if (!loader.load(fileName,data)) {
      setStatusString("loadPNG failed: ", loader.getStatusString());
      return false;
    }

    _lti_debug("total number of bytes written in file: " \
          <<getFileSize(s_fileName)<<"\n");

    if(!DeleteFile(fileName)){
      char buf[50];
      std::string status = "deleting temporary image file " \
        "failed  ErrorCode: ";
      status.append(itoa(GetLastError(),buf,10));
      setStatusString(status.c_str());
      return false;
    }
    return true;
#endif

  }
#endif

#if defined(HAVE_LIBJPEG) || defined(HAVE_LOCALJPEG)

  bool url::getJPGImage(image& data) {

#ifdef __linux
    std::ifstream* in;
    std::string t;
    if (handler->getDataStream(in) && handler->getContentType(t)) {
      if (strEqualI(t,"image/jpeg")) {
        loadJPEG loader;
        if (loader.load(*in,data)) {
          return true;
        } else {
          setStatusString("loadJPEG failed: ", loader.getStatusString());
          return false;
        }
      } else {
        setStatusString("URL does not describe a JPG image, " \
                        "but a document of type ", t.c_str());
        return false;
      }
    } else {
      return false;
    }
#endif

#ifdef _LTI_WIN32

    std::string s_fileName;
    if(!getTempFilePath(s_fileName)){ return false;}
    const char * fileName = s_fileName.c_str();
    std::ofstream out(fileName,std::ios_base::out | std::ios_base::binary);
    if (!out) {
      setStatusString("ofstream bad");
      return false;
    }
    std::string t;
    loadJPEG loader;

    if (!handler->getContentType(t)) {return false;}

    if (!strEqualI(t,"image/jpeg")) {
      setStatusString("URL does not describe a JPG image, " \
                      "but a document of type ", t.c_str());
      return false;
    }
    if(!receiveAndWriteFile(out)){        return false;}

    if (!loader.load(fileName,data)) {
      setStatusString("loadJPEG failed: ", loader.getStatusString());
      return false;
    }

    _lti_debug("total number of bytes written in file: " \
          <<getFileSize(s_fileName)<<"\n");

    if(!DeleteFile(fileName)){
      char buf[50];
      std::string status = "deleting temporary image file " \
        "failed  ErrorCode: ";
      status.append(itoa(GetLastError(),buf,10));
      setStatusString(status.c_str());
      return false;
    }

    return true;
#endif

  }
#endif

  bool url::getBMPImage(image& data) {

#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
    std::ifstream* in;
    std::string t;
    if (handler->getDataStream(in) && handler->getContentType(t)) {
      if (strEqualI(t,"image/bmp")) {
        loadBMP loader;
        if (loader.load(*in,data)) {
          return true;
        } else {
          setStatusString("loadBMP failed: ", loader.getStatusString());
          return false;
        }
      } else {
        setStatusString("URL does not describe a BMP image, " \
                        "but a document of type ", t.c_str());
        return false;
      }
    } else {
      return false;
    }
#endif

#ifdef _LTI_WIN32

    std::string s_fileName;
    if(!getTempFilePath(s_fileName)){ return false;}
    const char * fileName = s_fileName.c_str();
    std::ofstream out(fileName,std::ios_base::out | std::ios_base::binary);

    if (!out) {
      setStatusString("ofstream bad");
      return false;
    }
    std::string t;
    loadBMP loader;

    if (!handler->getContentType(t)) {return false;}

    if (!strEqualI(t,"image/bmp")) {
      setStatusString("URL does not describe a BMP image, " \
                      "but a document of type ", t.c_str());
      return false;
    }
    if(!receiveAndWriteFile(out)){        return false;}

    if (!loader.load(fileName,data)) {
      setStatusString("loadBMP failed: ", loader.getStatusString());
      return false;
    }

    _lti_debug("total number of bytes written in file: " \
          <<getFileSize(s_fileName)<<"\n");

    if(!DeleteFile(fileName)){
      char buf[50];
      std::string status = "deleting temporary image file " \
        "failed  ErrorCode: ";
      status.append(itoa(GetLastError(),buf,10));
      setStatusString(status.c_str());
      return false;
    }
    return true;
#endif
  }

  bool url::putPNGImage(const image& data) {
    setStatusString("\"put\" not implemented yet");
    return false;
  }

  std::istream& url::getInputStream() {
    return *in;
  }

  std::ostream& url::getOutputStream() {
    return *out;
  }

  void url::setStatusString(const char* msg, const char *msg2) {
    status=msg;
    if (notNull(msg2)) {
      status+=msg2;
    }
  }

  const char* url::getStatusString() const {
    return status.c_str();
  }


  bool url::initConnect() {

#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
    struct hostent *hp;
    struct sockaddr_in sa;

    if (sock >= 0) {
      close(sock);
    }

    hp=gethostbyname(fields.getHostname().c_str());
    if (hp == NULL) {
      if (errno == 0) {
        setStatusString("Host unknown: ",fields.getHostname().c_str());
      } else {
#ifdef _LTI_MACOSX
      	setStatusString(fields.getHostname().c_str(), strerror(errno));
#else
        setStatusString(fields.getHostname().c_str(), sys_errlist[errno]);
#endif
        return false;
      }
    }

#ifdef _LTI_MACOSX
    memcpy(&sa.sin_addr,hp->h_addr_list[0],hp->h_length);
#else
    memcpy(&sa.sin_addr,hp->h_addr,hp->h_length);
#endif
    sa.sin_family=AF_INET;
    sa.sin_port=ntohs(fields.getPort());

    sock=socket(PF_INET,SOCK_STREAM,0);
    if (sock < 0) {
#ifdef _LTI_MACOSX
      setStatusString("Creating socket failed: ",strerror(errno));
#else
      setStatusString("Creating socket failed: ",sys_errlist[errno]);
#endif
      return false;
    }
    if (connect(sock, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
#ifdef _LTI_MACOSX
      setStatusString("Connection failed: ",strerror(errno));
#else
      setStatusString("Connection failed: ",sys_errlist[errno]);
#endif
      return false;
    }
    return true;
#endif

#ifdef _LTI_WIN32

    struct sockaddr_in sa;
    struct hostent *hp;


    memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */
    hp=gethostbyname(fields.getHostname().c_str());        /* who are we? */
    if (hp == NULL) {
      if (errno == 0) {
        setStatusString("Host unknown: ",fields.getHostname().c_str());
      } else {
        setStatusString(fields.getHostname().c_str(), sys_errlist[errno]);
        return false;
      }
    }

    memcpy(&sa.sin_addr,hp->h_addr,hp->h_length);
    sa.sin_family=AF_INET;
    sa.sin_port=htons(fields.getPort());

    winsock = socket(PF_INET, SOCK_STREAM, 0);        /* create the socket */
    if (winsock == INVALID_SOCKET){
      char buf[50];
      std::string status = "INVALID_SOCKET socket() error-code: ";
      status.append(itoa(WSAGetLastError(),buf,10));
      setStatusString(status.c_str());
      return false;
    }

    if (connect(winsock,(sockaddr *)&sa, sizeof sa) == SOCKET_ERROR) {
      char buf[50];
      std::string status = "INVALID_SOCKET connect() error-code: ";
      status.append(itoa(WSAGetLastError(),buf,10));
      setStatusString(status.c_str());
      closesocket(winsock);
      return false;
    }

    return true;
#endif
  }


  bool url::openStreams() {

#ifdef _LTI_WIN32
    return true;//no streams required for windows implementation
#endif

#if defined(_LTI_LINUX) || defined(_LTI_MACOSX)
    delete in;
    delete out;

    getStreamFd fdgetter;

    // we need unbuffered streams, otherwise we cannot read images
    in=fdgetter.newInputStream(sock,false);
    out=fdgetter.newOutputStream(sock);

    // The following does not work with gcc 3.2
// #ifdef HAVE_STREAMBUF_FD

//     in=new std::ifstream(sock);
//     out=new std::ofstream(sock);

//     if (!(*in)) {
//       setStatusString("Could not open input stream");
//       return false;
//     }
//     // we need unbuffered streams, otherwise we cannot read images
//     in->setbuf(0,0);
//     // in->flags(std::ios::in|std::ios::binary);
//     if (!(*out)) {
//       setStatusString("Could not open output stream");
//       return false;
//     }
//     return true;
// #else
//     return false;
// #endif

    if (!(*in)) {
      setStatusString("Could not open input stream");
      return false;
    }
    //in->setbuf(0,0);
    // in->flags(std::ios::in|std::ios::binary);
    if (!(*out)) {
      setStatusString("Could not open output stream");
      return false;
    }
    return true;

#endif

  }

  const std::string& url::getPath() const {
    return fields.getPath();
  }



#ifdef _LTI_WIN32

  bool url::getTempFilePath(std::string& filepath){

    const parameters& param = getParameters();

    // Check if the temporary directory exists
    int theDir;
    struct _stat dirStat;

    theDir = _stat(param.tmpDirectory.c_str(),&dirStat);
    if ((theDir < 0) || ((_S_IFDIR & dirStat.st_mode) == 0)) {
      // an error has occured
      setStatusString("Directory for temporal files not found");
      return false;
    }

    filepath = param.tmpDirectory;

    if (param.tmpDirectory.rfind('\\') < param.tmpDirectory.size()-1)
      filepath += "\\";

    char buf_tmp[50];

    std::string s_pid = itoa(_getpid(),buf_tmp,10);
    std::string s_instanceNumber = itoa(instanceNumber,buf_tmp,10);
    std::string s_name=fields.getPath();

    std::string::size_type pos = s_name.find_last_of("/");
    if(pos!=std::string::npos) s_name.erase(0,pos+1);

    filepath += "image_"+s_pid+"_"+s_instanceNumber+"_"+s_name;
    _lti_debug("temporary filepath: "<<filepath<<"\n");
    return true;
  }

  bool url::receiveAndWriteFile(std::ofstream& out){

    static const int bufferSize = 1024;
    char buf[bufferSize];// received bytes are written in this buffer
    int rv;              // number of actually received bytes
    int tn = 0;          // debug variable: total number of received bytes
    _lti_debug("receiving...");

    out.clear();
    while (true){
      rv=recv(winsock,buf,bufferSize ,0); //receive bytes
      if(rv==SOCKET_ERROR) {                          //check for error
        char buf[50];
        std::string status = "SOCKET_ERROR: recv(): error-code: ";
        status.append(itoa(WSAGetLastError(),buf,10));
        setStatusString(status.c_str());
        return false;
      }
      else if(rv>0){        //rv bytes received
        out.write(buf,rv);//write bytes into buffer
        tn+=rv;           //count bytes
        _lti_debug(".");
      }
      else if(rv==0){ //connection has been gracefully closed
        break;
      }
      else {return false;}
    };
    out.close();
    _lti_debug("\ntotal number of received bytes: "<<tn<<"\n");
    return true;
  }

  int url::instanceNumber=0;

  int url::getFileSize(std::string fileName) {

    HANDLE hFile;
    DWORD dwFileSize;
    LPCTSTR filePath = fileName.c_str();         //FILE PATH

    if ((hFile=CreateFile(filePath,
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          (HANDLE)NULL)) == (HANDLE)(-1) ) {
      _lti_debug("file open failed\n");
      return 0xFFFFFFFF;
    }

    dwFileSize = GetFileSize (hFile, NULL); // Get the size of the file.
    if (dwFileSize == 0xFFFFFFFF){
      _lti_debug("\nGetFileSize failed!");
      return 0xFFFFFFFF;
    }
    if (!CloseHandle(hFile)) {
      _lti_debug("could not close handle\n");
      return 0xFFFFFFFF;
    }
    return dwFileSize;
  }

#endif

}
