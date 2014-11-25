/***************************************************************************
                ulxr_http_server.cpp  -  a simple http server
                             -------------------
    begin                : Sam Apr 20 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_http_server.cpp 1083 2007-09-29 11:14:11Z ewald-arnold $

 ***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

// #define ULXR_SHOW_TRACE
// #define ULXR_DEBUG_OUTPUT
// #define ULXR_SHOW_READ
// #define ULXR_SHOW_WRITE
// #define ULXR_SHOW_XML

#ifndef ULXR_OMIT_TCP_STUFF

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <sys/stat.h>
#include <csignal>

#include <fstream>
#include <iostream>
#include <cctype>
#include <memory>

#include <ulxmlrpcpp/ulxr_http_server.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_callparse.h>
#include <ulxmlrpcpp/ulxr_callparse_wb.h>
#include <ulxmlrpcpp/ulxr_file_resource.h>

#ifdef __WIN32__
#include <process.h>
#include <Winsock2.h>
//#include <windows.h>
#endif

#ifdef __unix__
#include <sys/socket.h>
#endif

#ifdef ULXR_MULTITHREADED

#ifdef __unix__
#include <pthread.h>
#endif

#endif


namespace ulxr {


#ifdef ULXR_MULTITHREADED

/* Helper class for thread handling
 */
class HttpServer::ThreadData
{
 public:

#ifdef __unix__
  typedef pthread_t  handle_t;
#elif defined(__WIN32__)
  typedef HANDLE     handle_t;
#else
#error unsupported platform here
#endif

 /* Constucts the thread data
  * @param  server  pointer to the server object
  * @param  prot    pointer to the private connection
  */
  ThreadData (HttpServer *server, HttpProtocol *prot);

 /* Gets info, if thread should continue to run.
  * @return true: thread should continue
  */
  bool shouldRun() const;

 /* Signals thread to terminate
  */
  void requestTermination();

 /* Gets the thread handle.
  * @return handle from operation system
  */
  handle_t getHandle() const;

 /* Sets the thread handle.
  * @param  handle   handle from operation system
  */
  void setHandle(handle_t hd);

 /* Gets the connection.
  * @return connection
  */
  HttpProtocol *getProtocol() const;

 /* Increments invocation counter.
  */
  void incInvoked();

 /* Returns the invocation counter.
  * @return number of processed requests
  */
  unsigned numInvoked() const;

 /* Gets the server object.
  * @return server object
  */
  HttpServer *getServer() const;

 private:

  bool             run;
  handle_t         handle;
  unsigned         ctrInvoked;
  HttpProtocol    *protocol;
  HttpServer      *server;
};

#endif //  ULXR_MULTITHREADED


ULXR_API_IMPL0 HttpServer::HttpServer (HttpProtocol* prot, bool wbxml)
{
  init();
  wbxml_mode = wbxml;
  base_protocol = prot;
  base_protocol->setChunkedTransfer(false);
}


#ifdef ULXR_MULTITHREADED

ULXR_API_IMPL0 HttpServer::HttpServer (HttpProtocol* prot, unsigned num_threads, bool wbxml)
{
  prot->setChunkedTransfer(false);
  init();
  wbxml_mode = wbxml;
  for (unsigned i = 0; i < num_threads; ++i)
#ifdef _MSC_VER
    threads.push_back(new ThreadData(this, (HttpProtocol*)(prot->detach())));
#else
    threads.push_back(new ThreadData(this, dynamic_cast<HttpProtocol*>(prot->detach())));
#endif
  base_protocol = 0;
}

#endif // ULXR_MULTITHREADED


ULXR_API_IMPL(void) HttpServer::init()
{
  http_root_dir = ULXR_GET_STRING(ULXR_DATADIR)
                + ULXR_DIRSEP
                + ULXR_GET_STRING(ULXR_PACKAGE)
                + ULXR_DIRSEP
                + ULXR_PCHAR("http");
//  ULXR_TRACE(ULXR_PCHAR("http-root is ") << http_root_dir);
  rpc_resource_root = ULXR_PCHAR("/RPC2");
  rpc_dispatcher = 0;
  pico_shall_run = true;
  wbxml_mode = false;
}


ULXR_API_IMPL(void) HttpServer::releaseHandlers(std::vector<MethodHandler*> &handlers)
{
   for (unsigned i = 0; i < handlers.size(); ++i)
     delete handlers[i];
   handlers.clear();
}


ULXR_API_IMPL0 HttpServer::~HttpServer ()
{
#ifdef ULXR_MULTITHREADED

  waitAsync(true);

  // don't delete base_protocol!!
  releaseThreads();

#endif

  for (unsigned i = 0; i < resources.size(); ++i)
  {
    resources[i]->close();
    delete resources[i];
  }

  releaseHandlers(getHandlers);
  releaseHandlers(putHandlers);
  releaseHandlers(deleteHandlers);
  releaseHandlers(postHandlers);
}


ULXR_API_IMPL(void) HttpServer::setRpcDispatcher(Dispatcher *disp)
{
  ULXR_TRACE(ULXR_PCHAR("setRpcDispatcher"));
  rpc_dispatcher = disp;
  rpc_dispatcher->setProtocol(0); // don't use internal protocol data
}


