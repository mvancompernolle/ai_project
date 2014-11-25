/***************************************************************************
          ulxr_connection.cpp  -  provide a connection for rpc-data
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_connection.cpp 1158 2009-08-30 14:34:24Z ewald-arnold $

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

// #define ULXR_SHOW_TRACE
// #define ULXR_DEBUG_OUTPUT
// #define ULXR_SHOW_READ
// #define ULXR_SHOW_WRITE
// #include <iostream>

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cstring>
#include <memory>
#include <ctype.h>
#include <cstdlib>
#include <cerrno>

#ifdef __WIN32__
#include <winsock2.h>
//#include <windows.h>
#endif

#ifdef __unix__
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/socket.h>

#if defined(ULXR_USE_EPOLL) && defined(HAVE_SYS_EPOLL_H)
#include <sys/epoll.h>
#endif

#endif

#include <csignal>
#include <cstdio>

#include <ulxmlrpcpp/ulxr_connection.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_htmlform_handler.h>


namespace ulxr {


ULXR_API_IMPL(void) Connection::init()
{
  setIsConnecting(false);
  connector = 0;
  ULXR_TRACE(ULXR_PCHAR("init"));
  fd_handle = -1;
  setTimeout(10);
  setConnectionTimeout(0, 0);
#if !defined(__WIN32__) && !defined(_WIN32)
  signal (SIGPIPE, SIG_IGN);  // prevent SIGKILL while write()-ing in closing pipe
#endif
}


ULXR_API_IMPL0 Connection::Connection()
{
  ULXR_TRACE(ULXR_PCHAR("Connection"));
  init();
}


ULXR_API_IMPL0 Connection::~Connection()
{
  ULXR_TRACE(ULXR_PCHAR("~Connection"));
  try
  {
    close();
  }
  catch(...)
  {
    // forget exception?
  }
//  delete connector;
  connector = 0;
}


ULXR_API_IMPL(bool) Connection::isOpen() const
{
  ULXR_TRACE( ((fd_handle >= 0)  ? ULXR_PCHAR("isOpen: true")
                                 : ULXR_PCHAR("isOpen: false")));
  return fd_handle >= 0;
}


ULXR_API_IMPL(ssize_t) Connection::low_level_write(char const *buff, long len)
{
  ULXR_TRACE(ULXR_PCHAR("low_level_write ") << len);

#ifndef __unix__
      return ::send(fd_handle, buff, len, 0);
#else
      return ::write(fd_handle, buff, len);
#endif

}


ULXR_API_IMPL(void) Connection::write(char const *buff, long len)
{
  ULXR_TRACE(ULXR_PCHAR("write ") << len);
  ULXR_DWRITE_WRITE(buff, len);
  ULXR_DOUT_WRITE(ULXR_PCHAR(""));

  long written;

  if (buff == 0 || !isOpen())
    throw RuntimeException(ApplicationError, ulxr_i18n(ULXR_PCHAR("Precondition failed for write() call")));

  if (len == 0)
    return;

#if defined(ULXR_USE_EPOLL) && defined(HAVE_SYS_EPOLL_H)

#ifdef __GNUC__
// #warning ("message using epoll()")
#endif

  while (buff != 0 && len > 0)
  {
    /* Epoll file descriptor */
    int epollfd;

    /* epoll register event structure */
    struct epoll_event ev_write;
    memset(&ev_write,0,sizeof(struct epoll_event));

    /* Create a epoll structure */
    if((epollfd = ::epoll_create(1/* Just a tip to the kernel */)) != -1)
    {
      /* Prepare the event structure to a dummy use of epoll */
      ev_write.events = /*EPOLLET |*/ EPOLLOUT;
      ev_write.data.ptr = NULL;

      /* Check-return var  */
      int ctlresult;

      if((ctlresult = ::epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_handle, &ev_write)) == 0)
      {
        /* epoll receive event structure */
        struct epoll_event ev[1];
        memset(&ev[0],0,sizeof(struct epoll_event));

        int toval = -1;
        if (getTimeout() != 0)
          toval = getTimeout()*1000;

        int waitresult;
        while ((waitresult = epoll_wait(epollfd, ev, 1, toval)) < 0)
        {
          if(errno == EINTR || errno == EAGAIN)
            //if was received signal then continue select
            continue;
          else
          {
//            std::cout << "errno " << errno << std::endl;
            CppString s = getErrorString(getLastError());
            ::close(epollfd);
            throw ConnectionException(SystemError,
                    ulxr_i18n(ULXR_PCHAR("Could not perform epoll_wait() call within write(): ")) + s, 500);
          }
        }

        ::close(epollfd);
        if(waitresult == 0)
        { /* Timeout */
          throw ConnectionException(SystemError,
                  ulxr_i18n(ULXR_PCHAR("Timeout while attempting to write.")), 500);
        }
        else
        {
          if(waitresult == 1)
          { /* It is possible write */
            if ( (written = low_level_write(buff, len)) < 0)
            {
              switch(getLastError())
              {
                case EAGAIN:
                case EINTR:
#ifdef __unix__
                    errno = 0;
#endif
                    continue;

                case EPIPE:
                  close();
                  throw ConnectionException(TransportError,
                          ulxr_i18n(ULXR_PCHAR("Attempt to write to a connection")
                        ULXR_PCHAR(" already closed by the peer")), 500);

                default:
                  throw ConnectionException(SystemError,
                          ulxr_i18n(ULXR_PCHAR("Could not perform low_level_write() call: ")
                        + getErrorString(getLastError())), 500);
              }
            }
            else
            {
              buff += written;
              len -= written;
            }
          }
          else
          { /* Should not reach here */
            throw ConnectionException(SystemError,
                                      ulxr_i18n(ULXR_PCHAR("Problem while attempting to write by epoll.")), 500);
          }
        }
      }
      else
      {
        ::close(epollfd);
        throw ConnectionException(SystemError,
                ulxr_i18n(ULXR_PCHAR("Could not perform epoll_ctl() call: ")), 500);
      }
    }
    else
    {
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Could not perform epoll_create() call: ")), 500);
    }
  }

