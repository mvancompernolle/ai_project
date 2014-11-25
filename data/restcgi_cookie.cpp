#include "test.h"
#include "../src/cookie.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;
using namespace restcgi;
static void test_response() {
    {cookies v(false); TEST_ASSERT(v.empty() && v.find("foo") == v.end());}
    {
        cookies v(false); cookie c(http_token("foo"), http_word("bar")); v.insert(c);
        cookies::const_iterator it = v.find("foo");
        TEST_ASSERT(!v.empty() && it->second.id() == "foo::");
        TEST_ASSERT(!v.insert(c));
        cookie_attrs a; a.domain(".foo.com"); a.http_only(true); a.max_age(3000);
        cookie c1(http_token("foo"), http_word("woo"), a);
        v.insert(c1);
        it = v.find("foo");
        TEST_ASSERT(it->second.id() == "foo::" && (++it)->second.id() == "foo:.foo.com:");
        ostringstream oss; oss << v;
        TEST_ASSERT(oss.str() == "foo=bar; Version=1, foo=woo; Domain=\".foo.com\"; Max-Age=3000; HttpOnly; Version=1");
    }
    try {cookie_attrs v; v.domain("foo.com"); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
}
static void test_request() {
    {cookies v(true); v.insert(""); TEST_ASSERT(v.empty());}
    {
        cookies v(true); v.insert("a=1");
        cookies::const_iterator it = v.begin();
        TEST_ASSERT(!v.empty() && it->first == "a" && it->second.value<int>() == 1 && ++it == v.end());
    } {
        cookies v(true); v.insert("$version=1; a=1");
        cookies::const_iterator it = v.begin();
        TEST_ASSERT(!v.empty() && it->first == "a" && it->second.value<int>() == 1 && it->second.attrs().version() == 1 && ++it == v.end());
    } {
        cookies v(true); v.insert("a = \"-1\"");
        cookies::const_iterator it = v.begin();
        TEST_ASSERT(!v.empty() && it->first == "a" && it->second.value<int>() == -1 && ++it == v.end());
    } {
        cookies v(true);
        v.insert("a=b; $path=/foo/bar");
        cookies::const_iterator it = v.begin();
        TEST_ASSERT(!v.empty() && it->first == "a" && it->second.value<string>() == "b");
        TEST_ASSERT(it->second.attrs().path().front() == "foo" && ++it == v.end());
    } {
        cookies v(true);
        v.insert("a=b; $Path=/foo/bar, c=555");
        cookies::const_iterator it = v.begin();
        TEST_ASSERT(it->first == "a" && it->second.value<string>() == "b" && it->second.attrs().path().front() == "foo");
        TEST_ASSERT((++it)->first == "c" && it->second.value<int>() == 555);
        TEST_ASSERT(++it == v.end());
    } {
        cookies v(true);
        v.insert("a=b; $path=/foo/bar, a=555; $path=/foo");
        cookies::const_iterator it = v.begin();
        TEST_ASSERT(it->first == "a" && it->second.value<string>() == "b" && it->second.attrs().path().front() == "foo");
        TEST_ASSERT((++it)->first == "a" && it->second.value<int>() == 555 && it->second.attrs().path().front() == "foo");
        TEST_ASSERT(++it == v.end());
    } {
        cookies v(true);
        TEST_ASSERT(!v.insert("a=b $path=/foo/bar, c=555"));
        cookies::const_iterator it = v.begin();
        TEST_ASSERT(it->first == "a" && it->second.value<string>() == "b");
        TEST_ASSERT((++it)->first == "c" && it->second.value<int>() == 555);
        TEST_ASSERT(++it == v.end());
    }
}
namespace restcgi_test {
    void cookie_tests(test_utils::test& t) {
        t.add("restcgi cookie response", test_response);
        t.add("restcgi cookie request", test_request);
    }
}
