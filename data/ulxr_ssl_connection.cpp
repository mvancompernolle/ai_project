/***************************************************************************
                ulxr_ssl_connection.cpp  -  ssl connection
                             -------------------
    begin                : Mon May 3 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_ssl_connection.cpp 1063 2007-08-19 11:46:20Z ewald-arnold $

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

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#ifdef ULXR_INCLUDE_SSL_STUFF

#include <openssl/err.h>
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_except.h>


static int s_server_session_id_context      = 1;
static int s_server_auth_session_id_context = 2;

namespace ulxr {


bool SSLConnection::ssl_initialized = false;


static int password_cb(char *buf,int num, int /*rwflag*/, void *userdata)
{
  ULXR_TRACE(ULXR_PCHAR("password_cb"));
  SSLConnection *conn = (SSLConnection *)userdata;
  std::string pass = conn->getPassword();

  if(num < (int)pass.length()+1)
    return 0;

  strcpy(buf, pass.c_str());
  return(strlen(buf));
}


ULXR_API_IMPL0 SSLConnection::SSLConnection(bool I_am_server, const CppString &domain, unsigned port)
 : TcpIpConnection(I_am_server, domain, port)
{
  ULXR_TRACE(ULXR_PCHAR("SSLConnection"));
  init();
}


ULXR_API_IMPL0 SSLConnection::SSLConnection(bool I_am_server, long adr, unsigned port)
 : TcpIpConnection(I_am_server, adr, port)
{
  ULXR_TRACE(ULXR_PCHAR("SSLConnection"));
  init();
}


ULXR_API_IMPL(void)
  SSLConnection::setCryptographyData (const std::string &in_password,
                                      const std::string &in_certfile,
                                      const std::string &in_keyfile)
{
  password = in_password;
  keyfile = in_keyfile;
  certfile = in_certfile;
}


ULXR_API_IMPL(void) SSLConnection::initializeCTX()
{
  ULXR_TRACE(ULXR_PCHAR("initializeCTX"));
  SSL_METHOD *meth = SSLv23_method();
  ssl_ctx = SSL_CTX_new (meth);
  if (!ssl_ctx)
  {
    ERR_print_errors_fp(stderr);
    exit(2);
  }

  SSL_CTX_set_default_passwd_cb(ssl_ctx, password_cb);
  SSL_CTX_set_default_passwd_cb_userdata(ssl_ctx, this);

  ssl = 0;

  if (isServerMode())
  {
    if (0 >= SSL_CTX_set_session_id_context(ssl_ctx,
      (const unsigned char *)&s_server_session_id_context,
      sizeof s_server_session_id_context))
    {
      ERR_print_errors_fp(stderr);
      exit(2);
    }
  }
}


ULXR_API_IMPL(void) SSLConnection::init()
{
  ULXR_TRACE(ULXR_PCHAR("init"));
  session = 0;

  if (!ssl_initialized)
  {
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    ssl_initialized = true;
  }

  initializeCTX();
}


ULXR_API_IMPL0 SSLConnection::~SSLConnection()
{
  ULXR_TRACE(ULXR_PCHAR("~SSLConnection"));
  if (ssl_ctx != 0)
    SSL_CTX_free(ssl_ctx);
  ssl_ctx = 0;

  ULXR_TRACE(ULXR_PCHAR("~SSLConnection 2"));

  if (0 != session)
    SSL_SESSION_free(session);
  session = 0;
}


ULXR_API_IMPL(void) SSLConnection::close()
{
  ULXR_TRACE(ULXR_PCHAR("close"));

  if (!isServerMode()) // clients keep session
  {
    if (0 != session)
      SSL_SESSION_free(session);

    session = SSL_get1_session(ssl);
  }

  ULXR_TRACE(ULXR_PCHAR("close 2"));

  TcpIpConnection::close();
  if (ssl != 0)
    SSL_free(ssl);
  ssl = 0;
}


ULXR_API_IMPL(ssize_t) SSLConnection::low_level_write(char const *buff, long len)
{
  ULXR_TRACE(ULXR_PCHAR("low_level_write"));
  ssize_t ret;

  if (isConnecting())
    return TcpIpConnection::low_level_write(buff, len);

  while (true)
  {
    ULXR_TRACE(ULXR_PCHAR("low_level_write 2"));
    ret = SSL_write(ssl, buff, len);
    ULXR_TRACE(ULXR_PCHAR("low_level_write 3 ") << ret);

    if (ret >= 0)
      break;

    ULXR_TRACE(ULXR_PCHAR("low_level_write 4"));
    switch (SSL_get_error(ssl, ret))
    {
      case SSL_ERROR_NONE:
        ULXR_TRACE(ULXR_PCHAR("SSL_ERROR_NONE"));
        break;

      case SSL_ERROR_WANT_WRITE:
        ULXR_TRACE(ULXR_PCHAR("SSL_ERROR_WANT_WRITE"));
        continue;

      default:
        ULXR_TRACE(ULXR_PCHAR("default"));
        throw ConnectionException(SystemError,
                                  ulxr_i18n(ULXR_PCHAR("Could not perform SSL_write() call: ")), 500);
    }
  }
  ULXR_TRACE(ULXR_PCHAR("/low_level_write ") << ret);
  return ret;
}


ULXR_API_IMPL(bool) SSLConnection::hasPendingInput() const
{
  ULXR_TRACE(ULXR_PCHAR("hasPendingInput "));

  if (isConnecting())
    return TcpIpConnection::hasPendingInput();

  int avail = SSL_pending(ssl);
  ULXR_TRACE(ULXR_PCHAR("hasPendingInput ") << avail);
  return avail != 0;
}