#else // ULXR_USE_EPOLL

#ifdef __GNUC__
//#warning ("message using select()")
#endif

  fd_set wfd;

  timeval wait;
  wait.tv_sec = getTimeout();
  wait.tv_usec = 0;

  timeval *pwait = 0;
  if (wait.tv_sec != 0)
    pwait = &wait;

  while (buff != 0 && len > 0)
  {
    FD_ZERO(&wfd);
    FD_SET((unsigned) fd_handle, &wfd);
    int ready;
    wait.tv_sec = getTimeout();
    wait.tv_usec = 0;
    while((ready = select(fd_handle+1, 0, &wfd, 0, pwait)) < 0)
    {
      if(errno == EINTR || errno == EAGAIN)
      {
        //if was received signal then continue select
        wait.tv_sec = getTimeout();
        wait.tv_usec = 0;
        continue;
      }
      else
         throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Could not perform select() call: "))
                                     + getErrorString(getLastError()), 500);
    }
    if(ready == 0)
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Timeout while attempting to write.")), 500);

    if(FD_ISSET(fd_handle, &wfd))
    {
      if ( (written = low_level_write(buff, len)) < 0)
      {
        switch(getLastError())
        {
          case EAGAIN:
          case EINTR:
#ifdef __unix__
            errno = 0;
#endif
          continue;

          case EPIPE:
            close();
            throw ConnectionException(TransportError,
                                       ulxr_i18n(ULXR_PCHAR("Attempt to write to a connection")
                                            ULXR_PCHAR(" already closed by the peer")), 500);
          /*break; */

          default:
            throw ConnectionException(SystemError,
                                      ulxr_i18n(ULXR_PCHAR("Could not perform low_level_write() call: ")
                                           + getErrorString(getLastError())), 500);

        }
      }
      else
      {
        buff += written;
        len -= written;
      }
    }
  }


