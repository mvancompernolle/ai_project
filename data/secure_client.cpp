/***************************************************************************
             secure_client.cpp  --  test file for a secure rpc client

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: secure_client.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_responseparse.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>


#include "crypt.cpp"


int main(int argc, char **argv)
{
  try
  {
    ulxr::intializeLog4J(argv[0]);

    ulxr::CppString host = ULXR_PCHAR("localhost");
    if (argc > 1)
      host = ULXR_GET_STRING(argv[1]);

    unsigned port = 32003;
    if (argc > 2)
      port = ulxr_atoi(argv[2]);

    ulxr::TcpIpConnection conn (false, host, port);
    ulxr::HttpProtocol prot(&conn);
    ulxr::Requester client(&prot);

    // prepare call
    ulxr::MethodCall secure_shutdown (ULXR_PCHAR("secure_shutdown"));

    // put call in encrypted envelope, base64 encoded  for transmission
    ulxr::MethodCall secureDispatcher (ULXR_PCHAR("secureDispatcher"));
    secureDispatcher.addParam(ulxr::Base64(encrypt(secure_shutdown.getXml(), ULXR_PCHAR(""))));
    ULXR_TRACE(secure_shutdown.getXml());

    // send call
    ulxr::MethodResponse resp = client.call(secureDispatcher, ULXR_PCHAR("/SecureRPC"), ULXR_PCHAR("ali-baba"), ULXR_PCHAR("open-sesame"));
    if (resp.isOK() )
    {
      // base64-decode and decrypt response
      ulxr::Base64 respdata = resp.getResult();
      ULXR_TRACE(respdata.getString());

      ulxr::Cpp8BitString xml_resp = ulxr::getLatin1(decrypt(respdata.getString(), ULXR_PCHAR("")));
      ULXR_TRACE(ULXR_GET_STRING(xml_resp));

      // parse response to access result
      ulxr::MethodResponseParser parser;
      bool done = false;
      if (!parser.parse(xml_resp.data(), xml_resp.length(), done))
      {
        throw ulxr::XmlException(parser.mapToFaultCode(parser.getErrorCode()),
                               ulxr_i18n(ULXR_PCHAR("Problem while parsing decrypted xml request")),
                               parser.getCurrentLineNumber(),
                               ULXR_GET_STRING(parser.getErrorString(parser.getErrorCode())));
      }

      resp = parser.getMethodResponse();

      ULXR_COUT << ULXR_PCHAR("secure call result: \n");
      ULXR_COUT << resp.getXml(0);
    }
    else
    {
      ULXR_COUT << ULXR_PCHAR("Error while transmitting secured method call\n");
      ULXR_COUT << ULXR_PCHAR("envelope call result: \n");
      ULXR_COUT << resp.getXml(0);
    }
  }
  catch(ulxr::Exception &ex)
  {
     ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()) << std::endl;
  }
  catch(...)
  {
     ULXR_COUT << ULXR_PCHAR("unknown Error occured.\n");
  }

  return 0;
}
