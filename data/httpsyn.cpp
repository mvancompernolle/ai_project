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
#include "httpsyn.h"
#include "utils.h"
#include <stdexcept>
namespace restcgi {
    const char http_token::ctypes_[] = {
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,1,0,1,1,1,1,1, 0,0,1,1,0,1,1,0,
        1,1,1,1,1,1,1,1, 1,1,0,0,0,0,0,0, // 3f
        0,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,0,0,0,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,0,1,0,1,0, // 7f
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    };
    http_token::http_token() {}
    http_token::http_token(const std::string& s) {
        if (s.empty())
            throw std::invalid_argument("invalid HTTP token: no chars");
        if (!is_valid(s))
            throw std::invalid_argument("invalid HTTP token (encoded for printability): \"" + encode_ctl(s) + "\"");
        string_ = s;
    }
    bool http_token::is_valid(const std::string& v) {
        if (v.empty())
            return false;
        std::string::const_iterator f = v.begin();
        for (; f != v.end(); ++f)
            if (!ctypes_[(unsigned char)*f])
                return false;
        return true;
    }
    bool parse(std::string::const_iterator& first, std::string::const_iterator last, http_token& v, bool noskipls) {
        std::string::const_iterator f = first;
        if (!noskipls)
            for (;; ++f) { // Skip leading isspace.
                if (f == last)
                    return false;
                if (!isspace(*f))
                    break;
            }
        if (!http_token::ctypes_[(unsigned char)*f])
            return false;
        std::string::const_iterator begin = f++;
        for (; f != last; ++f)
            if (!http_token::ctypes_[(unsigned char)*f])
                break;
        v.string_.assign(begin, f);
        first = f;
        return true;
    }
    const char http_text::ctypes_[] = {
        0,0,0,0,0,0,0,0, 0,2,2,0,0,2,0,0,
        0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
        2,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, // 3f
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,0, // 7f
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    };
    http_text::http_text() {}
    http_text::http_text(const std::string& s) {
        std::string::const_iterator f = s.begin();
        if (!restcgi::parse(f, s.end(), *this) || f != s.end())
            throw std::invalid_argument("invalid HTTP TEXT (encoded for printability): \"" + encode_ctl(s) + "\"");
    }
    http_text& http_text::operator +=(const std::string& rhs) {
        http_text txt(rhs);
        string_.append(txt.string_);
        return *this;
    }
    bool parse(std::string::const_iterator& first, std::string::const_iterator last, http_text& v, const char* terms, bool lastok) {
        std::string::const_iterator f = first;
        std::string::const_iterator anchor = f;
        std::string s;
        char c = 0;
        for (; f != last;) {
            c = *f;
            if (terms && ::strchr(terms, c)) // Term char.
                break;
            char type = http_text::ctypes_[(unsigned char)c];
            if (type == 1) // Non-space, valid char.
                ++f;
            else if (!type) { // Invalid char.
                break;
            } else { // Space of some kind.
                std::string::const_iterator t = f;
                if (c == ' ' && ++t != last && http_text::ctypes_[(unsigned char)*t] == 1) // Single, lone ' '.
                    f = t;
                else { // Odd cases.
                    s.append(anchor, f); // Save up to LWS.
                    bool halt = false;
                    bool found = false;
                    for (;;) {
                        if (c == '\r') { // CR must be followed by LF and SP or HT.
                            t = f;
                            if (++t == last || *t != '\n' || ++t == last || (*t != ' ' && *t != '\t')) {
                                halt = true;
                                break;
                            }
                            f = t;
                        } else if (c == '\n') { // CR must come before LF.
                            halt = true;
                            break;
                        }
                        found = true;
                        if (++f == last)
                            break;
                        c = *f;
                        if (http_text::ctypes_[(unsigned char)c] != 2)
                            break;
                    }
                    if (found)
                        s.append(1, ' '); // Reduce LWS to single space.
                    anchor = f; // New anchor.
                    if (halt) // Invalid sequence.
                        break;
                }
            }
        }
        if (terms) {
            if (f == last) {
                if (!lastok)
                    return false;
            } else if (!::strchr(terms, c)) // Should have stopped on term.
                return false;
        } else if (f == first) // Got nothing.
            return false;
        v.string_ =  s.append(anchor, f);
        first = f;
        return true;
    }
    const char http_word::ESC_CHAR = '\\';
    http_word::http_word() : token_(false) {}
    http_word::http_word(const std::string& s) {
        std::string::const_iterator it = s.begin();
        if (s.empty() || isspace(s[0]) || !parse(it, s.end(), *this) || it != s.end())
            throw std::invalid_argument("invalid HTTP word (between >< and encoded for printability): >" + encode_ctl(s) + "<");
    }
    http_word::http_word(const http_token& tok) : string_(tok.string()), token_(true) {}
    http_word::http_word(const http_text& txt) : string_(txt.string()), token_(false) {}
    http_word& http_word::operator +=(const std::string& rhs) {
        if (!rhs.empty()) {
            http_text txt(rhs);
            string_.append(txt.string());
            token_ = false;
        }
        return *this;
    }
    http_word& http_word::operator +=(const http_text& rhs) {
        if (!rhs.string().empty()) {
            string_.append(rhs.string());
            token_ = false;
        }
        return *this;
    }
    http_word& http_word::operator +=(const http_word& rhs) {
        if (!rhs.string_.empty()) {
            string_.append(rhs.string_);
            if (token_)
                token_ = rhs.token_;
        }
        return *this;
    }
    std::string http_word::encoding(bool force_quotes) const {
        bool istok = token_;
        if (!istok) // Check if it is.
            istok = http_token::is_valid(string_);
        if (istok) // Token: no need to quote unless forced.
            return force_quotes ? ("\"" + string_ + "\"") : string_;
        // Quote text, escape '\' and '"'.
        std::string s = "\"";
        std::string::const_iterator f = string_.begin();
        std::string::const_iterator anchor = f;
        for (; f != string_.end(); ++f) // Escape escape char and quote.
            if (*f == ESC_CHAR || *f == '"') {
                s.append(anchor, f);
                s.append(1, ESC_CHAR);
                anchor = f;
            }
        s.append(anchor, string_.end());
        return s.append(1, '"');
    }
    bool parse(std::string::const_iterator& first, std::string::const_iterator last, http_word& v) {
        std::string::const_iterator f = first;
        for (;; ++f) { // Skip leading isspace.
            if (f == last)
                return false;
            if (!isspace(*f))
                break;
        }
        if (*f != '"') { // Not quoted: must be token.
            http_token tok;
            bool parsed = parse(f, last, tok);
            if (parsed) {
                first = f;
                v = tok;
            }
            return parsed;
        }
        std::string s;
        for (++f;;) {
            if (f == last)
                return false;
            if (*f == '"') { // No need to get text: done.
                ++f;
                break;
            }
            // Get text up to '"' (note this does folding).
            http_text txt;
            if (!parse(f, last, txt, "\""))
                return false;
            ++f; // Skip over quote.
            const std::string& ts = txt.string();
            s.append(ts); // Add to previous (if any).
            // Check if there are an odd number of ESC chars, which would
            // indicate that this quote is escaped.
            int escs = 0;
            for (std::string::const_reverse_iterator r = ts.rbegin(); *r == http_word::ESC_CHAR;) {
                ++escs;
                if (++r == ts.rend())
                    break;
            }
            if (!(escs & 1)) // Quote was not escaped: done.
                break;
            s.append(1, '"'); // Quote was escaped: add quote back in and get more.
        }
        // Unescape chars.
        std::string::const_iterator u = s.begin();
        std::string::const_iterator anchor = u;
        std::string unesc;
        for (; u != s.end();)
            if (*u == http_word::ESC_CHAR) { // Escape char.
                unesc.append(anchor, u);
                if (++u == s.end()) // Escape char without escapee.
                    return false;
                if (*u == http_word::ESC_CHAR) { // Escape escape.
                    unesc.append(1, http_word::ESC_CHAR);
                    ++u;
                }
                anchor = u;
            } else
                ++u;
        v.string_ = (anchor == s.begin()) ? s : unesc.append(anchor, u);
        v.token_ = false;
        first = f;
        return true;
    }
    namespace httpsyn {
        bool parse(std::string::const_iterator& first, std::string::const_iterator last, const char* v) {
            if (!v || !*v)
                return false;
            std::string::const_iterator f = first;
            for (;; ++f) { // Skip leading isspace.
                if (f == last)
                    return false;
                if (!isspace(*f))
                    break;
            }
            for (;; ++f, ++v) {
                if (!*v)
                    break;
                if (f == last)
                    return false;
                if (*f != *v)
                    return false;
            }
            first = f;
            return true;
        }
        bool parse_any(std::string::const_iterator& first, std::string::const_iterator last) {
            std::string::const_iterator f = first;
            for (;; ++f) { // Skip leading isspace.
                if (f == last)
                    return false;
                if (!isspace(*f))
                    break;
            }
            first = f;
            return true;
        }
        bool parse_eor(std::string::const_iterator& first, std::string::const_iterator last) {
            std::string::const_iterator f = first;
            for (; f != last; ++f)
                if (!isspace(*f))
                    return false;
            first = f;
            return true;
        }
        bool parse_skip(std::string::const_iterator& first, std::string::const_iterator last, const char* terms) {
            if (!terms || !*terms)
                return false;
            std::string::const_iterator f = first;
            for (;; ++f) {
                if (f == last)
                    return false;
                if (::strchr(terms, *f))
                    break;
            }
            first = f + 1;
            return true;
        }
    }
}
