/***************************************************************************
       ssl_connection.cpp  -  ssl connection with certificate support
                             -------------------
    begin                : Sun May 29 2005
    copyright            : (C) 2005 Dmitry Nizovtsev <funt@alarit.com>
                                    Olexander Shtepa <isk@alarit.com>

    $Id: ssl_connection.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

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

#ifdef ULXR_INCLUDE_SSL_STUFF

#include <ulxmlrpcpp/contrib/ssl_connection.h>

#include <openssl/err.h>

//STL
#include <deque>
#include <iostream>

#include <errno.h>


namespace funtik {


ULXR_API_IMPL0 SSLConnectionException::SSLConnectionException():
	ulxr::ConnectionException(ulxr::SystemError,ULXR_PCHAR("SSL error"),500)
{
	this->_what+="SSLError:";
    this->_what+=get_error_queue();
}

ULXR_API_IMPL0 SSLConnectionException::SSLConnectionException(ulxr::CppString strPhrase, int stat):
	ulxr::ConnectionException(ulxr::SystemError,ULXR_PCHAR("SSL error"),stat)
{
	this->_what+=ulxr::getLatin1(strPhrase);
}

ULXR_API_IMPL0 SSLConnectionException::SSLConnectionException(SSL *ssl, int ret_code):
	ulxr::ConnectionException(ulxr::SystemError,ULXR_PCHAR("SSL error"),500)
{
	this->_what+="SSLError:";
	int err=SSL_get_error(ssl,ret_code);
    switch (err)
	{
		case SSL_ERROR_NONE:
        this->_what+=" SSL_ERROR_NONE";
                break;
        case SSL_ERROR_ZERO_RETURN:
                this->_what+=" SSL_ERROR_ZERO_RETURN";
                break;
        case SSL_ERROR_WANT_READ:
                this->_what+=" SSL_ERROR_WANT_READ";
                break;
        case SSL_ERROR_WANT_WRITE:
                this->_what+=" SSL_ERROR_WANT_WRITE";
                break;
        case SSL_ERROR_WANT_CONNECT:
                this->_what+=" SSL_ERROR_WANT_CONNECT";
                break;

#ifdef ULXR_HAVE_SSL_ERROR_WANT_ACCEPT
        case SSL_ERROR_WANT_ACCEPT:
                this->_what+=" SSL_ERROR_WANT_ACCEPT";
                break;
#endif

        case SSL_ERROR_WANT_X509_LOOKUP:
                this->_what+=" SSL_ERROR_WANT_X509_LOOKUP";
                break;
        case SSL_ERROR_SYSCALL:
                {
                        std::string error_queue=get_error_queue();
                        if ( !error_queue.empty())
                                this->_what+=error_queue;
                        else
                        {
                                this->_what+=" ";
                                if (ret_code==0)
                                        this->_what+="an EOF was observed";
                                else if (ret_code==-1)
                                  this->_what+=ulxr::getLatin1(ulxr::getLastErrorString(errno));
                                else
                                        this->_what+="unknown error";
                        }
                }
                break;
        case SSL_ERROR_SSL:
                this->_what+=get_error_queue();
                break;
        default:
                this->_what+=" unknown error code";
        }

}

ULXR_API_IMPL(std::string) SSLConnectionException::get_error_queue()
{
	typedef std::deque<unsigned long> __list_type;

    __list_type errors;
    unsigned long error_code;
    char buf[120];
    std::string __ret;

    while ((error_code=ERR_get_error())!=0)
                errors.push_front(error_code);
	for (__list_type::const_iterator error=errors.begin();error!=errors.end();++error)
	{
		__ret+="\n";
		ERR_error_string_n(*error,buf,sizeof(buf));
		__ret+=buf;
	}
	return __ret;
}

ULXR_API_IMPL(ulxr::CppString) SSLConnectionException::why() const
{
#ifdef ULXR_UNICODE
  return ulxr::getUnicode(_what.c_str());
#else
  return _what.c_str();
#endif
}

ULXR_API_IMPL(const char *) SSLConnectionException::what() const throw()
{
	return _what.c_str();
}

static int s_server_session_id_context      = 1;
static int s_server_auth_session_id_context = 2;

bool SSLConnection::ssl_initialized = false;



static int password_cb(char *buf,int num, int /*rwflag*/, void *userdata)
{
	ULXR_TRACE(ULXR_PCHAR("password_cb"));
	SSLConnection *conn = (SSLConnection *)userdata;
	std::string pass = conn->getPassword();

	if((unsigned int)num < pass.length()+1)
	    return 0;

	strncpy(buf, pass.c_str(),num);
	return(strlen(buf));
}






