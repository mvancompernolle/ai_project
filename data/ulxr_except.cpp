/***************************************************************************
                    ulxr_except.h  -  exception handling
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_except.cpp 940 2006-12-30 18:22:05Z ewald-arnold $

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

#include <ulxmlrpcpp/ulxr_except.h>


namespace ulxr {

ULXR_API_IMPL0 Exception::Exception(int fc, const CppString &s)
  :
#ifdef ULXR_USE_STD_EXCEPTION
   std::exception(),
#endif
   reason(s), faultcode(fc)
{
  ULXR_DOUT("=== Exception: " << s);
}


ULXR_API_IMPL0 Exception::~Exception() throw()
{
}


ULXR_API_IMPL(CppString) Exception::why() const
{
  return reason;
}


#ifdef ULXR_USE_STD_EXCEPTION

ULXR_API_DECL(const char *) Exception::what() const throw()
{
  what_helper = getLatin1(why());
  return what_helper.c_str();
}

#endif


ULXR_API_IMPL(int) Exception::getFaultCode() const
{
  return faultcode;
}


/////////////////////////////////////////////////////////////////////////


ULXR_API_IMPL0 ConnectionException::ConnectionException(int fc, const CppString &phrase, int stat)
  : Exception(fc, phrase)
  , status(stat)
{
}


ULXR_API_IMPL0 ConnectionException::~ConnectionException() throw()
{
}


ULXR_API_IMPL(int) ConnectionException::getStatusCode() const
{
  return status;
}


/////////////////////////////////////////////////////////////////////////


ULXR_API_IMPL0 RuntimeException::RuntimeException(int fc, const CppString &s)
  : Exception(fc, s)
{
}


ULXR_API_IMPL0 RuntimeException::~RuntimeException() throw()
{
}


/////////////////////////////////////////////////////////////////////////


ULXR_API_IMPL0 XmlException::XmlException(int fc, const CppString &s, int l,
                                       const CppString &err)
  : Exception(fc, s), line(l), xmlerror(err)
{
}


ULXR_API_IMPL0 XmlException::~XmlException() throw()
{
}


ULXR_API_IMPL(int) XmlException::getErrorLine() const
{
  return line;
}


ULXR_API_IMPL(CppString) XmlException::getErrorString() const
{
  return xmlerror;
}


/////////////////////////////////////////////////////////////////////////


ULXR_API_IMPL0 ParameterException::ParameterException(int fc, const CppString &s)
  : Exception(fc, s)
{
}


ULXR_API_IMPL0 ParameterException::~ParameterException()  throw()
{
}


/////////////////////////////////////////////////////////////////////////


ULXR_API_IMPL0 MethodException::MethodException(int fc, const CppString &s)
  : Exception(fc, s)
{
}


ULXR_API_IMPL0 MethodException::~MethodException()  throw()
{
}


}  // namespace ulxr
