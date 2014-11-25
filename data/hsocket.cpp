/* 
 *------------------------------------------------------------------*
	FILE NAME:      hsocket.cpp
	DESCRIPTION:	Socket Wrapper Class Implementation.
	AUTHOR:	        Made by Hojin Choi <pynoos@users.sourceforge.net>
 *------------------------------------------------------------------*
 */

// pread requires below define"( in linux )

#include "hsocket.h"

#if defined(WIN32) && !defined(__CYGWIN32__)
	static WSAData * g_wsaData = NULL;
	int g_HSockets = 0;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HInetAddr::HInetAddr( const char * pHost, unsigned short port )
{
	Set( pHost, port );
}

void HInetAddr::Set( const char * pHost, unsigned short port )
{
	if( !pHost ) 
	{
		pHost = "0.0.0.0";
	}
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_addr.s_addr = inet_addr( pHost );
	m_sockaddr.sin_port = htons( port );
	m_socklen = (unsigned int) sizeof m_sockaddr;
}

void HInetAddr::Set( unsigned int addr, unsigned short port )
{
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_addr.s_addr = htonl( addr );
	m_sockaddr.sin_port = htons( port );
	m_socklen = (unsigned int) sizeof m_sockaddr;
}

const char * HInetAddr::GetIPAddr() const 
{
	return inet_ntoa( m_sockaddr.sin_addr );
}

unsigned int HInetAddr::GetAddr() const
{
	return ntohl( m_sockaddr.sin_addr.s_addr );
}

unsigned short HInetAddr::GetPort() const 
{
	return ntohs( m_sockaddr.sin_port );
};

void HInetAddr::GetSockName( HSocket & sock )
{
	getsockname( sock.GetFd(), (struct sockaddr *) & m_sockaddr, (socklen_t *) & m_socklen );
}

void HInetAddr::GetPeerName( HSocket & sock )
{
	getpeername( sock.GetFd(), (struct sockaddr *) & m_sockaddr, (socklen_t *) & m_socklen );
}

#if !defined(WIN32) && !defined(__CYGWIN32__)
HUnixAddr::HUnixAddr( const char * path  )
{
	Set( path );
}

void HUnixAddr::Set( const char * path )
{
	static char nullpath[107] = "";
	if( !nullpath[0] )
	{
		memset( nullpath,'X',106);
		nullpath[106] = 0;
	}
	m_sockaddr.sun_family = AF_UNIX;
	if( !path ) 
	{
		path = nullpath;
	}
	strncpy( m_sockaddr.sun_path, path, 107 );
	m_sockaddr.sun_path[107] = 0;
	m_socklen = strlen(m_sockaddr.sun_path) + sizeof (m_sockaddr.sun_family );
}
#endif 

int HFDObject::Select( int length, HFDObject ** ppFDArray, int msec )
{
	fd_set readfds;
	unsigned int max=0;
	int i=0;

	FD_ZERO( & readfds );
	if( !ppFDArray )
	{
		return H_ESELECT;	// return -2 : error occured.
	}

	for( i=0; i<length; i++ ) 
	{
		FD_SET( ppFDArray[i]->GetFd(), & readfds );	
		if( max < (unsigned int) ppFDArray[i]->GetFd() ){
			max = (unsigned int) ppFDArray[i]->GetFd();
		}
	}

	struct timeval tval, * ptval = & tval;
	tval.tv_sec = msec / 1000;
	tval.tv_usec = (msec % 1000)*1000;

	if( msec < 0 )
	{
		ptval = 0;
	}
	
	int ret;
	ret = select( max+1, & readfds, 0, 0, ptval );
	if( 0 == ret )
	{
		return H_TIMEOUT;
	}
	if( 0 > ret )
	{
		return H_ESELECT;	// return -2.
	}

	for( i=0; i<length; i++ ) 
	{
		if( FD_ISSET( ppFDArray[i]->GetFd(), & readfds ) )
		{
			return i;
		}
	}
	return H_ESELECT;
}

