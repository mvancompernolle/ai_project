#include "test.h"
#include "../src/rest.h"
#include "../src/endpoint.h"
#include "../src/resource.h"
#include "../src/method.h"
#include "../src/exception.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;
using namespace boost;
using namespace restcgi;
class test11 : public resource {
public:
    test11() : resource(method_e::GET | method_e::DEL | method_e::HEAD | method_e::OPTIONS) {}
    pointer locate(uri_path_type& path) {
        uri_path_ = path;
        path.clear();
        return pointer();
    }
    version read(bool veronly) {return version();}
    void on_responding(status_code_e& sc, response_hdr& rh, content_hdr& ch) {
        ch.content_type("text/xml");
        ch.content_length(9);
    }
    void write(ocontent::pointer oc) {
        *oc << "<a>hi</a>";
    }
    uri_path_type uri_path_;
};
class test1 : public resource {
public:
    test1(bool inhibit11 = false) : resource(0), inhibit11_(inhibit11) {}
    pointer locate(uri_path_type& path) {
        if (!path.empty() && !inhibit11_) {
            path.pop_front();
            return pointer(new test11());
        }
        return pointer();
    }
    bool inhibit11_;
};
static void test_locate() {
    /* locate signature changed, locate is covered in subsequent tests
    {
        resource::pointer tr(new test1());
        resource::pointer r = tr;
        uri_path up;
        rest::locate(up, r);
        TEST_ASSERT(tr == r);
    } {
        resource::pointer r(new test1());
        uri_path up("foo/bar");
        shared_ptr<test11> tr = dynamic_pointer_cast<test11>(rest::locate(up, r));
        TEST_ASSERT(tr && tr->uri_path_.encoding() == "bar");
    } {
        resource::pointer r(new test1(true));
        uri_path up("foo");
        try {rest::locate(up, r); TEST_ASSERT(false);} catch (const bad_request& e) {(void)e;}
    }
    */
}
class test2c : public resource {
public:
    test2c() : resource(method_e::GET | method_e::DEL | method_e::HEAD | method_e::OPTIONS) {}
    version read(bool veronly) {return version();}
    id_version_type create(icontent::pointer id) {
        date_time dt("Wed, 09 Jan 2002 03:04:08 GMT");
        return id_version_type(uri_path_type("123"), version(version_tag("1.2"), dt));
    }
    void on_responding(status_code_e& sc, response_hdr& rh, content_hdr& ch) {
        if (me() != method_e::OPTIONS)
            ch.content_type("application/x-www-form-urlencoded");
        ch.content_length(10);
    }
    void write(ocontent::pointer oc) {
        *oc << "a=123&b=hi";
    }
};
class test2 : public resource {
public:
    test2(bool thro = false) : resource(method_e::GET | method_e::POST | method_e::DEL | method_e::HEAD | method_e::OPTIONS), thro(thro) {}
    version read(bool veronly) {
        if (thro) {
            conflict e("a needs b");
            e.insert("woo", "hoo");
            throw e;
        }
        return version();
    }
    pointer create_child() {return pointer(new test2c());}
    void on_responding(status_code_e& sc, response_hdr& rh, content_hdr& ch) {
        if (me() != method_e::OPTIONS)
            ch.content_type("application/x-www-form-urlencoded");
        ch.content_length(10);
    }
    void write(ocontent::pointer oc) {
        *oc << "a=123&b=hi";
    }
    bool thro;
};
static void test_get() {
    {
        const char* p[] = {"REQUEST_METHOD=GET","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=/hello/foo","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/hello/foo", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        method::pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test2());
        rest::apply(m, r);
        TEST_ASSERT(oss.str() == "Status: 200 OK\r\nContent-Length: 10\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\na=123&b=hi");
    } {
        const char* p[] = {"REQUEST_METHOD=HEAD","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=/hello/foo","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/hello/foo", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        method::pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test2());
        rest::apply(m, r);
        TEST_ASSERT(oss.str() == "Status: 200 OK\r\nContent-Length: 10\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n");
    } {
        const char* p[] = {"REQUEST_METHOD=PUT","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=/hello/foo","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/hello/foo","HTTP_CONTENT_TYPE=text/html", 0};
        env e(p);
        istringstream iss("2\nmy content");
        ostringstream oss;
        method::pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test2());
        try {rest::apply(m, r); TEST_ASSERT(false);} catch (const method_not_allowed& e) {
            TEST_ASSERT(e.allow() == "GET, POST, DELETE, OPTIONS, HEAD");
        }
    }
}
static void test_post() {
    {
        const char* p[] = {"REQUEST_METHOD=POST","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=/hello/foo","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/hello/foo", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        method::pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test2());
        rest::apply(m, r);
        TEST_ASSERT(oss.str() == "Status: 201 Created\r\nETag: 1.2\r\nContent-Location: /hello/foo/123\r\n"
            "Last-Modified: Wed, 09 Jan 2002 03:04:08 GMT\r\nContent-Length: 10\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\na=123&b=hi");
    }
}
static void test_options() {
    {
        const char* p[] = {"REQUEST_METHOD=OPTIONS","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=/hello/foo","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/hello/foo", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        method::pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test2());
        rest::apply(m, r);
        TEST_ASSERT(oss.str() == "Status: 200 OK\r\nAllow: GET, POST, DELETE, OPTIONS, HEAD\r\nContent-Length: 0\r\n\r\n");
    } {
        const char* p[] = {"REQUEST_METHOD=OPTIONS","QUERY_STRING=","PATH_INFO=/*","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/*", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        method::pointer m = endpoint::create(e, iss, oss)->receive();
        TEST_ASSERT(rest::special_case(m) && oss.str() == "Status: 200 OK\r\nContent-Length: 0\r\n\r\n");
    }
}
static void test_rest() {
    {
        const char* p[] = {"REQUEST_METHOD=OPTIONS","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=","SCRIPT_URI=http://woo.com/cgi-bin/my.exe", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::method_pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test2());
        rest().process(m, r);
        TEST_ASSERT(oss.str() == "Status: 200 OK\r\nAllow: GET, POST, DELETE, OPTIONS, HEAD\r\nContent-Length: 0\r\n\r\n");
    } {
        const char* p[] = {"REQUEST_METHOD=OPTIONS","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=","SCRIPT_URI=http://woo.com/cgi-bin/my.exe","HTTP_HOST=woo.com", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::method_pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r1(new test1()); resource::pointer r2(new test2());
        rest::vhosts_resources_type rs; rs.insert(std::make_pair("foo.com", r1)); rs.insert(std::make_pair("woo.com", r2));
        rest().process(m, rs);
        TEST_ASSERT(oss.str() == "Status: 200 OK\r\nAllow: GET, POST, DELETE, OPTIONS, HEAD\r\nContent-Length: 0\r\n\r\n");
    } {
        const char* p[] = {"REQUEST_METHOD=GET","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=/foo/bar","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/foo/bar", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::method_pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test1());
        rest rp;
        rp.process(m, r);
        shared_ptr<test11> tr = dynamic_pointer_cast<test11>(rp.resource());
        TEST_ASSERT(tr && tr->uri_path_.encoding() == "bar");
        TEST_ASSERT(oss.str() == "Status: 200 OK\r\nContent-Length: 9\r\nContent-Type: text/xml\r\n\r\n<a>hi</a>");
    } {
        const char* p[] = {"REQUEST_METHOD=PUT","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=","HTTP_CONTENT_TYPE=text/html","SCRIPT_URI=http://woo.com/cgi-bin/my.exe", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::method_pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test2());
        rest().process(m, r);
        TEST_ASSERT(oss.str() == "Status: 405 Method Not Allowed\r\nAllow: GET, POST, DELETE, OPTIONS, HEAD\r\n\r\n");
    } {
        const char* p[] = {"REQUEST_METHOD=GET","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=/foo/bar","SCRIPT_URI=http://woo.com/cgi-bin/my.exe/foo/bar","HTTP_CONTENT_TYPE=text/html", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::method_pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test2());
        sc_ctmpls sccts;
        sccts.insert(sc_ctmpls::value_type(status_code_e::BAD_REQUEST, ctmpl("<a>%REQUEST_METHOD% got a \"%exception_what%\"</a>", "text/xml")));
        rest().process(m, r, sccts);
        TEST_ASSERT(oss.str() == "Status: 400 Bad Request\r\nContent-Type: text/xml\r\n\r\n<a>GET got a \"URI path contains an unknown resource: foo/bar\"</a>");
    } {
        const char* p[] = {"REQUEST_METHOD=GET","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=","SCRIPT_URI=http://woo.com/cgi-bin/my.exe","HTTP_CONTENT_TYPE=text/html", 0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::method_pointer m = endpoint::create(e, iss, oss)->receive();
        resource::pointer r(new test2(true));
        sc_ctmpls sccts;
        sccts.insert(sc_ctmpls::value_type(status_code_e::CONFLICT, ctmpl("<a>%REQUEST_METHOD% had conflict \"%exception_errmsg%\": %woo%</a>", "text/xml")));
        rest().process(m, r, sccts);
        TEST_ASSERT(oss.str() == "Status: 409 Conflict\r\nContent-Type: text/xml\r\n\r\n<a>GET had conflict \"a needs b\": hoo</a>");
    }
}
class test2a : public resource {
public:
    test2a() : resource(method_e::GET | method_e::PUT | method_e::DEL) {}
    version read(bool veronly) {return v;}
    version update(icontent::pointer ic) {return version_tag("444");}
    void on_responding(status_code_e& sc, response_hdr& rh, content_hdr& ch) {
        ch.content_type("text/plain");
    }
    void write(ocontent::pointer oc) {
        *oc << "woowoo";
    }
    version v;
};
static void test_version() {
    {
        const char* p[] = {"REQUEST_METHOD=PUT","PATH_INFO=","SCRIPT_URI=http://woo.com/cgi-bin/my.exe","HTTP_CONTENT_TYPE=text/html","HTTP_IF_MATCH=443",0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::method_pointer m = endpoint::create(e, iss, oss)->receive();
        test2a* t = new test2a();
        t->v = version_tag(443);
        resource::pointer r(t);
        rest().process(m, r);
        TEST_ASSERT(oss.str() == "Status: 200 OK\r\nETag: 444\r\nContent-Type: text/plain\r\n\r\nwoowoo");
    } {
        const char* p[] = {"REQUEST_METHOD=GET","PATH_INFO=","SCRIPT_URI=http://woo.com/cgi-bin/my.exe","HTTP_CONTENT_TYPE=text/html",
                           "HTTP_IF_NONE_MATCH=443","HTTP_IF_MODIFIED_SINCE=Wed, 09 Jan 2002 03:04:08 GMT",0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::method_pointer m = endpoint::create(e, iss, oss)->receive();
        test2a* t = new test2a();
        t->v = version(version_tag(443), date_time("Wed, 09 Jan 2002 03:04:09 GMT"));
        resource::pointer r(t);
        rest().process(m, r);
        TEST_ASSERT(oss.str() == "Status: 200 OK\r\nETag: 443\r\nLast-Modified: Wed, 09 Jan 2002 03:04:09 GMT\r\nContent-Type: text/plain\r\n\r\nwoowoo");
    } {
        const char* p[] = {"REQUEST_METHOD=PUT","PATH_INFO=","SCRIPT_URI=http://woo.com/cgi-bin/my.exe","HTTP_CONTENT_TYPE=text/html","HTTP_IF_MATCH=443",0};
        env e(p);
        istringstream iss;
        ostringstream oss;
        endpoint::method_pointer m = endpoint::create(e, iss, oss)->receive();
        test2a* t = new test2a();
        t->v = version_tag(444);
        resource::pointer r(t);
        rest().process(m, r);
        TEST_ASSERT(oss.str() == "Status: 412 Precondition Failed\r\n\r\n");
    }
}
namespace restcgi_test {
    void resource_tests(test_utils::test& t) {
        t.add("restcgi resource locate", test_locate);
        t.add("restcgi resource get", test_get);
        t.add("restcgi resource post", test_post);
        t.add("restcgi resource options", test_options);
        t.add("restcgi resource rest", test_rest);
        t.add("restcgi resource version", test_version);
    }
}