ULXR_API_IMPL(void) HttpServer::interpreteHttpHeader(
    HttpProtocol *protocol,
    CppString &head_resource,
    CppString &head_method,
    CppString &head_version)
{
  ULXR_TRACE(ULXR_PCHAR("interpreteHttpHeader"));
  head_method = ULXR_PCHAR("");
  head_resource = ULXR_PCHAR("");
  head_version = ULXR_PCHAR("");

  CppString s = ulxr::stripWS(protocol->getFirstHeaderLine());
  std::size_t pos = s.find(' ');
  if (pos != CppString::npos)
  {
    head_method = s.substr(0, pos);
    s.erase(0, pos+1);
  }
  else
  {
    head_method = s;
    s = ULXR_PCHAR("");
  }
  makeUpper(head_method);

  s = stripWS(s);
  pos = s.find(' ');
  if (pos != CppString::npos)
  {
    head_resource = s.substr(0, pos);
    s.erase(0, pos+1);
  }
  else
  {
    head_resource = s;
    s = ULXR_PCHAR("");
  }

  s = stripWS(s);
  head_version = s;
  pos = head_version.find('/');
  if (pos != CppString::npos)
    head_version.erase(0, pos+1);

  protocol->setPersistent(!protocol->determineClosing(head_version));
}


ULXR_API_IMPL(void) HttpServer::setHttpRoot(const CppString &rt)
{
  ULXR_TRACE(ULXR_PCHAR("setHttpRoot"));
  http_root_dir = rt;
  int rl = rt.length();
  if (rl != 0 && http_root_dir[rl-1] == ULXR_DIRSEP[0])
    http_root_dir.erase(rl-1);
}


ULXR_API_IMPL(void) HttpServer::checkValidPath(const CppString &resource)
{
  ULXR_TRACE(ULXR_PCHAR("checkValidPath"));
  if (resource.find(ULXR_PCHAR("..")) != CppString::npos)
    throw ConnectionException(ApplicationError,
                        ulxr_i18n(ULXR_PCHAR("Bad request to resource")), 400);
}


ULXR_API_IMPL(CppString) HttpServer::stripResource(const CppString &in_resource)
{
  ULXR_TRACE(ULXR_PCHAR("stripResource ") << in_resource);
  CppString resource = in_resource;
  CppString s = ULXR_PCHAR("http:");
  if (resource.substr(0, s.length()) == s)
    resource.erase(0, s.length());

  s = ULXR_PCHAR("https:");
  if (resource.substr(0, s.length()) == s)
    resource.erase(0, s.length());

  s = ULXR_PCHAR("//");
  if (resource.substr(0, s.length()) == s)
    resource.erase(0, s.length());

#ifdef _MSC_VER
  size_t pos = 0;
#else
  std::size_t pos = 0;
#endif
  if ((pos = resource.find(ULXR_PCHAR("/"))) != CppString::npos)
    resource.erase(0, pos);
  else
    resource = ULXR_PCHAR("");

  return resource;
}


ULXR_API_IMPL(void)
  HttpServer::executeHttpMethod(HttpProtocol *protocol,
                                Cpp8BitString &conn_pending_data,
                                const CppString& name,
                                const CppString& in_resource)
{
   ULXR_TRACE(ULXR_PCHAR("executeHttpMethod ") << ULXR_PCHAR(" ")
              << name << ULXR_PCHAR(" ") << in_resource);
   CppString resource = stripResource(in_resource);
   checkValidPath(resource);
   ULXR_TRACE(ULXR_PCHAR("executeHttpMethod resource: ") << resource);

   if (name == ULXR_PCHAR("GET"))
   {
      for (unsigned int i = 0; i < getHandlers.size(); ++i)
        if (getHandlers[i]->handle(protocol, name, resource, conn_pending_data))
          return;

      executeHttpGET(protocol, resource);
   }

   else if (name == ULXR_PCHAR("POST"))
   {
      for (unsigned int i = 0; i < postHandlers.size(); ++i)
        if (postHandlers[i]->handle(protocol, name, resource, conn_pending_data))
          return;

      executeHttpPOST(protocol, conn_pending_data, resource);
   }

   else if (name == ULXR_PCHAR("PUT"))
   {
      for (unsigned int i = 0; i < putHandlers.size(); ++i)
        if (putHandlers[i]->handle(protocol, name, resource, conn_pending_data))
          return;

      executeHttpPUT(protocol, conn_pending_data, resource);
   }

   else if (name == ULXR_PCHAR("DELETE"))
   {
      for (unsigned int i = 0; i < deleteHandlers.size(); ++i)
        if (deleteHandlers[i]->handle(protocol, name, resource, conn_pending_data))
          return;

      executeHttpDELETE(protocol, resource);
   }

   else
     executeUnknownHttpMethod(protocol, conn_pending_data, name, resource);

   return;
}


ULXR_API_IMPL(void)
  HttpServer::executeUnknownHttpMethod(HttpProtocol * /* protocol */,
                                       Cpp8BitString & /* conn_pending_data */,
                                       const CppString& name,
                                       const CppString& /* resource */)
{
     throw ConnectionException(NotConformingError,
                         ulxr_i18n(ULXR_PCHAR("Unimplemented http method: "))+name, 501);
}


