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
#include "env.h"
#include <stdlib.h>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#ifdef _WIN32
#pragma warning (disable: 4996)
#define environ _environ
#endif
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
namespace restcgi {
    static const char HEADER_PREFIX_CSTR[] = "HTTP_";
    env::env() : map_override_mode_(true) {}
    env::env(const char **pcstr) : map_override_mode_(false) {
        for (const char **pp = pcstr; *pp; ++pp) {
            std::string v;
            std::string s = *pp;
            size_t pos = s.find('=');
            if (pos != std::string::npos) {
                v = s.substr(pos + 1);
                s.erase(pos);
            }
            map_.insert(std::make_pair(s, v));
        }
    }
    env::env(const map_type& m) : map_override_mode_(false),  map_(m) {}
    std::string env::server_software() const {return find("SERVER_SOFTWARE");}
    std::string env::server_name() const {return find("SERVER_NAME");}
    std::string env::gateway_interface() const {return find("GATEWAY_INTERFACE");}
    std::string env::server_protocol() const {return find("SERVER_PROTOCOL");}
    std::string env::server_port() const {return find("SERVER_PORT");}
    std::string env::request_method() const {return find("REQUEST_METHOD");}
    std::string env::path_info() const {return find("PATH_INFO");}
    std::string env::path_translated() const {return find("PATH_TRANSLATED");}
    std::string env::script_name() const {return find("SCRIPT_NAME");}
    std::string env::request_uri() const {return find("REQUEST_URI");}
    std::string env::script_filename() const {return find("SCRIPT_FILENAME");}
    std::string env::script_url() const {return find("SCRIPT_URL");}
    std::string env::script_uri() const {return find("SCRIPT_URI");}
    std::string env::query_string() const {return find("QUERY_STRING");}
    std::string env::remote_host() const {return find("REMOTE_HOST");}
    std::string env::remote_addr() const {return find("REMOTE_ADDR");}
    std::string env::auth_type() const {return find("AUTH_TYPE");}
    std::string env::remote_user() const {return find("REMOTE_USER");}
    std::string env::remote_ident() const {return find("REMOTE_IDENT");}
    std::string env::redirect_request() const {return find("REDIRECT_REQUEST");}
    std::string env::redirect_url() const {return find("REDIRECT_URL");}
    std::string env::redirect_status() const {return find("REDIRECT_STATUS");}
    std::string env::content_type() const {return find("HTTP_CONTENT_TYPE");}
    std::string env::content_length() const {return find("HTTP_CONTENT_LENGTH");}
    std::string env::accept() const {return find("HTTP_ACCEPT");}
    std::string env::accept_language() const {return find("HTTP_ACCEPT_LANGUAGE");}
    std::string env::accept_encoding() const {return find("HTTP_ACCEPT_ENCODING");}
    std::string env::accept_charset() const {return find("HTTP_ACCEPT_CHARSET");}
    std::string env::user_agent() const {return find("HTTP_USER_AGENT");}
    void env::override(const std::string& name, const std::string& value) {
        map_.insert(std::make_pair(name, value));
    }
    bool env::hdr_find(const std::string& hname, std::string& value) const {
        return find(to_env_name(hname).c_str(), value);
    }
    bool env::find(const char* name, std::string& value) const {
        if (!map_.empty()) { // Check map first.
            map_type::const_iterator it = map_.find(name);
            if (it != map_.end()) {
                value = it->second;
                return true;
            }
            if (!map_override_mode_) // If override mode then continue.
                return false;
        }
        const char* s = ::getenv(name); // Check system env.
        if (!s)
            return false;
        value = s;
        return true;
    }
    std::string env::find(const char* name) const {
        std::string value;
        find(name, value);
        return value;
    }
    std::string env::to_env_name(const std::string& hname) {
        std::string s = boost::to_upper_copy(hname);
        boost::replace_all(s, "-", "_");
        return HEADER_PREFIX_CSTR + s;
    }
    std::string env::from_env_name(const std::string& ename) {
        std::string s = boost::to_lower_copy(ename);
        if (ARRAY_SIZE(HEADER_PREFIX_CSTR) < s.size())
            s.erase(0, ARRAY_SIZE(HEADER_PREFIX_CSTR) - 1);
        boost::replace_all(s, "_", "-");
        return s;
    }
    env::hdr_iterator::hdr_iterator(const env& e) : end_(false), p_(0) {
        if (e.map_.empty())
            p_ = environ;
        else {
            it_ = e.map_.begin();
            it_end_ = e.map_.end();
            if (it_ == it_end_)
                end_ = true;
        }
        if (!end_)
            increment(true); // Find first, if any.
    }
    void env::hdr_iterator::increment(bool initialize) {
        if (!initialize) {
            if (end_)
                throw std::domain_error("attempt to increment env::hdr_iterator past end");
            p_ ? (void)++p_ : (void)++it_; // Bump from last time.
        }
        // Find next.
        bool found = false;
        if (p_) {
            for (; *p_; ++p_)
                if (!::strncmp(HEADER_PREFIX_CSTR, *p_, ARRAY_SIZE(HEADER_PREFIX_CSTR) - 1)) {
                    const char* eq = ::strchr(*p_, '=');
                    std::string ename(*p_, eq ? eq - *p_ : ::strlen(*p_));
                    std::string value(eq ? eq + 1 : "");
                    current_ = referent_type(from_env_name(ename), value);
                    found = true;
                    break;
                }
        } else {
            for (; it_ != it_end_; ++it_)
                if (it_->first.rfind(HEADER_PREFIX_CSTR, ARRAY_SIZE(HEADER_PREFIX_CSTR) - 2) == 0) {
                    current_ = referent_type(from_env_name(it_->first), it_->second);
                    found = true;
                    break;
                }
        }
        if (found)
            boost::algorithm::trim(current_.second); // Be sure value is trimmed.
        else
            end_ = true;
    }
    env::hdr_iterator& env::hdr_iterator::operator ++() {
        increment();
        return *this;
    }
    env::hdr_iterator env::hdr_iterator::operator ++(int) {
        hdr_iterator pre = *this;
        increment();
        return pre;
    }
    bool env::hdr_iterator::operator ==(const hdr_iterator& rhs) const {
        if (end_ || rhs.end_)
            return end_ && rhs.end_;
        return p_ ? p_ == rhs.p_ : it_ == rhs.it_;
    }
    const env::hdr_iterator::referent_type& env::hdr_iterator::operator *() const {
        if (end_)
            throw std::domain_error("attempt to reference env::hdr_iterator at end");
        return current_;
    }
    const env::hdr_iterator::referent_type* env::hdr_iterator::operator ->() const {
        if (end_)
            throw std::domain_error("attempt to dereference env::hdr_iterator at end");
        return &current_;
    }
}
