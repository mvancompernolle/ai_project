/***************************************************************************
 *   Copyright (C) 2004-2006 by Ilya A. Volynets-Evenbakh                  *
 *   ilya@total-knowledge.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "requestlistener.h"
#include "requesthandler.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <functional>
#include <algorithm>
#include <socket++/sockinet.h>
#include <sptk4/CGuard.h>

namespace container {

RequestListener::RequestListener(sptk::CThreadPool& pool)
	: m_maxfd(-1)
	, m_pool(pool)
{
	FD_ZERO(&m_accept_fds);
}


RequestListener::~RequestListener()
{
}

class AcceptConnection: public std::unary_function<RequestListener::Acceptor*, void>
{
	sptk::CThreadPool& m_pool;
	fd_set *m_set;
public:
	explicit AcceptConnection(sptk::CThreadPool& pool, fd_set* fds)
		: m_pool(pool), m_set(fds){}
	void operator()(std::pair<const int, RequestListener::Acceptor*>& pair)
	{
		if(FD_ISSET(pair.first, m_set)) {
			Connection *con=pair.second->accept();
			if(con) {
				RequestHandler *rh=new RequestHandler(con);
				m_pool.queue(rh);
			}
		}
	}
};

}



/*!
 * \fn container::RequestListener::acceptRequests()
 * Accepts and queues requests into m_pool for processing.
 * If interrupted bu signal, presumes it's time to say bye bye
 */
void container::RequestListener::acceptRequests()
{
	while(true) {
		m_listenerLock.lock();
		fd_set rfds=m_accept_fds;
		int maxfd=m_maxfd;
		m_listenerLock.unlock();
		int ret=::select(maxfd+1,&rfds,0,0,0);
		if(ret==-1)
			throw servlet::IOError();
		{
			sptk::CGuard guard(m_listenerLock);
			try {
				for_each(m_listeners.begin(), m_listeners.end(), AcceptConnection (m_pool,&rfds));
			} catch(...) {
				std::cerr<<"Exception while accepting connections"<<std::endl;
				throw;
			}
		}
	}
}

container::RequestListener::Acceptor::~Acceptor()
{
}

void container::RequestListener::Acceptor::init(container::RequestListener::Acceptor::FileDesc& sock)
	throw(servlet::IOError)
{
	m_sock=sock;
	sock=-1;
	unsigned long flags = ::fcntl(m_sock,F_GETFL);
	if((long)flags==-1)
		throw servlet::IOError("getting socket flags");
	if(fcntl(m_sock, F_SETFL, flags|O_ASYNC)==-1)
		throw servlet::IOError("setting O_ASYNC socket flag");
}

container::RequestListener::TCPAcceptor::TCPAcceptor(std::string host, unsigned short port)
	throw(servlet::IOError, container::ResolverError)
{
	protoent* p=::getprotobyname("tcp");
	if(!p)
		throw servlet::IOError("cannot get protocol number for TCP protocol");
	if(host.empty())
		host = "0.0.0.0";
	sockinetaddr bindto(host.c_str(),port);
	FileDesc sock=::socket(PF_INET,SOCK_STREAM,p->p_proto);
	if(sock==-1)
		throw servlet::IOError("creating socket");
	int ret=::bind(sock,bindto.addr(),bindto.size());
	if(ret==-1)
		throw servlet::IOError("binding socket");
	ret=::listen(sock,MAX_BACKLOG);
	if(ret==-1)
		throw servlet::IOError("listening on socket");
	init(sock);
}

container::RequestListener::UnixAcceptor::UnixAcceptor(const std::string& path)
	throw(servlet::IOError)
	: m_path(path)
	, m_socklock(::open((path+".lck").c_str(),O_CREAT|O_NONBLOCK,0600))
{
	if(m_socklock==-1)
		throw servlet::IOError("unable to open socket lock file "+path+".lck");
	if(::flock(m_socklock, LOCK_EX|LOCK_NB)==-1)
		throw servlet::IOError("Unable to lock socket "+path+". Is another instance of CPPSERV running?");
	::unlink(path.c_str());
	FileDesc sock=::socket(PF_UNIX,SOCK_STREAM,0);
	if(sock==-1)
		throw servlet::IOError("unable to create listener socket");
	struct sockaddr_un bindto={AF_UNIX,{0,}};
	::strncpy(bindto.sun_path, path.c_str(), sizeof(bindto.sun_path));
	int ret=::bind(sock,(sockaddr*)&bindto,sizeof(bindto));
	if(ret==-1)
		throw servlet::IOError("unable to bind to listener socket \""+path+"\"");
	ret=::listen(sock,MAX_BACKLOG);
	if(ret==-1)
		throw servlet::IOError("unable to listen to listener socket \""+path+"\"");
	// No threading is going on yet, so it's ok to use strerror here
	if(::chmod(path.c_str(),0666) == -1)
		std::cerr << "Error setting mode on socket "
		    << path <<": " << strerror(errno) << std::endl;
	init(sock);
}

container::RequestListener::UnixAcceptor::~UnixAcceptor()
{
	if(::unlink(m_path.c_str())==-1) {
		char errbuf[1024];
		const char* err = strerror_r(errno, errbuf, sizeof(errbuf));
		std::cerr<<"Error removing "<<m_path<<": "<<err<<std::endl;
	}
}

container::RequestListener::Acceptor::FileDesc::~FileDesc()
{
	if(m_fd!=-1)
		close(m_fd);
}

container::Connection* container::RequestListener::Acceptor::accept()
{
	struct sockaddr_in remote;
	socklen_t remlen=sizeof(remote);
	int con=::accept(m_sock,(sockaddr*)&remote,&remlen);
	if(con==-1){
		char errbuf[1024];
		const char* err = strerror_r(errno, errbuf, sizeof(errbuf));
		std::cerr<<"Error accepting request: "<<err<<'\n';
		//Error may not be fatal, so let caller handle it
		return 0;
	}
	try {
		if(::fcntl(con, F_SETFL, 0)==-1)
			throw servlet::IOError("clearing O_ASYNC socket flag");
		Connection *c=new Connection(con);
		return c;
	} catch (sockerr& e) {
		std::cerr<<"Error at "<<__FILE__<<": "<<__LINE__<<": "<<e.errstr()<<std::endl;
		return 0;
	}
}


/*!
 * \fn container::RequestListener::addAcceptor(container::RequestListener::Acceptor*)
 */
void container::RequestListener::addAcceptor(container::RequestListener::Acceptor* acc)
{
	sptk::CGuard guard(m_listenerLock);
	m_listeners[acc->m_sock]=acc;
	FD_SET(acc->m_sock, &m_accept_fds);
	if(acc->m_sock > m_maxfd)
		m_maxfd=acc->m_sock;
}