ULXR_API_IMPL(CppString) HttpServer::createLocalName(const CppString &resource)
{
  ULXR_TRACE(ULXR_PCHAR("createLocalName ")  << resource);
  ULXR_TRACE(ULXR_PCHAR("  http-root: ")  << http_root_dir);
  CppString filename = http_root_dir;
  if (resource.length() == 0 || resource[0] != ULXR_DIRSEP[0])
    filename += ULXR_DIRSEP[0];

  CppString fullname = filename + resource;

#ifndef __unix__
  std::size_t pos = fullname.find (ULXR_CHAR('/'));
  while (pos != CppString::npos)
  {
    fullname[pos] = ULXR_DIRSEP[0];
    pos = fullname.find (ULXR_CHAR('/'), pos+1);
  }
#else
  std::size_t pos = fullname.find (ULXR_CHAR('\\'));
  while (pos != CppString::npos)
  {
    fullname[pos] = ULXR_DIRSEP[0];
    pos = fullname.find (ULXR_CHAR('\\'), pos+1);
  }
#endif

  return fullname;
}


CppString /*ULXR_API_IMPL0*/
  HttpServer::guessMimeType(const CppString &name) const
{
  // simply guess from extension for now
  std::size_t pos = name.rfind('.');
  const ulxr::Char *mime = ULXR_PCHAR("");
  if (pos != CppString::npos)
  {
    CppString ext = name.substr(pos+1);
    makeLower(ext);
    if (   ext == ULXR_PCHAR("html")
        || ext == ULXR_PCHAR("htm"))
      mime = ULXR_PCHAR("text/html");

    else if (ext == ULXR_PCHAR("txt"))
      mime = ULXR_PCHAR("text/plain");

    else if (ext == ULXR_PCHAR("xml"))
      mime = ULXR_PCHAR("text/xml");

    else if (   ext == ULXR_PCHAR("jpeg")
             || ext == ULXR_PCHAR("jpg"))
      mime = ULXR_PCHAR("image/jpg");

    else if (ext == ULXR_PCHAR("png"))
      mime = ULXR_PCHAR("image/png");

    else if (ext == ULXR_PCHAR("gif"))
      mime = ULXR_PCHAR("image/gif");
  }

  return (CppString) mime;
}


ULXR_API_IMPL(void) HttpServer::addResource(CachedResource *cache)
{
  ULXR_TRACE(ULXR_PCHAR("addResource ") << (cache ? cache->getResourceName() : ULXR_PCHAR("")));
  if (0 == getResource(cache->getResourceName()))
    resources.push_back(cache);
}


ULXR_API_IMPL(CachedResource *) HttpServer::getResource(const CppString& resource)
{
  ULXR_TRACE(ULXR_PCHAR("getResource ") << resource);
  for (unsigned i = 0; i < resources.size(); ++i)
    if (resources[i]->getResourceName() == resource)
    {
      ULXR_TRACE(ULXR_PCHAR("getResource: true"));
      return resources[i];
    }

  ULXR_TRACE(ULXR_PCHAR("getResource: false"));
  return 0;
}


ULXR_API_IMPL(void) HttpServer::executeHttpGET(HttpProtocol *protocol,
                                            const CppString& in_resource)
{
  ULXR_TRACE(ULXR_PCHAR("executeHttpGET"));

  CppString filename;
  CppString resource = in_resource;
  if (resource == ULXR_PCHAR("/"))
  {
    filename = createLocalName(ULXR_DIRSEP ULXR_PCHAR("index.html")); // common assumption
    resource = ULXR_PCHAR("index.html");
  }
  else
    filename = createLocalName(resource);

  CachedResource *cache = getResource(resource);
  if (cache == 0)
  {
    cache = new FileResource(resource, filename);
    addResource(cache);
    cache->open();
  }
  else
    cache->reset();

  if (!cache->good())
    throw ConnectionException(SystemError,
                              ulxr_i18n(ULXR_PCHAR("Cannot open local input resource: "))+resource, 500);

  std::string s = cache->data();
  ULXR_TRACE(ULXR_PCHAR("executeHttpGET cachesize ") << s.length());
  protocol->sendResponseHeader(200, ULXR_PCHAR("OK"), guessMimeType(filename), s.length());
  protocol->writeBody(s.data(), s.length());
}