ULXR_API_IMPL0 SSLConnection::SSLConnection(bool I_am_server, const ulxr::CppString &domain, unsigned port)
 : ulxr::TcpIpConnection(I_am_server, domain, port)
{
	ULXR_TRACE(ULXR_PCHAR("SSLConnection"));
	init();
}


ULXR_API_IMPL0 SSLConnection::SSLConnection(bool I_am_server, long adr, unsigned port)
 : ulxr::TcpIpConnection(I_am_server, adr, port)
{
	ULXR_TRACE(ULXR_PCHAR("SSLConnection"));
	init();
}


ULXR_API_IMPL(void)
  SSLConnection::setCryptographyData (const std::string &in_password,
                                      const std::string &in_certfile,
                                      const std::string &in_keyfile)
{

	m_strPassword = in_password;

	if(in_certfile.size()>0)
	{
		if (SSL_CTX_use_certificate_file(ssl_ctx, in_certfile.c_str(), SSL_FILETYPE_PEM) <= 0)
    		throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("SSLConnection::setCryptographyData: problem setting up certificate from file: ")+ULXR_GET_STRING(in_certfile)), 500);
   		m_strCertFileName = in_certfile;
	}

	if(in_keyfile.size()>0)
	{
		if (SSL_CTX_use_PrivateKey_file(ssl_ctx, in_keyfile.c_str(), SSL_FILETYPE_PEM) <= 0)
			throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("SSLConnection::setCryptographyData: problem setting up key from file: ")+ULXR_GET_STRING(in_keyfile)), 500);
		m_strKeyFileName = in_keyfile;
	}
}




ULXR_API_IMPL(void) SSLConnection::initializeCTX()
{
	ULXR_TRACE(ULXR_PCHAR("initializeCTX"));

//free context
	if (ssl_ctx != 0)
    	    SSL_CTX_free(ssl_ctx);

	SSL_METHOD *meth = SSLv23_method();

	ssl_ctx = SSL_CTX_new (meth);

//!!!
	if (!ssl_ctx)
		throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("Cann`t initialize CTX context")), 500);


	SSL_CTX_set_default_passwd_cb(ssl_ctx, password_cb);
	SSL_CTX_set_default_passwd_cb_userdata(ssl_ctx, this);


	ssl = 0;

	if (isServerMode())
	{
		if (0 >= SSL_CTX_set_session_id_context(ssl_ctx,
		(const unsigned char *)&s_server_session_id_context,
		sizeof s_server_session_id_context))
		{
			throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("Context session error")), 500);

    	}
    }
}


ULXR_API_IMPL(void) SSLConnection::init()
{
	ULXR_TRACE(ULXR_PCHAR("init"));
	session = 0;
	m_iAuthType=0;
	ssl_ctx=0;
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
	close();

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

	if (!isServerMode()) // clients keeps session
	{
		if (0 != session)
		{
			SSL_SESSION_free(session);
			session=0;
		}
		if(0 != ssl)
		    session = SSL_get1_session(ssl);
	}

	ULXR_TRACE(ULXR_PCHAR("close 2"));

	ulxr::TcpIpConnection::close();

	if (ssl != 0)
        	SSL_free(ssl);
	ssl = 0;
}


