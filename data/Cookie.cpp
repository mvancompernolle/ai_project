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

#include <servlet/Cookie.h>
#include <servlet/IllegalArgumentException.h>
#include <algorithm>
#include <stdexcept>

using namespace std;

namespace servlet
{
//--------------------------------------------------------------------
Cookie::Cookie(const std::string& name, const std::string& value)
    : maxAge(-1)
    , secure(false)
    , version(1)
{
        string lowerName = name;
        std::transform(name.begin(), name.end(), lowerName.begin(), ::tolower);
        if (!isValidName(lowerName)
                || lowerName == "comment"       // rfc2019
                || lowerName == "discard"       // 2019++
                || lowerName == "domain"
                || lowerName == "expires"       // (old cookies)
                || lowerName == "max-age"       // rfc2019
                || lowerName == "path"
                || lowerName == "secure"
                || lowerName == "version"
                || lowerName[0] == '$'
            )
        {
            throw servlet::IllegalArgumentException("Cookie name \""+lowerName+"\" is not valid.");
        }
        this->name = name;
        this->value = value;
}
//--------------------------------------------------------------------
/*
        Tells weather 'value' is a valid cookie name.
        Quoting from RFC 2109 and RFC 2068.

          NAME           =       attr

          attr           =       token

          CTL            = <any US-ASCII control character
                           (octets 0 - 31) and DEL (127)>

          token          = 1*<any CHAR except CTLs or tspecials>

          tspecials      = "(" | ")" | "<" | ">" | "@"
                         | "," | ";" | ":" | "\" | <">
                         | "/" | "[" | "]" | "?" | "="
                         | "{" | "}" | SP | HT

          SP             = <US-ASCII SP, space (32)>
          HT             = <US-ASCII HT, horizontal-tab (9)>

*/
bool  Cookie::isValidName(const std::string& value) const {
    int len = value.length();

    for (int i = 0; i < len; i++) {
        char c=value[i];
        if(c <= 0x1F || c>= 0x7f)
            return false;
        switch(c){
            case '(':
            case ')':
            case '<':
            case '>':
            case '@':
            case ',':
            case ';':
            case ':':
            case '\\':
            case '"':
            case '/':
            case '[':
            case ']':
            case '?':
            case '=':
            case '{':
            case '}':
            case ' ': //SP
            case '\t'://HT
                return false;
        }
    }
    return true;
}
//--------------------------------------------------------------------
}