bool HInetAddrList::Add( const char * pcIP, const char * pcMask )
{
	const char * hostFullMask = "255.255.255.255";
	if( !pcMask )
	{
		pcMask = hostFullMask;
	}
	const char * slash = strchr( pcIP, '/' );
	if( slash )
	{
		pcMask = slash + 1;
	}
	else
	{
		slash = pcIP + strlen(pcIP);
	}

	// get ip address
	// like as "222.101.22.2"
	// inet_addr has a bug if ip is "255.255.255.255".
	// so check if ip is that or not.
	char buf[128];
	memset( buf, 0, 128 );
	if( strlen( pcIP ) > 127 )
	{
		return false;
	}
	strncpy( buf, pcIP, 127 );
	buf[ slash-pcIP ] = 0;
	if( 0 == strcmp( buf, hostFullMask ) )
	{
		m_addrs[m_count] = (unsigned int) -1;
	} 
	else 
	{
		m_addrs[m_count] = inet_addr(buf);
		if( m_addrs[m_count] == (unsigned int) -1 ) 
		{
			return false;
		}
	}

	// check mask in "255.255.0.0" style
	if( strchr( pcMask, '.' ) )
	{
		// if mask is invalid then 255.255.255.255 is selected.
		m_masks[m_count] = inet_addr(pcMask);
	}
	// check mask in "24" that is like "255.255.255.0", 24 bits "1" proceeded.
	else 
	{
		register unsigned long addr;
		register unsigned short bits = atoi( pcMask );
		addr = 0x00000000; 
		if( bits )
		{ 
			addr = 0x80000000;
			bits = (bits-1) % 32; //32 is max;
			while( bits-- )
			{
				addr |= addr >> 1;
			}
		}
		m_masks[m_count] = htonl( addr ); 
	}
	m_count++;
	return true;
}

bool HInetAddrList::Check( unsigned int addr ) const
{
	int i=0;
	while( i< m_count )
	{
		if( (m_addrs[i] & m_masks[i]) == (addr & m_masks[i]) )
		{
			return true;
		}
		i++;
	}
	return false;
}

bool HInetAddrList::Check( HInetAddr & addr ) const
{
	return Check( addr.m_sockaddr.sin_addr.s_addr );
}

bool HInetAddrList::CheckRemote( HSocket * pSocket ) const
{
	HInetAddr addr;
	addr.GetPeerName( *pSocket );
	return Check( addr );
}

HSocket::HSocket() 
: m_pUnixAddr(0)
{
#ifdef WIN32
	// The first initializing socket only..
	if ( !g_wsaData )
	{
		g_wsaData = new WSAData;
		if( g_wsaData )
		{
			WSAStartup( 0x0002, g_wsaData );
		}
	}
	m_EventForSelect = CreateEvent( 0,false,0,NULL );
	g_HSockets++;
#endif
}

HSocket::~HSocket()
{
	if( INVALID_FD == m_fd )
	{
		HSocket::Close();
	}

#ifdef WIN32
	g_HSockets--;
	if( m_EventForSelect ) 
	{
		CloseHandle( m_EventForSelect );
	}
	if ( 0 != g_wsaData && 0 == g_HSockets ) 
	{
		WSACleanup();
		delete g_wsaData;
		g_wsaData = 0;
	}
#endif
}

bool HSocket::Bind() 
{
	if ( bind( m_fd, (struct sockaddr *) & m_sockaddr, sizeof (m_sockaddr)) == SOCKET_ERROR ) 
	{
		return false;
	}
	return true;
}

bool HSocket::Listen( int max )
{
	if( listen(m_fd, max) < 0 ) 
	{
		return false;
	}
	return true;
}

bool HSocket::Create( unsigned short port, int sockType, const char * hostName )
{
	m_port = port;
	HSocket::Close();
	m_fd = socket(AF_INET, sockType, 0);
	if ( INVALID_FD == m_fd ) 
	{
		return false;
	}

	// If bindable socket is being created, set the address:port reusable
	if( port || sockType == SOCK_STREAM ) 
	{
		int val = 1;
		setsockopt( m_fd, SOL_SOCKET, SO_REUSEADDR, (char *) & val, sizeof( val ) );
	}

	memset( &m_sockaddr,0,sizeof m_sockaddr);
	m_sockaddr.sin_family = AF_INET; 
	if (!hostName || !hostName[0]) 
	{
		m_sockaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	}
	else
	{
		m_sockaddr.sin_addr.s_addr = inet_addr(hostName);
	}
	m_sockaddr.sin_port = htons(port);
	if( port || sockType == SOCK_DGRAM || hostName )
	{
		return Bind();
	}
	return true;
}

