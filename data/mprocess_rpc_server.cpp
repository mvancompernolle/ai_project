/***************************************************************************
          mprocess_rpc_client.cpp.cpp  -  multi process rpc server
                             -------------------
    begin                : Sun May 29 2005
    copyright            : (C) 2005 Dmitry Nizovtsev <funt@alarit.com>
                                    Olexander Shtepa <isk@alarit.com>

    $Id: mprocess_rpc_server.cpp 1164 2010-01-06 10:03:51Z ewald-arnold $

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


#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <ulxmlrpcpp/ulxr_dispatcher.h>
#include <ulxmlrpcpp/ulxr_protocol.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_xmlparse.h>
#include <ulxmlrpcpp/ulxr_xmlparse_base.h>
#include <ulxmlrpcpp/ulxr_callparse.h>
#include <ulxmlrpcpp/ulxr_callparse_wb.h>

#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#include <ulxmlrpcpp/contrib/mprocess_rpc_server.h>


namespace funtik {


MultiProcessRpcServerError::MultiProcessRpcServerError(const std::string& what_arg):
  _what(what_arg)
{
}


MultiProcessRpcServerError::~MultiProcessRpcServerError() throw()
{

}

const char*  MultiProcessRpcServerError::what () const throw()
{
  return this->_what.c_str();
}



void MultiProcessRpcServer::doChild()
{
  ULXR_TRACE(ULXR_PCHAR("doChild"));

  close(m_poDispatcher->getProtocol()->getConnection()->getServerHandle());
  ulxr::MethodCall call=handleXmlRequest();

    ulxr::Protocol *poNowProtocol=m_poDispatcher->getProtocol();
    ulxr::MethodResponse resp = m_poDispatcher->dispatchCall(call);
  storeFuncResult(call,resp);
    if (poNowProtocol!=0 && !poNowProtocol->isTransmitOnly())
    poNowProtocol->sendRpcResponse(resp,m_wbxml_mode);
  if (poNowProtocol!=0 && !poNowProtocol->isPersistent())
      poNowProtocol->close();
//finish him!
  exit(0);
}

std::map<pid_t,MultiProcessRpcServer::ProcessContext> MultiProcessRpcServer::m_mapProcesses;


void  MultiProcessRpcServer::handleRequest()
{
  ULXR_TRACE(ULXR_PCHAR("handleRequest"));

  if(m_poDispatcher.get() == 0)
    throw MultiProcessRpcServerError("MultiProcessRpcServer::handleRequest: Dispatcher does not initialize");

  if(m_poDispatcher->getProtocol() == 0)
    throw MultiProcessRpcServerError("MultiProcessRpcServer::handleRequest: Protocol does not initialize");

  try{
    acceptConnection();

      if((m_maxProcesses != 0) && (m_mapProcesses.size() > m_maxProcesses))
        throw ulxr::Exception(ulxr::SystemError, ULXR_PCHAR("MultiProcessRpcServer::handleRequest: Max number of process already started."));

    blockSigchld();

    pid_t ppid=fork();

    if(ppid == -1)
          throw ulxr::Exception(ulxr::SystemError,ULXR_PCHAR("MultiProcessRpcServer::handleRequest: Can`t create process for handle request."));

    if(ppid == 0)
    {
//close server socket
      unblockSigchld();

      try{
        doChild();
      }
      catch(...)
      {
//finish him if error!!!
        exit(1);
      }
    }

//store Process info
    if(ppid > 0)
      storeProcessData(ppid);

    unblockSigchld();
  }
  catch(std::exception &ex)
  {
//cleaning
    unblockSigchld();

    if (!m_poDispatcher->getProtocol()->isPersistent())
      m_poDispatcher->getProtocol()->close();

    sweepProcessData();

    throw;
  }
//Close client connection
  if (!m_poDispatcher->getProtocol()->isPersistent())
    m_poDispatcher->getProtocol()->close();

  sweepProcessData();
}


void MultiProcessRpcServer::acceptConnection()
{
  if (!m_poDispatcher->getProtocol()->isOpen())
    m_poDispatcher->getProtocol()->accept();
  else
    m_poDispatcher->getProtocol()->resetConnection();
  m_poDispatcher->getProtocol()->setPersistent(false);
}


ulxr::MethodCall MultiProcessRpcServer::handleXmlRequest()
{
  char buffer[ULXR_RECV_BUFFER_SIZE];
  char *buff_ptr;

  std::auto_ptr<ulxr::XmlParserBase> parser;
  ulxr::MethodCallParserBase *cpb = 0;
  if (m_wbxml_mode)
  {
    ULXR_TRACE(ULXR_PCHAR("waitForCall in WBXML"));
    ulxr::MethodCallParserWb *cp = new ulxr::MethodCallParserWb();
    cpb = cp;
    parser.reset(cp);
  }
  else
  {
    ULXR_TRACE(ULXR_PCHAR("waitForCall in XML"));
    ulxr::MethodCallParser *cp = new ulxr::MethodCallParser();
    cpb = cp;
    parser.reset(cp);
  }

  bool done = false;
  long readed;
  while (!done && ((readed = m_poDispatcher->getProtocol()->readRaw(buffer, sizeof(buffer))) > 0) )
  {
    buff_ptr = buffer;

    if (!m_poDispatcher->getProtocol()->hasBytesToRead())
//      || parser->isComplete())
      done = true;

    while (readed > 0)
    {
      ulxr::Protocol::State state = m_poDispatcher->getProtocol()->connectionMachine(buff_ptr, readed);

      if (state == ulxr::Protocol::ConnError)
        throw ulxr::ConnectionException(ulxr::TransportError, ulxr_i18n(ULXR_PCHAR("MultiProcessRpcServer::handleXmlRequest: network problem occured")), 500);

      else if (state == ulxr::Protocol::ConnSwitchToBody)
      {
        if (!m_poDispatcher->getProtocol()->hasBytesToRead())
        {
#ifdef ULXR_SHOW_READ
          ulxr::Cpp8BitString super_data(buff_ptr, readed);
          while ((readed = m_poDispatcher->getProtocol()->getConnection()->read(buffer, sizeof(buffer))) > 0)
          super_data.append(buffer, readed);
          ULXR_DOUT_READ(ULXR_PCHAR("superdata 1 start:\n")
          << ULXR_GET_STRING(super_data)
          << ULXR_PCHAR("superdata 1 end:\n" ));
#endif
          throw ulxr::ConnectionException(ulxr::NotConformingError,
            ulxr_i18n(ULXR_PCHAR("MultiProcessRpcServer::handleXmlRequest: Content-Length of message not available")), 411);
        }
      }

      else if (state == ulxr::Protocol::ConnBody)
      {
        ULXR_DOUT_XML(ULXR_GET_STRING(std::string(buff_ptr, readed)));
        if (!parser->parse(buff_ptr, readed, done))
        {
          throw ulxr::XmlException(parser->mapToFaultCode(parser->getErrorCode()),
              ulxr_i18n(ULXR_PCHAR("MultiProcessRpcServer::handleXmlRequest: Problem while parsing xml request")),
              parser->getCurrentLineNumber(),
              ULXR_GET_STRING(parser->getErrorString(parser->getErrorCode())));
        }
        readed = 0;
      }
    }
  }
  return cpb->getMethodCall();
}


void  MultiProcessRpcServer::serverLoop()
{
  ULXR_TRACE(ULXR_PCHAR("Main process start"));

  while(m_iState==RUN)
  {
    handleRequest();
  }//while
  terminateAllProcess(true);
}


unsigned  MultiProcessRpcServer::getNumProcesses() const
{
  return m_mapProcesses.size();
}

void  MultiProcessRpcServer::setMaxNumProcesses(unsigned uNumProcesses)
{
  m_maxProcesses=uNumProcesses;
}


unsigned  MultiProcessRpcServer::getMaxNumProcesses() const
{
  return m_maxProcesses;
}


bool MultiProcessRpcServer::terminateAllProcess(bool bForce,long lTimeout)
{
  if(getNumProcesses() == 0)
    return true;

  sweepProcessData();
  std::map<pid_t,ProcessContext> mapProcInfo1=getProcInfo();
  for(std::map<pid_t,ProcessContext>::iterator it=mapProcInfo1.begin(); it!=mapProcInfo1.end();it++)
  {
    if(it->second.iState == FINISH)
      continue;
    if((0 > kill(it->first, SIGTERM)) && (errno!=ESRCH))
        continue;
  }


  if(!waitChildren(lTimeout))
  {
      if(bForce)
      {
      sweepProcessData();
      std::map<pid_t,ProcessContext> mapProcInfo2=getProcInfo();
      for(std::map<pid_t,ProcessContext>::iterator it=mapProcInfo2.begin(); it!=mapProcInfo2.end();it++)
      {
        if(it->second.iState == FINISH)
          continue;
        if((0 > kill(it->first, SIGKILL)) && (errno!=ESRCH))
            continue;
      }

      waitChildren();
      }
      else
      return false;
  }
  return false;
}

void MultiProcessRpcServer::sweepProcessData()
{
  for(std::map<pid_t,ProcessContext>::iterator it=m_mapProcesses.begin(); it!=m_mapProcesses.end();)
  {
    if(it->second.iState == FINISH)
      m_mapProcesses.erase(it++);
    else
      ++it;
  }

}

MultiProcessRpcServer::MultiProcessRpcServer(unsigned /*uNumProcess*/,
                                             bool bHandleSigchld,
                                             bool wbxml_mode)
{
  m_poDispatcher.reset(new ulxr::Dispatcher());

  m_bHandleSigchld=bHandleSigchld;

  if(m_bHandleSigchld)
    sigchild_register();
  m_wbxml_mode=wbxml_mode;
}

