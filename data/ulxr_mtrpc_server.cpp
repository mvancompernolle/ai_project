/***************************************************************************
           ulxr_mtrpc_server.cpp  -  a simple multithreaded rpc server
                             -------------------
    begin                : Wed Oct 10 2003
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_mtrpc_server.cpp 1076 2007-09-02 08:07:32Z ewald-arnold $

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


//#define ULXR_SHOW_TRACE
//#define ULXR_DEBUG_OUTPUT
//#define ULXR_SHOW_READ
//#define ULXR_SHOW_WRITE

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#ifdef __unix__
#include <sys/socket.h>
#endif

#ifdef ULXR_MULTITHREADED

#ifdef __WIN32__
#include <winsock2.h>
#endif

#include <ulxmlrpcpp/ulxr_mtrpc_server.h>

#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>

#ifdef __unix__
#include <csignal>
#endif

#include <iostream>

namespace ulxr {


/* Helper class for thread handling
 */
class ULXR_API_DECL0 MultiThreadRpcServer::ThreadData
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
  * @param server       pointer to the according rpc-server
  * @param connection   points to the connection data
  */
  ThreadData (MultiThreadRpcServer *server, Protocol *prot);

 /* Gets info, if thread should continue to run.
  * @return true: thread should continue
  */
  bool shouldRun() const;

 /* Signals thread to terminate
  */
  void setTerminate();

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
  Protocol *getProtocol() const;

 /* Gets the server object.
  * @return server object
  */
  MultiThreadRpcServer *getServer() const;

 /* Increments invocation counter.
  */
  void incInvoked();

 /* Returns the invocation counter.
  * @return number of processed requests
  */
  unsigned numInvoked() const;

 private:

  bool                  run;
  handle_t              handle;
  unsigned              ctrInvoked;
  Protocol             *protocol;
  MultiThreadRpcServer *server;
};


ULXR_API_IMPL0 MultiThreadRpcServer::MultiThreadRpcServer(Protocol *prot, unsigned num_threads, bool wbxml)
{
  wbxml_mode = wbxml;
  for (unsigned i = 0; i < num_threads; ++i)
#ifdef _MSC_VER
      threads.push_back(new ThreadData(this, (Protocol*)(prot->detach())));
#else
      threads.push_back(new ThreadData(this, dynamic_cast<Protocol*>(prot->detach())));
#endif
}


ULXR_API_IMPL0 MultiThreadRpcServer::~MultiThreadRpcServer()
{
  waitAsync(true);

  releaseThreads();
}


ULXR_API_IMPL(void)
MultiThreadRpcServer::addMethod (MethodAdder::StaticMethodCall_t adr,
                                 const CppString &ret_signature,
                                 const CppString &name,
                                 const CppString &signature,
                                 const CppString &help)
{
    dispatcher.addMethod(adr, ret_signature, name, signature, help);
}


ULXR_API_IMPL(void)
MultiThreadRpcServer::addMethod (MethodAdder::DynamicMethodCall_t wrapper,
                                 const CppString &ret_signature,
                                 const CppString &name,
                                 const CppString &signature,
                                 const CppString &help)
{
  dispatcher.addMethod(wrapper, ret_signature, name, signature, help);
}


ULXR_API_IMPL(void)
MultiThreadRpcServer::addMethod (MethodAdder::SystemMethodCall_t adr,
                                 const CppString &ret_signature,
                                 const CppString &name,
                                 const CppString &signature,
                                 const CppString &help)
{
  dispatcher.addMethod(adr, ret_signature, name, signature, help);
}


ULXR_API_IMPL(void)
MultiThreadRpcServer::addMethod (MethodAdder::StaticMethodCall_t adr,
                                 const Signature &ret_signature,
                                 const CppString &name,
                                 const Signature &signature,
                                 const CppString &help)
{
  dispatcher.addMethod(adr, ret_signature, name, signature, help);
}


ULXR_API_IMPL(void)
MultiThreadRpcServer::addMethod (MethodAdder::DynamicMethodCall_t wrapper,
                                 const Signature &ret_signature,
                                 const CppString &name,
                                 const Signature &signature,
                                 const CppString &help)
{
  dispatcher.addMethod(wrapper, ret_signature, name, signature, help);
}