#endif // ULXR_USE_EPOLL

}


ULXR_API_IMPL(bool) Connection::hasPendingInput() const
{
  return false;
}


ULXR_API_IMPL(ssize_t) Connection::low_level_read(char *buff, long len)
{
#ifndef __unix__
    return ::recv(fd_handle, buff, len, 0);
#else
    return ::read(fd_handle, buff, len);
#endif
}


ULXR_API_IMPL(ssize_t) Connection::read(char *buff, long len)
{
  long readed = 0;

  ULXR_TRACE(ULXR_PCHAR("read 1"));

  if (buff == 0 || !isOpen())
    throw RuntimeException(ApplicationError,
                           ulxr_i18n(ULXR_PCHAR("Precondition failed for read() call")));

  ULXR_TRACE(ULXR_PCHAR("read 2 ") << len);

  if (len <= 0)
    return 0;

  ULXR_TRACE(ULXR_PCHAR("read 3"));

#if !(defined(ULXR_USE_EPOLL) && defined(HAVE_SYS_EPOLL_H))

  fd_set rfd;

  timeval wait;
  wait.tv_sec = getTimeout();
  wait.tv_usec = 0;

  timeval *pwait = 0;
  if (wait.tv_sec != 0)
    pwait = &wait;

  FD_ZERO(&rfd);
  FD_SET((unsigned) fd_handle, &rfd);
  int ready;

#endif

  if (hasPendingInput())
  {
    ULXR_TRACE(ULXR_PCHAR("read 3 pending"));
    if( (readed = low_level_read(buff, len)) < 0)
    {
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Could not perform read() call on pending input: "))
                                + getErrorString(getLastError()), 500);
    }
    ULXR_TRACE(ULXR_PCHAR("read pending readed ") + HtmlFormHandler::makeNumber(readed)
                + ULXR_PCHAR(" and wanted ") + HtmlFormHandler::makeNumber(len));
  }

#if defined(ULXR_USE_EPOLL) && defined(HAVE_SYS_EPOLL_H)

  else
  {
    /* Epoll file descriptor */
    int epollfd;

    /* epoll register event structure */
    struct epoll_event ev_read;
    memset(&ev_read,0,sizeof(struct epoll_event));

    /* Create a epoll structure */
    if((epollfd = ::epoll_create(1/* Just a tip to the kernel */)) != -1)
    {
      /* Prepare the event structure to a dummy use of epoll */
      ev_read.events = /*EPOLLET |*/ EPOLLIN;
      ev_read.data.ptr = NULL;

      /* Check-return var  */
      int ctlresult;

      if((ctlresult = ::epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_handle, &ev_read)) == 0)
      {
        /* epoll receive event structure */
        struct epoll_event ev[1];
        memset(&ev[0],0,sizeof(struct epoll_event));

        int toval = -1;
        if (getTimeout() != 0)
          toval = getTimeout()*1000;

        ULXR_TRACE(ULXR_PCHAR("read epoll"));
        int waitresult;
        while ((waitresult = epoll_wait(epollfd, ev, 1, toval)) < 0)
        {
          ULXR_TRACE(ULXR_PCHAR("read epoll err"));
          if(errno == EINTR || errno == EAGAIN)
            //if was received signal then continue select
            continue;
          else
          {
//            std::cout << "errno " << errno << std::endl;
            CppString s = getErrorString(getLastError());
            ::close(epollfd);
            throw ConnectionException(SystemError,
                    ulxr_i18n(ULXR_PCHAR("Could not perform epoll_wait() call within read(): ")) + s, 500);
          }
        }

        ULXR_TRACE(ULXR_PCHAR("read epoll closing with result ") + HtmlFormHandler::makeNumber(waitresult)
                    + ULXR_PCHAR(" and errno ") + HtmlFormHandler::makeNumber(errno));
        ::close(epollfd);
        if(waitresult == 0)
        { /* Timeout */
          throw ConnectionException(SystemError,
                  ulxr_i18n(ULXR_PCHAR("Timeout after ")) + HtmlFormHandler::makeNumber(toval) + ulxr_i18n(ULXR_PCHAR("ms while attempting to read (using epoll).")), 500);
        }
        else
        {
          if(waitresult == 1)
          { /* There is data to read */
            if( (readed = low_level_read(buff, len)) < 0)
            {
              throw ConnectionException(SystemError,
                                        ulxr_i18n(ULXR_PCHAR("Could not perform read() call: "))
                                        + getErrorString(getLastError()), 500);
            }
            ULXR_TRACE(ULXR_PCHAR("read readed ") + HtmlFormHandler::makeNumber(readed)
                        + ULXR_PCHAR(" and wanted ") + HtmlFormHandler::makeNumber(len));
          }
          else
          { /* Should not reach here */
            throw ConnectionException(SystemError,
                                      ulxr_i18n(ULXR_PCHAR("Problem while attempting to read by epoll.")), 500);
          }
        }
      }
      else
      {
        ::close(epollfd);
        throw ConnectionException(SystemError,
                                  ulxr_i18n(ULXR_PCHAR("Could not perform epoll_ctl() call: ")), 500);
      }
    }
    else
    {
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Could not perform epoll_create() call: ")), 500);
    }
  }