ULXR_API_IMPL(ssize_t) SSLConnection::low_level_write(char const *buff, long len)
{
	ULXR_TRACE(ULXR_PCHAR("low_level_write"));

  if (isConnecting())
    return TcpIpConnection::low_level_write(buff, len);

	ssize_t ret;
	while (true)
	{
		ULXR_TRACE(ULXR_PCHAR("low_level_write 2"));
		ret = SSL_write(ssl, buff, len);
		ULXR_TRACE(ULXR_PCHAR("low_level_write 3 ") << ret);
		if (ret >= 0)
			break;
		else
			throw SSLConnectionException(ssl, ret);

	}
	return ret;
}


ULXR_API_IMPL(bool) SSLConnection::hasPendingInput() const
{
  if (isConnecting())
    return TcpIpConnection::hasPendingInput();

	int avail = SSL_pending(ssl);
	ULXR_TRACE(ULXR_PCHAR("hasPendingInput ") << avail);
	return avail != 0;
}


ULXR_API_IMPL(ssize_t) SSLConnection::low_level_read(char *buff, long len)
{
	ULXR_TRACE(ULXR_PCHAR("low_level_read"));

  if (isConnecting())
    return TcpIpConnection::low_level_read(buff, len);

	ssize_t ret;
	while (true)
	{
		ULXR_TRACE(ULXR_PCHAR("low_level_read 2"));
		ret = SSL_read(ssl, buff, len);
		ULXR_TRACE(ULXR_PCHAR("low_level_read 3 ") << ret);
		if (ret >= 0)
			break;
      	else
	       throw SSLConnectionException(ssl, ret);
	}
  return ret;
}


ulxr::Connection *ULXR_API_IMPL0 SSLConnection::detach()
{
	ULXR_TRACE(ULXR_PCHAR("detach"));
	SSLConnection *clone = new SSLConnection(*this);
//reset ssl_ctx,ssl,session old pointers will be stored in clone object
	ssl_ctx=0;
	ssl=0;
	session=0;

//init new CTX
	cut();

	clone->setServerData(getServerData()); // from TcpIpConnection
	if (getServerData() != 0)
		getServerData()->incRef();

	return clone;
}


ULXR_API_IMPL(void) SSLConnection::createSSL()
{
	ULXR_TRACE(ULXR_PCHAR("createSSL"));
	if(ssl != 0)
	    SSL_free(ssl);
	ssl = SSL_new (ssl_ctx);
	if (ssl == 0)
		throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("problem creating SSL conext object")), 500);

	int err = SSL_set_fd (ssl, getHandle());
	if (err == 0)
		throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("problem set file descriptor for SSL")), 500);

	if (isServerMode())
	{
		if (0 >= SSL_set_session_id_context(ssl,
		(const unsigned char *)&s_server_auth_session_id_context,
		sizeof(s_server_auth_session_id_context)))
		{
			throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("Context session error")), 500);

		}
	}
}


ULXR_API_IMPL(void) SSLConnection::open()
{
	ULXR_TRACE(ULXR_PCHAR("open"));

	ulxr::TcpIpConnection::open();

  doConnect();  // CONNECT in non-SSL mode!

	int err;
	createSSL();

	if (0 != session)
	{
		ULXR_TRACE(ULXR_PCHAR("SSL_set_session"));
		SSL_set_session(ssl, session);
	}

	err = SSL_connect (ssl);
	X509 *peer_cert=0;
	try{
		if (err <= 0)
			throw SSLConnectionException(ssl, err);

		peer_cert=SSL_get_peer_certificate(ssl);
		if(!checkAccess(peer_cert))
			throw SSLConnectionException(ULXR_PCHAR("Fault fingerprint of certificate. Access denied."),500);

		if(peer_cert!=0)
			X509_free(peer_cert);
	}
	catch(SSLConnectionException& ex)
	{
//sweep all
		if(peer_cert!=0)
			X509_free(peer_cert);
	    close();
	    throw;
	}
}