MultiProcessRpcServer::MultiProcessRpcServer(ulxr::Protocol* poProtocol,
                                             unsigned /*uNumProcess*/,
                                             bool bHandleSigchld,
                                             bool wbxml_mode)
{
  m_poDispatcher.reset(new ulxr::Dispatcher(poProtocol));

  m_bHandleSigchld=bHandleSigchld; // ea added

  if(m_bHandleSigchld)
    sigchild_register();
  m_wbxml_mode=wbxml_mode;
}

MultiProcessRpcServer::~MultiProcessRpcServer()
{
  terminateAllProcess(false);

  if(m_bHandleSigchld)
    sigchild_unregister();
}

void MultiProcessRpcServer::sigchild_handler(int /*signal*/)
{
  pid_t ppid;
  while((ppid=waitpid(0,0,WNOHANG))>0)
        MultiProcessRpcServer::m_mapProcesses[ppid].iState=FINISH;

}


void  MultiProcessRpcServer::sigchild_register()
{
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler=MultiProcessRpcServer::sigchild_handler;
  sa.sa_flags=SA_NOCLDSTOP | SA_RESTART;
  sigaction(SIGCHLD,&sa,&this->old_sigchld);
}

void  MultiProcessRpcServer::sigchild_unregister()
{
  sigaction(SIGCHLD,&this->old_sigchld,0);
}

