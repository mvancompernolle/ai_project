/***************************************************************************
 *   Copyright (C) 2004-2--6 by Ilya A. Volynets-Evenbakh                  *
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
#include <cservdemon.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <requestlistener.h>
#include <serverconfig/xmlserverconfig.h>
#include <servlet/IOError.h>
#include <servlet/cppserv-ver.h>

//TODO: Path resolution (for phys_base calculation)
//TODO: for now Unix-specific. Replace with TR2 when available
#include <limits.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/types.h>

namespace container {

CServDemon::CServDemon()
	: m_cinName("/dev/zero")
	, m_coutName("./servlet.log")
	, m_cerrName(m_coutName)
	, m_detach(false)
	, m_pidF(-1)
	, m_pidFPath("/var/run/cppserv.pid")
	, m_lockedPidFile(false)
{
}


CServDemon::~CServDemon()
{
	if(m_pidF != -1) {
		::flock(m_pidF,LOCK_UN);
		::close(m_pidF);
		if(m_lockedPidFile)
			::unlink(m_pidFPath.c_str());
	}
}


static void version()
{
	std::cout<<"CPPSERV version "<<CPPSERV_VERSION<<std::endl;
}

}


/*!
    \fn container::CServDemon::run()
 */
void container::CServDemon::run()
{
	try {
		m_conf.load();
		m_conf.processConfig();//This should create and load all application objects...
	} catch (servlet::ServletException& e) {
		std::cerr<<"Error parsing config file: "<<e.getMsg()<<std::endl;
		e.printStackTrace(std::cerr);
		exit(1);
	}
	try {
		m_conf.getListener()->acceptRequests();
	} catch (servlet::Traceable& e) {
		std::cerr<<"Error while processing requests: "<<e.getMsg()<<std::endl;
		e.printStackTrace(std::cerr);
	} catch (...) {
		std::cerr<<"Unknown error while processing requests"<<std::endl;
	}
}


/*!
    \fn container::CServDemon::parseOptions(int argc, char** argv)
 */
void container::CServDemon::parseOptions(int argc, char** argv)
{
	const char* opts="c:dl:p:P:t:D:ho:v";
	char o;
	while((o=getopt(argc,argv,opts))!=-1){
		switch(o){
		case 'c':
			setConfigPath(optarg);
			break;
		case 'd':
			m_detach=true;
			break;
		case 'P':
			m_pidFPath=optarg;
			break;
		case 't':
			m_conf.setGlobalParam("num_threads", optarg);
			break;
		case 'D':
			m_conf.setGlobalParam("queue_depth", optarg);
			break;
		case 'o':
			m_coutName = m_cerrName = optarg;
			break;
		case 'v':
			version();
			exit(0);
		case '?':
		default:
			std::cerr<<"Unknown option: "<<o<<std::endl;
		case 'h':
			help();
			break;
		}
	}
}


/*!
 * \fn static void container::CServDemon::handleSig11(int signum, siginfo_t *info, void *par)
 * @param signum Signal number
 * @param info info struct
 * @param par param
 * @todo This implementaion is completely broken. At the very least connection should be closed
		and RequestHandler this thread runs on cleaned up
 */

void container::CServDemon::handleSig11(int,siginfo_t*, void*)
{
	std::cerr<<"Received sig11\n";
	pthread_exit(0);
}

/*!
	\fn container::CServDemon::handleTermSig(int signum,siginfo_t* info, void* par)
	This function handles termination signales (SIGTERM, SIGINT), by telling dispatcher
	to stop running.
 */
void container::CServDemon::handleTermSig(int /*signum*/,siginfo_t* /*info*/, void* /*par*/)
{
	///FIXME: stop the daemon
	//	disp->stop();
}

/*!
	\fn container::CServDemon::supervisorTerm(int signum,siginfo_t* info, void* par)
	This function handles termination signales (SIGTERM, SIGINT), by telling dispatcher
	to stop running.
 */
void container::CServDemon::supervisorTerm(int /*signum*/,siginfo_t* /*info*/, void* /*par*/)
{
}


/*!
    \fn container::CServDemon::help()
 */
void container::CServDemon::help()
{
	version();
	std::cout<<"Usage:\n"
		"cppserv [-c <config.xml>] [-d] [-l <hostname>] [-p <num>] [-P <pidfile>] [-t <num>] [-h]\n"
		" -c allows to specify path to configuration file. Default is /etc/engine.xml\n"
		" -d casuses engine to detach from controlling terminal and run in daemon mode\n"
		" -l <hostname> specify hostname/IP to listen on. Default is to listen on all\n"
		" -p <port> port to listen on. Default is 9004\n"
		" -P <pidfile> file to save PID in. Default /var/run/cppserv.pid\n"
		" -t <num> number of threads in task queue. Default 15.\n"
		" -D <num> queue depth. Default 15\n"
		" -o <path> stdout and stderr output file when daemonizing (default ./servlet.log).\n"
		" -h Display this help message and exit\n";
	exit(1);
}

void container::CServDemon::setConfigPath(const char* path)
{
	m_conf.setPath(path);
	//boost::filesystem::path cfgfile(path);
	//m_conf.setGlobalParam("phys_path", cfgfile.parent_path().directory_string());
	//TODO: right now boost::filesystem API is unstable
	//TODO: replace this with TR2 implementation later, when it's available
	char rpath[PATH_MAX];
	if(!::realpath(path, rpath))
		throw servlet::IOError("Unable to figure out absolute base path for config file");
	char* sep = ::strrchr(rpath, '/');
	if(!sep)
		sep = rpath;
	*sep=0;
	m_conf.setGlobalParam("phys_base", rpath);
}