ULXR_API_IMPL(void) HttpServer::executeHttpRPC(HttpProtocol *protocol,
                                            Cpp8BitString &conn_pending_data)
{
  ULXR_TRACE(ULXR_PCHAR("executeHttpRPC"));

#ifdef ULXR_ENFORCE_NON_PERSISTENT
  protocol->setPersistent(false);
#endif

  if (!protocol->hasHttpProperty(ULXR_PCHAR("content-length")))
  {
#ifdef ULXR_SHOW_READ
    char buffer [ULXR_RECV_BUFFER_SIZE];
    unsigned readed;
    Cpp8BitString super_data;
    while ((readed = protocol->readRaw(buffer, sizeof(buffer))) > 0)
      super_data.append(buffer, readed);
    ULXR_DOUT_READ(ULXR_PCHAR("superdata 2 start:\n")
                   << ULXR_GET_STRING(super_data)
                   << ULXR_PCHAR("superdata 2 end:\n"));
#endif
    throw ConnectionException(NotConformingError,
                        ulxr_i18n(ULXR_PCHAR("Content-Length of message not available")), 411);
  }

  else if (  !protocol->hasHttpProperty(ULXR_PCHAR("content-type"))
           || (   (protocol->getHttpProperty(ULXR_PCHAR("content-type")).find(ULXR_PCHAR("text/xml")) == CppString::npos)
               && (protocol->getHttpProperty(ULXR_PCHAR("content-type")).find(ULXR_PCHAR("wbxml")) == CppString::npos)
              )
          )
    throw ConnectionException(NotConformingError,
                        ulxr_i18n(ULXR_PCHAR("Bad request, mime type not 'text/xml'")), 400);

  Cpp8BitString xml_data = conn_pending_data;

  char conn_buffer [ULXR_RECV_BUFFER_SIZE];
  long conn_readed;
  bool done = false;
  /** TODO: @todo readBody */
  while (!done && ((conn_readed = protocol->readRaw(conn_buffer, sizeof(conn_buffer))) > 0) )
  {
    if (!protocol->hasBytesToRead())
      done = true;
    xml_data.append(conn_buffer, conn_readed);
  }

  std::auto_ptr<XmlParserBase> parser;
  MethodCallParserBase *cpb = 0;
  if (wbxml_mode)
  {
    ULXR_TRACE(ULXR_PCHAR("waitForResponse in WBXML"));
    MethodCallParserWb *cp = new MethodCallParserWb();
    cpb = cp;
#ifdef _MSC_VER
  std::auto_ptr<XmlParserBase> temp(cp);
  parser = temp;
#else
    parser.reset(cp);
#endif
  }
  else
  {
    ULXR_TRACE(ULXR_PCHAR("waitForResponse in XML"));
    MethodCallParser *cp = new MethodCallParser();
    cpb = cp;
#ifdef _MSC_VER
  std::auto_ptr<XmlParserBase> temp(cp);
  parser = temp;
#else
    parser.reset(cp);
#endif
  }

  ULXR_DOUT_XML(ULXR_GET_STRING(std::string(xml_data.data(), xml_data.length())));
  if (!parser->parse(xml_data.data(), xml_data.length(), true))
  {
    throw XmlException(parser->mapToFaultCode(parser->getErrorCode()),
                       ulxr_i18n(ULXR_PCHAR("Problem while parsing xml request")),
                       parser->getCurrentLineNumber(),
                       ULXR_GET_STRING(parser->getErrorString(parser->getErrorCode())));
  }
  MethodCall call = cpb->getMethodCall();
  MethodResponse resp = rpc_dispatcher->dispatchCall(call);
  if (!protocol->isTransmitOnly())
    protocol->sendRpcResponse(resp, wbxml_mode);
}


ULXR_API_IMPL(void)
  HttpServer::executeHttpPOST(HttpProtocol *protocol,
                              Cpp8BitString &conn_pending_data,
                              const CppString& resource)
{
  ULXR_TRACE(ULXR_PCHAR("executeHttpPOST"));

  if (resource == rpc_resource_root && rpc_dispatcher != 0)
    executeHttpRPC(protocol, conn_pending_data);
  else
  {
    if (rpc_dispatcher == 0)
      throw ConnectionException(ApplicationError,
                          ulxr_i18n(ULXR_PCHAR("No RPC-Dispatcher set")), 500);
    else
      throw ConnectionException(ApplicationError,
                          ulxr_i18n(ULXR_PCHAR("Method POST not allowed for this resource (Bad RPC-Request to \""))
                               + rpc_resource_root + ULXR_PCHAR("\" ?): ")+resource, 501);
  }
//  if (!protocol->hasHttpProperty("content-length"))
//    throw HttpException(411, ulxr_i18n("Content-Length of message not available"));
}


ULXR_API_IMPL(void) HttpServer::executeHttpPUT(HttpProtocol *protocol,
                                            Cpp8BitString &conn_pending_data,
                                            const CppString& in_resource)
{
  ULXR_TRACE(ULXR_PCHAR("executeHttpPUT"));

  CppString filename;
  CppString resource = in_resource;
  if (resource == ULXR_PCHAR("/"))
  {
    filename = createLocalName(ULXR_DIRSEP ULXR_PCHAR("index.html")); // common assumption
    resource = ULXR_PCHAR("index.html");
  }
  else
    filename = createLocalName(resource);

  CachedResource *cache = getResource(resource);
  if (cache == 0)
  {
    cache = new FileResource(resource, filename, false);
    addResource(cache);
  }
  cache->clear();

/*
  if (!protocol->hasHttpProperty("content-length"))
    throw HttpException(NotConformingError,
                        411, ulxr_i18n("Content-Length of message not not available"));
*/
  if (!cache->good() )
    throw ConnectionException(SystemError,
                        ulxr_i18n(ULXR_PCHAR("Cannot create local resource: "))+resource, 500);

#ifdef ULXR_USE_WXSTRING
  cache->write(conn_pending_data.c_str(), conn_pending_data.length());
#else
  cache->write(conn_pending_data.data(), conn_pending_data.length());
#endif

  char conn_buffer [ULXR_RECV_BUFFER_SIZE];
  long conn_readed;
  bool done = false;
  /** TODO: @todo readBody */
  while (!done && ((conn_readed = protocol->readRaw(conn_buffer, sizeof(conn_buffer))) > 0) )
  {
    if (!protocol->hasBytesToRead())
      done = true;
    cache->write(conn_buffer, conn_readed);
  }

  if (!cache->good() )
    throw ConnectionException(SystemError,
                        ulxr_i18n(ULXR_PCHAR("Cannot write to local resource: "))+resource, 500);

  protocol->sendResponseHeader(200, ULXR_PCHAR("OK"), ULXR_PCHAR(""), 0);
  return;
}


