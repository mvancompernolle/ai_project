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
#include "ctmpl.h"
#include "env.h"
namespace restcgi {
    const char ctmpl::ESCAPE_CHAR = '%';
    const char ctmpl::ctypes_[] = {
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,2,0,0,
        2,2,2,2,2,2,2,2, 2,2,0,0,0,0,0,0,
        0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,0,0,0,0,1,
        0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,0,0,0,0,0, // 7F
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    };
    ctmpl::ctmpl(const std::string& s, const std::string& content_type)
        : string_(s), content_type_(content_type) {
    }
    std::string ctmpl::eval(const env* e, const map_type& map) const {
        std::string s;
        size_t next;
        size_t anchor = 0;
        for (size_t pos = 0;; pos = next) {
            next = string_.find(ESCAPE_CHAR, pos);
            if (next == std::string::npos) {
                if (!anchor) // No variables at all.
                    return string_;
                break;
            }
            std::string::const_iterator begin = string_.begin() + next;
            std::string::const_iterator it = begin + 1;
            if (it == string_.end()) break;
            bool found = false;
            if (ctypes_[(unsigned char)*it] == 1) { // Possible start of name.
                for (; ++it != string_.end() && ctypes_[(unsigned char)*it];) // While legal name char.
                    ;
                if (it == string_.end()) break;
                if (*it == ESCAPE_CHAR) { // Ending escape char: found variable name.
                    std::string name(string_, next + 1, it - begin - 1);
                    std::string value;
                    if (find(name, value, e, map)) {
                        found = true;
                        s.append(string_, anchor, next - anchor); // Append up to start of name.
                        s.append(value); // Append variable value.
                    }
                    // Skip ending escape char.
                    if (++it == string_.end()) {
                        if (found)
                            return s;
                        break;
                    }
                }
            }
            next += it - begin;
            if (found)
                anchor = next;
        }
        s.append(string_, anchor, std::string::npos); // Append to end.
        return s;
    }
    std::string ctmpl::eval(const env& e, const map_type& map) const {return eval(&e, map);}
    std::string ctmpl::eval(const map_type& map) const {return eval(0, map);}
    bool ctmpl::find(const std::string& name, std::string& value, const env* e, const map_type& map) {
        // Check map.
        map_type::const_iterator it = map.find(name);
        if (it != map.end()) {
            value = it->second;
            return true;
        }
        if (!e)
            return false;
        // Check hdr first.
        if (e->hdr_find(name, value))
            return true;
        // Check env.
        return e->find(name.c_str(), value);
    }
}
