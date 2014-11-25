#include "test.h"
#include "../src/hdr.h"
#include "../src/env.h"
#include "../src/date_time.h"
#include <sstream>
#include <iostream>
#include <stdexcept>
using namespace std;
using namespace restcgi;
static void test_insert() {
    {request_hdr h; TEST_ASSERT(h.begin() == h.end());}
    {response_hdr h; TEST_ASSERT(h.allow("foo") && h.begin()->second == "foo" && !h.insert("allow", "a"));}
    {response_hdr h; TEST_ASSERT(h.insert("woo", "foo") && h.begin()->second == "foo" && !h.insert("woo", "a"));}
    {content_hdr h; TEST_ASSERT(h.content_type("foo") && !strcmp(h.begin()->first.name_as_cstring(), "Content-Type") && h.begin()->second == "foo");}
    {
        response_hdr h; date_time t, dt("Mon, 20 Aug 2007 16:24:29 GMT"); 
        TEST_ASSERT(h.expires(dt) && h.expires(t, t) && t == dt);
    } {
        response_hdr h;
        TEST_ASSERT(h.retry_after(5) && h.insert("pragma", "foo"));
        response_hdr::const_iterator it = h.begin();
        TEST_ASSERT(!strcmp(it->first.name_as_cstring(), "Pragma") && it->second == "foo");
        TEST_ASSERT(!strcmp((++it)->first.name_as_cstring(), "Retry-After") && it->second == "5" && ++it == h.end());
        TEST_ASSERT(!h.insert("retry-after", "woowoo"));
        date_time dt;
        size_t secs;
        TEST_ASSERT(h.retry_after(dt, secs) && dt.is_null() && secs == 5);
        it = h.begin();
        TEST_ASSERT(it->second == "foo" && (++it)->second == "5" && ++it == h.end());
    } {
        content_hdr h;
        TEST_ASSERT(h.insert("hi", 1.2) && h.insert("bye", "foo"));
        request_hdr::const_iterator it = h.begin();
        TEST_ASSERT(it->second == "foo" && (++it)->second == "1.2" && ++it == h.end());
        TEST_ASSERT(!h.insert("hi", "bar"));
        TEST_ASSERT(!h.insert("bye", "bar"));
        it = h.begin();
        TEST_ASSERT(it->second == "foo" && (++it)->second == "1.2" && ++it == h.end());
        TEST_ASSERT(h.content_md5("woowoo"));
        it = h.begin();
        TEST_ASSERT(it->second == "foo" && (++it)->second == "1.2" && (++it)->second == "woowoo" && ++it == h.end());
    } {
        const char* p[] = {"HTTP_CONTENT_TYPE=text/html", "HTTP_FOO_BAR=true", "HTTP_CONTENT_LENGTH=7", "SCRIPT_URI=/foo/bar", "HTTP_HOST=myhost", "HTTP_CONNECTION=hello", 0};
        env e(p);
        request_hdr rh;
        content_hdr ch;
        copy(e, rh, ch);
        {
            content_hdr::const_iterator it = ch.begin();
            TEST_ASSERT(!strcmp(it->first.name_as_cstring(), "Content-Length") && it->second == "7");
            TEST_ASSERT(!strcmp((++it)->first.name_as_cstring(), "Content-Type") && it->second == "text/html");
            TEST_ASSERT(++it == ch.end());
            ostringstream oss;
            oss << ch;
            TEST_ASSERT(oss.str() == "Content-Length: 7\r\nContent-Type: text/html\r\n");
            size_t len;
            TEST_ASSERT(ch.find("content-length", len) && len == 7);
            len = 0;
            TEST_ASSERT(ch.find("CONTENT-LENGTH", len) && len == 7);
            len = 0;
            TEST_ASSERT(ch.content_length(len, 0) && len == 7);
        } {
            request_hdr::const_iterator it = rh.begin();
            TEST_ASSERT(!strcmp(it->first.name_as_cstring(), "Connection") && it->second == "hello");
            TEST_ASSERT(!strcmp((++it)->first.name_as_cstring(), "Host") && it->second == "myhost");
            TEST_ASSERT(!strcmp((++it)->first.name_as_cstring(), "foo-bar") && it->second == "true");
            TEST_ASSERT(++it == rh.end());
            bool b = false;
            TEST_ASSERT(rh.find("Foo-Bar", b) && b);
            ostringstream oss;
            oss << rh;
            TEST_ASSERT(oss.str() == "Connection: hello\r\nHost: myhost\r\nfoo-bar: true\r\n");
        }
    }
}
static void test_cookie() {
    {
        const char* p[] = {"HTTP_CONTENT_TYPE=text/html", "HTTP_COOKIE=a=b; $Path=/foo/bar, c=555", 0};
        env e(p);
        request_hdr rh;
        content_hdr ch;
        copy(e, rh, ch);
        cookies::const_iterator it = rh.cookies().begin();
        TEST_ASSERT(it->first == "a" && it->second.value<string>() == "b" && it->second.attrs().path().front() == "foo");
        TEST_ASSERT((++it)->first == "c" && it->second.value<int>() == 555);
        TEST_ASSERT(++it == rh.cookies().end());
    } {
        response_hdr h;
        h.cookies().insert(cookie("foo", "bar"));
        response_hdr::const_iterator it = h.begin();
        TEST_ASSERT(!strcmp(it->first.name_as_cstring(), "Set-Cookie") && it->second == "foo=bar; Version=1");
        TEST_ASSERT(++it == h.end());
        cookie_attrs a; a.secure(true);
        h.cookies().insert(cookie("woo", "hoo xy;", a));
        it = h.begin();
        TEST_ASSERT(!strcmp(it->first.name_as_cstring(), "Set-Cookie") && it->second == "foo=bar; Version=1, woo=\"hoo xy;\"; Secure; Version=1");
        TEST_ASSERT(++it == h.end());
    }
}
namespace restcgi_test {
    void hdr_tests(test_utils::test& t) {
        t.add("restcgi hdr insert", test_insert);
        t.add("restcgi hdr cookie", test_cookie);
    }
}