ULXR_API_IMPL(void)
  HttpServer::executeHttpDELETE(HttpProtocol *protocol,
                                const CppString& in_resource)
{
  ULXR_TRACE(ULXR_PCHAR("executeHttpDELETE"));
  CppString filename;
  CppString resource = in_resource;
  if (resource == ULXR_PCHAR("/"))
  {
    filename = createLocalName(ULXR_DIRSEP ULXR_PCHAR("index.html")); // common assumption
    resource = ULXR_PCHAR("index.html");
  }
  else
    filename = createLocalName(resource);

  CachedResource *cache = getResource(resource);
  if (cache == 0)
  {
    cache = new FileResource(resource, filename, false);
    addResource(cache);
  }
  cache->clear();

  if (!cache->good())
    throw ConnectionException(SystemError,
                        ulxr_i18n(ULXR_PCHAR("Cannot remove local resource: "))+resource, 500);

  protocol->sendResponseHeader(200, ULXR_PCHAR("OK"), ULXR_PCHAR(""), 0);
}

/*

extern "C" void installhandler(int sig, void *context);
extern "C" void handlesigaction(int sig, siginfo_t *info, void *context);


void handlesigaction(int sig, siginfo_t *info,void *context)
{
  fprintf(stderr,"Signal %d von Proze�%d/User %d empfangen ...\n",
                 sig,info->si_pid,info->si_uid);
  switch(sig)
  {
    case SIGINT:  // fallthrough
    case SIGTERM:
//    beende=1;
    break;
  }
}


void installhandler(int sig)
{
  struct sigaction action;
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = handlesigaction;
  sigemptyset(&action.sa_mask);
  sigaction(sig,&action,NULL);
}
*/


ULXR_API_IMPL(void) HttpServer::requestTermination()
{
  pico_shall_run = false;
}


ULXR_API_IMPL(std::size_t) HttpServer::runPicoHttpd()
{
  ULXR_TRACE(ULXR_PCHAR("runPicoHttpd"));
#ifdef ULXR_MULTITHREADED
  if (threads.size() != 0)
    return dispatchAsync();
  else
#endif
    return runPicoHttpd(base_protocol, 0);
}


ULXR_API_IMPL(void) HttpServer::enterLoop()
{
}


ULXR_API_IMPL(void) HttpServer::leaveLoop()
{
}

ULXR_API_IMPL(void) HttpServer::beforeHttpTransaction()
{
}


ULXR_API_IMPL(void) HttpServer::afterHttpTransaction()
{
}


ULXR_API_IMPL(std::size_t)
  HttpServer::runPicoHttpd(HttpProtocol *protocol,
#ifdef ULXR_MULTITHREADED
                           ThreadData *td)
#else
                           ThreadData *)
#endif
{
  ULXR_TRACE(ULXR_PCHAR("runPicoHttpd(HttpProtocol) ")
             << std::hex << (void*) protocol
             << std::dec);

  enterLoop();

  pico_shall_run = true;
//  installhandler(SIGTERM);

  rpc_dispatcher->setProtocol(0);

  while (pico_shall_run
#ifdef ULXR_MULTITHREADED
         && td->shouldRun()
#endif
        )
  {
    ULXR_TRACE(ULXR_PCHAR("runPicoHttpd 0"));
    try
    {

#ifdef ULXR_MULTITHREADED
      if (td != 0)
        td->incInvoked();
#endif // ULXR_MULTITHREADED

      beforeHttpTransaction();
      ULXR_TRACE(ULXR_PCHAR("runPicoHttpd 1"));
      performHttpTransaction(protocol);
      ULXR_TRACE(ULXR_PCHAR("runPicoHttpd 2"));
      afterHttpTransaction();
    }

    catch(ConnectionException &ex)
    {
      forwardThreadedError(ex);
      try
      {
        if (protocol->isOpen())
          protocol->sendNegativeResponse(ex.getStatusCode(), ex.why());  // try to return error
        protocol->close();
      }
      catch(...)
      {
        ULXR_TRACE(ULXR_PCHAR("nested exception"));
        protocol->close();
      }
      ULXR_TRACE(ULXR_PCHAR("Transportation error occured: ") << ULXR_GET_STRING(ex.why()));
    }

    catch(Exception &ex)
    {
      forwardThreadedError(ex);

      try
      {
        if (protocol->isOpen())
          protocol->sendNegativeResponse(500, ex.why()); // try to return error
        protocol->close();
      }
      catch(...)
      {
        ULXR_TRACE(ULXR_PCHAR("nested exception"));
        protocol->close();
      }
      ULXR_TRACE(ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()));
    }

    catch(std::exception &ex)
    {
      forwardThreadedError(Exception(ApplicationError, ULXR_GET_STRING(ex.what())));

      try
      {
        if (protocol->isOpen())
          protocol->sendNegativeResponse(500, ULXR_GET_STRING(ex.what())); // try to return error
        protocol->close();
      }
      catch(...)
      {
        ULXR_TRACE(ULXR_PCHAR("nested exception"));
        protocol->close();
      }
      ULXR_TRACE(ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.what()));
    }

    catch(...)
    {
      ULXR_TRACE(ULXR_PCHAR("runPicoHttpd unknown exception"));
      RuntimeException ex (SystemError, ULXR_PCHAR("Unknown error occured"));
      forwardThreadedError(ex);

      try
      {
        if (protocol->isOpen())
          protocol->sendNegativeResponse(500, ex.why());  // try to return error
        protocol->close();
      }
      catch(...)
      {
        ULXR_TRACE(ULXR_PCHAR("runPicoHttpd nested exception"));
        protocol->close();
      }
      ULXR_TRACE(ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()));
    }
  }

  leaveLoop();
  ULXR_TRACE(ULXR_PCHAR("runPicoHttpd return 0, remaining Threads: ") << numThreads());
  return 0;
}