bool HSocket::CreateUnix( const char * pathName, int sockType, bool clean )
{
#if defined(WIN32) || defined(__CYGWIN32__)
	return false;
#else
	if( m_pUnixAddr )
	{
		delete m_pUnixAddr;
	}
	m_pUnixAddr = new sockaddr_un;

	if( clean ) 
	{
		unlink( pathName );
	}
	if( m_pUnixAddr ) 
	{
		strncpy( m_pUnixAddr->sun_path, pathName, 107 ); // 107 why? read text books!
		m_pUnixAddr->sun_family = AF_UNIX;
		m_pUnixAddr->sun_path[107] = 0;
	}

	int len;
	
  #ifdef SUN_LEN
	len = SUN_LEN( m_pUnixAddr );
//	m_pUnixAddr->sun_len = sizeof(*m_pUnixAddr);
  #else
	len = strlen( m_pUnixAddr->sun_path ) + sizeof( m_pUnixAddr->sun_family );
  #endif
  
	m_port = 0;
	m_fd = socket( AF_UNIX, sockType, 0 );
	if( INVALID_FD == m_fd )
	{
		return false;
	}
	SetRecvBufferSize( 65535 );
	if( 0 > bind( m_fd, (struct sockaddr *) m_pUnixAddr, len ) )
	{
		return false;
	}
	return true;
#endif //#if defined(WIN32)
}

bool HSocket::SetAsBroadcasting()
{
	int val = 1;
	return 0 == setsockopt( m_fd, SOL_SOCKET, SO_BROADCAST, (char *) & val, sizeof( val ) );
}

bool HSocket::SetAsNoLinger()
{
	struct linger l;
	l.l_onoff = 1;
	l.l_linger = 0;
	return 0 == setsockopt( m_fd, SOL_SOCKET, SO_LINGER, (char *) & l, sizeof (l) );
}

bool HSocket::SetAsKeepAlive()
{
	int val = 1;
	return 0 == setsockopt( m_fd, SOL_SOCKET, SO_KEEPALIVE, (char *) & val, sizeof( val ) );
}

int HSocket::GetRecvBufferSize()
{
	int len = 0;
	socklen_t sizeInt = (socklen_t) sizeof len;
	if( 0 != getsockopt( m_fd, SOL_SOCKET, SO_RCVBUF, (char *) & len, & sizeInt ) )
	{
		return -1;
	}
	return len;
}

int HSocket::GetSendBufferSize()
{
	int len = 0;
	socklen_t sizeInt = (socklen_t) sizeof len;
	if( 0 != getsockopt( m_fd, SOL_SOCKET, SO_SNDBUF, (char *) & len, & sizeInt ) )
	{
		return -1;
	}
	return len;
}

bool HSocket::SetRecvBufferSize( int size )
{
	return 0 == setsockopt( m_fd, SOL_SOCKET, SO_RCVBUF, (char *) & size, sizeof size );
}

bool HSocket::SetSendBufferSize( int size )
{
	return 0 == setsockopt( m_fd, SOL_SOCKET, SO_SNDBUF, (char *) & size, sizeof size );
}

bool HSocket::Connect(const char * hostName, unsigned short port)
{
	bool b_findChr;
	if( m_fd == INVALID_FD ) 
	{
		m_fd = socket(AF_INET, SOCK_STREAM, 0);	// domain = IPv4, type = stream, 
	}

	if( m_fd != INVALID_FD ) 
	{
		m_sockaddr.sin_family = AF_INET;
		if (!hostName)
		{
			m_sockaddr.sin_addr.s_addr = htonl( INADDR_ANY );
		}
		else 
		{
			struct hostent * h;
			b_findChr=false;
			for(int i=0; hostName[i] && hostName[i]!='.'; i++)
			{
				if( hostName[i] < '0' || hostName[i] > '9' )
				{
					 b_findChr = true;
					 break;
				}
			}
			if( b_findChr )
			{
				h = gethostbyname( hostName );
				if(!h)
				{
					m_sockaddr.sin_addr.s_addr = inet_addr(hostName); 
				}
				else
				{
					m_sockaddr.sin_addr.s_addr = *(unsigned long *)(h->h_addr_list[0]);
				}
			}
			else 
			{
				m_sockaddr.sin_addr.s_addr = inet_addr(hostName);
			}
		}
		m_sockaddr.sin_port = htons(port);
		return 0 == ::connect( m_fd,(struct sockaddr *) & m_sockaddr, sizeof m_sockaddr );
	}
	return false;
}

