#include "test.h"
#include "../src/utils.h"
#include "../src/date_time.h"
#include <iostream>
#include <stdexcept>
using namespace std;
using namespace restcgi;
static void test_convert() {
    struct tm tm; tm.tm_mday = 9; tm.tm_mon = 0; tm.tm_year = 102; tm.tm_hour = 3; tm.tm_min = 4; tm.tm_sec = 8; tm.tm_wday = 3;
    date_time t(tm);
    TEST_ASSERT(uripp::convert(t) == "Wed, 09 Jan 2002 03:04:08 GMT");
    {
        date_time v;
        TEST_ASSERT(!uripp::convert("   ", v) && v.is_null());
        TEST_ASSERT(uripp::convert("Wed, 09 Jan 2002 03:04:08 GMT", v) && v == t);
        v = date_time();
        TEST_ASSERT(uripp::convert("    Wed, 09 Jan 2002 03:04:08 GMT     \n\t\n", v) && v == t);
        try {uripp::convert("Wed, 09 Jan 2002 03:04:08 GMT   \na", v); TEST_ASSERT(false);} catch (const std::invalid_argument&e) {(void)e;}
    }
    {date_time v; TEST_ASSERT(uripp::convert("    Wednesday, 09-Jan-02 03:04:08 GMT     ", v) && v == t);}
    {date_time v; TEST_ASSERT(uripp::convert("    Wed Jan  9 03:04:08 2002\t\n     ", v) && v == t);}
}
namespace restcgi_test {
    void utils_tests(test_utils::test& t) {
        t.add("restcgi utils convert", test_convert);
    }
}