ULXR_API_IMPL(void)
MultiThreadRpcServer::addMethod (MethodAdder::SystemMethodCall_t adr,
                                 const Signature &ret_signature,
                                 const CppString &name,
                                 const Signature &signature,
                                 const CppString &help)
{
  dispatcher.addMethod(adr, ret_signature, name, signature, help);
}


ULXR_API_IMPL(void)
MultiThreadRpcServer::removeMethod(const CppString &name)
{
  dispatcher.removeMethod(name);
}


ULXR_API_IMPL(void) MultiThreadRpcServer::enterLoop()
{
}


ULXR_API_IMPL(void) MultiThreadRpcServer::leaveLoop()
{
}


ULXR_API_IMPL(void) MultiThreadRpcServer::preProcessCall(MethodCall &/*call*/)
{
}


ULXR_API_IMPL(void) MultiThreadRpcServer::preProcessResponse(MethodResponse &/*resp*/)
{
}


ULXR_API_IMPL(void *) MultiThreadRpcServer::serverLoop(Protocol *protocol, ThreadData *td)
{
  ULXR_TRACE(ULXR_PCHAR("entered new server thread"));
  enterLoop();
  Dispatcher waiter(protocol, wbxml_mode);
  while (td->shouldRun())
  {
    try
    {
      ULXR_TRACE(ULXR_PCHAR("waitForCall()"));
      MethodCall call = waiter.waitForCall();
      preProcessCall(call);

      td->incInvoked();
      ULXR_TRACE(ULXR_PCHAR("server thread ")
                 << std::hex << (void*) td->getHandle() << std::dec
                 << ULXR_PCHAR(" received new call"));

      MethodResponse resp = dispatcher.dispatchCall(call);
      preProcessResponse(resp);

      if (!protocol->isTransmitOnly())
      {
        ULXR_TRACE(ULXR_PCHAR("NOT TransmitOnly"));
        protocol->sendRpcResponse(resp, wbxml_mode);
      }
      else
      {
        ULXR_TRACE(ULXR_PCHAR("IS TransmitOnly"));
      }

      if (!protocol->isPersistent())
        protocol->close();
    }

    catch(ConnectionException &ex)
    {
      forwardThreadedError(ex);

      if (protocol->isOpen())
      {
        try
        {
          MethodResponse resp(ex.getStatusCode(), ex.why() );
          if (!protocol->isTransmitOnly())
            protocol->sendRpcResponse(resp, wbxml_mode);
        }
        catch(...)
        {
          // nothing
        }
        protocol->close();
      }
    }

    catch(Exception& ex)
    {
      forwardThreadedError(ex);

      if (protocol->isOpen())
      {
        try
        {
          MethodResponse resp(1, ex.why() );
          if (!protocol->isTransmitOnly())
            protocol->sendRpcResponse(resp, wbxml_mode);
        }
        catch(...)
        {
          // nothing
        }
        protocol->close();
      }
    }

    catch(std::exception& ex)
    {
      forwardThreadedError(Exception(ApplicationError, ULXR_GET_STRING(ex.what())));

      if (protocol->isOpen())
      {
        try
        {
          MethodResponse resp(1, ULXR_GET_STRING(ex.what()) );
          if (!protocol->isTransmitOnly())
            protocol->sendRpcResponse(resp, wbxml_mode);
        }
        catch(...)
        {
          // nothing
        }
        protocol->close();
      }
    }

    catch(...)
    {
      RuntimeException ex (SystemError, ULXR_PCHAR("Unknown error occured"));
      forwardThreadedError(ex);

      if (protocol->isOpen())
      {
        try
        {
          MethodResponse resp(1, ex.why() );
          if (!protocol->isTransmitOnly())
            protocol->sendRpcResponse(resp, wbxml_mode);
        }
        catch(...)
        {
          // nothing
        }
        protocol->close();
      }
    }
  }

  ULXR_TRACE(ULXR_PCHAR("Leaving server thread ")
              << std::hex << (void*) td->getHandle() << std::dec);
  leaveLoop();
  return 0;
}