bool HSocket::Connect( const char * pathName )
{
#if defined(WIN32)
	return false;
#else
	// delete before being created.
	if( m_fd >=0 ) 
	{
		Close();
	}

	struct sockaddr_un unixAddr;
	if( m_pUnixAddr ) 
	{
		strncpy( unixAddr.sun_path, pathName, 107 ); // 107 why? read text books!
		unixAddr.sun_family = AF_UNIX;
		unixAddr.sun_path[107] = 0;
	}

	int len;
	len = strlen( unixAddr.sun_path ) + sizeof( unixAddr.sun_family );
	m_fd = socket( AF_UNIX, SOCK_STREAM, 0 );
	if( INVALID_FD ==  m_fd )
	{
		return false;
	}
	return 0 == ::connect( m_fd, (struct sockaddr *) & unixAddr, len );
#endif //#if defined(WIN32)
}

bool HSocket::Accept( HSocket * pSocket )
{
	if( !pSocket )
	{
		return false;
	}
	socklen_t len = (socklen_t) sizeof pSocket->m_sockaddr;

accept_again:
	pSocket->m_fd = accept(m_fd, (struct sockaddr *) & pSocket->m_sockaddr, & len);
	if( INVALID_FD == pSocket->m_fd )
	{
		if( errno == EINTR )
		{
			goto accept_again;
		}
		return false;
	}
	SetAsKeepAlive();
	return true;
}

bool HSocket::Accept( HSocket & pSocket )
{
	return Accept( & pSocket );
}

int HSocket::Receive( char * buf, int size, int timeOut, int flag)
{
	int ret;

	if( timeOut <= 0 ) 
	{
		ret=recv( m_fd, buf, size, flag );
#if !defined(WIN32) || defined(__CYGWIN32__)
		if( ret < 0 && errno == ENOTSOCK )
		{
			if( flag != MSG_PEEK )
			{
				ret=read( m_fd, buf, size );
			}
			else
			{
#ifdef H_PREAD
				ret=pread( m_fd, (void*)buf, (size_t)size, (off_t)0 );
#else
				ret=-1;
#endif
			}
		}
#endif //!defined(WIN32) || defined(__CYGWIN32__)
	}
	else
	{
		int r;
		while(1)
		{
			//timeOut recv
			switch(Select(1, this, timeOut))
			{
			case H_ESELECT:
				//signal.. ignore
				break;
			case H_TIMEOUT:
				//timeout
				return H_TIMEOUT;
			case  0:
				r = recv( m_fd, buf, size, flag);
				if( r <= 0 )
				{
					return r;
				}
				return r;
			}
		}
	}
	return ret;
}

int HSocket::ReceiveFrom( char * buf, int size, int flag, HInetAddr & addr )
{
	return recvfrom( m_fd, buf, size, flag,(struct sockaddr * ) & addr.m_sockaddr, (socklen_t *) & addr.m_socklen );
}

int HSocket::ReceiveFrom( char * buf, int size, int flag, HUnixAddr & addr )
{
#if defined(WIN32) || defined(__CYGWIN32__)
	return -1;
#else
	return recvfrom( m_fd, buf, size, flag,(struct sockaddr * ) & addr.m_sockaddr, (socklen_t *) & addr.m_socklen );
#endif
}

int HSocket::Send(const char * buf, int length)
{
	if( length < 0 ) 
	{
		length = strlen( (char*)buf );
	}

	int ret = send( m_fd, buf, length, 0);

#if !defined(WIN32) || defined(__CYGWIN32__)
	if( 0 > ret && errno == ENOTSOCK )
	{
		ret = write( m_fd, buf, length );
	}
#endif
	return ret;
}

int HSocket::SendTo(const char * buf, int length, const HInetAddr & addr )
{
	return sendto( m_fd, buf, length, 0, (const struct sockaddr * ) & addr.m_sockaddr, addr.m_socklen );
}

