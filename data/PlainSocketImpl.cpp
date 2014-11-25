//==============================================================================
// This file is part of the OpenTop C++ Library.
// (c) Copyright 2000-2002 ElCel Technology Limited
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// Licensees holding valid OpenTop Commercial licences may use this file
// in accordance with the OpenTop Commercial License Agreement included
// with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.elcel.com/products/opentop/pricing.html or email
// sales@elcel.com for information about OpenTop Commercial License Agreements.
//
//==============================================================================
//
// $Revision: 1.13 $
// $Date: 2003/09/10 21:15:23 $
//
//==============================================================================
//
// Notes on resource management
// ----------------------------
//
// In our Java-like networking library, a Socket may easily be created, an
// InputStream or OutputStream requested, and then the Socket itself could
// go out of scope and be deleted.  We want to support this sequence of events
// without placing a burden on the underlying operating system.
//
// One way to achieve this is to duplicate socket handles in the
// SocketStream classes.  In this way we use the OS's ability to reference-
// count the socket descriptor.  This has the advantage of using robust OS
// facilities, but with the downside that it is wasteful of OS socket handles
// which are a limited resource in some OSs.
//
// The chosen way is to implement our own reference-counted socket handle model.
// When an OS Socket is created, its handle is wrapped in a SocketDescriptor
// object which is reference-counted by virtue of deriving from the ManagedObject class.
// The SocketDescriptor is responsible for closing the socket handle
// when all references to the handle have been destroyed.
//
// Using a third-party object in this way avoids creating a circular reference
// between the input/output streams and the Socket.
//==============================================================================

#include "PlainSocketImpl.h"
#include "BindException.h"
#include "ConnectException.h"
#include "InetAddress.h"
#include "NetUtils.h"
#include "NoRouteToHostException.h"
#include "SocketImplFactory.h"
#include "Socket.h"
#include "SocketDescriptor.h"
#include "SocketException.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "SocketTimeoutException.h"

#include "ot/base/IllegalStateException.h"
#include "ot/base/NullPointerException.h"
#include "ot/base/NumUtils.h"
#include "ot/base/StringUtils.h"
#include "ot/base/SystemUtils.h"
#include "ot/base/Tracer.h"

#include <stdlib.h>

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif //WIN32

OT_NET_NAMESPACE_BEGIN

//==============================================================================
// PlainSocketImpl::PlainSocketImpl
//
// Default (and only) constructor.
//
// Instances of PlainSocketImpl are created exclusively by a SocketImplFactory.
//==============================================================================
PlainSocketImpl::PlainSocketImpl() : 
	m_localPort(-1),
	m_remotePort(-1),
	m_bBlocking(true),
	m_bPassive(false),
	m_nTimeoutMS(0),
	m_sockType(0)
{
}

//==============================================================================
// PlainSocketImpl::~PlainSocketImpl
//
//==============================================================================
PlainSocketImpl::~PlainSocketImpl()
{
	// It is not our business to close the underlying
	// OS socket (it may be in use - and that's the job
	// of the SockerDescriptor).
}