ulxr::Dispatcher* MultiProcessRpcServer::getDispatcher() const
{
  return m_poDispatcher.get();
}

void MultiProcessRpcServer::resetDispatcher(ulxr::Protocol* poProtocol)
{
  if(poProtocol != 0)
  {
    m_poDispatcher.reset();
    m_poDispatcher.reset(new ulxr::Dispatcher(poProtocol));
  }

  else
  {
    ulxr::Protocol* tmp_poProtocol=m_poDispatcher->getProtocol();
    m_poDispatcher.reset();
    m_poDispatcher.reset(new ulxr::Dispatcher(tmp_poProtocol));
  }

}

void  MultiProcessRpcServer::setState(int iState)
{
  m_iState=iState;
}


int  MultiProcessRpcServer::getState() const
{
  return m_iState;
}

bool MultiProcessRpcServer::waitConnection(bool bInterruptBySig)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_poDispatcher->getProtocol()->getConnection()->getServerHandle(), &fds);

    int ready;
    while((ready = select(m_poDispatcher->getProtocol()->getConnection()->getServerHandle()+1, &fds, 0, 0, 0)) < 0)
    {
  if(errno == EINTR)
    {
      if(bInterruptBySig)
        return false;
//if was received signal then continue select
              continue;
    }
  else
          throw ulxr::ConnectionException(ulxr::SystemError,
                ULXR_PCHAR("MultiProcessRpcServer::waitConnection: Could not perform select() call: ")
                 + ulxr::getLastErrorString(errno), 500);
    }
    if(0 == ready)
    return false;

    return true;
}

void MultiProcessRpcServer::printProcess() const
{

  std::cout<<"All: "<<getNumProcesses()<<std::endl;
  std::map<pid_t,ProcessContext> mapProcInfo=getProcInfo();
  for(std::map<pid_t,ProcessContext>::iterator it=mapProcInfo.begin(); it!=mapProcInfo.end();it++)
    std::cout<<"Pid: "<<it->first<<" Start time: "<<it->second.timeStart<<" State: "
      <<it->second.iState<<std::endl;

}


