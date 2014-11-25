/***************************************************************************
                     ulxr_protocol.cpp  -  rpc prootocol
                             -------------------
    begin                : Mon May 3 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_protocol.cpp 1062 2007-08-19 09:07:58Z ewald-arnold $

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

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <ulxmlrpcpp/ulxr_connection.h>
#include <ulxmlrpcpp/ulxr_protocol.h>
#include <ulxmlrpcpp/ulxr_response.h>
#include <ulxmlrpcpp/ulxr_call.h>


namespace ulxr {


struct Protocol::AuthData
{
  AuthData(const CppString &user_, const CppString &pass_, const CppString &realm_)
  : user(user_)
  , pass(pass_)
  , realm(realm_)
  {}

  CppString  user;
  CppString  pass;
  CppString  realm;
};


struct Protocol::PImpl
{
   Connection     *connection;
   bool            delete_connection;
   State           connstate;
   long            content_length;
   long            remain_content_length;
   bool            persistent;

   std::vector<AuthData>  authdata;
};


ULXR_API_IMPL0 Protocol::Protocol(const Protocol &prot)
  : pimpl (new PImpl)
{
  *pimpl = *prot.pimpl;
}


ULXR_API_IMPL0 Protocol::Protocol(Connection *conn)
  : pimpl (new PImpl)
{
  pimpl->connection = conn;
  pimpl->delete_connection = false;
  ULXR_TRACE(ULXR_PCHAR("Protocol"));
  init();
}


ULXR_API_IMPL0 Protocol::~Protocol()
{
  ULXR_TRACE(ULXR_PCHAR("~Protocol"));
  if (pimpl->delete_connection)
    delete pimpl->connection;
  pimpl->connection = 0;
  delete pimpl;
  pimpl = 0;
}


ULXR_API_IMPL(void) Protocol::init()
{
  ULXR_TRACE(ULXR_PCHAR("init"));
  setPersistent(false);
  resetConnection();
}


ULXR_API_IMPL(void) Protocol::writeRaw(char const *buff, long len)
{
  ULXR_TRACE(ULXR_PCHAR("writeRaw"));
  getConnection()->write(buff, len);
}


ULXR_API_IMPL(long) Protocol::readRaw(char *buff, long len)
{
  ULXR_TRACE(ULXR_PCHAR("readRaw, want: ") << len);
  if (pimpl->remain_content_length >= 0)
  {
    ULXR_TRACE(ULXR_PCHAR("read 0 ") << len << ULXR_PCHAR(" ") << getRemainingContentLength());
    if (pimpl->remain_content_length < len)
      len = pimpl->remain_content_length;
  }

  long readed = getConnection()->read(buff, len);

  if (pimpl->remain_content_length >= 0)
    pimpl->remain_content_length -= readed;

  return readed;
}


ULXR_API_IMPL(void) Protocol::open()
{
  ULXR_TRACE(ULXR_PCHAR("open"));
  getConnection()->open();
  resetConnection();
}


ULXR_API_IMPL(bool) Protocol::isOpen() const
{
  const Connection *conn = getConnection();
  bool op = conn != 0 && conn->isOpen();
  ULXR_TRACE(ULXR_PCHAR("isOpen ") << op);
  return op;
}


ULXR_API_IMPL(bool) Protocol::accept(int _timeout)
{
  ULXR_TRACE(ULXR_PCHAR("accept"));
  bool res = getConnection()->accept(_timeout);
  resetConnection();
  return res;
}


ULXR_API_IMPL(void) Protocol::close()
{
  ULXR_TRACE(ULXR_PCHAR("close"));
  if (getConnection() != 0)
    getConnection()->close();
//  resetConnection();
}


ULXR_API_IMPL(Connection *) Protocol::getConnection() const
{
  ULXR_TRACE(ULXR_PCHAR("getConnection"));
  return pimpl->connection;
}


ULXR_API_IMPL(void) Protocol::setConnection(Connection *conn)
{
  ULXR_TRACE(ULXR_PCHAR("getConnection"));
  pimpl->connection = conn;
  pimpl->delete_connection = true;
  ULXR_TRACE(ULXR_PCHAR("/getConnection"));
}


ULXR_API_IMPL(Protocol::State) Protocol::getConnectionState() const
{
  return pimpl->connstate;
}


ULXR_API_IMPL(void) Protocol::setConnectionState(State state)
{
  pimpl->connstate = state;
}


ULXR_API_IMPL(void) Protocol::setRemainingContentLength(long len)
{
  pimpl->remain_content_length = len;
}


ULXR_API_IMPL(long) Protocol::getRemainingContentLength() const
{
  return pimpl->remain_content_length;
}


ULXR_API_IMPL(long) Protocol::getContentLength() const
{
  return pimpl->content_length;
}


ULXR_API_IMPL(void) Protocol::setContentLength(long len)
{
  pimpl->content_length = len;
}


ULXR_API_IMPL(Protocol::State)
  Protocol::connectionMachine(char * &/*buffer*/, long &/*len*/)
{
  ULXR_TRACE(ULXR_PCHAR("connectionMachine"));
  pimpl->connstate = ConnBody;
  return ConnBody;
}


