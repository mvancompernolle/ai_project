#include "test.h"
#include "../src/date_time.h"
#include <iostream>
#include <stdexcept>
using namespace std;
using namespace restcgi;
static void test() {
    struct tm tm; tm.tm_mday = 9; tm.tm_mon = 0; tm.tm_year = 102; tm.tm_hour = 3; tm.tm_min = 4; tm.tm_sec = 8;
    date_time t(tm);
    TEST_ASSERT(t.string() == "Wed, 09 Jan 2002 03:04:08 GMT");
    {date_time v(1187221162); TEST_ASSERT(v.string() == "Wed, 15 Aug 2007 23:39:22 GMT");}
    {date_time v("Wed, 09 Jan 2002 03:04:08 GMT"); TEST_ASSERT(t == v);}
    {date_time v("Wednesday, 09-Jan-02 03:04:08 GMT"); TEST_ASSERT(t == v);}
    {date_time v("Wed Jan  9 03:04:08 2002"); TEST_ASSERT(t == v);}
    {date_time v("Wed Jan  9 03:04:08 2002"); TEST_ASSERT(t == v);}
    {date_time v("Thu Jan 10 04:05:47 2002"); TEST_ASSERT(t + 90099 == v);}
    // invalid wday
    try {date_time v("Mon Jan  9 03:04:08 2002"); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
    // invalid mday
    struct tm tm1; tm1.tm_mday = 31; tm1.tm_mon = 3; tm1.tm_year = 102; tm1.tm_hour = 3; tm1.tm_min = 4; tm1.tm_sec = 8;
    try {date_time v(tm1); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
    // bogus char
    try {date_time v("Thu, 28 Feb 2002 03:04:08 xGMT"); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
    try {date_time v("Thu, 28 Feb 2002  03:04:08 GMT"); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
    // wrong calendar
    try {date_time v("Fri, 29 Feb 2002 03:04:08 GMT"); TEST_ASSERT(false);} catch (const std::invalid_argument& e) {(void)e;}
}
namespace restcgi_test {
    void date_time_tests(test_utils::test& t) {
        t.add("restcgi date_time", test);
    }
}