#else // ULXR_USE_EPOLL

  else
  {
    ULXR_TRACE(ULXR_PCHAR("read 3a"));

    wait.tv_sec = getTimeout();
    wait.tv_usec = 0;
    while((ready = ::select(fd_handle+1, &rfd, 0, 0, pwait)) < 0)
    {
      ULXR_TRACE(ULXR_PCHAR("read ~select"));
      if(errno == EINTR || errno == EAGAIN)
      {
        //if was received signal then continue select
         wait.tv_sec = getTimeout();
         wait.tv_usec = 0;
         continue;
      }

      else
      {
          ULXR_TRACE(ULXR_PCHAR("read ConnEx"));
          throw ConnectionException(SystemError,
                                    ulxr_i18n(ULXR_PCHAR("Could not perform select() call: "))
                                    + getErrorString(getLastError()), 500);
       }
    }

    ULXR_TRACE(ULXR_PCHAR("read 4"));
    if(ready == 0)
      throw ConnectionException(SystemError,
                                ulxr_i18n(ULXR_PCHAR("Timeout while attempting to read (using select).")), 500);

    ULXR_TRACE(ULXR_PCHAR("read 5"));

    if(FD_ISSET(fd_handle, &rfd))
    {
      while ( (readed = low_level_read(buff, len)) < 0)
      {
        ULXR_TRACE(ULXR_PCHAR("read 6: ") << getErrorString(getLastError()));
        switch(getLastError())
        {
          case EAGAIN:
          case EINTR:
  #ifdef __unix__
            errno = 0;
  #endif
          continue;

          default:
            throw ConnectionException(SystemError,
                                      ulxr_i18n(ULXR_PCHAR("Could not perform read() call: "))
                                      + getErrorString(getLastError()), 500);
        }
      }
    }
  }


#endif // ULXR_USE_EPOLL

  ULXR_TRACE (ULXR_PCHAR("readed ") << readed);
  ULXR_DWRITE_READ(buff, readed);

  // have Content-length field and got unexpected EOF?
  // otherwise caller gets until EOF
  if (readed == 0 /*&& getBytesToRead() >= 0*/)
  {
    ULXR_TRACE (ULXR_PCHAR("readed == 0"));
    close();
    throw ConnectionException(TransportError,
                               ulxr_i18n(ULXR_PCHAR("Attempt to read from a connection")
                               ULXR_PCHAR(" already closed by the peer")), 500);
  }

  return readed;
}


