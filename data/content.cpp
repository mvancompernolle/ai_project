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
#include "content.h"
#include "endpoint.h"
//????? application/x-www-form-urlencoded
//????? text/xml; charset=UTF-8
namespace restcgi {
    content::content(const boost::shared_ptr<endpoint>& ep, const hdr_type& h)
        : endpoint_(ep), hdr_(h) {
    }
    content::~content() {}
    icontent::icontent(const boost::shared_ptr<endpoint>& ep, const hdr_type& h) : content(ep, h) {}
    icontent::~icontent() {}
    std::istream& icontent::istream() {return endpoint_->is_;}
    ocontent::ocontent(const boost::shared_ptr<endpoint>& ep, const hdr_type& h) : content(ep, h) {}
    ocontent::~ocontent() {}
    std::ostream& ocontent::ostream() {return endpoint_->os_;}
}
