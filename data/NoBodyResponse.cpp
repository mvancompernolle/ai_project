/*
 * ====================================================================
 *
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2001 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution, if
 *    any, must include the following acknowlegement:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowlegement may appear in the software itself,
 *    if and wherever such third-party acknowlegements normally appear.
 *
 * 4. The names "The Jakarta Project", "Tomcat", and "Apache Software
 *    Foundation" must not be used to endorse or promote products derived
 *    from this software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache"
 *    nor may "Apache" appear in their names without prior written
 *    permission of the Apache Group.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 */


#include "NoBodyResponse.h"

namespace servlet
{
//--------------------------------------------------------------------

NoBodyResponse::NoBodyResponse(HttpServletResponse & r)
        : resp(r)
{
}

//--------------------------------------------------------------------

void NoBodyResponse::setContentLength()
{
        if (!didSetContentLength)
                resp.setContentLength(no_body.get_context_length());
}

//--------------------------------------------------------------------

void NoBodyResponse::setContentLength(int len)
{
        resp.setContentLength(len);
        didSetContentLength = true;
}

//--------------------------------------------------------------------

void NoBodyResponse::setCharacterEncoding(const std::string & charset)
{
        resp.setCharacterEncoding(charset);
}

//--------------------------------------------------------------------

void NoBodyResponse::setContentType(const std::string & type)
{
        resp.setContentType(type);
}

//--------------------------------------------------------------------

std::string NoBodyResponse::getContentType()
{
        return resp.getContentType();
}

//--------------------------------------------------------------------

std::ostream& NoBodyResponse::getOutputStream()
{
        return no_body;
}

//--------------------------------------------------------------------

std::string NoBodyResponse::getCharacterEncoding()
{
        return resp.getCharacterEncoding();
}

//--------------------------------------------------------------------

void NoBodyResponse::setBufferSize(int size)
{
        resp.setBufferSize(size);
}

//--------------------------------------------------------------------

int NoBodyResponse::getBufferSize()
{
        return resp.getBufferSize();
}

//--------------------------------------------------------------------

void NoBodyResponse::reset()
{
        resp.reset();
}

//--------------------------------------------------------------------

bool NoBodyResponse::isCommitted()
{
        return resp.isCommitted();
}

//--------------------------------------------------------------------

void NoBodyResponse::flushBuffer()
{
        resp.flushBuffer();
}

//--------------------------------------------------------------------
/*
void NoBodyResponse::setLocale(const Locale & loc)
{
        resp.setLocale(loc);
}
//--------------------------------------------------------------------
const Locale & NoBodyResponse::getLocale()
{
        return resp.getLocale();
}
*/
//--------------------------------------------------------------------

void NoBodyResponse::addCookie(const Cookie & cookie)
{
        resp.addCookie(cookie);
}

//--------------------------------------------------------------------

bool NoBodyResponse::containsHeader(const std::string & name) const
{
        return resp.containsHeader(name);
}

//--------------------------------------------------------------------

void NoBodyResponse::setStatus(int sc)
{
        resp.setStatus(sc);
}

//--------------------------------------------------------------------

void NoBodyResponse::setHeader(const std::string & name, const std::string & value)
{
        resp.setHeader(name, value);
}

//--------------------------------------------------------------------

void NoBodyResponse::setIntHeader(const std::string & name, int value)
{
        resp.setIntHeader(name, value);
}

//--------------------------------------------------------------------

void NoBodyResponse::setDateHeader(const std::string & name, long date)
{
        resp.setDateHeader(name, date);
}

//--------------------------------------------------------------------

void NoBodyResponse::sendError(int sc, const std::string & msg)
{
        resp.sendError(sc, msg);
}

//--------------------------------------------------------------------

void NoBodyResponse::sendError(int sc)
{
        resp.sendError(sc);
}

//--------------------------------------------------------------------

void NoBodyResponse::sendRedirect(const std::string & location)
{
        resp.sendRedirect(location);
}

//--------------------------------------------------------------------

std::string NoBodyResponse::encodeURL(const std::string & url)
{
        return resp.encodeURL(url);
}

//--------------------------------------------------------------------

std::string NoBodyResponse::encodeRedirectURL(const std::string & url)
{
        return resp.encodeRedirectURL(url);
}

//--------------------------------------------------------------------

void NoBodyResponse::addHeader(const std::string & name, const std::string & value)
{
        resp.addHeader(name, value);
}

//--------------------------------------------------------------------

void NoBodyResponse::addDateHeader(const std::string & name, long value)
{
        resp.addDateHeader(name, value);
}

//--------------------------------------------------------------------

void NoBodyResponse::addIntHeader(const std::string & name, int value)
{
        resp.addIntHeader(name, value);
}

//--------------------------------------------------------------------
}