ULXR_API_IMPL(void) Protocol::sendRpcResponse(const MethodResponse &resp, bool wbxml_mode)
{
  ULXR_TRACE(ULXR_PCHAR("sendRpcResponse"));
  if (wbxml_mode)
  {
    std::string xml = resp.getWbXml();
    getConnection()->write(xml.c_str(), xml.length());
  }
  else
  {
    CppString xml = resp.getXml(0)+ULXR_PCHAR("\n");

#ifdef ULXR_UNICODE
    Cpp8BitString utf = unicodeToUtf8(xml);
    getConnection()->write(utf.c_str(), utf.length());
#else
    getConnection()->write(xml.c_str(), xml.length());
#endif
  }
}


ULXR_API_IMPL(void) Protocol::sendRpcCall(const MethodCall &call,
                                       const CppString &/*resource*/,
                                       bool wbxml_mode)
{
  ULXR_TRACE(ULXR_PCHAR("sendRpcCall"));
  if (wbxml_mode)
  {
    std::string xml = call.getWbXml();
    getConnection()->write(xml.c_str(), xml.length());
  }
  else
  {
    CppString xml = call.getXml(0)+ULXR_PCHAR("\n");
#ifdef ULXR_UNICODE
    Cpp8BitString utf = unicodeToUtf8(xml);
    getConnection()->write(utf.c_str(), utf.length());
#else
    getConnection()->write(xml.c_str(), xml.length());
#endif
  }
}


ULXR_API_IMPL(void)
  Protocol::addAuthentication(const CppString &user,
                              const CppString &pass,
                              const CppString &realm)
{
  ULXR_TRACE(ULXR_PCHAR("addAuthentication"));
  pimpl->authdata.push_back(AuthData(stripWS(user), stripWS(pass), stripWS(realm)));
}


ULXR_API_IMPL(bool) Protocol::checkAuthentication(const CppString &realm) const
{
  ULXR_TRACE(ULXR_PCHAR("checkAuthentication ") << realm);
  if (pimpl->authdata.size() == 0)
    return true;   // accept all

  ULXR_TRACE(ULXR_PCHAR("checkAuthentication 1"));
  CppString user, pass;
  if (!getUserPass(user, pass))
    return false;

  ULXR_TRACE(ULXR_PCHAR("checkAuthentication 2 ")
             << ULXR_PCHAR("user: ") << user
             << ULXR_PCHAR(" pass: ") << pass);
  for (unsigned i = 0; i < pimpl->authdata.size(); ++i)
    if (   pimpl->authdata[i].user == user
        && pimpl->authdata[i].pass == pass
        && pimpl->authdata[i].realm == realm)
      return true;

  ULXR_TRACE(ULXR_PCHAR("checkAuthentication 3"));
  return false;
}


ULXR_API_IMPL(bool) Protocol::getUserPass(CppString & /* user */,
                                       CppString & /* pass */) const
{
  ULXR_TRACE(ULXR_PCHAR("getUserPass"));
  return false;
}


ULXR_API_IMPL(void) Protocol::rejectAuthentication(const CppString & /* realm */)
{
  ULXR_TRACE(ULXR_PCHAR("rejectAuthentication"));
}


ULXR_API_IMPL(void) Protocol::setMessageAuthentication(const CppString & /* user */,
                                                    const CppString & /* pass */)
{
  ULXR_TRACE(ULXR_PCHAR("setMessageAuthentication"));
}


ULXR_API_IMPL(void) Protocol::setTransmitOnly()
{
  ULXR_TRACE(ULXR_PCHAR("setTransmitOnly"));
}


ULXR_API_IMPL(bool) Protocol::isTransmitOnly()
{
  ULXR_TRACE(ULXR_PCHAR("isTransmitOnly"));
  return false; // always return a value
}


ULXR_API_IMPL(void) Protocol::resetConnection()
{
  ULXR_TRACE(ULXR_PCHAR("resetConnection"));
  pimpl->connstate = ConnStart;
  pimpl->remain_content_length = -1;
  pimpl->content_length = -1;
//  getConnection()->setTimeout(getConnection()->getDefaultTimeout());
}


ULXR_API_IMPL(void) Protocol::setPersistent(bool pers)
{
  ULXR_TRACE(ULXR_PCHAR("setPersistent ") << pers);
  pimpl->persistent = pers;

  Connection *conn = getConnection();
  if (pers)
    conn->setTimeout(conn->getPersistentTimeout());
  else
    conn->setTimeout(conn->getDefaultTimeout());
}


ULXR_API_IMPL(bool) Protocol::isPersistent() const
{
  return pimpl->persistent;
}


}  // namespace ulxr