ULXR_API_IMPL(void) SSLConnection::accept()
{
	ULXR_TRACE(ULXR_PCHAR("accept"));
//Set SSL context before every connect. Authentification options could be changed.
	ulxr::TcpIpConnection::accept();
	createSSL();

	int err = SSL_accept (ssl);
	X509 *peer_cert=0;
	try{
	    if (err <= 0)
			throw SSLConnectionException(ssl, err);
//check access
		peer_cert=SSL_get_peer_certificate(ssl);
   	    if(!checkAccess(peer_cert))
			throw SSLConnectionException(ULXR_PCHAR("Fault fingerprint of certificate. Access denied."),500);

		if(peer_cert!=0)
			X509_free(peer_cert);
	}
	catch(SSLConnectionException& ex)
	{
//sweep all
		if(peer_cert!=0)
			X509_free(peer_cert);
	    close();
	    throw;
	}
  /* Get the cipher - opt */
	ULXR_TRACE(ULXR_PCHAR("SSL connection using ") << ULXR_GET_STRING(SSL_get_cipher (ssl)));
}


ULXR_API_IMPL(ulxr::CppString) SSLConnection::getInterfaceName()
{
  ULXR_TRACE(ULXR_PCHAR("getInterfaceName"));
  return ULXR_PCHAR("ssl");
}



ULXR_API_IMPL(void) SSLConnection::cut()
{
	ULXR_TRACE(ULXR_PCHAR("cut"));
	ulxr::TcpIpConnection::cut();
	initializeCTX();
}


ULXR_API_IMPL(std::string) SSLConnection::getPassword() const
{
	return m_strPassword;
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

ULXR_API_IMPL(void ) SSLConnection::setPassword(const std::string &strPassword)
{
	m_strPassword=strPassword;
}

ULXR_API_IMPL(void) SSLConnection::setCertFile(const std::string &strCertFileName)
{

//
	if (SSL_CTX_use_certificate_file(ssl_ctx, strCertFileName.c_str(), SSL_FILETYPE_PEM) <= 0)
    	throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("SSLConnection::setCertFile: problem setting up certificate from file: ")+ULXR_GET_STRING(strCertFileName)), 500);
    m_strCertFileName=strCertFileName;
}

ULXR_API_IMPL(void) SSLConnection::setKeyFile(const std::string &strKeyFileName)
{
	if (SSL_CTX_use_PrivateKey_file(ssl_ctx, strKeyFileName.c_str(), SSL_FILETYPE_PEM) <= 0)
		throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("SSLConnection::setKeyFile: problem setting up key from file: ")+ULXR_GET_STRING(strKeyFileName)), 500);
	m_strKeyFileName=strKeyFileName;
}

ULXR_API_IMPL(std::string) SSLConnection::getKeyFileName() const
{
	return 	m_strKeyFileName;
}

ULXR_API_IMPL(std::string) SSLConnection::getCertFileName() const
{
	return m_strCertFileName;
}



ULXR_API_IMPL(void ) SSLConnection::disableAuth(int iType)
{
	if ( 0 == iType )
	{
		m_iAuthType=0;
	}
	else
	{
		m_iAuthType^=iType;
	}

	activateAuth();
}

ULXR_API_IMPL(void) SSLConnection::enableAuth(int iType)
{
	m_iAuthType|=iType;

	activateAuth();
}

ULXR_API_IMPL(void) SSLConnection::setCAFile(const std::string &strCAFileName)
{
	m_strCAFileName=strCAFileName;

	if (!SSL_CTX_load_verify_locations(ssl_ctx, m_strCAFileName.c_str(),NULL))
        throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("TLS engine: cannot load list of CA data from: ")+ULXR_GET_STRING(m_strCAFileName)), 500);


	  	/* SSL_load_client_CA_file is a misnomer, it just creates a list of CNs. */

   	STACK_OF(X509_NAME)* cert_names=SSL_load_client_CA_file(m_strCAFileName.c_str());
   	if(cert_names == 0)
   		throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("TLS engine: cannot load list of CA data from: ")+ULXR_GET_STRING(m_strCAFileName)), 500);

	SSL_CTX_set_client_CA_list(ssl_ctx, cert_names);

        /* SSL_CTX_set_client_CA_list does not have a return value;
         * it does not really need one, but make sure
         * (we really test if SSL_load_client_CA_file worked) */

	if (SSL_CTX_get_client_CA_list(ssl_ctx) == 0)
	    		throw SSLConnectionException(ulxr_i18n(ULXR_PCHAR("Could not set client CA list from: ")+ULXR_GET_STRING(m_strCAFileName)), 500);


}


