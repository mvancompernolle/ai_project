/*
Copyright (c) 2009 zooml.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "date_time.h"
#include "utils.h"
#include <uripp/utils.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
static const char* wkday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char* weekday[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static const char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
namespace restcgi {
    const time_t date_time::NULL_TIME = 0;
    date_time::date_time() : time_(NULL_TIME) {}
    date_time::date_time(const time_t& t) : time_(t) {}
    date_time::date_time(const tm_type& tm, bool check_wday) : time_(to_time(tm, check_wday)) {}
    date_time::date_time(unsigned short y, unsigned char mon, unsigned char d, unsigned char h, unsigned char min, unsigned char s) {
        tm_type tm;
        ::memset(&tm, 0, sizeof(tm));
        tm.tm_year = y - 1900;
        tm.tm_mon = mon - 1;
        tm.tm_mday = d;
        tm.tm_hour = h;
        tm.tm_min = min;
        tm.tm_sec = s;
        time_ = to_time(tm, false);
    }
    date_time::date_time(const std::string& s) {
        std::string::const_iterator first = s.begin();
        if (!parse(first, s.end(), *this) || first != s.end())
            throw std::invalid_argument("invalid date and time: " + s);
    }
    date_time date_time::now() {return date_time(::time(0));}
    bool date_time::operator <(const date_time& rhs) const {
        if (is_null() && !rhs.is_null())
            return true;
        if (!is_null() && rhs.is_null())
            return false;
        return time_ < rhs.time_;
    }
    date_time& date_time::operator +=(int seconds) {
        time_t t = time_ + seconds;
        if (t < 0)
            throw std::invalid_argument("date and time over/underflow adding seconds: " + uripp::convert(seconds));
        time_ = t;
        return *this;
    }
    date_time& date_time::operator -=(int seconds) {
        time_t t = time_ - seconds;
        if (t < 0)
            throw std::invalid_argument("date and time over/underflow subtracting seconds: " + uripp::convert(seconds));
        time_ = t;
        return *this;
    }
    date_time::tm_type date_time::tm() const {
        if (is_null())
            throw std::domain_error("cannot convert null date and time to time structure");
        return to_tm(time_);
    }
    std::string date_time::string() const {
        if (is_null())
            throw std::domain_error("cannot convert null date and time to string");
        tm_type tm;
        to_tm(time_, tm);
        /// Sun, 06 Nov 1994 08:49:37 GMT
        std::ostringstream oss;
        oss << wkday[tm.tm_wday] << ", "
            << std::setw(2) << std::setfill('0') << tm.tm_mday << " "
            << month[tm.tm_mon] << " "
            << (tm.tm_year + 1900) << " "
            << std::setw(2) << std::setfill('0') << tm.tm_hour << ":"
            << std::setw(2) << std::setfill('0') << tm.tm_min << ":"
            << std::setw(2) << std::setfill('0') << tm.tm_sec << " GMT";
        return oss.str();
    }
    std::string date_time::iso_string() const {
        if (is_null())
            throw std::domain_error("cannot convert null date and time to string");
        tm_type tm;
        to_tm(time_, tm);
        /// 1994-11-06T08:49:37Z
        std::ostringstream oss;
        oss
            << (tm.tm_year + 1900) << "-"
            << std::setw(2) << std::setfill('0') << (tm.tm_mon + 1) << "-"
            << std::setw(2) << std::setfill('0') << tm.tm_mday << "T"
            << std::setw(2) << std::setfill('0') << tm.tm_hour << ":"
            << std::setw(2) << std::setfill('0') << tm.tm_min << ":"
            << std::setw(2) << std::setfill('0') << tm.tm_sec << "Z";
        return oss.str();
    }
    void date_time::assert_in_range(const tm_type& tm, bool check_wday) {
        // Note that this does not check everything,
        // e.g. the mday could be 31 in Feb.
        std::string err;
        if (check_wday && (tm.tm_wday < 0 || 6 < tm.tm_wday))
            err = std::string("week day (1-7): ") + uripp::convert(tm.tm_wday + 1);
        else if (tm.tm_mday < 1 || 31 < tm.tm_mday)
            err = std::string("month day (1-31): ") + uripp::convert(tm.tm_mday);
        else if (tm.tm_mon < 0 || 11 < tm.tm_mon)
            err = std::string("month (1-12): ") + uripp::convert(tm.tm_mon + 1);
        else if (tm.tm_year < 70)
            err = std::string("year (1970-): ") + uripp::convert(tm.tm_year + 1900);
        else if (tm.tm_hour < 0 || 23 < tm.tm_hour)
            err = std::string("hour (0-23): ") + uripp::convert(tm.tm_hour);
        else if (tm.tm_min < 0 || 59 < tm.tm_min)
            err = std::string("minute (0-59): ") + uripp::convert(tm.tm_min);
        else if (tm.tm_sec < 0 || 60 < tm.tm_sec) // Leap second.
            err = std::string("seconds (0-60): ") + uripp::convert(tm.tm_sec);
        if (!err.empty())
            throw std::invalid_argument("time structure out of range in " + err);
    }
    time_t date_time::to_time(const tm_type& tm, bool check_wday) {
        assert_in_range(tm, check_wday);
        time_t t;
        tm_type tt = tm;
#ifdef _WIN32
        t = _mkgmtime(&tt);
#else
        t = timegm(&tt);
#endif
        if (t == NULL_TIME)
            throw std::invalid_argument("invalid time structure on conversion to date and time");
        // Check that tm is canonical.
        std::string err;
        if (check_wday && tm.tm_wday != tt.tm_wday)
            err = std::string("week day (1-7) incorrect: ") + uripp::convert(tm.tm_wday + 1);
        else if (tm.tm_mday != tt.tm_mday)
            err = std::string("month day (1-31) incorrect: ") + uripp::convert(tm.tm_mday);
        else if (tm.tm_sec != tt.tm_sec)
            err = std::string("second (0-60) incorrect: ") + uripp::convert(tm.tm_sec);
        if (!err.empty()) {
            std::ostringstream oss;
            oss << "time structure for date (YYY-MM-DD) "
                << (tm.tm_year + 1900) << "-"
                << std::setw(2) << std::setfill('0') << (tm.tm_mon + 1) << "-"
                << std::setw(2) << std::setfill('0') << tm.tm_mday << ": "
                << err;
            throw std::invalid_argument(oss.str());
        }
        return t;
    }
    date_time::tm_type date_time::to_tm(const time_t& t) {
        tm_type tm;
        to_tm(t, tm);
        return tm;
    }
    void date_time::to_tm(const time_t& t, tm_type& tm) {
        if (t == NULL_TIME)
            throw std::invalid_argument("cannot convert null date and time to time structure");
#ifdef _WIN32
#  ifdef _USE_32BIT_TIME_T
        _gmtime32_s(&tm, &t);
#  else
        _gmtime64_s(&tm, &t);
#  endif
#else
        gmtime_r(&t, &tm);
#endif
    }
    bool parse(std::string::const_iterator& first, std::string::const_iterator last, date_time& v) {
        std::string::const_iterator f = first;
        date_time::tm_type tm;
        tiny_parser tp(f, last);
        tp.space();
        tp.push();
        tm.tm_wday = tp.array_item(wkday, ARRAY_SIZE(wkday));
        if (tm.tm_wday != -1 && tp(", ")) { // RFC 1123
            /// Sun, 06 Nov 1994 08:49:37 GMT
            if ((tm.tm_mday = tp.digits(2)) < 0 || !tp(" ") ||
                (tm.tm_mon = (int)tp.array_item(month, ARRAY_SIZE(month))) == -1 || !tp(" ") ||
                (tm.tm_year = tp.digits(4)) < 0 || !tp(" ") ||
                (tm.tm_hour = tp.digits(2)) < 0 || !tp(":") ||
                (tm.tm_min = tp.digits(2)) < 0 || !tp(":") ||
                (tm.tm_sec = tp.digits(2)) < 0 || !tp(" GMT"))
                return false;
            tm.tm_year -= 1900;
        } else if (tm.tm_wday != -1 && tp(" ")) { // asctime
            /// Sun Nov  6 08:49:37 1994
            if ((tm.tm_mon = tp.array_item(month, ARRAY_SIZE(month))) == -1 || !tp(" ") ||
                (tm.tm_mday = tp.digits(2, false)) < 0 || !tp(" ") ||
                (tm.tm_hour = tp.digits(2)) < 0 || !tp(":") ||
                (tm.tm_min = tp.digits(2)) < 0 || !tp(":") ||
                (tm.tm_sec = tp.digits(2)) < 0 || !tp(" ") ||
                (tm.tm_year = tp.digits(4)) < 0)
                return false;
            tm.tm_year -= 1900;
        } else { // RFC 850
            /// Sunday, 06-Nov-94 08:49:37 GMT
            tp.pop();
            if ((tm.tm_wday = tp.array_item(weekday, ARRAY_SIZE(weekday))) == -1 || !tp(", ") ||
                (tm.tm_mday = tp.digits(2)) < 0 || !tp("-") ||
                (tm.tm_mon = tp.array_item(month, ARRAY_SIZE(month))) == -1 || !tp("-") ||
                (tm.tm_year = tp.digits(2)) < 0 || !tp(" ") ||
                (tm.tm_hour = tp.digits(2)) < 0 || !tp(":") ||
                (tm.tm_min = tp.digits(2)) < 0 || !tp(":") ||
                (tm.tm_sec = tp.digits(2)) < 0 || !tp(" GMT"))
                return false;
            if (tm.tm_year < 70)
                tm.tm_year += 100;
        }
        v = date_time(tm, true);
        first = tp.first();
        return true;
    }
}
namespace uripp {
	bool convert(const std::string& s, restcgi::date_time& v) {
		if (s.empty() || isspaces(s.c_str()))
			return false;
		std::string::const_iterator first = s.begin();
		if (!parse(first, s.end(), v) || !isspaces(s.c_str() + (first - s.begin())))
			throw std::invalid_argument("cannot convert to date and time, invalid chars: " + s);
		return true;
	}
    std::string convert(const restcgi::date_time& v) {return v.string();}
}
