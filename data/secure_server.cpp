/***************************************************************************
               secure_server.cpp  --  test file for a secure rpc server

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: secure_server.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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

//#define DEBUG

#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cstdlib>
#include <iostream>
#include <cstring>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h> // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_callparse.h>

#include "crypt.cpp"

class SecureWorker
{
 public:

   SecureWorker ()
     : running(true)
     , secureDisp (0)
   {
     secureDisp.addMethod(ulxr::make_method(*this, &SecureWorker::shutdown),
                      ulxr::Signature() << ulxr::Struct(),
                      ULXR_PCHAR("secure_shutdown"),
                      ulxr::Signature(),
                      ULXR_PCHAR("Shut down secure Worker"));
   }


   bool isRunning()
   {
     return running;
   }


   ulxr::MethodResponse secureDispatcher (const ulxr::MethodCall &calldata)
   {

     // decrypt base64-coded method call
     ulxr::CppString mc_data = ulxr::Base64(calldata.getParam(0)).getString();

     ULXR_TRACE(mc_data);

     ulxr::Cpp8BitString xml_call = ulxr::getLatin1(decrypt(mc_data, ULXR_PCHAR("")));
     ULXR_TRACE(ULXR_GET_STRING(xml_call));

     // parse xml to get method call
     ulxr::MethodCallParser parser;
     bool done = false;
     if (!parser.parse(xml_call.data(), xml_call.length(), done))
     {
       return ulxr::MethodResponse (1, ulxr_i18n(ULXR_PCHAR("Error parsing encrypted call")));
     }

     // dispatch resulting call
     ulxr::CppString respxml = secureDisp.dispatchCall(parser.getMethodCall()).getXml(0);

     ULXR_TRACE(respxml);

     // return encrypted response base64-encoded
     return ulxr::MethodResponse (ulxr::Base64(encrypt(respxml, ULXR_PCHAR(""))));
   }

 private:

   ulxr::MethodResponse shutdown (const ulxr::MethodCall &/*calldata*/)
   {
     ULXR_COUT << ULXR_PCHAR("got secure signal to shut down\n");
     ulxr::MethodResponse resp;
     resp.setResult(ulxr::Boolean(running));
     running = false;
     return resp;
   }

   bool running;

   ulxr::Dispatcher secureDisp;
};


////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
  ulxr::intializeLog4J(argv[0]);

  ulxr::CppString host = ULXR_PCHAR("localhost");
  if (argc > 1)
    host = ULXR_GET_STRING(argv[1]);

  unsigned port = 32003;
  if (argc > 2)
    port = ulxr_atoi(argv[2]);

  ULXR_COUT << ULXR_PCHAR("Serving rpc requests at ") << host << ULXR_PCHAR(":") << port << std::endl;

  ulxr::CppString realm = ULXR_PCHAR("SecureRPCRealm");

  ulxr::TcpIpConnection conn (true, host, port);
  ulxr::HttpProtocol prot(&conn);

  prot.addAuthentication(ULXR_PCHAR("ali-baba"), ULXR_PCHAR("open-sesame"), realm);
  prot.addAuthentication(ULXR_PCHAR("bob"),      ULXR_PCHAR("let-me-in"),   realm);
  prot.addAuthentication(ULXR_PCHAR("sue"),      ULXR_PCHAR("let-me-out"),  realm);

  ulxr::Dispatcher server(&prot);

  try
  {
    SecureWorker worker;
    server.addMethod(ulxr::make_method(worker, &SecureWorker::secureDispatcher),
                     ulxr::Signature() << ulxr::Base64(),
                     ULXR_PCHAR("secureDispatcher"),
                     ulxr::Signature() << ulxr::Base64(),
                     ULXR_PCHAR("Provide access to secured methods"));

    while (worker.isRunning() )
            {
      ulxr::MethodCall call = server.waitForCall();
      if (prot.checkAuthentication(realm))
      {
        ulxr::MethodResponse resp = server.dispatchCall(call);
        if (!prot.isTransmitOnly())
          server.sendResponse(resp);
      }
      else
        prot.rejectAuthentication(realm);

      if (!prot.isPersistent())
        prot.close();
    }
  }
  catch(ulxr::Exception& ex)
  {
     ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()) << std::endl;
     if (prot.isOpen())
     {
       try{
         ulxr::MethodResponse resp(1, ex.why() );
         if (prot.isTransmitOnly())
           server.sendResponse(resp);
       }
       catch(...)
       {
         ULXR_COUT << ULXR_PCHAR("error within exception occured\n");
       }
       prot.close();
     }
  }
  return 0;
}
