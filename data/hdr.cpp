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
#include "hdr.h"
#include "env.h"
#include "date_time.h"
#include "exception.h"
#include <uripp/utils.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
enum fld_e {
    // THIS MUST BE IN SYNC WITH flds_traits_!!!
    E_ACCEPT,
    E_ACCEPT_CHARSET,
    E_ACCEPT_ENCODING,
    E_ACCEPT_LANGUAGE,
    E_ACCEPT_RANGES,
    E_AGE,
    E_ALLOW,
    E_AUTHORIZATION,
    E_CACHE_CONTROL,
    E_CONNECTION,
    E_CONTENT_ENCODING,
    E_CONTENT_LANGUAGE,
    E_CONTENT_LENGTH,
    E_CONTENT_LOCATION,
    E_CONTENT_MD5,
    E_CONTENT_RANGE,
    E_CONTENT_TYPE,
    E_DATE,
    E_ETAG,
    E_EXPECT,
    E_EXPIRES,
    E_FROM,
    E_HOST,
    E_IF_MATCH,
    E_IF_MODIFIED_SINCE,
    E_IF_NONE_MATCH,
    E_IF_RANGE,
    E_IF_UNMODIFIED_SINCE,
    E_LAST_MODIFIED,
    E_LOCATION,
    E_MAX_FORWARDS,
    E_PRAGMA,
    E_PROXY_AUTHENTICATE,
    E_PROXY_AUTHORIZATION,
    E_RANGE,
    E_REFERER,
    E_RETRY_AFTER,
    E_SERVER,
    E_TE,
    E_TRAILER,
    E_TRANSFER_ENCODING,
    E_UPGRADE,
    E_USER_AGENT,
    E_VARY,
    E_VIA,
    E_WARNING,
    E_WWW_AUTHENTICATE,
    // THIS MUST BE IN SYNC WITH flds_traits_!!!
};
namespace restcgi {
    const char hdr::FLD_NAME_END_CHAR = ':';
    const char hdr::EOL_CSTR[3] = "\r\n";
    const hdr::fld_traits hdr::flds_traits_[] = {
        // THIS MUST BE IN SYNC WITH fld_e!!!
        fld_traits("accept", "Accept", fc_request),
        fld_traits("accept-charset", "Accept-Charset", fc_request),
        fld_traits("accept-encoding", "Accept-Encoding", fc_request),
        fld_traits("accept-language", "Accept-Language", fc_request),
        fld_traits("accept-ranges", "Accept-Ranges", fc_response),
        fld_traits("age", "Age", fc_response),
        fld_traits("allow", "Allow", fc_entity_rsp),
        fld_traits("authorization", "Authorization", fc_request),
        fld_traits("cache-control", "Cache-Control", fc_general),
        fld_traits("connection", "Connection", fc_general),
        fld_traits("content-encoding", "Content-Encoding", fc_entity_cnt),
        fld_traits("content-language", "Content-Language", fc_entity_cnt),
        fld_traits("content-length", "Content-Length", fc_entity_cnt),
        fld_traits("content-location", "Content-Location", fc_entity_rsp),
        fld_traits("content-md5", "Content-MD5", fc_entity_cnt),
        fld_traits("content-range", "Content-Range", fc_entity_cnt),
        fld_traits("content-type", "Content-Type", fc_entity_cnt),
        fld_traits("date", "Date", fc_general),
        fld_traits("etag", "ETag", fc_response),
        fld_traits("expect", "Expect", fc_request),
        fld_traits("expires", "Expires", fc_entity_rsp),
        fld_traits("from", "From", fc_request),
        fld_traits("host", "Host", fc_request),
        fld_traits("if-match", "If-Match", fc_request),
        fld_traits("if-modified-since", "If-Modified-Since", fc_request),
        fld_traits("if-none-match", "If-None-Match", fc_request),
        fld_traits("if-range", "If-Range", fc_request),
        fld_traits("if-unmodified-since", "If-Unmodified-Since", fc_request),
        fld_traits("last-modified", "Last-Modified", fc_entity_rsp),
        fld_traits("location", "Location", fc_response),
        fld_traits("max-forwards", "Max-Forwards", fc_request),
        fld_traits("pragma", "Pragma", fc_general),
        fld_traits("proxy-authenticate", "Proxy-Authenticate", fc_response),
        fld_traits("proxy-authorization", "Proxy-Authorization", fc_request),
        fld_traits("range", "Range", fc_request),
        fld_traits("referer", "Referer", fc_request),
        fld_traits("retry-after", "Retry-After", fc_response),
        fld_traits("server", "Server", fc_response),
        fld_traits("te", "TE", fc_request),
        fld_traits("trailer", "Trailer", fc_general),
        fld_traits("transfer-encoding", "Transfer-Encoding", fc_general),
        fld_traits("upgrade", "Upgrade", fc_general),
        fld_traits("user-agent", "User-Agent", fc_request),
        fld_traits("vary", "Vary", fc_response),
        fld_traits("via", "Via", fc_general),
        fld_traits("warning", "Warning", fc_general),
        fld_traits("www-authenticate", "WWW-Authenticate", fc_response),
        // THIS MUST BE IN SYNC WITH fld_e!!!
    };
    const hdr::fld_traits* hdr::flds_traits_end_ = hdr::flds_traits_ + ARRAY_SIZE(hdr::flds_traits_);
    hdr::hdr(int categories) : categories_(categories | fc_other) {}
    hdr::~hdr() {}
    bool hdr::insert(int fld_traits_off, const std::string& v) {
        return insert(key(flds_traits_ + fld_traits_off), v);
    }
    std::string hdr::find(int fld_traits_off) const {
        const_iterator it = map_.find(key(flds_traits_ + fld_traits_off));
        return it == end() ? std::string() : it->second;
    }
    bool hdr::erase(const std::string& name) {
        map_type::iterator it = map_.find(key(name, true));
        if (it == end())
            return false;
        map_.erase(it);
        return true;
    }
    std::ostream& hdr::operator <<(std::ostream& os) const {
        for (const_iterator it = begin(); it != end(); ++it)
            os << it->first.name_as_cstring() << FLD_NAME_END_CHAR << " " << it->second << EOL_CSTR;
        return os;
    }
    bool hdr::insert(const key& k, const std::string& value, bool update) {
        if (value.empty())
            return false;
        if (!(k.category() & categories_)) // Invalid category.
            return false;
        map_type::iterator it = map_.find(k);
        if (it != end()) {
            if (!update)
                return false;
            it->second = value;
        } else {
            const_iterator iit = map_.insert(it, std::make_pair(k, value));
            on_inserted(iit); // Call down.
        }
        return true;
    }
    void hdr::throw_bad_request(const char* name, const char* what) {
        throw bad_request(std::string("HTTP header ") + name + " error: " + what);
    }
    const hdr::fld_traits* hdr::find_fld_traits(const std::string& lower_name) {
        fld_traits test(lower_name.c_str());
        std::pair<const fld_traits*, const fld_traits*> er = std::equal_range(flds_traits_, flds_traits_end_, test);
        return (er.first != er.second) ? er.first : 0;
    }
    hdr::fld_traits::fld_traits(const char* ln, const char* n, fld_category cat)
        : ln_(ln), n_(n), category_(cat) {
    }
    hdr::key::key(const std::string& name, bool lookup) {
        if (name.empty())
            throw std::invalid_argument("header field name is empty");
        std::string ln = boost::to_lower_copy(name);
        p_ = lookup ? find_fld_traits(ln) : 0;
        if (!p_) {
            name_ = name;
            lower_name_ = ln;
        }
    }
    bool hdr::key::operator <(const key& rhs) const {
        if (p_) {
            if (rhs.p_) {
                if (p_->category() < rhs.p_->category())
                    return true;
                if (p_->category() > rhs.p_->category())
                    return false;
                return p_ < rhs.p_; // they are sorted
            }
            return p_->category() < fc_other;
        }
        if (rhs.p_)
            return fc_other < rhs.p_->category();
        return lower_name_.compare(rhs.lower_name_) < 0;
    }
    general_hdr::general_hdr(int categories)
        : hdr(categories | fc_general), on_updated_(false), cookies_((categories_ & fc_request) != 0) {
        cookies_.attach(this); // Attach to get updates.
    }
    general_hdr::~general_hdr() {cookies_.detach(this);}
    void general_hdr::on_inserted(const_iterator it) {
        if (!on_updated_ &&
            (((categories_ & fc_request) && !::strcmp(it->first.lower_name_as_cstring(), "cookie")) ||
             ((categories_ & fc_response) && !::strcmp(it->first.lower_name_as_cstring(), "set-cookie")))) {
            // Request hdr initialization case or application has
            // set hdr directly: keep cookies in sync.
            cookies_.clear();
            cookies_.insert(it->second);
        }
    }
    void general_hdr::on_updated_reset() {on_updated_ = false;}
    void general_hdr::on_updated(const cookies_type& v) {
        if (categories_ & fc_response) {
            boost::shared_ptr<void> guard(this, std::mem_fun(&general_hdr::on_updated_reset));
            on_updated_ = true;
            if (v.empty())
                erase("Set-Cookie");
            else {
                std::ostringstream oss;
                oss << v;
                insert(key("Set-Cookie"), oss.str(), true);
            }
        }
    }
    std::string general_hdr::cache_control() const {return find(E_CACHE_CONTROL);}
    bool general_hdr::cache_control(const std::string& v) {return insert(E_CACHE_CONTROL, v);}
    std::string general_hdr::connection() const {return find(E_CONNECTION);}
    bool general_hdr::connection(const std::string& v) {return insert(E_CONNECTION, v);}
    bool general_hdr::date(date_time& v, const date_time& dflt) const {
        bool ok;
        try {ok = uripp::convert(find(E_DATE), v);}
        catch (const std::exception& e) {throw_bad_request("Date", e.what());}
        if (!ok)
            v = dflt;
        return ok;
    }
    bool general_hdr::date(const date_time& v) {return insert(E_DATE, uripp::convert(v));}
    std::string general_hdr::pragma() const {return find(E_PRAGMA);}
    bool general_hdr::pragma(const std::string& v) {return insert(E_PRAGMA, v);}
    std::string general_hdr::trailer() const {return find(E_TRAILER);}
    bool general_hdr::trailer(const std::string& v) {return insert(E_TRAILER, v);}
    std::string general_hdr::transfer_encoding() const {return find(E_TRANSFER_ENCODING);}
    bool general_hdr::transfer_encoding(const std::string& v) {return insert(E_TRANSFER_ENCODING, v);}
    std::string general_hdr::upgrade() const {return find(E_UPGRADE);}
    bool general_hdr::upgrade(const std::string& v) {return insert(E_UPGRADE, v);}
    std::string general_hdr::via() const {return find(E_VIA);}
    bool general_hdr::via(const std::string& v) {return insert(E_VIA, v);}
    std::string general_hdr::warning() const {return find(E_WARNING);}
    bool general_hdr::warning(const std::string& v) {return insert(E_WARNING, v);}
    request_hdr::request_hdr() : general_hdr(fc_request) {}
    std::string request_hdr::accept() const {return find(E_ACCEPT);}
    std::string request_hdr::accept_charset() const {return find(E_ACCEPT_CHARSET);}
    std::string request_hdr::accept_encoding() const {return find(E_ACCEPT_ENCODING);}
    std::string request_hdr::accept_language() const {return find(E_ACCEPT_LANGUAGE);}
    std::string request_hdr::authorization() const {return find(E_AUTHORIZATION);}
    std::string request_hdr::expect() const {return find(E_EXPECT);}
    std::string request_hdr::from() const {return find(E_FROM);}
    std::string request_hdr::host() const {return find(E_HOST);}
    std::string request_hdr::if_match() const {return find(E_IF_MATCH);}
    bool request_hdr::if_modified_since(date_time& v, const date_time& dflt) const {
        bool ok;
        try {ok = uripp::convert(find(E_IF_MODIFIED_SINCE), v);}
        catch (const std::exception& e) {throw_bad_request("If-Modified-Since", e.what());}
        if (!ok)
            v = dflt;
        return ok;
    }
    std::string request_hdr::if_none_match() const {return find(E_IF_NONE_MATCH);}
    std::string request_hdr::if_range() const {return find(E_IF_RANGE);}
    bool request_hdr::if_unmodified_since(date_time& v, const date_time& dflt) const {
        bool ok;
        try {ok = uripp::convert(find(E_IF_UNMODIFIED_SINCE), v);}
        catch (const std::exception& e) {throw_bad_request("If-Unmodified-Since", e.what());}
        if (!ok)
            v = dflt;
        return ok;
    }
    bool request_hdr::max_forwards(size_t& v, size_t dflt) const {
        bool ok;
        try {ok = uripp::convert(find(E_MAX_FORWARDS), v);}
        catch (const std::exception& e) {throw_bad_request("Max-Forwards", e.what());}
        if (!ok)
            v = dflt;
        return ok;
    }
    std::string request_hdr::proxy_authorization() const {return find(E_PROXY_AUTHORIZATION);}
    std::string request_hdr::range() const {return find(E_RANGE);}
    std::string request_hdr::referer() const {return find(E_REFERER);}
    std::string request_hdr::te() const {return find(E_TE);}
    std::string request_hdr::user_agent() const {return find(E_USER_AGENT);}
    response_hdr::response_hdr() : general_hdr(fc_response | fc_entity_rsp) {}
    std::string response_hdr::accept_ranges() const {return find(E_ACCEPT_RANGES);}
    bool response_hdr::accept_ranges(const std::string& v) {return insert(E_ACCEPT_RANGES, v);}
    bool response_hdr::age(size_t& v, size_t dflt) const {
        bool ok;
        try {ok = uripp::convert(find(E_AGE), v);}
        catch (const std::exception& e) {throw_bad_request("Age", e.what());}
        if (!ok)
            v = dflt;
        return ok;
    }
    bool response_hdr::age(size_t v) {return insert(E_AGE, uripp::convert(v));}
    std::string response_hdr::allow() const {return find(E_ALLOW);}
    bool response_hdr::allow(const std::string& v) {return insert(E_ALLOW, v);}
    std::string response_hdr::content_location() const {return find(E_CONTENT_LOCATION);}
    bool response_hdr::content_location(const std::string& v) {return insert(E_CONTENT_LOCATION, v);}
    std::string response_hdr::etag() const {return find(E_ETAG);}
    bool response_hdr::etag(const std::string& v) {return insert(E_ETAG, v);}
    bool response_hdr::expires(date_time& v, const date_time& dflt) const {
        bool ok;
        try {ok = uripp::convert(find(E_EXPIRES), v);}
        catch (const std::exception& e) {throw_bad_request("Expires", e.what());}
        if (!ok)
            v = dflt;
        return ok;
    }
    bool response_hdr::expires(const date_time& v) {return insert(E_EXPIRES, uripp::convert(v));}
    bool response_hdr::last_modified(date_time& v, const date_time& dflt) const {
        bool ok;
        try {ok = uripp::convert(find(E_LAST_MODIFIED), v);}
        catch (const std::exception& e) {throw_bad_request("Last-Modified", e.what());}
        if (!ok)
            v = dflt;
        return ok;
    }
    bool response_hdr::last_modified(const date_time& v) {return insert(E_LAST_MODIFIED, uripp::convert(v));}
    std::string response_hdr::location() const {return find(E_LOCATION);}
    bool response_hdr::location(const std::string& v) {return insert(E_LOCATION, v);}
    std::string response_hdr::proxy_authenticate() const {return find(E_PROXY_AUTHENTICATE);}
    bool response_hdr::proxy_authenticate(const std::string& v) {return insert(E_PROXY_AUTHENTICATE, v);}
    bool response_hdr::retry_after(date_time& dt, size_t& secs) const {
        std::string s = find(E_RETRY_AFTER);
        if (s.empty())
            return false;
        std::string::const_iterator it = s.begin();
        if (parse(it, s.end(), dt))
            return true;
        try {uripp::convert(s, secs);}
        catch (const std::exception& e) {throw_bad_request("Retry-After", e.what());}
        return true;
    }
    bool response_hdr::retry_after(const date_time& v) {return insert(E_RETRY_AFTER, uripp::convert(v));}
    bool response_hdr::retry_after(size_t v) {return insert(E_RETRY_AFTER, uripp::convert(v));}
    std::string response_hdr::server() const {return find(E_SERVER);}
    bool response_hdr::server(const std::string& v) {return insert(E_SERVER, v);}
    std::string response_hdr::vary() const {return find(E_VARY);}
    bool response_hdr::vary(const std::string& v) {return insert(E_VARY, v);}
    std::string response_hdr::www_authenticate() const {return find(E_WWW_AUTHENTICATE);}
    bool response_hdr::www_authenticate(const std::string& v) {return insert(E_WWW_AUTHENTICATE, v);}
    content_hdr::content_hdr() : hdr(fc_entity_cnt) {}
    std::string content_hdr::content_encoding() const {return find(E_CONTENT_ENCODING);}
    bool content_hdr::content_encoding(const std::string& v) {return insert(E_CONTENT_ENCODING, v);}
    std::string content_hdr::content_language() const {return find(E_CONTENT_LANGUAGE);}
    bool content_hdr::content_language(const std::string& v) {return insert(E_CONTENT_LANGUAGE, v);}
    bool content_hdr::content_length(size_t& v, size_t dflt) const {
        bool ok;
        try {ok = uripp::convert(find(E_CONTENT_LENGTH), v);}
        catch (const std::exception& e) {throw_bad_request("Content-Length", e.what());}
        if (!ok)
            v = dflt;
        return ok;
    }
    bool content_hdr::content_length(size_t v) {return insert(E_CONTENT_LENGTH, uripp::convert(v));}
    std::string content_hdr::content_md5() const {return find(E_CONTENT_MD5);}
    bool content_hdr::content_md5(const std::string& v) {return insert(E_CONTENT_MD5, v);}
    std::string content_hdr::content_range() const {return find(E_CONTENT_RANGE);}
    bool content_hdr::content_range(const std::string& v) {return insert(E_CONTENT_RANGE, v);}
    std::string content_hdr::content_type() const {return find(E_CONTENT_TYPE);}
    bool content_hdr::content_type(const std::string& v) {return insert(E_CONTENT_TYPE, v);}
    RESTCGI_API void copy(const env& e, request_hdr& rh, content_hdr& ch) {
        for (env::hdr_iterator it = e.hdr_begin(); it != e.hdr_end(); ++it) {
            hdr::key k(it->first, true);
            if (k.category() == hdr::fc_entity_cnt) // It's a content field.
                ch.insert(k, it->second);
            else // General, request, and "other" fields into the request.
                rh.insert(k, it->second);
        }
    }
    content_hdr content_hdr_from_type(const std::string& type) {
        content_hdr ch;
        ch.content_type(type);
        return ch;
    }
}
