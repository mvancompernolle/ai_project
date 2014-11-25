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
#include "rest.h"
#include "method.h"
#include "resource.h"
#include "exception.h"
#include <uripp/path.h>
#include <stdexcept>
namespace restcgi {
    rest::rest() {}
    rest::~rest() {}
    bool rest::special_case(method_pointer m) {
        const uripp::path& path = m->uri_path();
        if (m->e() != method_e::OPTIONS || path.size() != 1 || path.front() != "*")
            return false;
        // Just respond, no need to contact a resource.
        // See http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html#sec9.2
        content_hdr ch;
        ch.content_length(0);
        m->respond(ch);
        return true;
    }
    bool rest::special_case() {return special_case(method_);}
    rest::resource_pointer rest::locate(method_pointer m, resource_pointer& r) {
        uripp::path p = m->uri_path();
        // Locate resource corresponding to URI path in method.
        p.absolute(false); // Strip root slash.
        bool chain_children = r->chain_children();
        for (; true;) {
            r->method(m);
            if (p.empty())
                break;
            resource::pointer next = r->locate(p); // Might throw.
            if (!next) { // Stop locating.
                if (!p.empty()) // Still more path.
                    throw bad_request("URI path contains an unknown resource: " + p.encoding());
                break; // Found.
            }
            if (chain_children) r->child(r, next);
            r = next;
        }
        return r;
    }
    rest::resource_pointer rest::locate(resource_pointer& r) {return locate(method_, r);}
    void rest::apply(method_pointer m, resource_pointer r) {
        // Test if method supported.
        int e = m->e().enumeration();
        int mask = r->methods_allowed_mask();
        if (!(mask & e))
            throw method_not_allowed(method_e::mask_to_string(mask));
        r->method(m);
        // Examine the method type and call resource.
        switch (e) {
        case method_e::GET: r->get_method(); break;
        case method_e::PUT: r->put_method(); break;
        case method_e::POST: r->post_method(); break;
        case method_e::DEL: r->del_method(); break;
        case method_e::OPTIONS: r->options_method(); break;
        case method_e::HEAD: r->head_method(); break;
        default:
            throw internal_server_error(std::string("restcgi internal error: ") + m->e().cstring() + " method cannot be applied");
        }
        // Resource should have called responder.
        if (!m->responded())
            throw internal_server_error("resource " + m->uri_path().encoding() + " " + m->e().cstring() + " method implemented but no response generated");
    }
    void rest::apply() {apply(method_, resource_);}
    void rest::on_exception(method_pointer m, const sc_ctmpls& sccts) {
        if (m && !m->responded())
            try {
                try {
                    throw;
                } catch (const exception&) {
                    throw;
                } catch (const std::exception& e) {
                    throw internal_server_error(e.what());
                } catch (...) {
                    throw internal_server_error("unknown exception");
                }
            } catch (const exception& e) {
                response_hdr rh;
                exception::map_type map;
                e.copy_hdr_flds(map);
                for (exception::map_type::const_iterator it = map.begin(); it != map.end(); ++it)
                    rh.insert(it->first, it->second);
                status_code_e sc = e.status_code();
                // Check if application defined a template for this status code.
                sc_ctmpls::const_iterator it = sccts.find(sc);
                if (it != sccts.end() && !e.inhibit_content()) { // Content.
                    content_hdr ch;
                    ch.content_type(it->second.content_type());
                    // Send headers and status code.
                    ocontent::pointer oc = m->respond(ch, sc, rh);
                    // Content from template.
                    *oc << it->second.eval(m->env(), e.map());
                } else // Send just status code and rsp hdr.
                    m->respond(sc, rh);
            }
    }
    void rest::on_exception(const sc_ctmpls& sccts) {on_exception(method_, sccts);}
    void rest::process(method_pointer m, resource_pointer root, const sc_ctmpls& sccts) {
        if (!m || !root)
            throw std::invalid_argument("rest::process called with null pointer");
        method_ = m;
        root_ = root;
        try {
            // Check and perform special case responses.
            if (method_ && !special_case()) { // Not special case.
                // Locate the resource specified by the
                // URI path in the method request.
                resource_ = root_;
                resource_ = locate(resource_);
                if (!resource_)
                    throw std::domain_error("rest::locate returned null resource pointer");
                // Apply the method to the resource.
                apply();
            }
        } catch (...) {
            on_exception(sccts);
        }
    }
    void rest::process(method_pointer m, const vhosts_resources_type& vhrs) {
    	std::string host = m->request_hdr().host();
    	vhosts_resources_type::const_iterator it = vhrs.find(host);
    	if (it == vhrs.end()) throw std::invalid_argument("rest::request for virtual host \"" + host + "\" has no resource");
    	process(m, it->second);
    }
}
