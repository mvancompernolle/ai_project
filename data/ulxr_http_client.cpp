/***************************************************************************
                ulxr_http_client.cpp  -  a simple http client
                             -------------------
    begin                : Sam Apr 20 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_http_client.cpp 1078 2007-09-04 17:18:47Z ewald-arnold $

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

//#define ULXR_SHOW_TRACE
//#define ULXR_DEBUG_OUTPUT
//#define ULXR_SHOW_READ
//#define ULXR_SHOW_WRITE

#ifndef ULXR_OMIT_TCP_STUFF

#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>

#if !defined(__BORLANDC__) && !defined(_MSC_VER)
#include <unistd.h>
#endif

#include <cctype>
#include <stdio.h>
#include <sys/stat.h>

#include <fstream>

#include <ulxmlrpcpp/ulxr_http_client.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>


#ifdef feof  // remove legacy stuff
#undef feof
#endif


namespace ulxr {


ULXR_API_IMPL(void) BodyProcessor::process(const char * /* buffer */, unsigned /* len */)
{
}


ULXR_API_IMPL0 BodyProcessor::~BodyProcessor()
{
}


/////////////////////////////////////////////////////////////////////


ULXR_API_IMPL0 HttpClient::HttpClient (HttpProtocol* prot)
{
  ULXR_TRACE(ULXR_PCHAR("HttpClient(HttpProtocol*)"));
  protocol = prot;
  protocol->setChunkedTransfer(false);
}


ULXR_API_IMPL(void) HttpClient::interpreteHttpHeader()
{
   ULXR_TRACE(ULXR_PCHAR("interpreteHttpHeader"));
   head_version = ULXR_PCHAR("");
   head_status = 500;
   head_phrase = ULXR_PCHAR("Internal error");
   protocol->splitHeaderLine(head_version, head_status, head_phrase);
   protocol->setPersistent(!protocol->determineClosing(head_version));
}


ULXR_API_IMPL(void) HttpClient::receiveResponse(BodyProcessor &proc)
{
  ULXR_TRACE(ULXR_PCHAR("receiveResponse"));
  protocol->resetConnection();

  char buffer[ULXR_RECV_BUFFER_SIZE];
  char *buff_ptr;

  bool done = false;
  long readed;
  while (!done && ((readed = protocol->readRaw(buffer, sizeof(buffer))) > 0) )
  {
    buff_ptr = buffer;

    if (!protocol->hasBytesToRead())
      done = true;

    while (readed > 0)
    {
      Protocol::State state = protocol->connectionMachine(buff_ptr, readed);
      if (state == Protocol::ConnError)
        throw ConnectionException(TransportError,
                                  ulxr_i18n(ULXR_PCHAR("network problem occured")), 500);

      // switch to appropriate method when header is completely read
      else if (   state == Protocol::ConnSwitchToBody
               || state == Protocol::ConnBody)
      {
        interpreteHttpHeader();
        proc.process(buff_ptr, readed);
        readed = 0;
      }
    }
  }
}


/** Helper class to store the message body in a string.
  */
class StringProcessor : public BodyProcessor
{
 public:
   StringProcessor(Cpp8BitString &str_ref)
      : target(str_ref) {}

 public:
   virtual void process(const char *buffer, unsigned len)
   {
      target.append(buffer, len);
   }

 private:
   Cpp8BitString &target;
};


ULXR_API_IMPL(void) HttpClient::msgPUT(const Cpp8BitString &msg, const CppString &type,
                                    const CppString &resource)
{
  ULXR_TRACE(ULXR_PCHAR("msgPUT"));

  if (!protocol->isOpen() )
    protocol->open();

  sendAuthentication();
  protocol->sendRequestHeader(ULXR_PCHAR("PUT"), resource, type, msg.length());
#ifdef ULXR_USE_WXSTRING
  protocol->writeBody(msg.data(), msg.length());
#else
  protocol->writeBody(msg.data(), msg.length());
#endif

  BodyProcessor bp;
  receiveResponse(bp);
  if (getHttpStatus() != 200)
    throw ConnectionException(TransportError, getHttpPhrase(), getHttpStatus());

  if (!protocol->isPersistent() )
    protocol->close();
}


ULXR_API_IMPL(void) HttpClient::filePUT(const CppString &filename,
                                     const CppString &type,
                                     const CppString &resource)
{
  ULXR_TRACE(ULXR_PCHAR("filePUT"));

  if (!protocol->isOpen() )
    protocol->open();

  FILE *ifs = fopen (getLatin1(filename).c_str(), "rb");
  if (ifs == 0)
    throw Exception(SystemError,
                    ulxr_i18n(ULXR_PCHAR("Cannot open file: "))+filename);

  struct stat statbuf;
  if (0 != stat (getLatin1(filename).c_str(), &statbuf) )
    throw Exception(SystemError,
                    ulxr_i18n(ULXR_PCHAR("Could not get information about file: "))+filename);

  sendAuthentication();
  protocol->sendRequestHeader(ULXR_PCHAR("PUT"), resource, type, statbuf.st_size);

  char buffer [ULXR_SEND_BUFFER_SIZE];
  long readed;
  try {
    while (!feof(ifs))
    {
      readed = fread(buffer, 1, sizeof(buffer), ifs);
      if (readed < 0)
        throw Exception(SystemError,
                        ulxr_i18n(ULXR_PCHAR("Could not read from file: "))+filename);
      protocol->writeBody(buffer, readed);
    }
  }
  catch (...)
  {
    fclose(ifs);
    throw;
  }

//  bool eof_reached = feof(ifs);
  fclose(ifs);

  BodyProcessor bp;
  receiveResponse(bp);

  if (getHttpStatus() != 200)
    throw ConnectionException(TransportError,
                              getHttpPhrase(), getHttpStatus());

  if (!protocol->isPersistent() )
    protocol->close();
}