#ifdef ULXR_SHOW_TRACE
ULXR_API_IMPL(void) HttpServer::forwardThreadedError(const Exception &ex) const
#else
ULXR_API_IMPL(void) HttpServer::forwardThreadedError(const Exception &) const
#endif
{
   ULXR_TRACE(ULXR_CHAR("Threaded error occured: ") << ex.why());
}


ULXR_API_IMPL(void) HttpServer::performHttpTransaction(HttpProtocol *protocol)
{
  ULXR_TRACE(ULXR_PCHAR("performHttpTransaction ")
             << std::hex << (void*) protocol
             << std::dec);
  if (!protocol->isOpen())
  {
    if (!protocol->accept())
      return;
  }
  else
    protocol->resetConnection();

  CppString head_resource;
  CppString head_method;
  CppString head_version;

  ULXR_TRACE(ULXR_PCHAR("pHT 1"));
  Cpp8BitString conn_pending_data;
  char  conn_buffer [ULXR_RECV_BUFFER_SIZE];
  long  conn_readed = 0;
  char *buff_ptr;
  bool  done = false;

  ULXR_TRACE(ULXR_PCHAR("pHT 2 ") << done << ULXR_PCHAR(" ") << conn_readed);
  while (!done && ((conn_readed = protocol->readRaw(conn_buffer, sizeof(conn_buffer))) > 0) )
  {
    buff_ptr = conn_buffer;

    ULXR_TRACE(ULXR_PCHAR("pHT 3 ") << conn_readed);
    while (conn_readed > 0)
    {
      Protocol::State state = protocol->connectionMachine(buff_ptr, conn_readed);
      if (state == Protocol::ConnError)
        throw ConnectionException(TransportError,
                                  ulxr_i18n(ULXR_PCHAR("Network problem occured")), 500);

      // switch to appropriate method when header is completely read
      else if (   state == Protocol::ConnSwitchToBody
               || state == Protocol::ConnBody)
      {
        interpreteHttpHeader(protocol, head_resource, head_method, head_version);
        ULXR_TRACE(   ULXR_PCHAR("head_resource ") << head_resource << std::endl
                   << ULXR_PCHAR("head_method ") << head_method << std::endl
                   << ULXR_PCHAR("head_version ") << head_version);
        try
        {
          conn_pending_data.assign(buff_ptr, conn_readed);
          conn_readed = 0;

#ifdef ULXR_ENFORCE_NON_PERSISTENT
          protocol->setPersistent(false);
#endif

          CppString realm = getRealm(head_resource);
          if (protocol->checkAuthentication(realm))
            executeHttpMethod(protocol,
                              conn_pending_data,
                              head_method,
                              head_resource);
          else
            protocol->rejectAuthentication(realm);

          done = true;
          conn_readed = 0;
          if (protocol->isOpen() && !protocol->isPersistent())
            protocol->close();
        }

        catch(ConnectionException &ex)
        {
          if (protocol->isOpen())
            protocol->sendNegativeResponse(500, ex.why());
          protocol->close();
          throw;
        }
      }
    }
  }
}


ULXR_API_IMPL(void)
  HttpServer::addHttpHandler(const CppString &in_name, MethodHandler *handler)
{
  ULXR_TRACE(ULXR_PCHAR("addHttpHandler"));
  CppString name = in_name;
  makeUpper(name);
  if (name == ULXR_PCHAR("GET"))
     getHandlers.push_back(handler);

  else if (name == ULXR_PCHAR("POST"))
     postHandlers.push_back(handler);

  else if (name == ULXR_PCHAR("PUT"))
     putHandlers.push_back(handler);

  else if (name == ULXR_PCHAR("DELETE"))
     deleteHandlers.push_back(handler);

  else
     throw RuntimeException(ApplicationError,
                            ULXR_PCHAR("Attempt to register a handler for")
                            ULXR_PCHAR(" an unknown method"));
}


