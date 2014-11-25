/***************************************************************************
        ulxr_dispatcher.cpp  -  answer rpc requests ("rpc-server")
                             -------------------
    begin                : Sat Mar 23 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_dispatcher.cpp 1028 2007-07-25 15:11:31Z ewald-arnold $

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

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#if defined(__BORLANDC__) || defined (_MSC_VER)
#include <utility>
#endif

#include <algorithm>
#include <memory>

#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_protocol.h>
#include <ulxmlrpcpp/ulxr_callparse.h>
#include <ulxmlrpcpp/ulxr_callparse_wb.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>

namespace ulxr {


ULXR_API_IMPL0
  Dispatcher::MethodCallDescriptor::MethodCallDescriptor(const MethodCall &call)
{
  method_name = call.getMethodName();
  documentation = ULXR_PCHAR("");
  return_signature = ULXR_PCHAR("");

  signature = call.getSignature(false);

  calltype = CallNone;
  invoked = 0;
  enabled = true;
}


ULXR_API_IMPL0 Dispatcher::MethodCallDescriptor::MethodCallDescriptor(
    CallType type,
    const CppString &ret_sig,
    const CppString &name,
    const CppString &sig,
    const CppString &help)
{
  method_name = name;
  documentation = help;
  return_signature = ret_sig;
  signature = sig;
  calltype = type;
  invoked = 0;
  enabled = true;
}


ULXR_API_IMPL(unsigned long) Dispatcher::MethodCallDescriptor::getInvoked() const
{
  return invoked;
}


ULXR_API_IMPL(void) Dispatcher::MethodCallDescriptor::incInvoked() const
{
  ++invoked;
}


ULXR_API_IMPL(bool) Dispatcher::MethodCallDescriptor::isEnabled() const
{
  return enabled;
}


ULXR_API_IMPL(void) Dispatcher::MethodCallDescriptor::setEnabled(bool ena) const
{
  enabled = ena;
}


ULXR_API_IMPL(CppString)
  Dispatcher::MethodCallDescriptor::getSignature(bool with_name,
                                                 bool with_return) const
{
  ULXR_TRACE(ULXR_PCHAR("getSignature"));
  CppString s;
  CppString rs = return_signature;
  if (rs.length() == 0)
    rs = ULXR_PCHAR("void");  // emergency brake

  CppString sig = signature;
  if (sig.length() == 0)
    sig = ULXR_PCHAR("void");  // emergency brake

  if (with_return && with_name)
    s = rs + ULXR_PCHAR(" ") + method_name + ULXR_PCHAR("(") + sig + ULXR_PCHAR(")");

  else if (!with_return && with_name)
    s = method_name + ULXR_PCHAR("(") + sig + ULXR_PCHAR(")");

  else if (with_return && !with_name)
  {
    s = rs;
    if (sig.length() != 0)
      s += ULXR_PCHAR(",") + sig;
  }

  else if (!with_return && !with_name)
    s = sig;

  return s;
}


ULXR_API_IMPL(CppString) Dispatcher::MethodCallDescriptor::getMethodName() const
{
  return method_name;
}


ULXR_API_IMPL(CppString) Dispatcher::MethodCallDescriptor::getParameterSignature() const
{
  return signature;
}


ULXR_API_IMPL(CppString) Dispatcher::MethodCallDescriptor::getReturnValueSignature() const
{
  return return_signature;
}


ULXR_API_IMPL(CppString) Dispatcher::MethodCallDescriptor::getDocumentation() const
{
  return documentation;
}


ULXR_API_IMPL(Dispatcher::CallType) Dispatcher::MethodCallDescriptor::getCallType() const
{
  return calltype;
}



///////////////////////////////////////////////////////////////////////////////////////



ULXR_API_IMPL0 Dispatcher::Dispatcher (Protocol* prot, bool wbxml)
{
  wbxml_mode = wbxml;
  protocol = prot;
  setupSystemMethods();
}


void free_dynamic_method (const Dispatcher::MethodCallMap::value_type &method)
{
   if (method.first.getCallType() == Dispatcher::CallDynamic)
   {
     ULXR_TRACE(ULXR_PCHAR("Now deleting dynamic function: ") + method.first.getSignature(true, true));
     delete method.second.dynamic_function;
     const_cast<Dispatcher::MethodCallMap::value_type&>(method).second.dynamic_function = 0;
   }
}


ULXR_API_IMPL0 Dispatcher::~Dispatcher ()
{
   ULXR_TRACE(ULXR_PCHAR("~Dispatcher ()"));
   std::for_each(methodcalls.begin(), methodcalls.end(), free_dynamic_method);
   methodcalls.clear();
}


ULXR_API_IMPL(void) Dispatcher::removeMethod(const CppString &name)
{
   ULXR_TRACE(ULXR_PCHAR("removeMethod ") << name);
   MethodCallMap::iterator it;
   for(it = methodcalls.begin(); it != methodcalls.end(); ++it)
   {
     if (name == (*it).first.getMethodName())
     {
       free_dynamic_method(*it);
       methodcalls.erase(it);
     }
   }
}


ULXR_API_IMPL(void) Dispatcher::addMethod (StaticMethodCall_t func,
                                        const CppString &ret_signature,
                                        const CppString &name,
                                        const CppString &signature,
                                        const CppString &help)
{
  ULXR_TRACE(ULXR_PCHAR("addMethod(static)"));
  MethodCallDescriptor desc (CallStatic, ret_signature, name, signature, help);
  MethodCall_t mct;
  mct.static_function = func;
  addMethodDescriptor (desc, mct);
}


ULXR_API_IMPL(void) Dispatcher::addMethod (DynamicMethodCall_t func,
                                        const CppString &ret_signature,
                                        const CppString &name,
                                        const CppString &signature,
                                        const CppString &help)
{
  ULXR_TRACE(ULXR_PCHAR("addMethod(dynamic)"));
  MethodCallDescriptor desc (CallDynamic, ret_signature, name, signature, help);
  MethodCall_t mct;
  mct.dynamic_function = func;  // takes ownership
  addMethodDescriptor (desc, mct);
}


ULXR_API_IMPL(void) Dispatcher::addMethod (SystemMethodCall_t func,
                                        const CppString &ret_signature,
                                        const CppString &name,
                                        const CppString &signature,
                                        const CppString &help)
{
  ULXR_TRACE(ULXR_PCHAR("addMethod(system)"));
  MethodCallDescriptor desc (CallSystem, ret_signature, name, signature, help);
  MethodCall_t mct;
  mct.system_function = func;
  addMethodDescriptor (desc, mct);
}


ULXR_API_IMPL(void) Dispatcher::addMethod (StaticMethodCall_t func,
                                        const Signature &ret_signature,
                                        const CppString &name,
                                        const Signature &signature,
                                        const CppString &help)
{
  addMethod(func, ret_signature.getString(), name, signature.getString(), help);
}


ULXR_API_IMPL(void) Dispatcher::addMethod (DynamicMethodCall_t func,
                                        const Signature &ret_signature,
                                        const CppString &name,
                                        const Signature &signature,
                                        const CppString &help)
{
  addMethod(func, ret_signature.getString(), name, signature.getString(), help);
}


ULXR_API_IMPL(void) Dispatcher::addMethod (SystemMethodCall_t func,
                                        const Signature &ret_signature,
                                        const CppString &name,
                                        const Signature &signature,
                                        const CppString &help)
{
  addMethod(func, ret_signature.getString(), name, signature.getString(), help);
}


ULXR_API_IMPL(void)
  Dispatcher::addMethodDescriptor (const MethodCallDescriptor &desc,
                                   MethodCall_t mct)
{
  ULXR_TRACE("addMethodDescriptor " << desc.getSignature(true, false));
  if (methodcalls.find(desc) != methodcalls.end() )
    throw RuntimeException(ApplicationError,
                           ulxr_i18n(ULXR_PCHAR("Method exists already: ") + desc.getSignature(true, false)));

  methodcalls.insert(std::make_pair(desc, mct));
}


ULXR_API_IMPL(MethodCall) Dispatcher::waitForCall(int _timeout)
{
  ULXR_TRACE(ULXR_PCHAR("waitForCall"));
  if (!protocol->isOpen())
  {
    if (!protocol->accept(_timeout))
      return MethodCall();  // // @todo throw exception?
  }
  else
    protocol->resetConnection();

#ifdef ULXR_ENFORCE_NON_PERSISTENT
  protocol->setPersistent(false);
#endif

  char buffer[ULXR_RECV_BUFFER_SIZE];
  char *buff_ptr;


  std::auto_ptr<XmlParserBase> parser;
  MethodCallParserBase *cpb = 0;
  if (wbxml_mode)
  {
    ULXR_TRACE(ULXR_PCHAR("waitForCall in WBXML"));
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
    ULXR_TRACE(ULXR_PCHAR("waitForCall in XML"));
    MethodCallParser *cp = new MethodCallParser();
    cpb = cp;
#ifdef _MSC_VER
  std::auto_ptr<XmlParserBase> temp(cp);
  parser = temp;
#else
    parser.reset(cp);
#endif
  }

  bool done = false;
  long readed;
  while (!done && ((readed = protocol->readRaw(buffer, sizeof(buffer))) > 0) )
  {
    buff_ptr = buffer;
    while (readed > 0)
    {
      Protocol::State state = protocol->connectionMachine(buff_ptr, readed);
      if (state == Protocol::ConnError)
        throw ConnectionException(TransportError, ulxr_i18n(ULXR_PCHAR("network problem occured")), 500);

      else if (state == Protocol::ConnSwitchToBody)
      {
        if (!protocol->hasBytesToRead())
        {
#ifdef ULXR_SHOW_READ
          Cpp8BitString super_data(buff_ptr, readed);
          while ((readed = protocol->readRaw(buffer, sizeof(buffer))) > 0)
            super_data.append(buffer, readed);
          ULXR_DOUT_READ(ULXR_PCHAR("superdata 1 start:\n")
                         << ULXR_GET_STRING(super_data)
                         << ULXR_PCHAR("superdata 1 end:\n" ));
#endif
          throw ConnectionException(NotConformingError,
                                    ulxr_i18n(ULXR_PCHAR("Content-Length of message not available")), 411);
        }
      }

      else if (state == Protocol::ConnBody)
      {
        ULXR_DOUT_XML(ULXR_GET_STRING(std::string(buff_ptr, readed)));
        if (!parser->parse(buff_ptr, readed, done))
        {
//          ULXR_DOUT("errline: " << parser->XML_GetCurrentLineNumber());
//          ULXR_DWRITE(buff_ptr, readed);
//          ULXR_DOUT("") ;

          throw XmlException(parser->mapToFaultCode(parser->getErrorCode()),
                             ulxr_i18n(ULXR_PCHAR("Problem while parsing xml request")),
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

  ULXR_TRACE(ULXR_PCHAR("waitForCall got " << cpb->getMethodCall().getXml()));
  return cpb->getMethodCall();
}


ULXR_API_IMPL(const Dispatcher::MethodCallDescriptor) * const Dispatcher::getMethod(unsigned index)
{
  ULXR_TRACE(ULXR_PCHAR("getMethod"));
  unsigned i = 0;
  MethodCallMap::iterator it;
  for (it = methodcalls.begin(); it != methodcalls.end(); ++it, ++i)
    if (i == index)
      return &(*it).first;

 throw RuntimeException(ApplicationError,
                        ulxr_i18n(ULXR_PCHAR("Index too big for Dispatcher::getMethod()")));
}


ULXR_API_IMPL(unsigned) Dispatcher::numMethods() const
{
  ULXR_TRACE(ULXR_PCHAR("numMethods"));
  unsigned i = 0;
  MethodCallMap::const_iterator it;
  for (it = methodcalls.begin(); it != methodcalls.end(); ++it)
    ++i;
  return i;
}


ULXR_API_IMPL(bool) Dispatcher::hasMethod(const MethodCall &call) const
{
  MethodCallDescriptor desc(call);
  return methodcalls.find(desc) != methodcalls.end();
}


ULXR_API_IMPL(MethodResponse) Dispatcher::dispatchCall(const MethodCall &call) const
{
  ULXR_TRACE(ULXR_PCHAR("dispatchCall"));
  try
  {
    return dispatchCallLoc(call);
  }

  catch (Exception &ex)
  {
    return MethodResponse (ex.getFaultCode(), ex.why());
  }

  catch (std::exception &ex)
  {
    return MethodResponse (ApplicationError, ULXR_GET_STRING(ex.what()));
  }

  catch (...)
  {
    return MethodResponse (SystemError, ulxr_i18n(ULXR_PCHAR("Unknown error occured")));
  }
}


ULXR_API_IMPL(MethodResponse) Dispatcher::dispatchCallLoc(const MethodCall &call) const
{
  ULXR_TRACE(ULXR_PCHAR("dispatchCallLoc: ") << call.getMethodName());

  MethodCallDescriptor desc(call);
  MethodCallMap::const_iterator it;
  if ((it = methodcalls.find(desc)) != methodcalls.end() )
  {
    MethodCall_t mc = (*it).second;
    if (!(*it).first.isEnabled())
    {
      CppString s = ulxr_i18n(ULXR_PCHAR("method \""));
      s += desc.getSignature(true, false);
      s += ulxr_i18n(ULXR_PCHAR("\": currently unavailable."));
      return MethodResponse (MethodNotFoundError, s);
    }

    else
    {
      if ((*it).first.calltype == CallSystem)
      {
        ULXR_TRACE(ULXR_PCHAR("Now calling system function: ") + (*it).first.getSignature(true, true));
        (*it).first.incInvoked();
        return mc.system_function(call, this);
      }

      else if ((*it).first.calltype == CallStatic)
      {
        ULXR_TRACE(ULXR_PCHAR("Now calling static function: ") + (*it).first.getSignature(true, true));
        (*it).first.incInvoked();
        return mc.static_function(call);
      }

      else if ((*it).first.calltype == CallDynamic)
      {
        ULXR_TRACE(ULXR_PCHAR("Now calling dynamic function: ") + (*it).first.getSignature(true, true));
        (*it).first.incInvoked();
        return mc.dynamic_function->call(call);
      }

      else
      {
        CppString s = ulxr_i18n(ULXR_PCHAR("method \""));
        s += desc.getSignature(true, false);
        s += ulxr_i18n(ULXR_PCHAR("\": internal problem to find method."));
        return MethodResponse (MethodNotFoundError, s);
      }
    }
  }

  CppString s = ulxr_i18n(ULXR_PCHAR("method \""));
  s += desc.getSignature(true, false);
  s += ulxr_i18n(ULXR_PCHAR("\" unknown method and/or signature."));
  return MethodResponse (MethodNotFoundError, s);
}


ULXR_API_IMPL(void) Dispatcher::sendResponse(const MethodResponse &resp)
{
  ULXR_TRACE(ULXR_PCHAR("sendResponse"));
  protocol->sendRpcResponse(resp, wbxml_mode);
}


ULXR_API_IMPL(void) Dispatcher::setupSystemMethods()
{
  ULXR_TRACE(ULXR_PCHAR("setupSystemMethods"));

  addMethod(&Dispatcher::xml_pretty_print,
            ULXR_PCHAR(""), ULXR_PCHAR("ulxmlrpcpp.pretty_print"), ULXR_PCHAR("bool"),
            ulxr_i18n(ULXR_PCHAR("Enable pretty-printed xml responses.")));

  //--

  addMethod(&Dispatcher::system_listMethods,
            ULXR_PCHAR("array"), ULXR_PCHAR("system.listMethods"), ULXR_PCHAR(""),
            ulxr_i18n(ULXR_PCHAR("Lists all methods implemented by this server.")));

  addMethod( &Dispatcher::system_listMethods,
            ULXR_PCHAR("array"),ULXR_PCHAR("system.listMethods"), ULXR_PCHAR("string"),
            ulxr_i18n(ULXR_PCHAR("Lists all methods implemented by this server (overloaded).")));

  addMethod( &Dispatcher::system_methodSignature,
            ULXR_PCHAR("array"), ULXR_PCHAR("system.methodSignature"), ULXR_PCHAR("string"),
            ulxr_i18n(ULXR_PCHAR("Returns an array of possible signatures for this method.")));

  addMethod(&Dispatcher::system_methodHelp,
            ULXR_PCHAR("string"), ULXR_PCHAR("system.methodHelp"), ULXR_PCHAR("string"),
            ulxr_i18n(ULXR_PCHAR("Returns a documentation string describing the use of this method.")));

  addMethod(&Dispatcher::system_getCapabilities,
            ULXR_PCHAR("struct"), ULXR_PCHAR("system.getCapabilities"), ULXR_PCHAR(""),
            ulxr_i18n(ULXR_PCHAR("Returns Structs describing available capabilities.")));
}


ULXR_API_IMPL(MethodResponse)
   Dispatcher::xml_pretty_print(const MethodCall &calldata,
                                const Dispatcher *disp)
{
  ULXR_TRACE(ULXR_PCHAR("xml_pretty_print"));
  if (calldata.numParams() > 1)
    throw ParameterException(InvalidMethodParameterError,
                             ulxr_i18n(ULXR_PCHAR("At most 1 parameter allowed for \"system.listMethods\"")));

  if (   calldata.numParams() == 1
      && calldata.getParam(0).getType() != RpcBoolean)
    throw ParameterException(InvalidMethodParameterError,
                             ulxr_i18n(ULXR_PCHAR("Parameter 1 not of type \"Boolean\" \"ulxmlrpcpp.xml_pretty_print\"")));

  bool enable = Boolean(calldata.getParam(0)).getBoolean();
  enableXmlPrettyPrint(enable);
  return MethodResponse (Void());
}


ULXR_API_IMPL(MethodResponse)
   Dispatcher::system_listMethods(const MethodCall &calldata,
                                  const Dispatcher *disp)
{
  ULXR_TRACE(ULXR_PCHAR("system_listMethods"));
  if (calldata.numParams() > 1)
    throw ParameterException(InvalidMethodParameterError,
                             ulxr_i18n(ULXR_PCHAR("At most 1 parameter allowed for \"system.listMethods\"")));

  if (   calldata.numParams() == 1
      && calldata.getParam(0).getType() != RpcStrType)
    throw ParameterException(InvalidMethodParameterError,
                             ulxr_i18n(ULXR_PCHAR("Parameter 1 not of type \"String\" \"system.listMethods\"")));

// FIXME: what to do with param 1 if present ??

  Array arr;
  CppString m_prev;

  MethodCallMap::const_iterator it;
  for (it = disp->methodcalls.begin(); it != disp->methodcalls.end(); ++it)
    if (   m_prev != (*it).first.method_name
        && (*it).first.method_name.length() != 0)
    {
      arr.addItem(RpcString((*it).first.method_name));
      m_prev = (*it).first.method_name;
    }
  return MethodResponse (arr);
}


ULXR_API_IMPL(MethodResponse)
   Dispatcher::system_methodSignature(const MethodCall &calldata,
                                      const Dispatcher *disp)
{
  ULXR_TRACE(ULXR_PCHAR("system_methodSignature"));
  if (calldata.numParams() != 1)
    throw ParameterException(InvalidMethodParameterError,
                             ulxr_i18n(ULXR_PCHAR("Exactly 1 parameter allowed for \"system.methodSignature\"")));

  if (calldata.getParam(0).getType() != RpcStrType)
    throw ParameterException(InvalidMethodParameterError,
                             ulxr_i18n(ULXR_PCHAR("Parameter 1 not of type \"String\" \"system.listMethods\"")));

  RpcString vs = calldata.getParam(0);
  CppString name = vs.getString();
  MethodCallMap::const_iterator it;
  Array ret_arr;
  for (it = disp->methodcalls.begin(); it != disp->methodcalls.end(); ++it)
  {
    Array sigarr;
    CppString sig = (*it).first.getSignature(true, true);
    if (name == (*it).first.method_name && sig.length() != 0)
    {
      std::size_t pos;
      while ((pos = sig.find(',')) != CppString::npos)
      {
        sigarr.addItem(RpcString(sig.substr(0, pos)));
        sig.erase(0, pos+1);
      }
      sigarr.addItem(RpcString(sig));
      ret_arr.addItem(sigarr);
    }
  }

  if (ret_arr.size() == 0)
    return MethodResponse (Integer(1));  // non-Array ==< no signatures
  else
    return MethodResponse (ret_arr);
}


ULXR_API_IMPL(MethodResponse)
   Dispatcher::system_methodHelp(const MethodCall &calldata,
                                 const Dispatcher *disp)
{
  ULXR_TRACE(ULXR_PCHAR("system_methodHelp"));
  if (calldata.numParams() != 1)
    throw ParameterException(InvalidMethodParameterError,
                             ulxr_i18n(ULXR_PCHAR("Exactly 1 parameter allowed for \"system.methodHelp\"")));

  if (calldata.getParam(0).getType() != RpcStrType)
    throw ParameterException(InvalidMethodParameterError,
                             ulxr_i18n(ULXR_PCHAR("Parameter 1 not of type \"String\" \"system.listMethods\"")));

  RpcString vs = calldata.getParam(0);
  CppString name = vs.getString();
  CppString s;

  MethodCallMap::const_iterator it;
  CppString s_prev;
  for (it = disp->methodcalls.begin(); it != disp->methodcalls.end(); ++it)
    if (name == (*it).first.method_name && (*it).first.documentation.length() != 0)
    {
      if (   s_prev != (*it).first.documentation
          && (*it).first.documentation.length() != 0)
      {
        if (s.length() != 0)
          s = ULXR_PCHAR("* ") +s + ULXR_PCHAR("\n* ");
        s += (*it).first.documentation;
      }
      s_prev = (*it).first.documentation;
    }

  return MethodResponse (RpcString(s));
}


ULXR_API_IMPL(void) Dispatcher::getCapabilities (Struct &str) const
{
  // parent::getCapabilities (str);  just in case..
  str.addMember(ULXR_PCHAR("specUrl"),
               RpcString(ULXR_PCHAR("http://xmlrpc-epi.sourceforge.net/specs/rfc.fault_codes.php")));
  str.addMember(ULXR_PCHAR("specVersion"), Integer(20010516));
}


ULXR_API_IMPL(MethodResponse)
  Dispatcher::system_getCapabilities(const MethodCall &calldata,
                                     const Dispatcher *disp)
{
  if (calldata.numParams() > 1)
    throw ParameterException(InvalidMethodParameterError,
                             ulxr_i18n(ULXR_PCHAR("No parameters allowed for \"system.listMethods\"")));

  Struct sysCap;
  disp->getCapabilities(sysCap);

  Struct opStr;
  opStr.addMember(ULXR_PCHAR("faults_interop"), sysCap);
  return MethodResponse (opStr);
}


ULXR_API_IMPL(Protocol*) Dispatcher::getProtocol() const
{
  return protocol;
}


ULXR_API_IMPL(void) Dispatcher::setProtocol(Protocol *prot)
{
  protocol = prot;
}

namespace hidden {

ULXR_API_IMPL0 MethodWrapperBase::~MethodWrapperBase()
{
}

}

}  // namespace ulxr