#ifdef ULXR_SHOW_TRACE
ULXR_API_IMPL(void) MultiThreadRpcServer::forwardThreadedError(const Exception &ex) const
#else
ULXR_API_IMPL(void) MultiThreadRpcServer::forwardThreadedError(const Exception &) const
#endif
{
   ULXR_TRACE(ULXR_CHAR("Threaded error occured: ") << ex.why());
}


ULXR_API_IMPL(void *) MultiThreadRpcServer::startThread(ThreadData *td)
{
  ULXR_TRACE(ULXR_PCHAR("startThread ")
             << std::hex << (void*) td
             << std::dec);
  return (void*) (td->getServer())->serverLoop(td->getProtocol(), td);
}


ULXR_API_IMPL(unsigned) MultiThreadRpcServer::dispatchAsync()
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
  ULXR_TRACE(ULXR_PCHAR("leaving dispatchAsync()"));
  return num_started;
}


ULXR_API_IMPL(unsigned) MultiThreadRpcServer::numThreads() const
{
  return threads.size();
}


ULXR_API_IMPL(void) MultiThreadRpcServer::terminateAllThreads(unsigned /*time*/)
{
  ULXR_TRACE(ULXR_PCHAR("Request to terminate all threads."));
  for (unsigned i1 = 0; i1 < threads.size(); ++i1)
    threads[i1]->setTerminate();
}


ULXR_API_IMPL(void) MultiThreadRpcServer::shutdownAllThreads(unsigned /*time*/)
{
  ULXR_TRACE(ULXR_PCHAR("Request to shutdown all threads."));
  for (unsigned i1 = 0; i1 < threads.size(); ++i1)
  {
    threads[i1]->setTerminate();
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
  }
}


ULXR_API_IMPL(void) MultiThreadRpcServer::waitAsync(bool term, bool stat)
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
     // maybe check (*status != 0) here
   }
   if (stat)
     printStatistics();

   releaseThreads();
}


ULXR_API_IMPL(void) MultiThreadRpcServer::releaseThreads()
{
   ULXR_TRACE(ULXR_PCHAR("releaseThreads()"));
   for (unsigned i = 0; i < threads.size(); ++i)
   {
     delete threads[i]->getProtocol();
     delete threads[i];
   }

   threads.clear();
}


ULXR_API_IMPL(void) MultiThreadRpcServer::printStatistics() const
{
   for (unsigned i = 0; i < threads.size(); ++i)
     ULXR_COUT << ULXR_PCHAR("Thread ")
               << std::dec << i
               << ULXR_PCHAR(" invoked ")
               << threads[i]->numInvoked()
               << ULXR_PCHAR(" times.\n");
}


/////////////////////////////////////////////////////////////


ULXR_API_IMPL0 MultiThreadRpcServer::ThreadData::ThreadData (MultiThreadRpcServer *serv, Protocol *prot)
{
  run = true;
  handle = 0;
  ctrInvoked = 0;
  protocol = prot;
  server = serv;
}


ULXR_API_IMPL(bool) MultiThreadRpcServer::ThreadData::shouldRun() const
{
  return run;
}


ULXR_API_IMPL(void) MultiThreadRpcServer::ThreadData::setTerminate()
{
  ULXR_TRACE(ULXR_PCHAR("Request to terminate a single thread."));

  run = false;
}


MultiThreadRpcServer::ThreadData::handle_t
 ULXR_API_IMPL0 MultiThreadRpcServer::ThreadData::getHandle() const
{
  return handle;
}


ULXR_API_IMPL(Protocol *) MultiThreadRpcServer::ThreadData::getProtocol() const
{
  return protocol;
}


ULXR_API_IMPL(MultiThreadRpcServer *) MultiThreadRpcServer::ThreadData::getServer() const
{
  return server;
}


ULXR_API_IMPL(void) MultiThreadRpcServer::ThreadData::setHandle(handle_t hd)
{
  handle = hd;
}


ULXR_API_IMPL(void) MultiThreadRpcServer::ThreadData::incInvoked()
{
  ctrInvoked++;
}


ULXR_API_IMPL(unsigned) MultiThreadRpcServer::ThreadData::numInvoked() const
{
  return ctrInvoked;
}


}  // namespace ulxr


#endif // ULXR_MULTITHREADED

