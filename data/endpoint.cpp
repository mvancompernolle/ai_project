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
#include "endpoint.h"
#include "method.h"
namespace restcgi {
    endpoint::endpoint(const env_type& e, std::istream& is, std::ostream& os)
        : env_(e), is_(is), os_(os) {
    }
    endpoint::~endpoint() {}
    endpoint::pointer endpoint::create() {return create(env_type(), std::cin, std::cout);}
    endpoint::pointer endpoint::create(std::istream& is, std::ostream& os) {return create(env_type(), is, os);}
    endpoint::pointer endpoint::create(const env_type& e, std::istream& is, std::ostream& os) {
        pointer p(new endpoint(e, is, os));
        p->this_ = p; // Save for passing to methods.
        return p;
    }
    endpoint::method_pointer endpoint::receive() {return method::create(this_.lock());}
    void endpoint::env_override(const std::string& name, const std::string& value) {
        env_.override(name, value);
    }
}
