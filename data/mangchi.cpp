/************************************************************************
 * mangchi, the TCP relayer
 * made by Hojin Choi <pynoos@users.sourceforge.net>
 * 2002. 10.
 ************************************************************************/

#include "hsocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <iostream.h>

#if !defined(WIN32)
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#ifdef HAVE_LINUX_SOCKIOS_H
#include <linux/sockios.h>
#endif

const char * localip = "0.0.0.0";
const int M_CLIENT = 1;
const int M_SERVER = 2;
const int M_REVEAL = 3;
const int BUFFERSIZE = 1024;

/* --- service variable --- */
int mode = 0;
int uid=0;
char ip1[BUFFERSIZE];
char ip2[BUFFERSIZE];
char bindip[BUFFERSIZE];
int port1;
int port2;
int debug = 0;
char ARGV0[BUFFERSIZE];
bool bDaemon = false;
char env1[BUFFERSIZE];
char env2[BUFFERSIZE];
char env3[BUFFERSIZE];
char env4[BUFFERSIZE];
/* --- service variable end --- */


void sig_handle( int sig )
{
	int status;
	int en = errno;
	while( waitpid( -1, & status, WNOHANG ) > 0 )
	{
		//null body.
	}
	errno = en;
}

void PERROR( const char * msg )
{
	fprintf( stderr, "%-5ld ", getpid() );
	//if( debug > 1 )
	//{
		perror( msg );
	//}
}

void HEXDump( const char * src, int len )
{
	static char text[32768];
	int textlength = 0;

	static char line[8+3*16+16+1]; //pid, hex, charaters, null
	const char * p = src;

	if( debug != 3 )
	{
		return;
	}

	while( p < src+len )
	{
		char * l = line + 8; // after pid.
		const char * r = p;
		memset( line, ' ', sizeof line );
		line[sprintf( line, "[%-5ld]", getpid() )] = ' '; // erase null character.

		while( p < src+len && p < r+16 )
		{
			l += sprintf( l, "%02X ", (unsigned char) *p );
			p++;
		}

		l = line + 8 + 3*16; // after hex.
		const char * q = r;
		while( q < src+len && q < r+16 )
		{
			if( !iscntrl(*q) )
			{
				*l = *q;
			}
			else
			{
				*l = '.';
			}
			l++;
			q++;
		}
		line[8 + 8*16+2] = '-'; // at the middle of hex
		line[sizeof line - 1] = 0;
		textlength += sprintf( text+textlength, "%s\n", line );
		if( textlength > 20000 )
		{
			textlength += sprintf( text+textlength, "\n" );
			cout << text;
			cout.flush();
			textlength = 0;
		}
	}
	textlength += sprintf( text+textlength, "\n" );
	cout << text;
	cout.flush();
}

