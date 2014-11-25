/***************************************************************************
             mprocess_server.cpp  -  multi process rpc server test
                             -------------------
    begin                : Sun May 29 2005
    copyright            : (C) 2005 funtik <funt@alarit.com>

    $Id: mprocess_server.cpp 1151 2009-08-12 15:12:01Z ewald-arnold $

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

#define ULXR_USE_INTRINSIC_VALUE_TYPES

#define ULXR_STRUCT_MEMBER_FROM_NAME_VALUE

//#define 	ULXR_DEBUG_OUTPUT	0

#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <iostream>
#include <memory>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug

#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_dispatcher.h>

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include <ulxmlrpcpp/contrib/mprocess_rpc_server.h>

#include <sys/types.h>
#include <unistd.h>


funtik::MultiProcessRpcServer server;
//flag for shutdown
int sst=0;

ulxr::MethodResponse check (const ulxr::MethodCall &calldata)
{

	pid_t pid=getpid();
	std::cout<<"It is a check query. Process handle pid: "<<pid<<std::endl;

	ulxr::RpcString rpcs = calldata.getParam(0);
	ulxr::CppString s = ULXR_PCHAR("It is RPC server.")
  			ULXR_PCHAR("Your query string is: ")+rpcs.getString();
	ulxr::MethodResponse resp;
	resp.setResult(ulxr::RpcString(s));
	return resp;
}

ulxr::MethodResponse finish_server (const ulxr::MethodCall &/*calldata*/)
{
	ulxr::CppString s = ULXR_PCHAR("Shutdown server.");
	pid_t pid=getppid();
	if(kill(pid,SIGTERM))
	{
	    std::cout<<"Cannt finish server work"<<std::endl;
	}
	ulxr::MethodResponse resp;
	resp.setResult(ulxr::RpcString(s));
	return resp;
}

void sigterm_handler(int /*signal*/)
{
	sst=1;
}

int main(int argc, char **argv)
{
	ulxr::CppString host = ULXR_PCHAR("localhost");
	if (argc > 1)
		host = ULXR_GET_STRING(argv[1]);

	unsigned port = 32000;
	if (argc > 2)
    	port = ulxr_atoi(argv[2]);


	ULXR_COUT << ULXR_PCHAR("Serving ") << ULXR_PCHAR(" securing ") << ULXR_PCHAR(" rpc requests at ")
            << host << ULXR_PCHAR(":") << port << std::endl;

	try{
    	    std::auto_ptr<ulxr::TcpIpConnection> conn = std::auto_ptr<ulxr::TcpIpConnection>(new ulxr::TcpIpConnection (true, 0, port));

	    ulxr::HttpProtocol prot(conn.get());

	    server.getDispatcher()->setProtocol(&prot);
	    ulxr::Dispatcher *dsp=server.getDispatcher();

	    struct sigaction sa;
    	sigemptyset(&sa.sa_mask);
	sa.sa_handler=sigterm_handler;
	sa.sa_flags=SA_NOCLDSTOP | SA_RESTART;
	sigaction(SIGTERM,&sa,0);


	    ulxr::Struct ulxrStruct;
	    ulxrStruct.addMember(ULXR_PCHAR("first"),ulxr::Integer());
    	    dsp->addMethod(ulxr::make_method(check),
                     ulxr::Signature() << (ulxrStruct<<ulxr::make_member(ULXR_PCHAR("uptime"),ulxr::Integer()) ),
                     ULXR_PCHAR("check_sig"),
                     ulxr::Signature()<<  ulxr::RpcString(),
                     ULXR_PCHAR("Testcase return  string"));

	    dsp->addMethod(ulxr::make_method(check),
                     ulxr::Signature() << ulxr::RpcString(),
                     ULXR_PCHAR("check"),
                     ulxr::Signature() << ulxr::RpcString(),
                     ULXR_PCHAR("Testcase return  string"));

	    dsp->addMethod(ulxr::make_method(finish_server),
                     ulxr::Signature() << ulxr::RpcString(),
                     ULXR_PCHAR("finish_server"),
                     ulxr::Signature(),
                     ULXR_PCHAR("Testcase shutdown server"));

	    server.setState(funtik::MultiProcessRpcServer::RUN);
   	    while((server.getState()==funtik::MultiProcessRpcServer::RUN) && sst==0)
	    {
			server.printProcess();

			if(!server.waitConnection())
			{
		    	std::cout<<"signale receive"<<std::endl;
			    continue;
			}
			try{
				server.handleRequest();
			}
			catch(ulxr::Exception& ex)
			{
		    	ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()) << std::endl;
			}

	    }//while
	    server.terminateAllProcess(true);
	}
	catch(ulxr::Exception& ex)
	{
		ULXR_COUT << ULXR_PCHAR("Error occured: ") << ULXR_GET_STRING(ex.why()) << std::endl;
	}
	ULXR_COUT << ULXR_PCHAR("Well done, Ready.\n");
	return 0;
}
