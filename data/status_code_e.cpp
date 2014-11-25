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
#include "status_code_e.h"
#include <stdexcept>
#include <sstream>
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
namespace restcgi {
    const char status_code_e::EOL_CSTR[3] = "\r\n";
    const char* status_code_e::cstrings_[] = {
        "",
        "100 Continue",
        "101 Switching Protocols",
        "200 OK",
        "201 Created",
        "202 Accepted",
        "203 Non-Authoritative Information",
        "204 No Content",
        "205 Reset Content",
        "206 Partial Content",
        "300 Multiple Choices",
        "301 Moved Permanently",
        "302 Found",
        "303 See Other",
        "304 Not Modified",
        "305 Use Proxy",
        "307 Temporary Redirect",
        "400 Bad Request",
        "401 Unauthorized",
        "402 Payment Required",
        "403 Forbidden",
        "404 Not Found",
        "405 Method Not Allowed",
        "406 Not Acceptable",
        "407 Proxy Authentication Required",
        "408 Request Time-out",
        "409 Conflict",
        "410 Gone",
        "411 Length Required",
        "412 Precondition Failed",
        "413 Request Entity Too Large",
        "414 Request-URI Too Large",
        "415 Unsupported Media Type",
        "416 Requested range not satisfiable",
        "417 Expectation Failed",
        "500 Internal Server Error",
        "501 Not Implemented",
        "502 Bad Gateway",
        "503 Service Unavailable",
        "504 Gateway Time-out",
        "505 HTTP Version not supported",
    };
    const int status_code_e::ints_[] = {
        0,
        100,
        101,
        200,
        201,
        202,
        203,
        204,
        205,
        206,
        300,
        301,
        302,
        303,
        304,
        305,
        307,
        400,
        401,
        402,
        403,
        404,
        405,
        406,
        407,
        408,
        409,
        410,
        411,
        412,
        413,
        414,
        415,
        416,
        417,
        500,
        501,
        502,
        503,
        504,
        505,
    };
    status_code_e::status_code_e(int code) : e_(null) {
        for (size_t i = 1; i < ARRAY_SIZE(ints_); ++i)
            if (code == ints_[i]) {
                e_ = (e)i;
                break;
            }
    }
    std::ostream& status_code_e::operator <<(std::ostream& os) const {
        return os << "Status: " << cstring() << EOL_CSTR;
    }
}
