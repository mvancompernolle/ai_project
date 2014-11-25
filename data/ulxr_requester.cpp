/***************************************************************************
            ulxr_requester.cpp  -  send rpc request ("rpc-client")
                             -------------------
    begin                : Sat Mar 23 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_requester.cpp 983 2007-07-12 09:52:14Z ewald-arnold $

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
//#define ULXR_SHOW_XML

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_protocol.h>
#include <ulxmlrpcpp/ulxr_connection.h>
#include <ulxmlrpcpp/ulxr_responseparse.h>
#include <ulxmlrpcpp/ulxr_responseparse_wb.h>

#ifdef __WIN32__
#include <windows.h>
#include <process.h>
#endif

#ifdef ULXR_MULTITHREADED

#ifdef __unix__
#include <pthread.h>
#endif

#endif

#include <cerrno>
#include <memory>

namespace ulxr {


ULXR_API_IMPL0 Requester::Requester(Protocol* prot, bool wbxml)
{
  wbxml_mode = wbxml;
  protocol = prot;
  cntPendingRequests = 0;
}


ULXR_API_IMPL0 Requester::~Requester()
{
}


void Requester::send_call (const MethodCall &calldata,
                           const CppString &rpc_root)
{
  ULXR_TRACE(ULXR_PCHAR("send_call ") << calldata.getMethodName());
  if (!protocol->isOpen() )
    protocol->open();
  else
    protocol->resetConnection();

#ifdef ULXR_ENFORCE_NON_PERSISTENT
  protocol->setPersistent(false);
#endif

  protocol->sendRpcCall(calldata, rpc_root, wbxml_mode);
}


ULXR_API_IMPL(MethodResponse) Requester::waitForResponse()
{
  ULXR_TRACE(ULXR_PCHAR("waitForResponse"));
  return waitForResponse(protocol, wbxml_mode);
}


ULXR_API_IMPL(MethodResponse)
Requester::waitForResponse(Protocol *protocol, bool wbxml_mode)
{
  ULXR_TRACE(ULXR_PCHAR("waitForResponse(Protocol, wbxml)"));
  char buffer[ULXR_RECV_BUFFER_SIZE];
  char *buff_ptr;

  std::auto_ptr<XmlParserBase> parser;
  MethodResponseParserBase *rpb = 0;
  if (wbxml_mode)
  {
    ULXR_TRACE(ULXR_PCHAR("waitForResponse in WBXML"));
    MethodResponseParserWb *rp = new MethodResponseParserWb();
    rpb = rp;
#ifdef _MSC_VER
  std::auto_ptr<XmlParserBase> temp(rp);
  parser = temp;
#else
    parser.reset(rp);
#endif
  }
  else
  {
    ULXR_TRACE(ULXR_PCHAR("waitForResponse in XML"));
    MethodResponseParser *rp = new MethodResponseParser();
    rpb = rp;
#ifdef _MSC_VER
    std::auto_ptr<XmlParserBase> temp(rp);
    parser = temp;
#else
    parser.reset(rp);
#endif
  }

  bool done = false;
  long readed;
  while (!done && protocol->hasBytesToRead()
               && ((readed = protocol->readRaw(buffer, sizeof(buffer))) > 0) )
  {
    buff_ptr = buffer;
    while (readed > 0)
    {
      Protocol::State state = protocol->connectionMachine(buff_ptr, readed);
      if (state == Protocol::ConnError)
      {
        done = true;
        throw ConnectionException(TransportError, ulxr_i18n(ULXR_PCHAR("network problem occured")), 400);
      }

      else if (state == Protocol::ConnSwitchToBody)
      {
#ifdef ULXR_SHOW_READ
        Cpp8BitString super_data (buff_ptr, readed);
        while ((readed = protocol->readRaw(buffer, sizeof(buffer))) > 0)
          super_data.append(buffer, readed);
        ULXR_DOUT_READ(ULXR_PCHAR("superdata 3 start:\n"));

        if (wbxml_mode)
        {
           ULXR_DOUT_READ(binaryDebugOutput(super_data));
        }
        else
        {
          ULXR_DOUT_READ(ULXR_GET_STRING(super_data));
        }
        ULXR_DOUT_READ(ULXR_PCHAR("superdata 3 end:\n") );
#endif
        if (!protocol->hasBytesToRead())
        {
          throw ConnectionException(NotConformingError,
                                    ulxr_i18n(ULXR_PCHAR("Content-Length of message not available")), 411);
        }

        CppString s;
        if (!protocol->responseStatus(s))
          throw ConnectionException(TransportError, s, 500);

      }

      else if (state == Protocol::ConnBody)
      {
        ULXR_DOUT_XML(ULXR_GET_STRING(std::string(buff_ptr, readed)));
        if (!parser->parse(buff_ptr, readed, false))
        {
          throw XmlException(parser->mapToFaultCode(parser->getErrorCode()),
                             ulxr_i18n(ULXR_PCHAR("Problem while parsing xml response")),
                             parser->getCurrentLineNumber(),
                             ULXR_GET_STRING(parser->getErrorString(parser->getErrorCode())));
        }
        readed = 0;
      }
    }

    if (!protocol->hasBytesToRead())
//        || parser->isComplete())
      done = true;
  }

  if (protocol->isOpen() && !protocol->isPersistent() )
    protocol->close();

  return rpb->getMethodResponse();
}


ULXR_API_IMPL(MethodResponse)
  Requester::call (const MethodCall& calldata, const CppString &rpc_root,
                   const CppString &user, const CppString &pass)
{
   ULXR_TRACE(ULXR_PCHAR("call(..,user, pass)"));
   protocol->setMessageAuthentication(user, pass);
   send_call (calldata, rpc_root);
   return waitForResponse();
}


ULXR_API_IMPL(MethodResponse)
  Requester::call (const MethodCall& calldata, const CppString &rpc_root)
{
   ULXR_TRACE(ULXR_PCHAR("call"));
   send_call (calldata, rpc_root);
   return waitForResponse();
}


/////////////////////////////////////////////////////////////////
//


namespace hidden {


ULXR_API_IMPL0 Receiver_t::Receiver_t(StaticReceiver_t recv)
 : static_recv(recv)
 , dynamic_recv(0)
{
}


ULXR_API_IMPL0 Receiver_t::Receiver_t(DynamicReceiver_t recv)
 : static_recv(0)
 , dynamic_recv(recv)
{
}


ULXR_API_IMPL(void) Receiver_t::receive(const MethodResponse &resp)
{
  if (0 != dynamic_recv)
    dynamic_recv->receive(resp);

  else if (0 != static_recv)
    static_recv(resp);
}


ULXR_API_IMPL(void) Receiver_t::free()
{
  if (0 != dynamic_recv)
  {
    delete dynamic_recv;
    dynamic_recv = 0;
  }
}


}  // namespace hidden


/////////////////////////////////////////////////////////////////
//


struct DispatcherData
{
  DispatcherData(Protocol *p, hidden::Receiver_t r, Requester *req)
    : requester(req)
    , prot(p)
    , recv(r)

    {}

  Requester            *requester;
  Protocol             *prot;
  hidden::Receiver_t    recv;
};


/////////////////////////////////////////////////////////////////
//


#ifdef ULXR_MULTITHREADED

void *dispatchThreaded(DispatcherData *data)
{
  data->requester->incPending();
  ULXR_TRACE(ULXR_PCHAR("dispatchThreaded"));
  try
  {
    MethodResponse resp = Requester::waitForResponse(data->prot, data->requester->isWbXml());
    ULXR_TRACE(ULXR_PCHAR("  recv.receive(resp)"));
    data->recv.receive(resp);
    data->requester->decPending();
    data->recv.free();
    delete data->prot;
    delete data;
    return 0;
  }

  catch(const Exception &ex)
  {
    data->requester->forwardException(ex);
    ULXR_TRACE(ULXR_PCHAR("catch(const Exception &ex) in dispatchThreaded"));
  }

  catch(const std::exception &ex)
  {
    data->requester->forwardException(ex);
    ULXR_TRACE(ULXR_PCHAR("catch(const std::exception &ex) in dispatchThreaded"));
  }

  catch(...)
  {
    data->requester->forwardException();
    ULXR_TRACE(ULXR_PCHAR("catch(...) in dispatchThreaded"));
  }

  data->requester->decPending();
  delete data->prot;
  delete data;
  return (void*)1;   // FIXME: more error handling ??
}


#endif // ULXR_MULTITHREADED


/////////////////////////////////////////////////////////////////
//


#ifdef ULXR_MULTITHREADED

void Requester::startDispatch(const MethodCall &methcall,
                              const CppString &rpc_root,
                              hidden::Receiver_t recv)
{
    ULXR_TRACE(ULXR_PCHAR("startDispatch"));
    send_call(methcall, rpc_root);
    Protocol *prot = protocol->detach();
    DispatcherData *dd = new DispatcherData (prot, recv, this);

#ifdef __unix__
    typedef void* (*pthread_sig)(void*);
    pthread_t handle;

    bool created = false;
    while (!created)
    {
      created = (0 == pthread_create(&handle, 0, (pthread_sig)dispatchThreaded, dd));
      if (!created)
      {
        switch(protocol->getConnection()->getLastError())
        {
          case EAGAIN: // fallthrough
          case EINTR:
#ifdef __unix__
            errno = 0;
#endif
            continue;
          break;

          default:
          {
            delete prot;
            throw Exception(SystemError,
                            ulxr_i18n(ULXR_PCHAR("Could not create thread which processes rpc response.\n"))
                                            + getLastErrorString(prot->getConnection()->getLastError()));
          }
        }
      }
    }

    unsigned detached = pthread_detach(handle);
    if (detached != 0)
      throw Exception(SystemError,
                      ulxr_i18n(ULXR_PCHAR("Could not detach thread which processes rpc response")));

#elif defined(__WIN32__)
    typedef unsigned int (__stdcall *thread_sig)(void*);
    unsigned handle;

    unsigned ret = _beginthreadex( 0, 16*1024, (thread_sig)dispatchThreaded,
                                  dd, CREATE_SUSPENDED, &handle );
    int resume = ResumeThread((void*)ret);
    if (resume < 0)
      throw Exception(SystemError,
                      ulxr_i18n(ULXR_PCHAR("Could not resume thread which processes rpc response")));

    bool created = ret != 0;
    if (!created)
      throw Exception(SystemError,
                      ulxr_i18n(ULXR_PCHAR("Could not create thread which processes rpc response")));
#else
#error unsupported platform here
#endif
}

#endif // ULXR_MULTITHREADED


#ifdef ULXR_MULTITHREADED

ULXR_API_IMPL(void)
Requester::call (const MethodCall& methcall,
                 const CppString &rpc_root,
                 const CppString &user,
                 const CppString &pass,
                 hidden::Receiver_t recv)
{
   ULXR_TRACE(ULXR_PCHAR("call (.., user, pass, rcv)"));
   protocol->setMessageAuthentication(user, pass);
   startDispatch(methcall, rpc_root, recv);
}


ULXR_API_IMPL(void)
Requester::call (const MethodCall& methcall,
                 const CppString &rpc_root,
                 hidden::Receiver_t recv)
{
   ULXR_TRACE(ULXR_PCHAR("call (.., rcv)"));
   startDispatch(methcall, rpc_root, recv);
}

#endif // ULXR_MULTITHREADED

ULXR_API_IMPL(void)
Requester::transmit (const MethodCall& calldata, const CppString &rpc_root,
                     const CppString &user, const CppString &pass)
{
   ULXR_TRACE(ULXR_PCHAR("transmit (.., user, pass)"));
   protocol->setMessageAuthentication(user, pass);
   protocol->setTransmitOnly();
   send_call(calldata, rpc_root);
}


ULXR_API_IMPL(void)
Requester::transmit (const MethodCall& calldata, const CppString &rpc_root)

{
   ULXR_TRACE(ULXR_PCHAR("transmit"));
   protocol->setTransmitOnly();
   send_call(calldata, rpc_root);
}


#ifdef ULXR_MULTITHREADED

ULXR_API_IMPL(unsigned) Requester::numPendingRequests() const
{
  ULXR_TRACE(ULXR_PCHAR("numPendingRequests ") << cntPendingRequests);
  return cntPendingRequests;
}


ULXR_API_IMPL(void) Requester::incPending()
{
  ULXR_TRACE(ULXR_PCHAR("incPending ") << cntPendingRequests);
  Mutex::Locker lock (pendingMutex);
  ++cntPendingRequests;
}


ULXR_API_IMPL(void) Requester::decPending()
{
  ULXR_TRACE(ULXR_PCHAR("decPending ") << cntPendingRequests);
  Mutex::Locker lock (pendingMutex);
  if (cntPendingRequests > 0)
    --cntPendingRequests;
}


ULXR_API_IMPL(void) Requester::forwardException()
{
}


ULXR_API_IMPL(void) Requester::forwardException(const std::exception &/*ex*/)
{
}


ULXR_API_IMPL(void) Requester::forwardException(const Exception &/*ex*/)
{
}


#endif // ULXR_MULTITHREADED

ULXR_API_IMPL(bool) Requester::isWbXml() const
{
  return wbxml_mode;
}


namespace hidden {

ULXR_API_IMPL0 ReceiverWrapperBase::~ReceiverWrapperBase()
{
}

}


}  // namespace ulxr