ULXR_API_IMPL(void)
  HttpServer::removeHttpHandler(const CppString &in_name,
                                MethodHandler *handler)
{
  ULXR_TRACE(ULXR_PCHAR("removeHttpHandler"));
  CppString name = in_name;
  makeUpper(name);
  std::vector<MethodHandler*> *handlers;
  if (name == ULXR_PCHAR("GET"))
     handlers = &getHandlers;

  else if (name == ULXR_PCHAR("POST"))
     handlers = &postHandlers;

  else if (name == ULXR_PCHAR("PUT"))
     handlers = &putHandlers;

  else if (name == ULXR_PCHAR("DELETE"))
     handlers = &deleteHandlers;

  else
     throw RuntimeException(ApplicationError,
                            ULXR_PCHAR("Attempt to remove a handler for")
                            ULXR_PCHAR(" an unknown method"));

  for (int i = handlers->size()-1; i >= 0; --i)
    if ((*handlers)[i] == handler)
       handlers->erase(handlers->begin()+i);
}


ULXR_API_IMPL(CppString) HttpServer::getRealm(const CppString &i_path) const
{
   ULXR_TRACE(ULXR_PCHAR("getRealm ") << i_path);

   if (i_path.length() == 0)
     return ULXR_PCHAR("");

   std::map<CppString, CppString>::const_iterator it = realmXrefs.find(i_path);
   if (it != realmXrefs.end())
     return (*it).second;  // exact match

   ULXR_TRACE(ULXR_PCHAR("getRealm 1"));
   CppString rsrc;
   CppString longest_path;
   for (it = realmXrefs.begin(); it != realmXrefs.end(); ++it)
   {
     ULXR_TRACE(ULXR_PCHAR("find ") << (*it).first << ULXR_PCHAR(" ")  << (*it).second);
     if (i_path.find((*it).first) != CppString::npos) // longest partial match from beginning
       if ((*it).first.length() > longest_path.length())
       {
         rsrc = (*it).second;
         longest_path = (*it).first;
       }
   }

   if (rsrc.length() != 0)
     return rsrc;

   ULXR_TRACE(ULXR_PCHAR("getRealm 3"));
   // second try: add trailing slash to requested resource
   CppString path2 = i_path + ULXR_PCHAR("/");
   for (it = realmXrefs.begin(); it != realmXrefs.end(); ++it)
   {
     ULXR_TRACE(ULXR_PCHAR("find ") << (*it).first << ULXR_PCHAR(" ")  << (*it).second);
     if (path2.find((*it).first) != CppString::npos) // longest partial match from beginning
       if ((*it).first.length() > longest_path.length())
       {
         rsrc = (*it).second;
         longest_path = (*it).first;
       }
   }

   if (rsrc.length() != 0)
     return rsrc;

   ULXR_TRACE(ULXR_PCHAR("getRealm 4"));
   return ULXR_PCHAR("");
}


ULXR_API_IMPL(void) HttpServer::addRealm(const CppString &path, const CppString &realm)
{
   ULXR_TRACE(ULXR_PCHAR("addRealm: ") + realm + ULXR_PCHAR(" at ") + path);
   realmXrefs[path] = realm;
}


ULXR_API_IMPL(void)
  HttpServer::addAuthentication(const CppString &user,
                                const CppString &pass,
                                const CppString &realm)
{
   ULXR_TRACE(ULXR_PCHAR("addAuthentication"));
#ifdef ULXR_MULTITHREADED
   for (unsigned i = 0; i < threads.size(); ++i)
     threads[i]->getProtocol()->addAuthentication(user, pass, realm);
#endif

   if (base_protocol != 0)
     base_protocol->addAuthentication(user, pass, realm);
}


#ifdef ULXR_MULTITHREADED

ULXR_API_IMPL(void *) HttpServer::startThread(ThreadData *td)
{
  ULXR_TRACE(ULXR_PCHAR("startThread ")
             << std::hex << (void*) td
             << std::dec);
   void *ptr = (void*) (td->getServer())->runPicoHttpd(td->getProtocol(), td);
   ULXR_TRACE(ULXR_PCHAR("/startThread "));
//   sleep(1);
   return ptr;
}


ULXR_API_IMPL(unsigned) HttpServer::dispatchAsync()
{
  ULXR_TRACE(ULXR_PCHAR("dispatchAsync()"));
  unsigned num_started = 0;

  for (unsigned i = 0; i < threads.size(); ++i)
  {
    ThreadData::handle_t tdh;
#ifdef __unix__
    typedef void* (*pthread_sig)(void*);
    int result = pthread_create(&tdh, 0, (pthread_sig)startThread, threads[i]);
    if (result == 0)
      ++num_started;
#elif defined(__WIN32__)
    unsigned tid;
    typedef unsigned int (__stdcall *thread_sig)(void*);
    tdh = (HANDLE)_beginthreadex(0, 16*1024, (thread_sig)startThread,
                                 threads[i], CREATE_SUSPENDED,
                                 &tid );
    int resume = ResumeThread(tdh);
    if (tdh >= 0 && resume >= 0)
      ++num_started;
#else
#error unsupported platform here
#endif
    threads[i]->setHandle(tdh);
  }
  ULXR_TRACE(ULXR_PCHAR("/dispatchAsync()"));
  return num_started;
}


ULXR_API_IMPL(unsigned) HttpServer::numThreads() const
{
  return threads.size();
}


