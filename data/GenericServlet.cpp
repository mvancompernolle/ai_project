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

#include <servlet/GenericServlet.h>
#include <servlet/ServletContext.h>

using namespace std;

namespace servlet
{
//--------------------------------------------------------------------

GenericServlet::GenericServlet() {
}

//--------------------------------------------------------------------
void GenericServlet::destroy() {
}

//--------------------------------------------------------------------
string GenericServlet::getInitParameter(const string & name) const{
        return getServletConfig().getInitParameter(name);
}
//--------------------------------------------------------------------

auto_ptr< vector < string > > GenericServlet::getInitParameterNames() const {
        return getServletConfig().getInitParameterNames();
}

//--------------------------------------------------------------------

ServletConfig& GenericServlet::getServletConfig() const{
        return *config;
}

//--------------------------------------------------------------------

ServletContext& GenericServlet::getServletContext() const {
        return getServletConfig().getServletContext();
}

//--------------------------------------------------------------------

string GenericServlet::getServletInfo() const {
        return "";
}

//--------------------------------------------------------------------
void GenericServlet::init(ServletConfig& config){
        this->config = &config;
        this->init();
}

//--------------------------------------------------------------------

void GenericServlet::init(){
}

//--------------------------------------------------------------------

void GenericServlet::log(const string& msg){
        getServletContext().log(getServletName() + ": " + msg);
}

//--------------------------------------------------------------------

void GenericServlet::log(const string & message, const exception &e){
        getServletContext().log(getServletName() + ": " + message, e);
}

//--------------------------------------------------------------------

string GenericServlet::getServletName() const{
        return config->getServletName();
        //return typeid(this).name();
}

//--------------------------------------------------------------------
}
