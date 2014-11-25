#include "test.h"
#include "../src/method.h"
#include "../src/content.h"
#include "../src/endpoint.h"
#include "../src/env.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;
using namespace restcgi;
static void test_put() {
    {
        const char* p[] = {"REQUEST_METHOD=PUT","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=/hello/foo","HTTP_CONTENT_TYPE=text/html","HTTP_CONTENT_LENGTH=7","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/hello/foo", 0};
        env e(p);
        istringstream iss("2\nmy content");
        ostringstream oss;
        endpoint::pointer ep = endpoint::create(e, iss, oss);
        method::pointer m = ep->receive();
        TEST_ASSERT(m->e() == method_e::PUT && m->uri_path().encoding() == "/hello/foo");
        {
            uripp::query::const_iterator it = m->uri_query().begin();
            TEST_ASSERT(it->first == "a" && it->second == "123");
            TEST_ASSERT((++it)->first == "b" && it->second == "x y+z");
            TEST_ASSERT(++it == m->uri_query().end());
            int i = 9;
            bool is_null = true;
            TEST_ASSERT(m->uri_query().find("a", i, is_null) && i == 123 && !is_null);
        }
        int i; string s0, s1;
        *m->icontent() >> i >> s0 >> s1;
        TEST_ASSERT(i == 2 && s0 == "my" && s1 == "content");
        content::hdr_type ch;
        ch.content_type("application/foo");
        ch.content_length(6);
        *m->respond(ch) << "woowoo";
        TEST_ASSERT(oss.str() == "Status: 200 OK\r\nContent-Length: 6\r\nContent-Type: application/foo\r\n\r\nwoowoo");
    } {
        const char* p[] = {"REQUEST_METHOD=POST","QUERY_STRING=restPUT=&a=123&b=x%20y+z","PATH_INFO=/hello/foo","HTTP_CONTENT_TYPE=text/html","HTTP_CONTENT_LENGTH=7","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/hello/foo", 0};
        env e(p);
        istringstream iss("2\nmy content");
        ostringstream oss;
        endpoint::pointer ep = endpoint::create(e, iss, oss);
        method::pointer m = ep->receive();
        TEST_ASSERT(m->e() == method_e::PUT && m->uri_path().encoding() == "/hello/foo");
        {
            uripp::query::const_iterator it = m->uri_query().begin();
            TEST_ASSERT(it->first == "a" && it->second == "123");
            TEST_ASSERT((++it)->first == "b" && it->second == "x y+z");
            TEST_ASSERT(++it == m->uri_query().end());
            int i = 9;
            bool is_null = true;
            TEST_ASSERT(m->uri_query().find("a", i, is_null) && i == 123 && !is_null);
        }
    }
    TEST_ASSERT(method_e::mask_to_string(method_e::GET|method_e::DEL) == "GET, DELETE");
}
static void test_delete() {
    {
        const char* p[] = {"REQUEST_METHOD=DELETE","QUERY_STRING=","PATH_INFO=/hello/foo","SCRIPT_URI=http://1.2.3.4:8080/cgi-bin/my.exe/hello/foo",
                           "HTTP_HOST=1.2.3.4",
                           "HTTP_USER_AGENT=Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.5) Gecko/20070713 Firefox/2.0.0.5",
                           "HTTP_ACCEPT=text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5",
                           "HTTP_ACCEPT_LANGUAGE=en-us,en;q=0.5",
                           "HTTP_KEEP_ALIVE=300",
                           0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::pointer ep = endpoint::create(e, iss, oss);
        method::pointer m = ep->receive();
        TEST_ASSERT(m->e() == method_e::DEL && m->uri_path().encoding() == "/hello/foo");
        TEST_ASSERT(m->uri_query().begin() == m->uri_query().end());
        TEST_ASSERT(m->request_hdr().host() == "1.2.3.4");
        TEST_ASSERT(m->request_hdr().user_agent() == "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.5) Gecko/20070713 Firefox/2.0.0.5");
        TEST_ASSERT(m->request_hdr().accept() == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
        TEST_ASSERT(m->request_hdr().accept_language() == "en-us,en;q=0.5");
        int ka = 0;
        TEST_ASSERT(m->request_hdr().find("keep-alive", ka) && ka == 300);
        TEST_ASSERT(!m->request_hdr().find("foobar", ka));
        string s;
        *m->icontent() >> s;
        TEST_ASSERT(s.empty() && iss.eof());
        method::response_hdr_type rh;
        rh.etag("1.1");
        m->respond(status_code_e::ACCEPTED, rh);
        TEST_ASSERT(oss.str() == "Status: 202 Accepted\r\nETag: 1.1\r\n\r\n");
    } {
        const char* p[] = {"REQUEST_METHOD=POST","QUERY_STRING=restDELETE=","PATH_INFO=/hello/foo",0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::pointer ep = endpoint::create(e, iss, oss);
        method::pointer m = ep->receive();
        TEST_ASSERT(m->e() == method_e::DEL && m->uri_path().encoding() == "/hello/foo");
    }
}
namespace restcgi_test {
    void method_tests(test_utils::test& t) {
        t.add("restcgi method put", test_put);
        t.add("restcgi method delete", test_delete);
    }
}
