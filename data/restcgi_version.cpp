#include "test.h"
#include "../src/version.h"
#include "../src/exception.h"
#include <iostream>
#include <stdexcept>
using namespace std;
using namespace restcgi;
static void test_tag() {
    {version_tag v; TEST_ASSERT(v.is_null());}
    {version_tag v("a"); TEST_ASSERT(!v.is_null() && v.string() == "a");}
    {version_tag v(1); TEST_ASSERT(!v.is_null() && v.string() == "1");}
    {version_tag v(2ULL); version_tag v1("2"); TEST_ASSERT(v1 == v);}
}
static void test_constraint() {
    // must be modified
    {
        version_constraint vc(version(version_tag(1)), true);
        vc.assert_satisfied(version(version_tag(2)), method_e::GET);
        try {vc.assert_satisfied(version(version_tag(1)), method_e::GET); TEST_ASSERT(false);} catch (const not_modified& e) {(void)e;}
        try {vc.assert_satisfied(version(version_tag(1)), method_e::PUT); TEST_ASSERT(false);} catch (const precondition_failed& e) {(void)e;}
    } {
        date_time dt("Wed, 09 Jan 2002 03:04:08 GMT");
        version_constraint vc(version(dt), true);
        vc.assert_satisfied(version(dt + 1), method_e::GET);
        try {vc.assert_satisfied(version(dt), method_e::GET); TEST_ASSERT(false);} catch (const not_modified& e) {(void)e;}
    } {
        date_time dt("Wed, 09 Jan 2002 03:04:08 GMT");
        version_constraint vc(version(version_tag(1), dt), true);
        vc.assert_satisfied(version(version_tag(2), dt), method_e::GET);
        vc.assert_satisfied(version(version_tag(1), dt + 1), method_e::GET);
        try {vc.assert_satisfied(version(version_tag(1), dt), method_e::GET); TEST_ASSERT(false);} catch (const not_modified& e) {(void)e;}
    }
    // must be same
    {
        version_constraint vc(version(version_tag(1)), false);
        vc.assert_satisfied(version(version_tag(1)), method_e::GET);
        try {vc.assert_satisfied(version(version_tag(2)), method_e::GET); TEST_ASSERT(false);} catch (const precondition_failed& e) {(void)e;}
    } {
        date_time dt("Wed, 09 Jan 2002 03:04:08 GMT");
        version_constraint vc(version(dt), false);
        vc.assert_satisfied(version(dt), method_e::GET);
        try {vc.assert_satisfied(version(dt + 1), method_e::GET); TEST_ASSERT(false);} catch (const precondition_failed& e) {(void)e;}
    } {
        date_time dt("Wed, 09 Jan 2002 03:04:08 GMT");
        version_constraint vc(version(version_tag(1), dt), false);
        vc.assert_satisfied(version(version_tag(1), dt), method_e::GET);
        try {vc.assert_satisfied(version(version_tag(1), dt + 1), method_e::GET); TEST_ASSERT(false);} catch (const precondition_failed& e) {(void)e;}
    }
}
namespace restcgi_test {
    void version_tests(test_utils::test& t) {
        t.add("restcgi version tag", test_tag);
        t.add("restcgi version constraint", test_constraint);
    }
}

