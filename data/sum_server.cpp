/***************************************************************************
               sum_server.cpp  --  test file for an sum rpc server

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


#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>

using namespace ulxr;

static int _stopped = 0;

static MethodResponse sumAndDifference (const MethodCall &calldata)
{
  Integer i = calldata.getParam(0);
  Integer j = calldata.getParam(1);

  Struct st;
  st.addMember(ULXR_GET_STRING("sum"), Integer(i.getInteger()+j.getInteger()));
  st.addMember(ULXR_GET_STRING("difference"), Integer(i.getInteger()-j.getInteger()));
  return MethodResponse (st);
}

static MethodResponse stopServer (const MethodCall &calldata)
{
  Integer i = calldata.getParam(0);
  Integer status(1);

  _stopped = 1;

  ULXR_COUT << ULXR_GET_STRING("!!! STOP !!!") << std::endl;

  return MethodResponse (status);
}

int main(int /*argc*/, char **/*argv*/)
{
  HttpProtocol *prot = 0;
  TcpIpConnection *conn = 0;
  Dispatcher* server = 0;

  try
  {
    conn = new TcpIpConnection(true, 0x7f000001, 32000);
    prot = new HttpProtocol (conn, ULXR_PCHAR("localhost"), 32000);
    server = new Dispatcher(prot);

    server->addMethod(&sumAndDifference,
		      Struct::getValueName(),
		      ULXR_GET_STRING("sumAndDifference"),
		      Integer::getValueName() + ULXR_GET_STRING(",") +
		      Integer::getValueName(),
		      ULXR_GET_STRING("sumAndDiffernence c-function"));

    server->addMethod(&stopServer,
		      Integer::getValueName(),
		      ULXR_GET_STRING("stopServer"),
		      Integer::getValueName(),
		      ULXR_GET_STRING("stopServer c-function"));
    int i = 0;

    while (!_stopped) {
      MethodCall call = server->waitForCall();
      MethodResponse resp = server->dispatchCall(call);
      if (!prot->isTransmitOnly())
        server->sendResponse(resp);

      if (!prot->isPersistent())
        prot->close();

      ULXR_COUT << ULXR_GET_STRING("Run #: ") << i++ << std::endl;
    }

    prot->close();
  }

  catch(ulxr::XmlException& xmlex)
  {
     ULXR_COUT << ULXR_GET_STRING("Xml Error occured: ") << xmlex.why() << std::endl;
     ULXR_COUT << ULXR_GET_STRING("  in line ") << xmlex.getErrorLine()
               << ULXR_GET_STRING(". Reason: ") << ULXR_GET_STRING(xmlex.getErrorString()) << std::endl;
     if (prot->isOpen())
     {
       try{
         MethodResponse resp(1, xmlex.why() );
         if (!prot->isTransmitOnly())
           server->sendResponse(resp);
       }
       catch(...)
       {
         ULXR_COUT << ULXR_GET_STRING("error within exception occured\n");
       }
       prot->close();
     }
  }

  catch(Exception& ex)
  {
     ULXR_COUT << ULXR_GET_STRING("Error occured: ") << ex.why() << std::endl;
     if (prot->isOpen())
     {
       try{
         MethodResponse resp(1, ex.why() );
         if (!prot->isTransmitOnly())
           server->sendResponse(resp);
       }
       catch(...)
       {
         ULXR_COUT << ULXR_GET_STRING("error within exception occured\n");
       }
       prot->close();
     }
  }
  ULXR_COUT << ULXR_GET_STRING("Terminating.\n");
}