ULXR_API_IMPL(ssize_t) SSLConnection::low_level_read(char *buff, long len)
{
  ULXR_TRACE(ULXR_PCHAR("low_level_read"));
  ssize_t ret;

  if (isConnecting())
    return TcpIpConnection::low_level_read(buff, len);

  while (true)
  {
    ULXR_TRACE(ULXR_PCHAR("low_level_read 2"));
    ret = SSL_read(ssl, buff, len);
    ULXR_TRACE(ULXR_PCHAR("low_level_read 3 ") << ret);

    if (ret >= 0)
      break;

    ULXR_TRACE(ULXR_PCHAR("low_level_read 4"));
    switch (SSL_get_error(ssl, ret))
    {
      case SSL_ERROR_NONE:
        ULXR_TRACE(ULXR_PCHAR("SSL_ERROR_NONE"));
        break;

      case SSL_ERROR_WANT_READ:
        ULXR_TRACE(ULXR_PCHAR("SSL_ERROR_WANT_READ"));
        continue;

      default:
        ULXR_TRACE(ULXR_PCHAR("default"));
        throw ConnectionException(SystemError,
                                  ulxr_i18n(ULXR_PCHAR("Could not perform SSL_read() call: ")), 500);
    }
  }
  ULXR_TRACE(ULXR_PCHAR("/low_level_read ") << ret);
  return ret;
}


TcpIpConnection *ULXR_API_IMPL0 SSLConnection::makeClone()
{
  return new SSLConnection(*this); // shallow copy !!
}


ULXR_API_IMPL(void) SSLConnection::createSSL()
{
  ULXR_TRACE(ULXR_PCHAR("createSSL"));
  ssl = SSL_new (ssl_ctx);
  if (ssl == 0)
    throw ConnectionException(SystemError,
                          ulxr_i18n(ULXR_PCHAR("problem creating SSL conext object")), 500);

  int err = SSL_set_fd (ssl, getHandle());
  if (err == 0)
    throw ConnectionException(SystemError,
                          ulxr_i18n(ULXR_PCHAR("problem set file descriptor for SSL")), 500);

  if (isServerMode())
  {
    if (0 >= SSL_set_session_id_context(ssl,
      (const unsigned char *)&s_server_auth_session_id_context,
      sizeof(s_server_auth_session_id_context)))
    {
      ERR_print_errors_fp(stderr);
      exit(2);
    }
  }
}


ULXR_API_IMPL(void) SSLConnection::open()
{
  ULXR_TRACE(ULXR_PCHAR("open"));

  TcpIpConnection::open();

  doConnect();  // CONNECT in non-SSL mode!

  int err;
  createSSL();

  if (0 != session)
  {
    ULXR_TRACE(ULXR_PCHAR("SSL_set_session"));
    SSL_set_session(ssl, session);
  }

  err = SSL_connect (ssl);
  if (err == 0)
    throw ConnectionException(SystemError,
                          ulxr_i18n(ULXR_PCHAR("problem starting SSL connection (client mode)")), 500);
}


ULXR_API_IMPL(bool) SSLConnection::accept(int in_timeout)
{
  ULXR_TRACE(ULXR_PCHAR("accept"));

  if (SSL_CTX_use_certificate_file(ssl_ctx, certfile.c_str(), SSL_FILETYPE_PEM) <= 0) {
//    ERR_print_errors_fp(stderr);
    throw ConnectionException(SystemError,
                          ulxr_i18n(ULXR_PCHAR("problem setting up certificate")), 500);
  }

  if (SSL_CTX_use_PrivateKey_file(ssl_ctx, keyfile.c_str(), SSL_FILETYPE_PEM) <= 0)
  {
//    ERR_print_errors_fp(stderr);
    throw ConnectionException(SystemError,
                          ulxr_i18n(ULXR_PCHAR("problem setting up private key")), 500);
  }


  if (!TcpIpConnection::accept(in_timeout))
    return false;

  createSSL();
  int err = SSL_accept (ssl);
  if (err == 0)
    throw ConnectionException(SystemError,
                          ulxr_i18n(ULXR_PCHAR("problem starting SSL connection (server mode)")), 500);

  /* Get the cipher - opt */
  ULXR_TRACE(ULXR_PCHAR("SSL connection using ") << ULXR_GET_STRING(SSL_get_cipher (ssl)));
  return true;
}


ULXR_API_IMPL(CppString) SSLConnection::getInterfaceName()
{
  ULXR_TRACE(ULXR_PCHAR("getInterfaceName"));
  return ULXR_PCHAR("ssl");
}


ULXR_API_IMPL(void) SSLConnection::cut()
{
  ULXR_TRACE(ULXR_PCHAR("cut"));
  TcpIpConnection::cut();
  initializeCTX();
}


ULXR_API_IMPL(std::string) SSLConnection::getPassword() const
{
  return password;
}


ULXR_API_IMPL(SSL *) SSLConnection::getSslObject() const
{
  return ssl;
}


ULXR_API_IMPL(SSL_CTX *) SSLConnection::getSslContextObject() const
{
  return ssl_ctx;
}


ULXR_API_IMPL(SSL_SESSION *) SSLConnection::getSslSessionObject() const
{
  return session;
}


}  // namespace ulxr

#endif // ULXR_INCLUDE_SSL_STUFF