/*!
    \fn container::Demon::start()
 */
void container::CServDemon::start()
{
	int pid;
	if(m_detach){
		// Change session, to completely detach from controlling terminal
		::setsid();
		pid=fork();
		if(pid==-1) {
			std::string err("Error forking while detaching:");
			err += strerror(errno);
			throw std::runtime_error(err);
		}
		if(pid!=0) //Detached
			exit(0);
		// HACK ALERT! This may result in some problems with buffering!
		// This most likely does screw up system-level buffering options
		//std::cin.flush();
		std::cout.flush();
		std::cerr.flush();
		int cin=::open(m_cinName.c_str(),O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR);
		if(cin == -1)std::cout<<"Unable to open CIN file ("<<m_cinName<<"): "<<strerror(errno)<<std::endl;
		int cout=::open(m_coutName.c_str(),O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
		if(cout == -1)std::cout<<"Unable to open COUT file ("<<m_coutName<<"): "<<strerror(errno)<<std::endl;
		int cerr=::open(m_cerrName.c_str(),O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
		if(cerr == -1)std::cout<<"Unable to open CIN file ("<<m_coutName<<"): "<<strerror(errno)<<std::endl;
		if(dup2(cin,0)==-1)std::cout<<"Couldn't reopen CIN! "<<errno<<':'<<strerror(errno)<<std::endl;
		if(dup2(cout,1)==-1)std::cout<<"Couldn't reopen COUT! "<<errno<<':'<<strerror(errno)<<std::endl;
		if(dup2(cerr,2)==-1)std::cout<<"Couldn't reopen CERR! "<<errno<<':'<<strerror(errno)<<std::endl;
		::close(cin);
		::close(cout);
		::close(cerr);
	}
	pid=getpid();
	m_pidF=::open(m_pidFPath.c_str(),O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
	if(m_pidF<0){
		std::cerr<<"Unable to open pid file: \""<<m_pidFPath<<"\": "<<strerror(errno)<<std::endl;
		return;
	}
	if(::flock(m_pidF,LOCK_EX|LOCK_NB)<0){
		std::cerr<<"Unable to lock pid file: \""<<m_pidFPath<<"\": "<<strerror(errno)<<std::endl;
		std::cerr<<"Is another cppserv already running?\n";
		return;
	} else {
		m_lockedPidFile = true;
	}
	char buf[20];
	::snprintf(buf,sizeof(buf)-1,"%d",pid);
	::ftruncate(m_pidF,0);
	::write(m_pidF,buf,strlen(buf));
	::fsync(m_pidF);
	if(m_detach) {
		// Handle SIGTERM and SIGINT in supervisor - we detect
		// them when waitpid() gets interrupted and exit then
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler=0;
		act.sa_sigaction=supervisorTerm;
		act.sa_flags=SA_SIGINFO;
		sigaction(SIGTERM,&act,0);
		sigaction(SIGINT,&act,0);

		// Now start the superwisor
		int pid;
		std::cerr<<"Starting supervisor"<<std::endl;
		while(true) {
			pid=fork();
			if(pid==-1) { // Ugh...
				std::cerr<<"Ihsa! Superwisor couldn't fork worker child!"<<std::endl;
				exit(1);
			}
			if(pid==0) { // child - go and do some real work
				std::cerr<<"Starting worker child"<<std::endl;
				m_pidF = -1; // Do not destroy PID file when child exits
				break;
			}
			int status=0;
			int waitret=waitpid(pid, &status, 0);
			if(waitret==-1) {
				if(errno==EINTR) { // Try to kill the child
					std::cerr<<"Got interrupted. Killing the child."<<std::endl;
					kill(pid, SIGTERM);
					// Sleep five seconds or until exit of the child, since it should deliver SIGCHLD to us.
					// NOTE: What happens if the child have exited by the time we got to sleep?
					// Worst case scenario - we sleep for nothing
					sleep(5);
					if(waitpid(pid, &status, WNOHANG)==0) {
						std::cerr<<"Child is sturdy one. I had to get harsh."<<std::endl;
						kill(pid, SIGKILL); // If this doesn't kill it, we are in big trouble...
						waitpid(pid, &status, 0);
					}
				}
				return;
			}
			// if waitpid returned valid value, we just continue,
			// log exit reason for previous child and restart worker process
			if(WIFEXITED(status)) {
				std::cerr<<"Worker exited with status "<<WEXITSTATUS(status)<<std::endl;
			} else if(WIFSIGNALED(status)){
				std::cerr<<"Worker was killed by signal "<<WTERMSIG(status)<<std::endl;
			} else {
				std::cerr<<"Worker terminated by aliens (or other means, unknown to me)"<<std::endl;
			}
		}
	}
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler=SIG_IGN;
	//act.sa_mask=0;
	act.sa_flags=0;//SA_RESTART;
	//Ignore SIGPIPE
	sigaction(SIGPIPE,&act,0);
	//FIXME: This is an ugly hack. We need to do something more reasonable
	//for SIGTERM handling
	//	CServDemon::disp=m_conf.getDisp();
	act.sa_handler=0;
	act.sa_sigaction=handleTermSig;
	act.sa_flags=SA_SIGINFO;
	sigaction(SIGTERM,&act,0);
	sigaction(SIGINT,&act,0);
	// Start working..
	run();
}