ULXR_API_IMPL(void ) SSLConnection::addFingerprintData(const std::string  &strFingerprint)
{
	m_mapFingerpintData[strFingerprint]=1;
}

ULXR_API_IMPL(void) SSLConnection::addRevocationData(const std::string &strRevocation)
{
	m_mapRevocationData[strRevocation]=1;
}

ULXR_API_IMPL(void) SSLConnection::resetFingerprintData()
{
	m_mapFingerpintData.clear();
}

ULXR_API_IMPL(void) SSLConnection::resetRevocationData()
{
	m_mapRevocationData.clear();
}

//use algoritm for calculate fingerprint like postfix
ULXR_API_IMPL(std::string) SSLConnection::calculateFingerprint(X509 * poCertificate) const
{
	unsigned int n;
	unsigned char md[EVP_MAX_MD_SIZE];
	char fingerprint[EVP_MAX_MD_SIZE * 3];
	static const char hexcodes[] = "0123456789ABCDEF";

	if (X509_digest(poCertificate, EVP_md5(), md, &n))
	{
		for (int j = 0; j < (int) n; j++)
		{
			fingerprint[j * 3] =
                hexcodes[(md[j] & 0xf0) >> 4];
			fingerprint[(j * 3) + 1] =
                        hexcodes[(md[j] & 0x0f)];
			if (j + 1 != (int) n)
            	    fingerprint[(j * 3) + 2] = ':';
			else
                    fingerprint[(j * 3) + 2] = '\0';
 		}
	}
    return std::string(fingerprint);
}


ULXR_API_IMPL(bool) SSLConnection::checkFingerprint(std::string strFingerprint) const
{

	if(m_mapFingerpintData.find(strFingerprint) == m_mapFingerpintData.end())
        return false;

	return true;
}


ULXR_API_IMPL(bool) SSLConnection::checkRevocationFingerprint(std::string strFingerprint) const
{
	if(m_mapRevocationData.find(strFingerprint) == m_mapRevocationData.end())
        return false;

	return true;
}

ULXR_API_IMPL(bool) SSLConnection::checkAccess(X509 * poCertificate)
{
	bool bReturn=true;
	std::string strCertFingerprint="";


//FINGERPRINT_AUTH
	if((0 !=(m_iAuthType&SSLConnection::FINGERPRINT_AUTH)) && bReturn)
	{
		if(poCertificate == NULL)
			bReturn=false;
		else
		{
			strCertFingerprint=strCertFingerprint.size()?strCertFingerprint:calculateFingerprint(poCertificate);
			if(checkFingerprint(strCertFingerprint))
				bReturn=true;
			else bReturn=false;
		}
	}

//CHECK_REVOCATIONCERT
	if((0 !=(m_iAuthType&SSLConnection::CHECK_REVOCATIONCERT)) && bReturn)
	{
		if(poCertificate == NULL)
			bReturn=false;
		else
		{
			strCertFingerprint=strCertFingerprint.size()?strCertFingerprint:calculateFingerprint(poCertificate);
			if(!checkRevocationFingerprint(strCertFingerprint))
				bReturn=true;
			else bReturn=false;
		}
	}
	return bReturn;
}

ULXR_API_IMPL(void) SSLConnection::activateAuth()
{
//	if(0 !=(m_iAuthType&SSLConnection::CA_AUTH) && ssl_ctx)
	if(0 !=m_iAuthType && ssl_ctx)
	{
		/* Set options*/
		SSL_CTX_set_verify(ssl_ctx,SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE|SSL_VERIFY_FAIL_IF_NO_PEER_CERT,NULL);
	}
	else
	{
		/* Set options*/
		SSL_CTX_set_verify(ssl_ctx,SSL_VERIFY_NONE,NULL);
	}
//if we don`t use
}


} // namespace funtik


#endif // ULXR_INCLUDE_SSL_STUFF




