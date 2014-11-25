#include <iostream>
#include "TestClient.h"
#include "networkApp.h"
#ifdef _MSC_VER
#include <io.h>
#endif
#include <iostream>
#include <fstream>

#define VersionString "Simple SOAP Test Client 2.1 build 0"
/**
	Main.cpp For Simple SOAP test Client

	This is the Client driver program for the Simple SOAP Test Client.  This
	client allows you to test compatablility of a Simple SOAP Client with the
	any SOAP Server.  This Client assumes 	the test server is running on the 
	same machine with port 8080 as the server port. 

	It reads through a list of file in the file given on the command line and
	sends the XML in A SOAP envelope to the server, printing out the entire
	RPC on the stdout. It then waits for the server's response prints that and 
	proceeds to the next file.

	This Program requires the Simple SOAP server Library from Scott Seeley
	and the Simple Soap TCP Server from Jasen Plietz
*/
/*
	History:

		18-Aug-2003		GJPC	Intial Type in

	$Log: TestClientMain.cpp,v $
	Revision 1.1  2005/03/22 22:27:20  gjpc
	This is the intial check in of the Simple SOAP library.
	
	The code compiles and executes under MSVC .NET and GNU 3.3
	
	It has been run under Debian, SUSE, CYGWIN and WinXP
	
	Revision 1.3  2004/04/23 16:58:00  gjpc
	expanded the Simple SOAP package to allow RPC's within RPC's
	added Tree morphing primatives
	extended the test client to allow for programable validation
*/


char *ServerObject = NULL;

int networkAppMain( int argc, char* argv[] )
{
    TestSOAPClient testSOAP;

	std::cout << VersionString << std::endl;
    if ( argc < 2 )
    {
		std::cout	<< "usage:  media.xml ServerObjectName TestListfile <IP Address> <port>" << std::endl 
			<< "Default IP 127.0.0.1 Default Port 8080" << std::endl ;
        return 1;
    }

	if ( access( argv[2], 4 ) != 0 )
    {
		std::cout	<< "Could not access file " << argv[2] ;
        return 1;
    }

	ServerObject = argv[1];			// set the server object name
	std::string listFile = argv[2];
	std::ifstream inpList;
	inpList.open( listFile.c_str(), std::ios_base::in );
	if ( !inpList.is_open() )
	{
		std::cout << "cannot open list file: " << listFile << std::endl;
		return 1;
	}

	// setup the server address
	testSOAP.setEndPoint( argc < 4 ? std::string( "127.0.0.1" ) : std::string( argv[3] ), 
						argc < 5 ? 8080 : atoi( argv[4] ) );

	std::string listPath = listFile;
	while ( listPath.end() != listPath.begin() )
	{
		if ( ( *--listPath.end() == ':' ) ||
				( *--listPath.end() == '/' ) ||
				( *--listPath.end() == '\\')	)
			break;
		else
			listPath.erase( --listPath.end() );

		if ( ( *(listPath.end()-1) == ':' ) ||
				( *(listPath.end()-1) == '/' ) ||
				( *(listPath.end()-1) == '\\')	)
			break;
		else
			listPath.erase(listPath.end()-1);
	}
	try
	{
		std::string rpcFilename; 
		while ( !inpList.eof() )
		{
			getline( inpList, rpcFilename );   //std::cout << rpcFilename << std::endl;
			// allow empty lines and comments
			if ( rpcFilename.empty() || ( rpcFilename[0] == '#' ) )
				continue;

			// open the file with the RPC in it
			std::ifstream inpRPC;
			//rpcFilename = listPath + rpcFilename;
			if ( rpcFilename[ rpcFilename.length()-1 ] == '\r' )
				rpcFilename[ rpcFilename.length()-1 ] = 0;
			inpRPC.open( rpcFilename.c_str(), std::ios_base::in );
			if ( !inpRPC.is_open() )
			{
				std::cout << "cannot open RPC file: " << rpcFilename << std::endl;
				return 1;
			}

			// dump into a ostring
			std::ostringstream rpcCall;
			rpcCall << inpRPC.rdbuf();

			// now mark the boudaries and process the calll to the server and its repsonse
			std::cout << "\r\n\n===================== " << rpcFilename << " ============================\r\n\n";
			std::cout << "\r\n\n--------------------- " << rpcFilename << " Return ---------------------\r\n\n" 
				<< testSOAP.Generic( rpcCall.str() );

			inpRPC.close();
		}
    }
	catch ( std::exception& e )
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	std::cout << std::endl << "Normal termination of test run" << std::endl;
	return 0;
}


int main( int argc, char* argv[] )
{
	return NetworkStart( argc, argv, networkAppMain );
}
