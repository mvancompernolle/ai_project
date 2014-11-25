/*
Copyright (c) 2009 zooml.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "method.h"
#include "endpoint.h"
#include <stdexcept>
namespace restcgi {
	const char method::QP_REST_PUT[] = "restPUT";
	const char method::QP_REST_DELETE[] = "restDELETE";
    method::method(const method_e& e, const endpoint_pointer& ep)
        : e_(e), endpoint_(ep)
        , uri_path_(endpoint_->env().path_info())
        , uri_query_(endpoint_->env().query_string())
        , responded_(false) {
    	if (e_ == method_e::POST) {
    		// Look for POST-only client workaround params.
    		uri_query_type::iterator it = uri_query_.find(QP_REST_PUT);
    		if (it != uri_query_.end()) {
    			uri_query_.erase(it);
    			e_ = method_e::PUT;
    		} else {
        		it = uri_query_.find(QP_REST_DELETE);
        		if (it != uri_query_.end()) {
        			uri_query_.erase(it);
        			e_ = method_e::DEL;
        		}
    		}
    	}
        // Read the request and input content headers from the env.
        content_hdr ch;
        copy(endpoint_->env(), request_hdr_, ch);
        // Create the input content stream.
        icontent_.reset(new icontent_type(endpoint_, ch));
    }
    method::~method() {}
    method::pointer method::create(const endpoint_pointer& ep) {
        std::string mname = ep->env().request_method();
        method_e type(mname);
        if (type.is_null()) { // Unrecognized.
            // Respond with "bad request" and throw.
            respond(ep, status_code_e::BAD_REQUEST);
            throw std::domain_error("unrecognized method in HTTP header: " + mname);
        }
        return method::pointer(new method(type, ep));
    }
    void method::respond(const status_code_e& sc, const response_hdr_type& rh) {respond(sc, rh, content_hdr(), false);}
    method::ocontent_pointer method::respond(const content_hdr& ch, const status_code_e& sc, const response_hdr_type& rh) {
        respond(sc, rh, ch, true);
        return ocontent_;
    }
    void method::respond(const status_code_e& sc, const response_hdr_type& rh, const content_hdr& ch, bool has_content) {
        if (responded_)
            throw std::domain_error("method response already attempted");
        responded_ = true;
        respond(endpoint_, sc, rh, ch);
        if (has_content)
            ocontent_.reset(new ocontent_type(endpoint_, ch));
    }
    void method::respond(endpoint_pointer ep, const status_code_e& sc, const response_hdr_type& rh, const content_hdr& ch) {
        // Status code.
        ep->os_ << sc;
        // Send general and response headers.
        ep->os_ << rh;
        // Send content header.
        ep->os_ << ch;
        // Send header termination.
        ep->os_ << hdr::EOL_CSTR;
    }
    const method::env_type& method::env() const {return endpoint_->env();}
}
