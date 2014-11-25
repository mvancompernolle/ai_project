#include "test.h"
#include "../src/env.h"
#include <iostream>
#include <stdexcept>
using namespace std;
using namespace restcgi;
static void test_params() {
    {env e; TEST_ASSERT(e.script_uri().empty());}
    {
        const char* p[] = {"SCRIPT_URI=/foo/bar", "SERVER_NAME=myserver", 0};
        env e(p);
        TEST_ASSERT(e.script_uri() == "/foo/bar" && e.server_name() == "myserver" && e.content_length().empty());
    }
    {
        const char* p[] = {"SCRIPT_URI=/foo/bar", "HTTP_CONTENT_LENGTH=7", 0};
        env e(p);
        TEST_ASSERT(e.script_uri() == "/foo/bar" && e.content_length() == "7");
    }
}
static void test_hdr() {
    {
        const char* p[] = {"SCRIPT_URI=/foo/bar", "HTTP_CONTENT_LENGTH=7", 0};
        env e(p);
        TEST_ASSERT(e.script_uri() == "/foo/bar" && e.content_length() == "7");
        string s;
        TEST_ASSERT(e.hdr_find("content-length", s) && s == "7");
        TEST_ASSERT(!e.hdr_find("woowoo", s));
    }
    {
        const char* p[] = {"SCRIPT_URI=/foo/bar", 0};
        env e(p);
        env::hdr_iterator it = e.hdr_begin();
        TEST_ASSERT(it == e.hdr_end());
        try {++it; TEST_ASSERT(false);} catch (const std::domain_error& e) {(void)e;}
    }
    {
        const char* p[] = {"HTTP_CONTENT_TYPE=text/html", "SCRIPT_URI=/foo/bar", "HTTP_CONTENT_LENGTH=7", 0};
        env e(p);
        env::hdr_iterator it = e.hdr_begin();
        TEST_ASSERT(it->first == "content-length" && it->second == "7");
        TEST_ASSERT((++it)->first == "content-type" && it->second == "text/html");
        TEST_ASSERT(it++ != e.hdr_end() && it == e.hdr_end());
    }
}
namespace restcgi_test {
    void env_tests(test_utils::test& t) {
        t.add("restcgi env params", test_params);
        t.add("restcgi env hdr", test_hdr);
    }
}
