#include "test.h"
#include "../src/status_code_e.h"
using namespace std;
using namespace restcgi;
static void test() {
    {status_code_e v; TEST_ASSERT(v.is_null());}
    {status_code_e v(status_code_e::CONTINUE); TEST_ASSERT(v == status_code_e::CONTINUE && !::strcmp(v.cstring(), "100 Continue") && (int)v == 100);}
    {status_code_e v(status_code_e::OK); TEST_ASSERT(v == status_code_e::OK && !::strcmp(v.cstring(), "200 OK"));}
    {status_code_e v(status_code_e::HTTP_VERSION_NOT_SUPPORTED); TEST_ASSERT(v == status_code_e::HTTP_VERSION_NOT_SUPPORTED && !::strcmp(v.cstring(), "505 HTTP Version not supported") && (int)v == 505);}
    {status_code_e v(409); TEST_ASSERT(v == status_code_e::CONFLICT && !::strcmp(v.cstring(), "409 Conflict") && (int)v == 409);}
}
namespace restcgi_test {
    void status_code_tests(test_utils::test& t) {
        t.add("restcgi status code", test);
    }
}