ULXR_API_IMPL(void) Connection::cut()
{
  ULXR_TRACE(ULXR_PCHAR("cut"));
  fd_handle = -1;
}


ULXR_API_IMPL(void) Connection::close()
{
  ULXR_TRACE(ULXR_PCHAR("close"));
  if (isOpen())
  {
#ifndef __unix__
    ULXR_TRACE(ULXR_PCHAR("closesocket"));
    ::closesocket(fd_handle);
#else

    int ret;
    ULXR_TRACE(ULXR_PCHAR("close"));
    do
      ret=::close(fd_handle);
    while(ret < 0 && (errno == EINTR || errno == EAGAIN));

    if(ret < 0)
      throw ConnectionException(TransportError,
                                ULXR_PCHAR("Close failed: ")+getErrorString(getLastError()), 500);
#endif
  }
  fd_handle = -1;
  ULXR_TRACE(ULXR_PCHAR("/close"));
}


ULXR_API_IMPL(int) Connection::getLastError()
{
#ifdef __WIN32__
    return(GetLastError());
#else
    return(errno);
#endif
}


ULXR_API_IMPL(CppString) Connection::getErrorString(int err_number)
{
#ifdef __WIN32__
    LPSTR lpMsgBuf;
    int ok = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        err_number,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPSTR)&lpMsgBuf,
        0,
        NULL);

    if (ok != 0 && lpMsgBuf != 0)
      return ULXR_GET_STRING(lpMsgBuf);
    else
    {
      char s[40];
      CppString errn  = ULXR_GET_STRING(itoa(err_number, s, 10));
      return ulxr_i18n(ULXR_PCHAR("Unknown connection problem, Windows error code: #"))+errn;
    }
#else
    return(getLastErrorString(err_number));
#endif
}


ULXR_API_IMPL(int) Connection::getHandle() const
{
  return fd_handle;
}


ULXR_API_IMPL(void) Connection::setHandle(int handle)
{
  fd_handle = handle;
}


ULXR_API_IMPL(bool) Connection::isConnecting() const
{
  return isconnecting;
}


ULXR_API_IMPL(void) Connection::setIsConnecting(bool connecting)
{
  isconnecting = connecting;
}


ULXR_API_IMPL(void) Connection::doConnect()
{
  ULXR_TRACE(ULXR_PCHAR("doConnect"));
  if(connector != 0)
  {
    ULXR_TRACE(ULXR_PCHAR("doConnect call"));
    setIsConnecting(true);
    try
    {
      connector->call();
    }
    catch(...)
    {
      setIsConnecting(false);
      throw;
    }
    setIsConnecting(false);
}
}


ULXR_API_IMPL0 ConnectorWrapperBase::~ConnectorWrapperBase()
{
}


ULXR_API_IMPL(void) Connection::setConnector(ConnectorWrapperBase *in_connector)
{
  ULXR_TRACE(ULXR_PCHAR("setConnector ") << (void*) in_connector);
  connector = in_connector;
}


ULXR_API_IMPL(void) Connection::setTimeout(unsigned to_sec)
{
  ULXR_TRACE(ULXR_PCHAR("current timeout ") << to_sec);
  current_to = to_sec;
}


ULXR_API_IMPL(void) Connection::setConnectionTimeout(unsigned def_to_sec, unsigned alive_to_sec)
{
  ULXR_TRACE(ULXR_PCHAR("connection timeout ") << def_to_sec << ULXR_PCHAR(" ") << alive_to_sec);
  default_to = def_to_sec;
  persist_to = alive_to_sec;
}


ULXR_API_IMPL(unsigned) Connection::getTimeout() const
{
  return current_to;
}


ULXR_API_IMPL(unsigned) Connection::getDefaultTimeout() const
{
  if (default_to == 0)
    return current_to;
  else
    return default_to;
}


ULXR_API_IMPL(unsigned) Connection::getPersistentTimeout() const
{
  if (persist_to == 0)
    return current_to;
  else
    return persist_to;
}


}  // namespace ulxr