ULXR_API_IMPL(void) HttpServer::terminateAllThreads(unsigned /*time*/)
{
  ULXR_TRACE(ULXR_PCHAR("Request to terminate all threads."));
  requestTermination();
  for (unsigned i1 = 0; i1 < threads.size(); ++i1)
  {
#ifdef ULXR_MULTITHREADED
    threads[i1]->requestTermination();
#endif
  }
}


ULXR_API_IMPL(void) HttpServer::shutdownAllThreads(unsigned /*time*/)
{
  ULXR_TRACE(ULXR_PCHAR("Request to shutdown all threads."));
  requestTermination();
  for (unsigned i1 = 0; i1 < threads.size(); ++i1)
  {
#ifdef ULXR_MULTITHREADED
    threads[i1]->requestTermination();
    try
    {
#ifdef __WIN32__
      threads[i1]->getProtocol()->shutdown(SD_BOTH);
#else
      threads[i1]->getProtocol()->shutdown(SHUT_RDWR);
#endif
    }
    catch(...)  // ignore expected errors
    {
    }
#endif
  }
}


ULXR_API_IMPL(void) HttpServer::waitAsync(bool term, bool stat)
{
   ULXR_TRACE(ULXR_PCHAR("waitAsync"));

   if (threads.size() == 0)
     return;

   if (term)
     terminateAllThreads(1000);

   ULXR_TRACE(ULXR_PCHAR("waitAsync: join"));
   for (unsigned i = 0; i < threads.size(); ++i)
   {
     ULXR_TRACE(ULXR_PCHAR(" join " << i));
#ifdef __unix__
     void *status;
     pthread_join(threads[i]->getHandle(), &status);
#elif defined(__WIN32__)
     WaitForSingleObject(threads[i]->getHandle(), INFINITE);
     CloseHandle(threads[i]->getHandle());
#else
#error unsupported platform here
#endif
     ULXR_TRACE(ULXR_PCHAR("waitAsync: joined ") << i);
     // maybe check (*status != 0) here
   }

   if (stat)
     printStatistics();

   releaseThreads();
   ULXR_TRACE(ULXR_PCHAR("/waitAsync"));
}


ULXR_API_IMPL(void) HttpServer::releaseThreads()
{
   ULXR_TRACE(ULXR_PCHAR("releaseThreads()"));
   for (unsigned i = 0; i < threads.size(); ++i)
   {
     delete threads[i]->getProtocol();
     delete threads[i];
   }

   threads.clear();
}


ULXR_API_IMPL(void) HttpServer::printStatistics() const
{
   for (unsigned i = 0; i < threads.size(); ++i)
     ULXR_COUT << ULXR_PCHAR("Thread ")
               << std::dec << i
               << ULXR_PCHAR(" invoked ")
               << threads[i]->numInvoked()
               << ULXR_PCHAR(" times.\n");
}


#endif // ULXR_MULTITHREADED


/////////////////////////////////////////////////////////////


#ifdef ULXR_MULTITHREADED

HttpServer::ThreadData::ThreadData (HttpServer *serv, HttpProtocol *prot)
{
  run = true;
  handle = 0;
  ctrInvoked = 0;
  protocol = prot;
  server = serv;
}


bool HttpServer::ThreadData::shouldRun() const
{
  return run;
}


void HttpServer::ThreadData::requestTermination()
{
  ULXR_TRACE(ULXR_PCHAR("Request to terminate a single thread."));
  run = false;
}


HttpServer::ThreadData::handle_t
  HttpServer::ThreadData::getHandle() const
{
  ULXR_TRACE(ULXR_PCHAR("HttpServer::ThreadData::getHandle ") << handle);
  return handle;
}


void HttpServer::ThreadData::setHandle(handle_t hd)
{
  ULXR_TRACE(ULXR_PCHAR("HttpServer::ThreadData::setHandle ") << hd);
  handle = hd;
}


HttpProtocol *HttpServer::ThreadData::getProtocol() const
{
  return protocol;
}


HttpServer *HttpServer::ThreadData::getServer() const
{
  return server;
}


void HttpServer::ThreadData::incInvoked()
{
  ctrInvoked++;
}


unsigned HttpServer::ThreadData::numInvoked() const
{
  return ctrInvoked;
}

#endif // ULXR_MULTITHREADED


/////////////////////////////////////////////////////////////////
//


ULXR_API_IMPL0 MethodHandler::MethodHandler(hidden::StaticMethodHandler handler)
 : static_handler(handler),
   dynamic_handler(0)
{
}


ULXR_API_IMPL0 MethodHandler::MethodHandler(hidden::DynamicMethodHandler handler)
 : static_handler(0),
   dynamic_handler(handler)
{
}


ULXR_API_IMPL0 MethodHandler::~MethodHandler()
{
  delete dynamic_handler;
}


ULXR_API_IMPL(bool) MethodHandler::handle(HttpProtocol *conn,
                                       const CppString &method,
                                       const CppString &resource,
                                       const Cpp8BitString &conn_data)
{
  ULXR_TRACE(ULXR_PCHAR("MethodHandler::handle"));

  if (0 != dynamic_handler)
  {
    return dynamic_handler->handle(conn, method, resource, conn_data);
  }

  else if (0 != static_handler)
  {
    return static_handler(conn, method, resource, conn_data);
  }

  return false;
}


}  // namespace ulxr


#endif // ULXR_OMIT_TCP_STUFF