//==============================================================================
// PlainSocketImpl::create
//
//==============================================================================
void PlainSocketImpl::create(bool bStream)
{
	if(m_rpSocketDescriptor)
		throw SocketException(OT_T("socket already created"));

	SocketDescriptor::OSSocketDescriptorType socketFD;

	m_sockType = bStream ? SOCK_STREAM : SOCK_DGRAM;
	
	// Reset member control flags just in case this SocketImpl has been 
	// used and closed before.  This probably isn't possible, but
	// it is sensible to guard against it anyway.
	m_bPassive = false;
	m_bBlocking = true;
	m_localPort = -1;
	m_remotePort = -1;

	//
	// Attempt to create a stream socket.
	//
	if( (socketFD = socket(AF_INET, m_sockType, 0)) == OT_INVALID_SOCKET)
	{
		static const String err(OT_T("unable to create socket: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}

	//
	// After successfully creating a socket, we must immediately wrap
	// the os-supplied socket descriptor in our own reference-counted
	// SocketDescriptor object.
	//
	setSocketDescriptor(new SocketDescriptor(socketFD));
}

//==============================================================================
// PlainSocketImpl::connect
//
// Open a socket connection with the host specified in the passed String
// using the port passed.
//
// Throws: UnknownHostException, SocketException
//==============================================================================
void PlainSocketImpl::connect(const String& host, int port)
{
	RefPtr<InetAddress> rpAddr = InetAddress::GetByName(host);
	connectToAddress(rpAddr.get(), port, 0);
}

//==============================================================================
// PlainSocketImpl::connect
//
//==============================================================================
void PlainSocketImpl::connect(InetAddress* pAddress, int port, size_t timeoutMS)
{
	if(!pAddress) throw NullPointerException();
	connectToAddress(pAddress, port, timeoutMS);
}

//==============================================================================
// PlainSocketImpl::connect
//
// Open a socket connection with the host specified in the passed InetAddress
// using the port passed.
//
// Throws: SocketException
//==============================================================================
void PlainSocketImpl::connect(InetAddress* pAddress, int port)
{
	if(!pAddress) throw NullPointerException();
	connectToAddress(pAddress, port, 0);
}

//==============================================================================
// PlainSocketImpl::close
//
// Close the underlying operating system socket.
//
// This forces a close of the OS socket descriptor, this is different to the
// action we take on destruction, where the reference count of the
// SocketDescriptor is simply decremented.
//
// In common with most io close methods, further close operation have no effect.
//==============================================================================
void PlainSocketImpl::close()
{
	if(m_rpSocketDescriptor)
	{
		if(m_rpOutputStream)
			m_rpOutputStream->flush();

		m_rpSocketDescriptor->close();
		m_rpSocketDescriptor.release();
	}
}

//==============================================================================
// PlainSocketImpl::connectToAddress
//
// Common socket initialization function
//==============================================================================
void PlainSocketImpl::connectToAddress(InetAddress* pAddress,
                                       int port, size_t timeoutMS)
{
	if(!pAddress) throw NullPointerException();

	if(isConnected())
		throw SocketException(OT_T("socket already connected"));

	testSocketIsValid(false /*dont check socket connected*/);

	//
	// create and initialize a sockaddr_in structure
	// specifying the requested port.
	//
	struct sockaddr_in sa;
	::memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port); // convert port to network byte order 
	::memcpy(&sa.sin_addr, pAddress->getAddress(), pAddress->getAddressLength());

	//
	// If a timeout value has been specified, then we need to set the socket
	// non-blocking
	//
	bool oldBlocking = m_bBlocking;
	if(timeoutMS != 0)
	{
		setBlocking(false);
	}

	//
	// Call ::connect() to create the socket connection.
	// ::connect() will return zero is successful, otherwise -1.
	//
	// The WIN32 documentation recommends comparing the return value
	// against the manifest constant SOCKET_ERROR, but as this is not 
	// mandatory, we will use the portable test of < 0.
	//
	if(::connect(m_rpSocketDescriptor->getFD(), reinterpret_cast<struct sockaddr*>(&sa), sizeof(struct sockaddr_in)) < 0)
	{
		int errorNum = NetUtils::GetLastSocketError();
		bool bConnected = false;

		//
		// If we have set the socket non-blocking (due to a timeout being specified)
		// then we can expect the connect() to fail with EINPROGRESS.  In this case
		// we must perform a select() to wait for the command's completion.
		//
		if(!m_bBlocking && errorNum == EINPROGRESS)
		{
			// See the late W.Richard Stevens' Unix Network Programming: 15.4
			// select for readability or writability
			if(NetUtils::SelectSocket(m_rpSocketDescriptor.get(), timeoutMS, true, true))
			{
				//
				// The select succeeded, but this may be due to an error 
				// on the socket.  This can be established using ::getsockopt
				// (there are other ways, but this is the Stevens method)
				//
				// Note: for Solaris portability reasons, we do not call
				// getIntOption, but call getsockopt directly
				//
				int error = 0;
				cel_socklen_t retLen = sizeof(error);

				if(::getsockopt(m_rpSocketDescriptor->getFD(), SOL_SOCKET, SO_ERROR, (char*)&error, &retLen) < 0)
				{
					error = errno;
				}
				
				if(error)
				{
					// If an error has occured then we treat it the same way whether or
					// not there is a timeout value.  For this reason we just set the
					// errorNum and allow the code to fall-through to the next section
					// where an appropriate exception is thrown.

					errorNum = error;
				}
				else
				{
					// Great!  The select() succeeded and the socket has no error
					// condition, this means that it is now connected okay.
					bConnected = true;
				}
			}
			else
			{
				// select timed-out.  This means the connection is
				// progressing asynchronously.  This isn't what we want, we will
				// close the socket making it invalid for further ops
				//
				// Note: We don't call our close() member because that attempts
				// to perform an orderly shutdown - which is not what we want here.
				// The close() call is wraped in a try block to guard against
				// a failure preventing us from throwing the required exception
				//
				try
				{
					m_rpSocketDescriptor->close();
				}
				catch(Exception& /*e*/)
				{
				}

				m_rpSocketDescriptor.release();
				static const String err(OT_T("Connection timed out"));
				throw SocketTimeoutException(err);
			}
		}
		
		if(!bConnected)
		{
			//
			// The connect failed for no good reason.
			//
			// Close the socket to avoid using any resources...
			//
			// Note: We don't call our close() member because that attempts
			// to perform an orderly shutdown - which is not what we want here.
			// The close() call is wraped in a try block to guard against
			// a failure preventing us from throwing the required exception
			//
			try
			{
				m_rpSocketDescriptor->close();
			}
			catch(Exception& /*e*/)
			{
			}

			m_rpSocketDescriptor.release();

			// ... and throw an appropriate exception
			String errMsg = NetUtils::GetSocketErrorString(errorNum);
			errMsg += OT_T(" for: ");
			errMsg += pAddress->getHostName();

			if(errorNum == ENETUNREACH || errorNum == EHOSTUNREACH)
			{
				throw NoRouteToHostException(errMsg);
			}
	#if 0 
			// For overall consistency, we should throw a SocketTimeoutException
			// when a timeout occurs, but this may confuse clients
			// that have not passed a timeout value and are expecting
			// a ConnectException a la JDK
			else if(errorNum == ETIMEDOUT)
			{
				throw SocketTimeoutException(errMsg);
			}
	#endif
			else
			{
				throw ConnectException(errMsg);
			}
		}
	}

	OT_DBG_ASSERT(m_rpSocketDescriptor);

	//
	// Success!
	//
	m_rpRemoteAddr = new InetAddress(*pAddress);
	m_remotePort = port;

	if(Tracer::IsEnabled())
	{
		String traceMsg = OT_T("socket: ");
		traceMsg += m_rpSocketDescriptor->toString() + OT_T(" connected to ");
		traceMsg += pAddress->toString() + OT_T(":") + NumUtils::ToString(port);
		Tracer::Trace(Tracer::Net, Tracer::Medium, traceMsg);
	}

	//
	// Restore the original blocking mode
	// (we don't need to do this after a failure because the socket FD
	// is always closed in these situations
	//
	setBlocking(oldBlocking);
}

//==============================================================================
// PlainSocketImpl::getInputStream
//
// Return a (reference counted) pointer to the input stream for this socket.
//
// Note: It is not valid to request the InputStream from a socket that has
// not yet been created or connected (incl passive sockets).
//==============================================================================
RefPtr<InputStream> PlainSocketImpl::getInputStream() const
{
	testSocketIsValid(true /*check connected*/);

	if(!m_rpInputStream)
	{
		((PlainSocketImpl*)this)->m_rpInputStream = new SocketInputStream(m_rpSocketDescriptor.get());
		// if we have cached the timeout, relay it to the stream
		((SocketInputStream*)m_rpInputStream.get())->setTimeout(m_nTimeoutMS);
	}
	return m_rpInputStream;
}

//==============================================================================
// PlainSocketImpl::getOutputStream
//
// Return a (reference counted) pointer to the output stream for this socket.
//
// Note: It is not valid to request the InputStream from a socket that has
// not yet been created or connected (incl passive sockets).
//==============================================================================
RefPtr<OutputStream> PlainSocketImpl::getOutputStream() const
{
	testSocketIsValid(true /*check connected*/);

	if(!m_rpOutputStream)
	{
		((PlainSocketImpl*)this)->m_rpOutputStream = new SocketOutputStream(m_rpSocketDescriptor.get());
	}
	return m_rpOutputStream.get();
}

//==============================================================================
// PlainSocketImpl::getLocalAddress
//
//==============================================================================
RefPtr<InetAddress> PlainSocketImpl::getLocalAddress() const
{
	//
	// If the local address has not been requested before, do so now...
	//
	if(!m_rpLocalAddr && m_rpSocketDescriptor)
	{
		struct sockaddr_in sa;
		::memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		cel_socklen_t sasize = sizeof(struct sockaddr_in);
		struct sockaddr* pAddr = reinterpret_cast<struct sockaddr*>(&sa);

		if(::getsockname(m_rpSocketDescriptor->getFD(), pAddr, &sasize) < 0)
		{
			String errMsg = NetUtils::GetSocketErrorString();
			throw SocketException(errMsg);
		}

		// Handle unsupported mutable
		((PlainSocketImpl*)this)->m_rpLocalAddr = InetAddress::FromNetworkAddress(pAddr, sasize);
	}
	
	return m_rpLocalAddr;
}

//==============================================================================
// PlainSocketImpl::getInetAddress
//
//==============================================================================
RefPtr<InetAddress> PlainSocketImpl::getInetAddress() const
{
	return m_bPassive ? m_rpLocalAddr : m_rpRemoteAddr;
}

//==============================================================================
// PlainSocketImpl::getRemoteAddress
//
//==============================================================================
RefPtr<InetAddress> PlainSocketImpl::getRemoteAddress() const
{
	return m_rpRemoteAddr;
}

//==============================================================================
// PlainSocketImpl::toString
//
//==============================================================================
String PlainSocketImpl::toString() const
{
	String ret = OT_T("addr=");
	ret += getInetAddress()->toString();
	ret += OT_T(",port=");
	ret += NumUtils::ToString(getPort());
	ret += OT_T(",localport=");
	ret += NumUtils::ToString(getLocalPort());
	return ret;
}

//==============================================================================
// PlainSocketImpl::listen
//
// To accept connections, a socket is first created with the socket function
// and bound to a local address with the bind function, a backlog for incoming
// connections is specified with listen, and then the connections are accepted 
// with the accept function.
//
// Sockets that are connection oriented, those of type SOCK_STREAM for example,
// are used with listen. The socket is put into passive mode where incoming
// connection requests are acknowledged and queued pending acceptance by the
// process.
//==============================================================================
void PlainSocketImpl::listen(size_t backlog)
{
	testSocketIsValid(false /*dont check connected*/);

	if(::listen(m_rpSocketDescriptor->getFD(), backlog) < 0)
	{
		static const String err(OT_T("error listening on socket: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}
}

//==============================================================================
// PlainSocketImpl::getSocketDescriptor
//
//==============================================================================
RefPtr<SocketDescriptor> PlainSocketImpl::getSocketDescriptor() const
{
	return m_rpSocketDescriptor;
}

//==============================================================================
// PlainSocketImpl::getLocalPort
//
//==============================================================================
int PlainSocketImpl::getLocalPort() const
{
	return m_localPort;
}

//==============================================================================
// PlainSocketImpl::getPort
//
//==============================================================================
int PlainSocketImpl::getPort() const
{
	return m_remotePort;
}

//==============================================================================
// PlainSocketImpl::accept
//
// Called on a passive socket (a socket in listening state).
//
// Calls to accept() will block until a new connection request is available.
// The JDK 1.3 provides the SO_TIMEOUT option to limit the time that the
// server waits for new requests.  This is not an OS socket option and
// is implemented in ServerSocket as a select() call.
//
//==============================================================================
void PlainSocketImpl::accept(SocketImpl* pSocket)
{
	if(!pSocket) throw NullPointerException();

	//
	// If we have a timeout value then we must set the socket Non Blocking
	// so that we can trap the potential problem of a RST being received
	// after the select() but before the accept().
	//
	const bool bServerBlocking = (m_nTimeoutMS == 0);
	setBlocking(bServerBlocking);

	//
	// If this is a non-blocking accept (ie a timeout value has been specified)
	// then there are a number of error conditions which we should silently 
	// ignore (see W.R. Stevens Unix Network Programming, Vol 1, 15.6)
	//

	SocketDescriptor::OSSocketDescriptorType socketFD;
	sockaddr_in remoteAddr;
	cel_socklen_t addrLen = sizeof(remoteAddr);
	sockaddr* pRemoteAddr = reinterpret_cast<struct sockaddr*>(&remoteAddr);
	
	while(true)
	{
		//
		// Attempt to accept a new socket
		//
		if(m_nTimeoutMS && !NetUtils::SelectSocket(m_rpSocketDescriptor.get(), m_nTimeoutMS, true, false))
		{
			static const String err(OT_T("Accept timed out"));
			throw SocketTimeoutException(err);
		}

		if( (socketFD = ::accept(m_rpSocketDescriptor->getFD(), pRemoteAddr, &addrLen)) == OT_INVALID_SOCKET)
		{
			int errorNum = NetUtils::GetLastSocketError();
			if(errorNum == EWOULDBLOCK ||
			   errorNum == ECONNABORTED ||
			   errorNum == EPROTO)
			{
				// this is valid so we just loop round
			}
			else
			{
				// Note: expect a EINTR if the socket has been closed.
				// Should we loop round on a EINTR or perhaps throw an
				// InterruptedIOException?
				throw SocketException(NetUtils::GetSocketErrorString());
			}
		}
		else if(addrLen != sizeof(sockaddr_in))
		{
			static const String err(OT_T("accept() returned invalid remoteAddr size"));
			throw SocketException(err);
		}
		else
		{
			break;
		}
	}

	//
	// We must cast the passed ptr into a PlainSocketImpl in order to gain access
	// to the protected members.  Even if the passed ptr is not a PlainSocketImpl
	// this should still work as the functions are virtual and declared in
	// PlainSocketImpl
	//
	RefPtr<SocketDescriptor> rpSocketFD = new SocketDescriptor(socketFD);
	((PlainSocketImpl*)pSocket)->setSocketDescriptor(rpSocketFD.get());

	OT_DBG_ASSERT(remoteAddr.sin_family == AF_INET);
	((PlainSocketImpl*)pSocket)->setPort(ntohs(remoteAddr.sin_port));
	((PlainSocketImpl*)pSocket)->setLocalPort(getLocalPort());
	((PlainSocketImpl*)pSocket)->setInetAddress(InetAddress::FromNetworkAddress(pRemoteAddr, addrLen).get());

	//
	// I haven't found any documentation to confirm this, but it seems that a socket
	// accepted from a non-blocking socket is itself initialised as a non-blocking
	// socket.  This makes some logical sense, as other options specified on the
	// server socket are inherited by the client socket (such as buffer sizes).
	//
	// This isn't the desired approach for our Sockets, so we try to
	// avoid this by explicitly setting the blocking mode on the new spcket.
	// In this way the m_bBlocking member will agree with the underlying socket.
	//
	if(!bServerBlocking)
	{
		NetUtils::SetBlockingSocket(rpSocketFD.get(), true);
	}
}

//==============================================================================
// PlainSocketImpl::available
//
//==============================================================================
size_t PlainSocketImpl::available() const
{
	return getInputStream()->available();
}

//==============================================================================
// PlainSocketImpl::bind
//
// Before a server socket enters the passive state (by listen()) it must be
// bound to a port and (optionally) a local interface.
//
// A null InetAddress means that the socket will be bound to ANY interface.
// A zero port indicates that the system will choose the next available port.
//==============================================================================
void PlainSocketImpl::bind(InetAddress* pAddress, int port)
{
	if(port == -1)
		throw IllegalArgumentException(OT_T("invalid port number"));

	testSocketIsValid(false /*dont check connected*/);

	//
	// Create a sockaddr structure with the requested port and i/p addr
	//
	struct sockaddr_in sa;
	::memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port); // convert port to network byte order 

	if(pAddress)
	{
		m_rpLocalAddr = pAddress;
	}
	else
	{
		m_rpLocalAddr = InetAddress::GetAnyHost();
	}
	OT_DBG_ASSERT(m_rpLocalAddr);

	::memcpy(&sa.sin_addr.s_addr, m_rpLocalAddr->getAddress(), m_rpLocalAddr->getAddressLength());

	sockaddr* pLocalAddr = reinterpret_cast<struct sockaddr*>(&sa);

	if(::bind(m_rpSocketDescriptor->getFD(), pLocalAddr, sizeof(struct sockaddr_in)) < 0)
	{
		String errMsg = NetUtils::GetSocketErrorString();
		throw BindException(errMsg);
	}

	//
	// If a port number of zero was specified, the underlying system will assign
	// a port number automatically.  We must call getsockname() to retrieve the
	// port number assigned.
	//
	if(port == 0)
	{
		cel_socklen_t sasize = sizeof(struct sockaddr_in);
		if(::getsockname(m_rpSocketDescriptor->getFD(), pLocalAddr, &sasize) < 0)
		{
			String errMsg = NetUtils::GetSocketErrorString();
			throw SocketException(errMsg);
		}
		port = ntohs(sa.sin_port);
	}

	setLocalPort(port);
		
	m_bPassive = true;
}

//==============================================================================
// PlainSocketImpl::setSocketDescriptor
//
// Provides protected access to the OS Socket Descriptor.  Called when this
// SocketImpl is connected to a real OS socket.
//==============================================================================
void PlainSocketImpl::setSocketDescriptor(SocketDescriptor* pSocketDescriptor)
{
	m_rpSocketDescriptor = pSocketDescriptor;
}

//==============================================================================
// PlainSocketImpl::setInetAddress
//
// Protected member called during accept() on a server socket
//==============================================================================
void PlainSocketImpl::setInetAddress(InetAddress* pAddress)
{
	m_rpRemoteAddr = pAddress;
}

//==============================================================================
// PlainSocketImpl::setLocalPort
//
// Protected member called during accept() on a server socket
//==============================================================================
void PlainSocketImpl::setLocalPort(int localPort)
{
	m_localPort = localPort;
}

//==============================================================================
// PlainSocketImpl::setPort
//
// Protected member called during accept() on a server socket
//==============================================================================
void PlainSocketImpl::setPort(int port)
{
	m_remotePort = port;
}

//==============================================================================
// PlainSocketImpl::getIntOption
//
// Helper function to get a socket option
//==============================================================================
int PlainSocketImpl::getIntOption(int level, int option) const
{
	testSocketIsValid(false /*dont check connected*/);
	
	int retValue = 0;
	cel_socklen_t retLen = sizeof(retValue);

	if(::getsockopt(m_rpSocketDescriptor->getFD(),
	                level,
	                option,
	                (char*)&retValue,
	                &retLen) < 0)
	{
		static const String err(OT_T("error retrieving socket option: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}

	return retValue;
}

//==============================================================================
// PlainSocketImpl::setIntOption
//
// Helper function to set a socket option
//==============================================================================
void PlainSocketImpl::setIntOption(int level, int option, int value)
{
	testSocketIsValid(false /*dont check connected*/);

	if(::setsockopt(m_rpSocketDescriptor->getFD(),
	                level,
	                option,
	                (char*)&value,
	                sizeof(value)) < 0)
	{
		static const String err(OT_T("error setting socket option: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}
}

//==============================================================================
// PlainSocketImpl::getVoidOption
//
// Helper function for generic option retrieval
//==============================================================================
void PlainSocketImpl::getVoidOption(int level, int option, void* pOut, size_t* pLen) const
{
	if(!pOut || !pLen) throw NullPointerException();

	testSocketIsValid(false /*dont check connected*/);
	
	cel_socklen_t retLen = *pLen;

	if(::getsockopt(m_rpSocketDescriptor->getFD(),
	                level,
	                option,
	                (char*)pOut,
	                &retLen) < 0)
	{
		static const String err(OT_T("error retrieving socket option"));
		String errMsg = err + NetUtils::GetSocketErrorString();	}
	
	*pLen = retLen;
}

//==============================================================================
// PlainSocketImpl::setVoidOption
//
// Helper function to set a generic socket option
//==============================================================================
void PlainSocketImpl::setVoidOption(int level, int option, void* pValue, size_t valLen)
{
	if(!pValue) throw NullPointerException();

	testSocketIsValid(false /*dont check connected*/);

	if(::setsockopt(m_rpSocketDescriptor->getFD(),
	                level,
	                option,
	                (char*)pValue,
	                valLen) < 0)
	{
		static const String err(OT_T("error setting socket option: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}
}

//==============================================================================
// PlainSocketImpl::testSocketIsValid
//
// Helper function to implement common tests
//==============================================================================
void PlainSocketImpl::testSocketIsValid(bool bTestConnected) const
{
	if(!m_rpSocketDescriptor)
	{
		const static String err = OT_T("Socket closed");
		throw SocketException(err);
	}

	if(bTestConnected && !isConnected())
	{
		const static String err = OT_T("Socket not connected");
		throw SocketException(err);
	}
}

//==============================================================================
// PlainSocketImpl::getTimeout
//
// Return a value for the pseudo-option SO_TIMEOUT
//==============================================================================
size_t PlainSocketImpl::getTimeout() const
{
	if(m_rpInputStream)
	{
		return ((SocketInputStream*)m_rpInputStream.get())->getTimeout();
	}
	else
	{
		return m_nTimeoutMS;
	}
}

//==============================================================================
// PlainSocketImpl::setTimeout
//
// Set the value for the pseudo-option SO_TIMEOUT.  As the socket option
// SO_RCVTIMEO is not widely supported (okay on winsock 2, not on Linux),
// we simulate it using a select() call.
//
// The SocketInputStream does its own i/o, so for active sockets
// we pass the time limit on to the InputStream.  For passive sockets
// we store the value so that it is available the next time we perform
// an accept() call.
//==============================================================================
void PlainSocketImpl::setTimeout(size_t timeoutMS)
{
	m_nTimeoutMS = timeoutMS;
	if(m_rpInputStream)
	{
		((SocketInputStream*)m_rpInputStream.get())->setTimeout(timeoutMS);
	}
}

//==============================================================================
// PlainSocketImpl::setBlocking
//
// Set the socket blocking/non-blocking
//==============================================================================
void PlainSocketImpl::setBlocking(bool bBlocking)
{
	testSocketIsValid(false /*dont check connected*/);

	if(bBlocking != m_bBlocking)
	{
		NetUtils::SetBlockingSocket(m_rpSocketDescriptor.get(), bBlocking);
		m_bBlocking = bBlocking;
	}
}

//==============================================================================
// PlainSocketImpl::shutdownInput
//
//==============================================================================
void PlainSocketImpl::shutdownInput()
{
	//
	// Issue a shutdown() call on the socket, and mark the socket descriptor
	// as shutdown for input operations, resulting in EndOfFile for subsequent
	// read operations
	//
	if(m_rpSocketDescriptor)
		NetUtils::ShutdownSocket(m_rpSocketDescriptor.get(), NetUtils::Read);
}

//==============================================================================
// PlainSocketImpl::shutdownOutput
//
//==============================================================================
void PlainSocketImpl::shutdownOutput()
{
	//
	// Issue a shutdown() call on the socket, and mark the socket descriptor
	// as shutdown for output operations, resulting in an IOException for 
	// any subsequent write operations.
	//
	// Note: to be consistent with shutdownInput(), we do not call
	// the OutputStream's close() method.  However, we do call flush()
	// just in case the OutputStream being used is modified in the future to
	// perform buffering.
	//
	if(m_rpOutputStream)
		m_rpOutputStream->flush();

	NetUtils::ShutdownSocket(m_rpSocketDescriptor.get(),
	                         NetUtils::Write);
}

//==============================================================================
// PlainSocketImpl::isConnected
//
//==============================================================================
bool PlainSocketImpl::isConnected() const
{
	return (m_rpSocketDescriptor && m_remotePort != -1);
}

//==============================================================================
// PlainSocketImpl::isBound
//
//==============================================================================
bool PlainSocketImpl::isBound() const
{
	return (m_rpSocketDescriptor && m_localPort != -1);
}

//==============================================================================
// PlainSocketImpl::isClosed
//
//==============================================================================
bool PlainSocketImpl::isClosed() const
{
	return !(m_rpSocketDescriptor.isNull());
}

//==============================================================================
// PlainSocketImpl::getAutoClose
//
//==============================================================================
bool PlainSocketImpl::getAutoClose() const
{
	if(m_rpSocketDescriptor)
		return ((m_rpSocketDescriptor->getSocketFlags() & SocketDescriptor::AutoCloseEnabled)!=0);
	else
		return false;
}

//==============================================================================
// PlainSocketImpl::setAutoClose
//
//==============================================================================
void PlainSocketImpl::setAutoClose(bool bEnable)
{
	testSocketIsValid(false /*dont check connected*/);
	if(bEnable)
		m_rpSocketDescriptor->modifySocketFlags(SocketDescriptor::AutoCloseEnabled, 0);
	else
		m_rpSocketDescriptor->modifySocketFlags(0, SocketDescriptor::AutoCloseEnabled);
}

OT_NET_NAMESPACE_END
