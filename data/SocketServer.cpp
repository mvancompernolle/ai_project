/**
*	TcpServer Socket library
*
*	Copyright (c) 2000 Jasen Plietz
*	All rights reserved.
*
*	No warranties are extended. Use at your own risk.
*
*	To contact the author with suggestions or comments, use Jasen@plietz.net
*
*/


// SocketServer.cpp

#pragma warning( disable: 4786 )
#include "networkApp.h"
#include <iostream>
#include <string>

/////////////////////////////////////////////////////////////////////////////
// InitNetwork

// Initializes windows socket libraries.  This should be a part of the framework, but
// I haven't decided a good way to do it yet.

#ifdef WIN32
#include <process.h>
int initSocketLayer()
{
    int retval = 0;
	// Initialize the winsock environment
	WORD ver_request = MAKEWORD( 2, 2 );
	WSADATA wsa_data;

	// Initialize the winsock environment.
	if ( WSAStartup( ver_request, &wsa_data ) )
    {
        // Failed to startup WinSock
		return -1;
    }
	// Confirm the winsock environment supports at least version 2.2.
    long nMajor = LOBYTE( wsa_data.wVersion );
    long nMinor = HIBYTE( wsa_data.wVersion );
	if ( 2 < nMajor )
    {
        // We can't support anything with a major value under 2.  Goodbye.
        retval = -1;
    }
    else if ( 2 == nMajor )
    {
        // Make sure that the minor value is at least 2
        if ( 2 > nMinor )
        {
            retval = -1;
        }
    }
    // All other versions should work.  Let's hope that future versions don't break
    // the application.
    else
    {
        std::cerr << "The version of winsock is newer than what we expected." << std::endl;
    }
    if ( 0 < retval )
    {
		WSACleanup();
    }
    return retval;
}

void closeSocketLayer()
{
    WSACleanup();
}

#else
// cop out on all other systems

int initSocketLayer()
{
	return 0;
}
void closeSocketLayer()
{
}
#endif // OS specific socket init


/////////////////////////////////////////////////////////////////////////////
// main c++ program entry point

int NetworkStart(int argc, char* argv[], int (networkAppMain)( int argc, char* argv[] ))
{
    int retval = 0;
    retval = initSocketLayer();

    if( 0 == retval )
    {
	    // load local information
	    Host::getLocalhost();
	    IpAddress::getLocalIpAddress();
        
        retval = networkAppMain( argc, argv );

        closeSocketLayer();
    }
   
    return retval;
}