int HSocket::SendTo(const char * buf, int length, const HUnixAddr & addr )
{
#if defined(WIN32) || defined(__CYGWIN32__)
	return -1;
#else
	return sendto( m_fd, buf, length, 0, (const struct sockaddr * ) & addr.m_sockaddr, addr.m_socklen );
#endif
}

#if defined(WIN32) && !defined(__CYGWIN32__)
int HSocket::HSelect( long lNetworkEvents )
{
	return WSAEventSelect( m_fd, m_EventForSelect, lNetworkEvents );
}
#endif

// static function
int HSocket::Select( int length, HSocket * ppSockArray[], int msec )
{
	return HFDObject::Select( length, (HFDObject **)(ppSockArray), msec );
}

// static function
int HSocket::Select( int length, HSocket pSockArray[], int msec )
{
	fd_set readfds;
	unsigned int max=0;
	int i=0;

	FD_ZERO( & readfds );
	if( !pSockArray )
	{
		return H_ESELECT;	// return -2 : error occured.
	}

	for( i=0; i<length; i++ ) 
	{
		FD_SET( pSockArray[i].GetFd(), & readfds );	
		if( max < (unsigned int) pSockArray[i].GetFd() ) 
		{
			max = (unsigned int) pSockArray[i].GetFd();
		}
	}

	struct timeval tval, * ptval = & tval;
	tval.tv_sec = msec / 1000;
	tval.tv_usec = (msec % 1000)*1000;

	if( msec < 0 )
	{
		ptval = 0;
	}
	
	int ret;
	ret = select( max+1, & readfds, 0, 0, ptval );
	if( 0 == ret )
	{
		return H_TIMEOUT;
	}
	if( 0 > ret )
	{
		return H_ESELECT;	// return -2.
	}

	for( i=0; i<length; i++ ) 
	{
		if( FD_ISSET( pSockArray[i].GetFd(), & readfds ) )
		{
			return i;
		}
	}
	return H_ESELECT;
}

bool HSocket::Close()
{
	int r = 0;
	if( INVALID_FD != m_fd )
	{
#if defined(WIN32) && !defined(__CYGWIN32__)
		r = closesocket( m_fd );
#else
		r = close( m_fd );
#endif
	}
	m_fd = INVALID_FD;

	return r == 0;
}

bool HSocket::Shutdown( int flag )
{
	return 0 == shutdown( m_fd, flag );
}

const char * HSocket::GetIPAddrByName( const char * hostname )
{
	static char _ip[32];
	struct hostent * hent;
	hent = gethostbyname( hostname );
	if( !hent ) return 0;
	sprintf( _ip, "%u.%u.%u.%u", 
		(unsigned char)hent->h_addr_list[0][0],
		(unsigned char)hent->h_addr_list[0][1],
		(unsigned char)hent->h_addr_list[0][2],
		(unsigned char)hent->h_addr_list[0][3] );
	return _ip;
}

const char * HSocket::GetRemoteIP() const
{
	struct sockaddr_in addr;
	socklen_t len = (socklen_t) sizeof addr;
	if( 0 > getpeername( m_fd, (struct sockaddr * )& addr, & len ) )	// On error	
	{
		return 0;
	}
	return inet_ntoa( addr.sin_addr );
}

unsigned short HSocket::GetRemotePort() const
{
	struct sockaddr_in addr;
	socklen_t len = (socklen_t) sizeof addr;
	if( 0 > getpeername( m_fd, (struct sockaddr * )& addr, & len ) )
	{
		return 0;
	}
	return ntohs( addr.sin_port );
}

const char * HSocket::GetLocalIP() const
{
	struct sockaddr_in addr;
	socklen_t len = (socklen_t) sizeof addr;
	if( 0 > getsockname( m_fd, (struct sockaddr * )& addr, & len ) )
	{
		return 0;
	}
	return inet_ntoa( addr.sin_addr );
}

unsigned short HSocket::GetLocalPort() const
{
	struct sockaddr_in addr;
	socklen_t len = (socklen_t) sizeof addr;
	if( 0 > getsockname( m_fd, (struct sockaddr * )& addr, & len ) )
	{
		return 0;
	}
	return ntohs( addr.sin_port );
}

/*---------------
 * library check function for autoconf
 *--------------- */

extern "C" char HSocketLibrary() {return 'x';}
