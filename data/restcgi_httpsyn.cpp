#include "test.h"
#include "../src/httpsyn.h"
#include <iostream>
#include <stdexcept>
using namespace std;
using namespace restcgi;
using namespace restcgi::httpsyn;
static void test_token() {
    {
        string s(""); http_token t; string::const_iterator it = s.begin();
        TEST_ASSERT(!parse(it, s.end(), t) && it == s.begin() && t.is_null());
    } {
        string s("()<>@,;:\\\"/[]?={}"); 
        for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
            http_token t;
            string::const_iterator itt = it;
            TEST_ASSERT(!parse(itt, s.end(), t) && it == itt && t.is_null());
        }
    } {
        string s("a"); http_token t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "a");
    } {
        string s("abc"); http_token t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "abc");
    } {
        string s("    \ta"); http_token t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "a");
    } {
        string s("    \ta="); http_token t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && *it == '=' && t.string() == "a");
    } {
        string s("    \ta123 "); http_token t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && *it == ' ' && t.string() == "a123");
    }
    try {http_token("a\t"); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
}
static void test_text() {
    {
        string s(""); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(!parse(it, s.end(), t) && it == s.begin() && t.empty());
    } {
        string s("\001\n\r\r"); 
        for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
            http_text t;
            string::const_iterator itt = it;
            TEST_ASSERT(!parse(itt, s.end(), t) && it == itt && t.empty());
        }
    } {
        string s("a"); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "a");
    } {
        string s("a \t \t \t \r\n\t"); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "a ");
    } {
        string s("ab\r\n    \t  c"); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "ab c");
    } {
        string s("    a\n    vvv"); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && *it == '\n' && t.string() == " a");
    } {
        string s("    a  \t\002    vvv"); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && *it == '\002' && t.string() == " a ");
    } {
        string s("    a  \t    vx"); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t, "v") && *it == 'v' && t.string() == " a ");
    } {
        string s("    a  \t    "); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(!parse(it, s.end(), t, "v") && it == s.begin() && t.empty());
    } {
        string s("    a  \t    \rv"); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(!parse(it, s.end(), t, "v") && it == s.begin() && t.empty());
    } {
        string s("v"); http_text t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t, "v") && *it == 'v' && t.empty());
    } {
        http_text t("x");
        t += "ab    \r\n    \t  c";
        TEST_ASSERT(t.string() == "xab c");
    } {
        http_text t("x");
        t += http_text("ab\r\n    \t  c\ty");
        TEST_ASSERT(t.string() == "xab c y");
    }
    try {http_text("a\r   "); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
    try {http_text("a  \r\n"); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
}
static void test_word() {
    {
        string s(""); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(!parse(it, s.end(), t) && it == s.begin() && t.empty());
    } {
        string s("   \n\r    \t"); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(!parse(it, s.end(), t) && it == s.begin() && t.empty());
    } {
        string s("\001\n\t\r"); 
        for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
            http_word t;
            string::const_iterator itt = it;
            TEST_ASSERT(!parse(itt, s.end(), t) && it == itt && t.empty());
        }
    } {
        string s("a"); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "a");
    } {
        string s("  a=b "); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && *it == '=' && t.string() == "a");
    } {
        string s("  \t     \"ab\r\n    \t  c\""); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "ab c");
    } {
        string s("    a\n    vvv"); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && *it == '\n' && t.string() == "a");
    } {
        string s("\"\\\"\""); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "\"");
    } {
        string s("\"\\\\\\\"\""); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "\\\"");
    } {
        string s("\"\\\"\\\"\""); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && it == s.end() && t.string() == "\"\"");
    } {
        string s("    \"a\\\"b\"c    vvv"); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && *it == 'c' && t.string() == "a\"b");
    } {
        string s("    \"a\\\\b\"c    vvv"); http_word t; string::const_iterator it = s.begin();
        TEST_ASSERT(parse(it, s.end(), t) && *it == 'c' && t.string() == "a\\b");
    } {
        http_word t("x");
        t += "ab\r\n    \t  c";
        TEST_ASSERT(t.string() == "xab c");
    } {
        http_word t("x");
        t += http_word("\"ab\r\n    \t  cy\"");
        TEST_ASSERT(t.string() == "xab cy");
    }
    try {http_word("\"a\r\""); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
    try {http_word("\"a\\\""); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
}
static void test_parse() {
    {
        string s(""); string::const_iterator it = s.begin(); 
        TEST_ASSERT(!parse(it, s.end(), "") && it == s.begin());
    } {
        string s("   \n\t   "); string::const_iterator it = s.begin(); 
        TEST_ASSERT(!parse(it, s.end(), "x") && it == s.begin());
    } {
        string s("aasdfasd"); string::const_iterator it = s.begin(); 
        TEST_ASSERT(!parse(it, s.end(), "") && it == s.begin());
    } {
        string s("   aasdfaxd"); string::const_iterator it = s.begin(); 
        TEST_ASSERT(parse(it, s.end(), "aa") && *it == 's');
    } {
        string s("   aasdfaxd"); string::const_iterator it = s.begin(); 
        TEST_ASSERT(parse(it, s.end(), "aasdfaxd") && it == s.end());
    }
}
namespace restcgi_test {
    void httpsyn_tests(test_utils::test& t) {
        t.add("restcgi httpsyn token", test_token);
        t.add("restcgi httpsyn text", test_text);
        t.add("restcgi httpsyn word", test_word);
        t.add("restcgi httpsyn parse", test_parse);
    }
}
