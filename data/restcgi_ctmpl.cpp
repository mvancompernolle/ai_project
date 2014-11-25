#include "test.h"
#include "../src/ctmpl.h"
#include "../src/endpoint.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;
using namespace restcgi;
static void test_app() {
    ctmpl::map_type me;
    {ctmpl v(""); TEST_ASSERT(v.eval(me) == "");}
    {
        string s("adfasdff asdf asdf asdfa dfa dfasf asfa3454352345 23452 5=wert3-t");
        ctmpl v(s); TEST_ASSERT(v.eval(me) == s);
    }
    {ctmpl v("%"); TEST_ASSERT(v.eval(me) == "%");}
    {ctmpl v("abc%"); TEST_ASSERT(v.eval(me) == "abc%");}
    {ctmpl v("%%"); TEST_ASSERT(v.eval(me) == "%%");}
    {
        string s("adfasdff %asdfasdf-asdfa_dfa dfasf asfa3454352345 23452 5=wert3-t");
        ctmpl v(s); TEST_ASSERT(v.eval(me) == s);
    } 
    ctmpl::map_type m; m.insert(make_pair("foo","bar"));
    {ctmpl v("%foo"); TEST_ASSERT(v.eval(m) == "%foo");}
    {ctmpl v("%foo%"); TEST_ASSERT(v.eval(m) == "bar");}
    {ctmpl v("%foo%%foo%"); TEST_ASSERT(v.eval(m) == "barbar");}
    {ctmpl v("%%foo%"); TEST_ASSERT(v.eval(m) == "%bar");}
    {ctmpl v("%%foo%%woowoo%"); TEST_ASSERT(v.eval(m) == "%bar%woowoo%");}
    {ctmpl v("%%foo%%woowoo% "); TEST_ASSERT(v.eval(m) == "%bar%woowoo% ");}
    {ctmpl v("%%woowoo%%foo%"); TEST_ASSERT(v.eval(m) == "%%woowoo%bar");}
    {ctmpl v("%%woowoo% %foo%"); TEST_ASSERT(v.eval(m) == "%%woowoo% bar");}
    {ctmpl v("  % %foo%"); TEST_ASSERT(v.eval(m) == "  % bar");}
    {ctmpl v("%a %foo%"); TEST_ASSERT(v.eval(m) == "%a bar");}
    {ctmpl v("%a %foo%q"); TEST_ASSERT(v.eval(m) == "%a barq");}
    {ctmpl v(" %a %foo%q"); TEST_ASSERT(v.eval(m) == " %a barq");}
}
static void test_hdr() {
    const char* p[] = {"REQUEST_METHOD=PUT","QUERY_STRING=a=123&b=x%20y+z","PATH_INFO=/hello/foo","HTTP_CONTENT_TYPE=text/html","HTTP_ACCEPT=text","HTTP_CONTENT_LENGTH=7", 0};
    env e(p);
    {ctmpl v(" %a %content-length%q"); TEST_ASSERT(v.eval(e) == " %a 7q");}
    {ctmpl v(" %REQUEST_METHOD%  %%accept%q"); TEST_ASSERT(v.eval(e) == " PUT  %textq");}
}
namespace restcgi_test {
    void ctmpl_tests(test_utils::test& t) {
        t.add("restcgi ctmpl app", test_app);
        t.add("restcgi ctmpl hdr", test_hdr);
    }
}