ULXR_API_IMPL(Cpp8BitString) HttpClient::msgGET(const CppString &resource)
{
  ULXR_TRACE(ULXR_PCHAR("msgGET"));
  Cpp8BitString ret;

  if (!protocol->isOpen() )
    protocol->open();

  sendAuthentication();
  protocol->sendRequestHeader(ULXR_PCHAR("GET"), resource, ULXR_PCHAR(""), 0);
  StringProcessor sp (ret);
  receiveResponse(sp);
  if (getHttpStatus() != 200)
    throw ConnectionException(TransportError, getHttpPhrase(), getHttpStatus());

  if (!protocol->isPersistent() )
    protocol->close();

  return ret;
}


/** Helper class to store the message body in a file.
  */
class FileProcessor : public BodyProcessor
{
 public:
   FileProcessor(std::ostream &ostr_ref, const CppString &fn)
      : target(ostr_ref), name(fn) {}

 public:
   virtual void process(const char *buffer, unsigned len)
   {
     target.write(buffer, len);
     if (!target.good() )
       throw Exception(SystemError, ulxr_i18n(ULXR_PCHAR("Cannot write to file: "))+name);
   }

 private:
   std::ostream  &target;
   CppString         name;
};


ULXR_API_IMPL(void) HttpClient::fileGET(const CppString &filename,
                                     const CppString &resource)
{
  ULXR_TRACE(ULXR_PCHAR("fileGET"));

  if (!protocol->isOpen() )
    protocol->open();

  std::ofstream ofs (getLatin1(filename).c_str(), std::ios::out | std::ios::binary);
  if (!ofs.good() )
    throw Exception(SystemError, ulxr_i18n(ULXR_PCHAR("Cannot create file: "))+filename);

  sendAuthentication();
  protocol->sendRequestHeader(ULXR_PCHAR("GET"), resource, ULXR_PCHAR(""), 0);

  FileProcessor fp(ofs, filename);
  receiveResponse(fp);

  if (getHttpStatus() != 200)
    throw ConnectionException(TransportError, getHttpPhrase(), getHttpStatus());

  if (!protocol->isPersistent() )
    protocol->close();
}


ULXR_API_IMPL(Cpp8BitString) HttpClient::msgPOST(
                     const Cpp8BitString &msg,
                     const CppString &type,
                     const CppString &resource)
{
  ULXR_TRACE(ULXR_PCHAR("msgPOST"));
  Cpp8BitString ret;

  if (!protocol->isOpen() )
    protocol->open();

  sendAuthentication();
  protocol->sendRequestHeader(ULXR_PCHAR("POST"), resource, type, msg.length());
  protocol->writeBody(msg.data(), msg.length());

  StringProcessor sp (ret);
  receiveResponse(sp);

  if (getHttpStatus() != 200)
    throw ConnectionException(TransportError, getHttpPhrase(), getHttpStatus());

  if (!protocol->isPersistent() )
    protocol->close();

  return ret;
}


ULXR_API_IMPL(void) HttpClient::doDELETE(const CppString &resource)
{
  ULXR_TRACE(ULXR_PCHAR("doDELETE"));

  if (!protocol->isOpen() )
    protocol->open();

  sendAuthentication();
  protocol->sendRequestHeader(ULXR_PCHAR("DELETE"), resource, ULXR_PCHAR(""), 0);

  BodyProcessor bp;
  receiveResponse(bp);

  if (getHttpStatus() != 200)
    throw ConnectionException(TransportError, getHttpPhrase(), getHttpStatus());

  if (!protocol->isPersistent() )
    protocol->close();
}


ULXR_API_IMPL(void) HttpClient::setMessageAuthentication(const CppString &user,
                                                      const CppString &pass)
{
  ULXR_TRACE(ULXR_PCHAR("setMessageAuthentication"));
  http_user = user;
  http_pass = pass;
}


ULXR_API_IMPL(void) HttpClient::sendAuthentication()
{
  ULXR_TRACE(ULXR_PCHAR("sendAuthentication"));
  if (http_user.length() != 0 && http_pass.length() != 0)
    protocol->setMessageAuthentication(http_user, http_pass);
}


ULXR_API_IMPL(int) HttpClient::getHttpStatus() const
{
  return head_status;
}


ULXR_API_IMPL(CppString) HttpClient::getHttpPhrase() const
{
  return head_phrase;
}


ULXR_API_IMPL(CppString) HttpClient::getHttpVersion() const
{
  return head_version;
}


}  // namespace ulxr

#endif // ULXR_OMIT_TCP_STUFF