void MultiProcessRpcServer::blockSigchld()
{
//check if sigchld already blocked
  sigset_t sigcur_set;
  while(sigprocmask(0, 0, &sigcur_set)<0)
  {
    if(EINTR == errno)
      continue;

     throw MultiProcessRpcServerError(std::string("MultiProcessRpcServer::blockSigchld: sigprocmask error - ") +
                      ulxr::getLatin1(ulxr::getLastErrorString(errno)));
  }

  if(sigismember(&sigcur_set,SIGCHLD) != 0)
    return;

  sigset_t sigchild_set;
  sigemptyset( &sigchild_set );
  sigaddset( &sigchild_set, SIGCHLD);

  while(sigprocmask(SIG_BLOCK, &sigchild_set, 0)<0)
  {
    if(EINTR == errno)
      continue;

     throw MultiProcessRpcServerError(std::string("MultiProcessRpcServer::blockSigchld: sigprocmask error - ") +
                      ulxr::getLatin1(ulxr::getLastErrorString(errno)));
  }

}

void MultiProcessRpcServer::unblockSigchld()
{
//check if sigchld already unblocked
  sigset_t sigcur_set;
  while(sigprocmask(0, 0, &sigcur_set)<0)
  {
    if(EINTR == errno)
      continue;

     throw MultiProcessRpcServerError(std::string("MultiProcessRpcServer::unblockSigchld: sigprocmask error - ") +
                      ulxr::getLatin1(ulxr::getLastErrorString(errno)));
  }

  if(sigismember(&sigcur_set,SIGCHLD) == 0)
    return;

  sigset_t sigchild_set;

  sigemptyset( &sigchild_set );
  sigaddset( &sigchild_set, SIGCHLD);


  while(sigprocmask(SIG_UNBLOCK, &sigchild_set, 0)<0)
  {
    if(EINTR == errno)
      continue;
     throw MultiProcessRpcServerError(std::string("MultiProcessRpcServer::unblockSigchld: sigprocmask error - ") +
                      ulxr::getLatin1(ulxr::getLastErrorString(errno)));
  }
}

void MultiProcessRpcServer::storeProcessData(pid_t pid)
{
  m_mapProcesses[pid]=(ProcessContext){time(0),RUN};
}

std::map<pid_t,MultiProcessRpcServer::ProcessContext> MultiProcessRpcServer::getProcInfo() const
{
  return m_mapProcesses;
}

bool MultiProcessRpcServer::waitChildren(long lTimeout) const
{
    lTimeout*=1000;
    int wait_flag=lTimeout>0?WNOHANG:0;
    long delta=lTimeout>0?lTimeout/4:0;
    while(true)
    {
        int ret,status;
    ret=wait3(&status,wait_flag,0);

    if(ret>0)
        continue;

    if(ret == -1)
    {
        if(errno==EINTR)
        continue;
        if(errno==ECHILD)
        return true;

        throw MultiProcessRpcServerError(std::string("MultiProcessRpcServer::waitChildren: wait3 error - ") +
                        ulxr::getLatin1(ulxr::getLastErrorString(errno)));
    }

    if(ret==0 && lTimeout>0)
    {
        timeval timeout={0,delta};
        int sel_ret= select(0,0,0,0,&timeout);
        if(sel_ret==-1)
        {
          if(errno==EINTR)
                continue;
          else
              throw MultiProcessRpcServerError(std::string("MultiProcessRpcServer::waitChildren: select error - ") +
                          ulxr::getLatin1(ulxr::getLastErrorString(errno)));
        }

        if(sel_ret==0)
        {
          lTimeout-=delta;
          continue;
        }
    }


    return false;
    }
    return false;
}

void MultiProcessRpcServer::storeFuncResult(const ulxr::MethodCall& callMethod,const ulxr::MethodResponse& respMethod) const
{

  std::cout<<"Peer name: ";
  struct sockaddr name;
  socklen_t namelen=sizeof(name);

  if(!getpeername(m_poDispatcher->getProtocol()->getConnection()->getServerHandle(),&name,&namelen)==0)
    std::cout << ulxr::getLatin1(ulxr::getLastErrorString(errno))<<"; ";
  else
  {
#ifdef HAVE_GETNAMEINFO
    char hbuf[NI_MAXHOST],
         sbuf[NI_MAXSERV];

//    if (getnameinfo(&name, name.sa_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),NI_NUMERICSERV)!=0)
    if (getnameinfo(&name, namelen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),NI_NUMERICSERV)!=0)
      std::cout<<"unknow; ";
    else
      std::cout<<hbuf<<":"<<sbuf<<"; ";
#endif
  }
  std::cout<<"Call method: "<<ulxr::getLatin1(callMethod.getMethodName())<<"; ";
  std::cout<<"Method response: "<<(respMethod.isOK()?"ok":"bad")<<std::endl;
}


} // namespace funtik