const char * myip()
{
	const int MAX_NIC = 10;
	struct ifconf ifc;
	struct ifreq ifr[MAX_NIC];

	int s;
	int nNumIFs;
	int i;
	int count;
	int max=2;
	static char ip[BUFFERSIZE];

	max++;

	ifc.ifc_len = sizeof ifr;
	ifc.ifc_ifcu.ifcu_req = ifr;

	if( (s=socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		PERROR("socket"); 
		exit(1);
	}

	int cmd = SIOCGIFCONF;
#if defined(_AIX)
	cmd = CSIOCGIFCONF;
#endif

	if( (s = ioctl(s, cmd, &ifc) ) < 0)
	{
		PERROR("ioctl");
		exit(1);
	}

	nNumIFs = ifc.ifc_len / sizeof ( struct ifreq );
	count = 0;
	strcpy( ip, localip );
	for( i=0; i<nNumIFs; i++ )
	{
		struct in_addr addr;
		if( ifc.ifc_ifcu.ifcu_req[i].ifr_ifru.ifru_addr.sa_family != AF_INET)
		{
			continue;
		}

		addr = ((struct sockaddr_in *) & ifc.ifc_ifcu.ifcu_req[i].ifr_ifru.ifru_addr)->sin_addr;
		if( addr.s_addr == htonl( 0x7f000001 ) )
		{
			continue;
		}
		strcpy( ip, inet_ntoa( addr ) );
		break;
	}
	return ip;
}


void readonly( HSocket * pSock )
{
	char buf[32768];
	pSock-> SetRecvBufferSize( 32768 );
	pSock-> SetSendBufferSize( 32768 );

	int size = -1;
	int s=0;

	char pair[BUFFERSIZE];
	if( debug )
	{
		s += sprintf( pair+s, "[%-5ld] %s:%ld",
			getpid(),
			pSock->GetRemoteIP(), pSock->GetRemotePort() );
		s += sprintf( pair+s, " - %s:%ld ", 
			pSock->GetLocalIP(),  pSock->GetLocalPort() );
		cout << pair << "CONNECTED" << endl;
		cout.flush();
	}

	while(1) 
	{
		int index = HSocket::Select( 1, & pSock );
		switch( index ) {
		case 0:
			size = pSock->Receive( buf, 32767 );
			if( size <=0 )
			{
				return;
			}
			buf[size] = 0;
			switch( debug )
			{
			case 1:
				cout << pair << " > " << size << endl;
				cout.flush();
				break;
			case 2:
				cout << buf;
				cout.flush();
				break;
			case 3:
				cout << pair << " > " << size << endl;
				cout.flush();
				HEXDump( buf, size );
				break;
			}
		}
	}
}

void relay( HSocket * pSock, HSocket * pSock2 )
{
	HSocket *w[2];
	char buf[32768];
	int size = -1, size2 = -1;
	int s = 0;
	w[0] = pSock;
	w[1] = pSock2;

	pSock-> SetRecvBufferSize( 32768 );
	pSock-> SetSendBufferSize( 32768 );
	pSock2->SetRecvBufferSize( 32768 );
	pSock2->SetSendBufferSize( 32768 );

	char pair[BUFFERSIZE] = "";
	if( debug )
	{
		s += sprintf( pair+s, "[%-5ld] %s:%ld",
			getpid(),
			pSock-> GetRemoteIP(),pSock-> GetRemotePort() );
		s += sprintf( pair+s, " - %s:%ld ", 
			pSock-> GetLocalIP(), pSock-> GetLocalPort() );

		s += sprintf( pair+s, "~ %s:%ld",
			pSock2->GetLocalIP(), pSock2->GetLocalPort() );
		s += sprintf( pair+s, " - %s:%ld ",
			pSock2->GetRemoteIP(),pSock2->GetRemotePort() );

		cout << pair << "CONNECTED" << endl;
	}

	while(1) 
	{
		int index = HSocket::Select( 2, w );
		switch( index ) {
		case 0:
			size = pSock->Receive( buf, 32767 );
			if( size <=0 )
			{
				goto exit;
			}
			buf[size] = 0;
			switch( debug )
			{
			case 1:
				cout << pair << " > " << size << endl;
				cout.flush();
				break;
			case 2:
				cout << buf;
				cout.flush();
				break;
			case 3:
				cout << pair << " > " << size << endl;
				cout.flush();
				HEXDump( buf, size );
				break;
			}
			size2 = 0;
			while( size2 < size )
			{
				if( (s = pSock2->Send( buf+size2, size-size2 )) <= 0 )
					goto exit;
				size2 += s;
			}
			buf[size2] = 0;
			break;
		case 1:
			size = pSock2->Receive( buf, 32768 );
			if( size <=0 )
				goto exit;
			buf[size] = 0;
			switch( debug )
			{
			case 1:
				cout << pair << " < " << size << endl;
				cout.flush();
				break;
			case 2:
				cout << buf;
				cout.flush();
				break;
			case 3:
				cout << pair << " < " << size << endl;
				cout.flush();
				HEXDump( buf, size );
				break;
			}

			size2 = 0;
			while( size2 < size )
			{
				if( (s = pSock->Send( buf+size2, size-size2 )) <= 0 )
					goto exit;
				size2 += s;
			}
			buf[size2] = 0;
			break;
		}
	}
exit:
	if( size < 0 ) 
	{
		PERROR("Relay");
	}
	pSock->Close();
	pSock2->Close();
	return;
}

void client_client()
{
	if( debug )
	{
		printf("client:client mode (%s -> %s:%d) : (%s -> %s:%d)\n", bindip, ip1,port1, bindip, ip2, port2 );
	}
	HSocket clientSock, clientSock2;
	setuid( uid );
	if( clientSock.Create(0, SOCK_STREAM, bindip ) 
		&& clientSock2.Create(0, SOCK_STREAM, bindip ) )
	{
		if( clientSock.Connect( ip1, port1 ) && clientSock2.Connect( ip2, port2) )
		{
			relay( & clientSock, & clientSock2 );
			return;
		};
		PERROR("Connection");
		return;
	}
	PERROR("Creation");
	return;
}

void client_server()
{
	if( debug )
	{
		printf("client:server mode (%s -> %s:%d) : (%s:%d listen)\n", bindip, ip1,port1, ip2, port2 );
	}
	HSocket clientSock, serverSock;
	if( clientSock.Create(0, SOCK_STREAM, bindip ) 
		&& serverSock.Create(port2, SOCK_STREAM, ip2 ) )
	{
		setuid( uid );
		if( serverSock.Listen() )
		{
			HSocket workSock;
			if( clientSock.Connect( ip1, port1 ) )
			{
				if( serverSock.Accept( workSock ) )
				{
					relay( & clientSock, & workSock );
					return;
				}
				PERROR("Accept");
				return;
			}
			PERROR("Connection");
			return;
		}
		PERROR("Listen");
		return;
	}
	PERROR("Creation");
	return;
	
}

void server_client()
{
	HSocket clientSock, serverSock;
	if( debug )
	{
		printf("server:client mode (%s:%d listen) : (%s -> %s:%d)\n", ip1,port1, bindip, ip2, port2 );
	}

	if( !port1 )
	{
		printf("Server port not specified.\n");
		exit(0);
	}

	if( !serverSock.Create(port1, SOCK_STREAM, ip1 ) )
	{
		PERROR("Server Creation");
		return;
	}

	setuid( uid );

	if( !serverSock.Listen() )
	{
		PERROR("Listen");
		return;
	}

	HSocket workSock;
server_client_again:
	while( serverSock.Accept( workSock ) )
	{
		if( fork() == 0 )
		{
			serverSock.Close();
			if( clientSock.Create(0, SOCK_STREAM, bindip) )
			{
				if( clientSock.Connect( ip2, port2 ) )
				{
					relay( & workSock, & clientSock );
				}
				else
				{
					PERROR("Connection");
				}
				workSock.Close();
				exit(0);
			}
			PERROR("Client Creation");
			exit(0);
		}
		workSock.Close();
	}
	if( errno == EINTR )
	{
		goto server_client_again;
	}
	PERROR("Accept");
	return;
}

void server_server()
{
	HSocket serverSock, serverSock2;
	if( debug )
	{
		printf("server:server mode (%s:%d listen) : (%s:%d listen)\n", ip1,port1, ip2, port2 );
	}
	if( !port1 || !port2 )
	{
		printf("Two ports required.\n");
		exit(0);
	}
	if( serverSock.Create(port1, SOCK_STREAM, ip1 )
		&& serverSock2.Create(port2, SOCK_STREAM, ip2 ) )
	{
		setuid( uid );
		if( serverSock.Listen() && serverSock2.Listen() )
		{
			HSocket workSock, workSock2;
server_server_1:
			while( serverSock.Accept( workSock ) )
			{
server_server_2:
				if( serverSock2.Accept( workSock2 ) )
				{
					if( fork() == 0 )
					{
						serverSock.Close();
						serverSock2.Close();
						relay( & workSock, & workSock2 );
						exit(0);
					}
					workSock.Close();
					workSock2.Close();
				}
				else
				{
					if( errno == EINTR )
						goto server_server_2;
					PERROR( "Accept" );
				}
			}
			if( errno == EINTR )
				goto server_server_1;
			PERROR( "Accept");
			return;
		}
		PERROR("Listen");
		return;
	}
	PERROR("Creation");
	return;
}

void server_only()
{
	HSocket serverSock;
	if( serverSock.Create(port1, SOCK_STREAM, ip1 ) )
	{
		if( serverSock.Listen() )
		{
			HSocket workSock;
			while( serverSock.Accept( workSock ) )
			{
				if( fork() == 0 )
				{
					readonly( & workSock );
					exit(0);
				}
				workSock.Close();
			}
			PERROR("Accept");
			return;
		}
		PERROR("Listen");
		return;
	}
	PERROR("Server Creation");
	return;
}

void help()
{
	cout << 
		"Usage: mangchi <options> <Address 1> [<Address 2>]\n"
		"options:\n"
		"       -cc, -cs, -ss, -sc, -s, -x : see below examples\n"
		"       -b <bind_ip>: in mode -cc, -cs and -sc, bind client connecting socket to the <bind_ip>.\n"
		"       -d : log briefly.\n"
		"       -D : log all.\n"
		"       -h : hex log all.\n"
		"       -a : deamonize.\n"
		"       -A <description>: deamonize with ps description.\n"
		"       -u <uid>: change user id as <uid> after socket binding.\n"
		"\n"
		"Example: mangchi -cc[d|D|h] <remote_ip 1>:<remote_port 1> <remote_ip 2>:<remote_port 2>\n"
		"         mangchi -cs[d|D|h] <remote_ip>:<remote_port> [<local_ip>:]<local_port>\n"
		"         mangchi -sc[d|D|h] [<local_ip>:]<local_port> <remote_ip>:<remote_port>\n"
		"         mangchi -ss[d|D|h] [<local_ip>:]<local_port> [<local_ip>:]<local_port>\n"
		"         mangchi -s[d|D|h]  [<local_ip>:]<local_port>\n"
		"         mangchi -x[d|D|h]  <local_port>\n\n" 
		"\n"
		"Written by Hojin Choi <pynoos@users.sourceforge.net>\n"
		"----------------------------------------------------------------------------------------------\n"
		"Description\n"
		"       mangchi -ccd 10.1.1.103:19 10.1.1.103:7\n"
		"          connect to 10.1.1.103:19 and connect to 10.1.1.103:7 and bond them.\n\n"
		"       mangchi -scd 8000 www.abc.com:80\n"
		"          listen at 8000 and accept connection and relay to www.abc.com:80\n"
		"          Every connection is forwarded to remote_ip:remote_port\n\n"
		"       mangchi -ssd 7000 8000\n"
		"          listen at 7000, 8000.\n"
		"          Both 7000 port connection and 8000 port connection can talk with each other.\n\n"
		"       mangchi -csd chollian.net:23 2300\n"
		"          Connect to chollian.net:23 and listen 2300 and accept a connection to relay.\n"
		"          This is for just one time connection.(comparing to sc option)\n\n"
		"       mangchi -sd 8000\n"
		"          All connection to 8000 port data are logged at console.\n\n"
		"       mangchi -x 8080\n"
		"          If you have IP 192.168.100.1 for e.g., then it works as below.\n"
		"             mangchi -sc 192.168.100.1:8080 127.0.0.1:8080\n"
		"          This is helpful ssh port forwarding to available world. (see ssh documents)\n"
		;
	return;
}

void parse( const char * ip_port, char * ip, int * port )
{
	char buf[BUFFERSIZE];
	char * p = 0;
	if( ip_port == 0 ) { return; }
	p=strchr( ip_port, ':');
	if( p == 0 )
	{
		strncpy( ip, localip, BUFFERSIZE );
		if( port ) *port = atoi( ip_port );
		return;
	}
	strncpy( ip, ip_port, BUFFERSIZE );
	ip[ p-ip_port ] = 0;
	if( port ) { *port = atoi( p+1 ); }
	return;
}

void register_signal_handler()
{
	struct sigaction sa;
	sa.sa_handler = sig_handle;
	sa.sa_flags   = SA_NOCLDSTOP;
	sigaction(SIGCHLD, & sa, 0 );
}

bool demonize( int argc, char * argv[] )
{
	sprintf(env1, "MANGCHI_MODE=%d", mode );
	putenv( env1 );
	sprintf(env2, "MANGCHI_ADDR1=%s:%d", ip1,port1 );
	putenv( env2 );
	sprintf(env3, "MANGCHI_ADDR2=%s:%d", ip2,port2 );
	putenv( env3 );
	sprintf(env4, "MANGCHI_UID=%d", uid );
	putenv( env4 );
	/*
	if( !getenv("MANGCHI") )
	{
		argv++;
		while( *argv )
		{
			strncat( temp, *argv, sizeof temp - strlen(temp) );
			strcat( temp, " " );
		}
	}
	*/
	char * tempArgv[2] = {0,0};
	const char * file = argv[0];
	switch( fork() )
	{
	case 0: 
		tempArgv[0] = ARGV0;
		execvp( file, tempArgv );
	case -1:
		PERROR("fork");
		break;
	default:
		return true;
	}
	return false;
}

void setup_fromenv()
{
	char temp[BUFFERSIZE];

	if( !getenv("MANGCHI_MODE") || !getenv("MANGCHI_ADDR1") || !getenv("MANGCHI_ADDR2") )
	{
		return;
	}
	mode = atoi( getenv("MANGCHI_MODE") );
	uid  = atoi( getenv("MANGCHI_UID") );
	parse( getenv("MANGCHI_ADDR1"), ip1, & port1 );
	parse( getenv("MANGCHI_ADDR2"), ip2, & port2 );
}

void setup( int argc, char * argv[] )
{
	strcpy( bindip, localip );

	char temp[BUFFERSIZE];
	strncpy( ARGV0, argv[0], sizeof ARGV0 );

	while(1)
	{
		int sw = getopt( argc, argv, "csxdDhA:ab:u:" );
		switch( sw )
		{
		case 'c':
			mode *= 10;
			mode += M_CLIENT;
			break;
		case 's':
			mode *= 10;
			mode += M_SERVER;
			break;
		case 'd':
			debug = 1;
			break;
		case 'D':
			debug = 2;
			break;
		case 'h':
			debug = 3;
			break;
		case 'b':
			strncpy( bindip, optarg,sizeof bindip );
			break;
		case 'x':
			mode = M_REVEAL;
			break;
		case 'a':
			bDaemon = true;
			break;
		case 'A':
			bDaemon = true;
			strncpy( ARGV0, optarg, sizeof ARGV0 );
			break;
		case 'u':
			uid = atoi( optarg );
			break;
		case -1:
			break;
		default:
			help();
			exit(0);
		}
		if( sw < 0 )
		{
			break;
		}
	}
	if( argc > optind )
	{
		parse( argv[optind++], ip1, & port1 );
	}	
	if( argc > optind )
	{
		parse( argv[optind], ip2, & port2 );
	}
	if( mode == M_REVEAL )
	{
		mode = M_SERVER*10+M_CLIENT;
		sprintf( temp, "%s:%d", myip(), port1 );
		parse( temp, ip1, & port1 );
		sprintf( temp, "127.0.0.1:%d", port1 );
		parse( temp, ip2, & port2 );
	}
}

int main( int argc, char *argv[] )
{
	void (*process)() = 0;
	uid = getuid();
	if( getenv("MANGCHI_MODE") )
	{
		setup_fromenv();
	}
	else
	{
		setup( argc, argv );
	}

	switch( mode )
	{
	case M_CLIENT*10+M_CLIENT: process = client_client; break;
	case M_CLIENT*10+M_SERVER: process = client_server; break;
	case M_SERVER*10+M_CLIENT: process = server_client; break;
	case M_SERVER*10+M_SERVER: process = server_server; break;
	case M_SERVER*10:          process = server_only;   break;
	default:
		help();
		exit(0);
	}

	register_signal_handler();

	if( bDaemon )
	{
		if( demonize( argc, argv ) )
		{
			exit(0);
		}
	}
	(*process)();
	exit(0);
}
