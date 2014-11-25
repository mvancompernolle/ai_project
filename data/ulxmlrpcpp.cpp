/***************************************************************************
              ulxmlrpcpp.cpp  -  common stuff for xml-rpc project
                             -------------------
    begin                : Sam Apr 20 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxmlrpcpp.cpp 1164 2010-01-06 10:03:51Z ewald-arnold $

 ***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms odebug  true: compiled with DEBUGf the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

//#define ULXR_UNICODE_ONLY_HELPERS
#define ULXR_NEED_EXPORTS
#include <ulxmlrpcpp/ulxmlrpcpp.h>  // always first header

#include <cctype>
#include <cerrno>
#include <sstream>
#include <iosfwd>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <memory>

#include <ulxmlrpcpp/ulxr_log4j.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_wbxmlparse.h>
#include <ulxmlrpcpp/ulxr_htmlform_handler.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>

#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
#include <ulxmlrpcpp/ulxr_mutex.h>
#endif


/** @brief Various general helper classes
  * @defgroup grp_ulxr_utilit General helper classes
  */

/** @brief Unicode converter functions
  * @defgroup grp_ulxr_unicode_cvt Unicode converter functions
  */

/** @brief Various low-level parser classes
  * @defgroup grp_ulxr_parser Low-level parser classes
  */

/** @brief XML-RPC handling
  * @defgroup grp_ulxr_rpc XML-RPC handling
  */

/** @brief HTTP and HTML handling
  * @defgroup grp_ulxr_http HTTP and HTML handling
  */


/** @mainpage

   \section abstract Abstract

   ulxmlrpcpp is a library to call methods on a remote server by using
   <a href="http://www.xmlrpc.com">XML-RPC</a>. It's main goal
   is to be as easy to use as possible but be fully compliant. It is object
   oriented and written in C++.

   \section docs_struct  Getting Started With The Documentation

   The following basic function groups are available in this documentation:
     - \ref grp_ulxr_value_type
     - \ref grp_ulxr_connection
     - \ref grp_ulxr_protocol
     - \ref grp_ulxr_rpc
     - \ref grp_ulxr_http
     - \ref grp_ulxr_utilit
     - \ref grp_ulxr_parser
     - \ref grp_ulxr_unicode_cvt
*/

namespace ulxr {


ULXR_API_IMPL(void) getVersion (int &major, int &minor, int &patch, bool &debug, CppString &/*info*/)
{
  CppString s (ULXR_GET_STRING(ULXR_VERSION));
  CppString num;
/*
#ifdef ULXR_USE_INTRINSIC_VALUE_TYPES
  info = ulxr_i18n("Conversion from intrinsic types to ulxr::Value() is activated.\n");
#else
  info = ulxr_i18n("Conversion from intrinsic types to ulxr::Value() is NOT activated.\n");
#endif
*/
  std::size_t pos = s.find(ULXR_CHAR('.'));
  bool good = true;
  if (pos != CppString::npos)
  {
    num = s.substr(0, pos);
    if (num.length() == 0)
      good = false;
    major = ulxr_atoi(getLatin1(num).c_str());
    s.erase(0, pos+1);
    pos = s.find('.');

    if (pos != CppString::npos)
    {
      num = s.substr(0, pos);
      if (num.length() == 0)
        good = false;
      minor = ulxr_atoi(getLatin1(num).c_str());
      s.erase(0, pos+1);

      if (s.length() == 0)
        good = false;
      patch = ulxr_atoi(getLatin1(s).c_str());
    }
    else
      good = false;
  }
  else
    good = false;

  if (!good)
  {
    major = -1;
    minor = -1;
    patch = -1;
  }

#ifdef DEBUG
  debug = true;
#else
  debug = false;
#endif
}

#if defined (ULXR_UNICODE) || defined(ULXR_UNICODE_ONLY_HELPERS)

ULXR_API_IMPL(Cpp16BitString) getUnicode(const std::string &latin1)
{
  Cpp16BitString ret;
  for (unsigned i = 0; i < latin1.length(); ++i)
    ret += (unsigned char) latin1[i];

  return ret;
}


ULXR_API_IMPL(std::string) getLatin1(const Cpp16BitString &uni)
{
  std::string ret;
  for (unsigned i = 0; i < uni.length(); ++i)
    ret += (unsigned char) uni[i];

  return ret;
}

#endif

ULXR_API_IMPL(CppString) stripWS(const CppString &s)
{
   unsigned start = 0;
   while (start < s.length() && ulxr_isspace(s[start]))
     ++start;

   unsigned end = s.length();
   while (end > start && ulxr_isspace(s[end-1]))
     --end;

   return s.substr(start, end-start);
}

namespace
{

static ulxr::Char b64_encodetable [64] =
{
 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
 'w', 'x', 'y', 'z', '0', '1', '2', '3',
 '4', '5', '6', '7', '8', '9', '+', '/'
};

}

ULXR_API_IMPL(CppString) encodeBase64(const CppString &normstr, bool add_crlf)
{
    CppString ret;
    unsigned len = normstr.length();
    unsigned idx = 0;
    bool hiteof = len == 0;
    bool just_nl = false;
    unsigned linelen = 0;

    while (!hiteof)
    {
        unsigned int igroup[3];
        unsigned int ogroup[4];
        unsigned int n;
        unsigned int c;

        igroup[0] = igroup[1] = igroup[2] = 0;
        for (n = 0; n < 3; n++)
        {
            if (idx >= len)
            {
                hiteof = true;
                break;
            }
#ifndef ULXR_UNICODE
            c = normstr[idx++] & 0xFF;
#else
            c = normstr[idx++];
            if (c > 0xFF)
            {
              std::basic_ostringstream<wchar_t> os; // std::wostringstream
              os << c;
              throw ParameterException(ApplicationError,
                                       ulxr_i18n(ULXR_PCHAR("encodeBase64(): Illegal character in input: #"))+ os.str());
            }
#endif
            igroup[n] = c;
        }

        if (n > 0)
        {
            ogroup[0] = b64_encodetable[igroup[0] >> 2];
            ogroup[1] = b64_encodetable[((igroup[0] & 3) << 4) | ((igroup[1] & 0xF0) >> 4)];
            ogroup[2] = b64_encodetable[((igroup[1] & 0x0F) << 2) | ((igroup[2] & 0xC0) >> 6)];
            ogroup[3] = b64_encodetable[igroup[2] & 0x3F];
/*
std::cout << "igroup " << std::hex
   << " 0x"  << igroup[0]
   << " 0x"  << igroup[1]
   << " 0x"  << igroup[2]
   << std::endl;

std::cout << "ogroup " << std::hex
   << " 0x"  << ogroup[0]
   << " 0x"  << ogroup[1]
   << " 0x"  << ogroup[2]
   << " 0x"  << ogroup[3]
   << std::endl;
*/
            // Replace characters in output stream with "=" pad
            // characters if fewer than three characters were
            // read from the end of the input stream.

            if (n < 3)
            {
                ogroup[3] = '=';
                if (n < 2)
                {
                    ogroup[2] = '=';
                }
            }

            just_nl = false;
            for (unsigned i = 0; i < 4; i++)
            {
                ret += ogroup[i];
                if (++linelen >= 72)
                {

                  linelen = 0;
                  just_nl = true;

                  if (add_crlf)
                    ret += ULXR_PCHAR("\r\n");
                }
            }
        }
    }

    if (!just_nl && add_crlf)
       ret += ULXR_PCHAR("\r\n");

    return ret;
}


namespace
{

static int b64_decodetable [256] =
{
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // 00 07
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // 08 0f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // 10 17
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // 18 2f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // 20 27
  0x80,  0x80,  0x80,  0x3e,  0x80,  0x80,  0x80,  0x3f, // 28 3f
  0x34,  0x35,  0x36,  0x37,  0x38,  0x39,  0x3a,  0x3b, // 30 07
  0x3c,  0x3d,  0x80,  0x80,  0x80,  0x00,  0x80,  0x80, // 38 0f
  0x80,  0x00,  0x01,  0x02,  0x03,  0x04,  0x05,  0x06, // 40 07
  0x07,  0x08,  0x09,  0x0a,  0x0b,  0x0c,  0x0d,  0x0e, // 48 0f
  0x0f,  0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16, // 50 07
  0x17,  0x18,  0x19,  0x80,  0x80,  0x80,  0x80,  0x80, // 58 0f
  0x80,  0x1a,  0x1b,  0x1c,  0x1d,  0x1e,  0x1f,  0x20, // 60 07
  0x21,  0x22,  0x23,  0x24,  0x25,  0x26,  0x27,  0x28, // 68 0f
  0x29,  0x2a,  0x2b,  0x2c,  0x2d,  0x2e,  0x2f,  0x30, // 70 07
  0x31,  0x32,  0x33,  0x80,  0x80,  0x80,  0x80,  0x80, // 78 0f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // 80 07
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // 88 0f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // 90 07
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // 98 0f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // a0 07
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // a8 0f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // b0 07
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // b8 0f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // c0 07
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // c8 0f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // d0 07
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // d8 0f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // e0 07
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // e8 0f
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // f0 07
  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80,  0x80, // f8 0f
};

}

ULXR_API_IMPL(CppString) decodeBase64(const CppString &b64str, bool errcheck)
{
    CppString ret;
    unsigned len = b64str.length();
    unsigned idx = 0;

    if (len == 0)
      return ret;

    while (true)
    {
        unsigned int a[4], b[4], o[3];

        for (unsigned j = 0; j < 4; /**/)
        {
//std::cout << "idx " << idx << " 0x" << std::hex << (unsigned) b64str[idx] << " " << std::dec << b64str[idx] << std::endl;
            if (idx >= len)
            {
                if (errcheck && (j > 0))
                    throw ParameterException(ApplicationError,
                                             ulxr_i18n(ULXR_PCHAR("decodeBase64(): Input data is incomplete.")));
                return ret;
            }

            unsigned c = (unsigned) b64str[idx++];

            if (c != ULXR_CHAR('\r') && c != ULXR_PCHAR('\n') && c != ULXR_PCHAR(' '))
            {
              if ((c > 0xFF) || (b64_decodetable[c] & 0x80))
              {
                  if (errcheck)
                  {
#ifdef ULXR_UNICODE
                    std::basic_ostringstream<wchar_t> os; // std::wostringstream
#else
                    std::ostringstream os;
#endif
                    os << (unsigned) c;
                    throw ParameterException(ApplicationError,
                                             ulxr_i18n(ULXR_GET_STRING("decodeBase64(): Illegal character in input: #"))+ os.str());
                  }
                  // Ignoring errors: discard invalid character.
                  j--;
                  continue;
              }
              a[j] = c;
              b[j] = b64_decodetable[c];
              ++j;
            }
        }

        o[0] = 0xff & ((b[0] << 2) | (b[1] >> 4));
        o[1] = 0xff & ((b[1] << 4) | ((b[2]) >> 2));
        o[2] = 0xff & ((b[2] << 6) |  (b[3]));
/*
std::cout << "b " << std::hex
   << " 0x"  << b[0]
   << " 0x"  << b[1]
   << " 0x"  << b[2]
   << " 0x"  << b[3]
   << std::endl;

std::cout << "o " << std::hex
   << " 0x"  << o[0]
   << " 0x"  << o[1]
   << " 0x"  << o[2]
   << std::endl;
*/
        int i = a[2] == ULXR_CHAR('=') ? 1 : (a[3] == ULXR_CHAR('=') ? 2 : 3);

        switch (i)
        {
           case 1:
             ret += o[0];
           break;

           case 2:
             ret += o[0];
             ret += o[1];
           break;

           case 3:
             ret += o[0];
             ret += o[1];
             ret += o[2];
           break;
        }

        if (i < 3)
          return ret;
    }
}


ULXR_API_IMPL(CppString) xmlEscape(const CppString &str, bool suppress_non_unicode)
{
  CppString ret;
  unsigned prev = 0;
  unsigned len = str.length();
  unsigned curs = 0;
  const Char *pc = str.data();

  while (curs != len)
  {
    Char c = *pc++;

    if (c == ULXR_CHAR('&'))
    {
      ret += str.substr(prev, curs-prev);
      ret += ULXR_PCHAR("&amp;");
      prev = curs+1;
    }

    else if (c == ULXR_CHAR('<'))
    {
      ret += str.substr(prev, curs-prev);
      ret += ULXR_PCHAR("&lt;");
      prev = curs+1;
    }

    else if (c == ULXR_CHAR('\n'))
    {
      ret += str.substr(prev, curs-prev);
      ret += ULXR_PCHAR("&#xA;");
      prev = curs+1;
    }

    else if (c == ULXR_CHAR('\r'))
    {
      ret += str.substr(prev, curs-prev);
      ret += ULXR_PCHAR("&#xD;");
      prev = curs+1;
    }

    else if (c == ULXR_CHAR('\t'))
    {
      ret += str.substr(prev, curs-prev);
      ret += ULXR_PCHAR("&#x9;");
      prev = curs+1;
    }

    else if (suppress_non_unicode && c < 0x20) // fffe, ffff, d800 .. dfff
    {
      // ignore non-unicode
      prev = curs+1;
    }

    else if (c == ULXR_CHAR('\0'))
    {
      ret += str.substr(prev, curs-prev);
      ret += ULXR_PCHAR("&#x0;");
      prev = curs+1;
    }

    ++curs;
  }
  ret += str.substr(prev, curs-prev);
  return ret;
}


namespace
{

CppString charRefDezCont (ULXR_PCHAR("0123456789"));
CppString charRefHexCont (ULXR_PCHAR("0123456789aAbBcCdDeEfF"));

}


// When we use Unicode, resolve "#xxx;" to Unicode
// otherwise resolve to utf8
static void resolveCharRef(const CppString &ins, unsigned &pos, CppString &outs)
{

  if (pos > ins.length()-3)
    throw ParameterException(InvalidCharacterError,
                             ulxr_i18n(ULXR_PCHAR("Error in xml character reference.")));

  pos += 2;

  unsigned wc = 0;
  if (   ins[pos] == ULXR_CHAR('x')
      || ins[pos] == ULXR_CHAR('X'))
  {
    ++pos;
    for (unsigned limit = 0;
            (limit < 6)
         && (charRefHexCont.find(ins[pos]) != CppString::npos)
         && (pos < ins.length());
         ++limit)
    {
      wc <<= 4;
      unsigned c = ulxr_toupper(ins[pos]);
      if (ulxr_isdigit(c))
        wc += c - ULXR_CHAR('0');
      else
        wc += c - ULXR_CHAR('A') + 0xa;
      ++pos;
    }
  }
  else
  {
    for (unsigned limit = 0;
            (limit < 8)
         && (charRefDezCont.find(ins[pos]) != CppString::npos)
         && (pos < ins.length());
         ++limit)
    {
      wc *= 10;
      unsigned c = ins[pos];
      if (ulxr_isdigit(c))
        wc += c - '0';
      else
        throw ParameterException(InvalidCharacterError,
                                 ulxr_i18n(ULXR_PCHAR("Error in xml character reference.")));
      ++pos;
    }
  }

  if (pos < ins.length()-1 && ins[pos] == ';')
  {
#ifdef ULXR_UNICODE
    outs = wc;
#else
    outs = unicodeToUtf8(wc);
#endif
    ++pos;
  }
  else
    throw ParameterException(InvalidCharacterError,
                             ulxr_i18n(ULXR_PCHAR("Error in xml character reference.")));

  return;
}


#ifdef ULXR_UNICODE
#define STRNCMP(s1, s2, n)  wcsncmp(s1, s2, n)
#else
#define STRNCMP(s1, s2, n)  strncmp(s1, s2, n)
#endif


ULXR_API_IMPL(CppString) xmlUnEscape(const CppString &str)
{
  CppString ret;
  unsigned prev = 0;
  unsigned len = str.length();
  unsigned curs = 0;
  CppString s;

  const Char *amp = ULXR_PCHAR("&amp;");
  const Char *lt = ULXR_PCHAR("&lt;");

  while (curs < len)
  {
    const Char *pc = str.data() + curs;
    Char c = *pc;
    if (c == '&')
    {
      if (curs == len-1)
        throw ParameterException(InvalidCharacterError,
                                 ulxr_i18n(ULXR_PCHAR("Error in xml reference, \"&\" is last character.")));

      if (   curs < len-1
          && ULXR_CHAR('#') == *(pc+1))
      {
        ret += str.substr(prev, curs-prev);
        resolveCharRef(str, curs, s);
        ret += s;
        prev = curs;
      }

      else if (len-curs >= 5 && STRNCMP(pc, amp, 5) == 0)
      {
        ret += str.substr(prev, curs-prev);
        ret += ULXR_PCHAR("&");
        curs += 5;
        prev = curs;
      }

      else if (len-curs >= 4 && STRNCMP(pc, lt, 4) == 0)
      {
        ret += str.substr(prev, curs-prev);
        ret += ULXR_PCHAR("<");
        curs += 4;
        prev = curs;
      }
      else
        throw ParameterException(InvalidCharacterError,
                                 ulxr_i18n(ULXR_PCHAR("Error in xml reference.")));
    }
    else
      ++curs;
}
  ret += str.substr(prev, curs-prev);

/*
  CppString ret = str;
  std::size_t pos = 0;

  while ((pos = ret.find(ULXR_CHAR('&'), pos)) != CppString::npos )
  {
    if (pos == ret.length()-1)
      throw ParameterException(InvalidCharacterError,
                               ulxr_i18n(ULXR_PCHAR("Error in xml reference, \"&\" is last character.")));

    if (   pos < ret.length()-1
        && ULXR_CHAR('#') == ret[pos+1])
      resolveCharRef(ret, pos);

    else if (ret.substr(pos, 5) == ULXR_PCHAR("&amp;"))
    {
      ret.replace (pos, 5, ULXR_PCHAR("&"));
      pos += 1;
    }

    else if (ret.substr(pos, 4) == ULXR_PCHAR("&lt;"))
    {
      ret.replace (pos, 4, ULXR_PCHAR("<"));
      pos += 1;
    }

    else
      throw ParameterException(InvalidCharacterError,
                               ulxr_i18n(ULXR_PCHAR("Unrecognized entity.")));
  }
*/
  return ret;
}


namespace
{
  static const ulxr::Char* malformed = ULXR_I18N_NOOP(ULXR_PCHAR("Malformed UTF8 encoded string"));
}


static unsigned decodeUtf8Group(const Cpp8BitString &val, unsigned &i)
{
  unsigned charNum = 0;
  unsigned short trigger = val[i];
  unsigned vl = val.length();
  if (trigger >= 0x80)
  {
    int remain = vl - i;
    if ((trigger & 0xE0) == 0xC0)
    {         // 110x xxxx
      if (   (remain > 1)
          && (val[i+1] & 0xC0) == 0x80)
      {
        charNum = ((val[i]   & 0x1F) << 6)
                  | (val[i+1] & 0x3F);
        i += 2;
      }
      else
      {
        throw ParameterException(ApplicationError, ulxr_i18n(malformed));
      }
    }

    else if ((trigger & 0xF0) == 0xE0)
    {  // 1110 xxxx
      if (   (remain > 2)
          && ((val[i+1] & 0xC0) == 0x80)
          && ((val[i+2] & 0xC0) == 0x80))
      {
        charNum = ((val[i]   & 0x0F) << 12)
                  |((val[i+1] & 0x3F) <<  6)
                  | (val[i+2] & 0x3F);
        i += 3;
      }
      else
      {
        throw ParameterException(ApplicationError, ulxr_i18n(malformed));
      }
    }

    else if ((trigger & 0xF8) == 0xF0)
    {   // 1111 0xxx
      if (   (remain > 3)
          && ((val[i+1] & 0xC0) == 0x80)
          && ((val[i+2] & 0xC0) == 0x80)
          && ((val[i+3] & 0xC0) == 0x80))
      {
        charNum = ((val[i]   & 0x07) << 18)
                  |((val[i+1] & 0x3F) << 12)
                  |((val[i+2] & 0x3F) <<  6)
                  | (val[i+3] & 0x3F);
        i += 4;
      }
      else
      {
        throw ParameterException(ApplicationError, ulxr_i18n(malformed));
      }
    }

    else if ((trigger & 0xFC) == 0xF8)
    {   // 1111 10xx
      if (   (remain > 4)
          && ((val[i+1] & 0xC0) == 0x80)
          && ((val[i+2] & 0xC0) == 0x80)
          && ((val[i+3] & 0xC0) == 0x80)
          && ((val[i+4] & 0xC0) == 0x80))
      {
        charNum = ((val[i]   & 0x03) << 24)
                  |((val[i+1] & 0x3F) << 18)
                  |((val[i+2] & 0x3F) << 12)
                  |((val[i+3] & 0x3F) <<  6)
                  | (val[i+4] & 0x3F);
        i += 5;
      }
      else
      {
          throw ParameterException(ApplicationError, ulxr_i18n(malformed));
      }
    }

    else if ((trigger & 0xFE) == 0xFC)
    {   // 1111 110x
      if (   (remain > 5)
          && ((val[i+1] & 0xC0) == 0x80)
          && ((val[i+2] & 0xC0) == 0x80)
          && ((val[i+3] & 0xC0) == 0x80)
          && ((val[i+4] & 0xC0) == 0x80)
          && ((val[i+5] & 0xC0) == 0x80))
      {
        charNum = ((val[i]   & 0x01) << 30)
                  |((val[i+1] & 0x3F) << 24)
                  |((val[i+2] & 0x3F) << 18)
                  |((val[i+3] & 0x3F) << 12)
                  |((val[i+4] & 0x3F) <<  6)
                  | (val[i+5] & 0x3F);
        i += 6;
      }
      else
      {
          throw ParameterException(ApplicationError, ulxr_i18n(malformed));
      }
    }
    else
      throw ParameterException(ApplicationError, ulxr_i18n(malformed));
  }

  else
  {
    i += 1;
    charNum = trigger;
  }
  return charNum;
}


ULXR_API_IMPL(Cpp8BitString) utf8ToAscii(const Cpp8BitString &val)
{
  Cpp8BitString ret;

  unsigned i = 0;
  while (i < val.length())
  {
    unsigned charNum = decodeUtf8Group(val, i);

    if (charNum < 0x100)
      ret += charNum;

    else
      throw ParameterException(ApplicationError, ulxr_i18n(ULXR_PCHAR("Ascii character too big: ")) + HtmlFormHandler::makeHexNumber(charNum));
  }

  return ret;
}

#if defined (ULXR_UNICODE) || defined(ULXR_UNICODE_ONLY_HELPERS)

ULXR_API_IMPL(Cpp16BitString) utf8ToUnicode(const Cpp8BitString &val)
{
 Cpp16BitString ret;

  unsigned i = 0;
  while (i < val.length())
  {
    unsigned charNum = decodeUtf8Group(val, i);

    if (charNum < 0x10000)
      ret += charNum;

    else if (charNum < 0x110000)
    {

      charNum -= 0x10000;
      ret += ((charNum >> 10) + 0xD800);
      ret += ((charNum & 0x3FF) + 0xDC00);
    }

    else
      throw ParameterException(ApplicationError, ulxr_i18n(ULXR_PCHAR("In utf8ToUnicode(), unicode character too big: ")) + HtmlFormHandler::makeHexNumber(charNum));
  }

  return ret;
}


ULXR_API_IMPL(Cpp8BitString) unicodeToUtf8(const Cpp16BitString &newval)
{
  Cpp8BitString val;
#ifdef __BORLANDC__
  val.reserve(newval.length());
#endif
  for (unsigned i = 0; i < newval.length(); ++i)
    val += unicodeToUtf8(newval[i]);

  return val;
}

#endif

ULXR_API_IMPL(Cpp8BitString) asciiToUtf8(const Cpp8BitString &newval)
{
  Cpp8BitString val;
#ifdef __BORLANDC__
  val.reserve(newval.length());
#endif
  for (unsigned i = 0; i < newval.length(); ++i)
    val += unicodeToUtf8((unsigned char) newval[i]);

  return val;
}


ULXR_API_IMPL(Cpp8BitString) unicodeToUtf8(const unsigned c)
{
  Cpp8BitString val;
  if (c < 0x80)
    val += c;

  else if (c < 0x800)
  {
    val += (0xC0 | (c >> 6));
    val += (0x80 | (c & 0x3F));
  }

// FIXME: surrogates ??
  else if (c < 0x10000)
  {
    val += (0xE0 |  (c >> 12));
    val += (0x80 | ((c >>  6) & 0x3F));
    val += (0x80 |  (c & 0x3F));
  }
/*
    else if (c < 0xFFFF)
    {
      val += (0xF0 | (c >> 12));
      val += (0x80 | ((c >> 6) & 0x3F));
      val += (0x80 | (c & 0x3F));
    }            else if (c < 0xFFFF)
    {
      val += (0xF0 | (c >> 12));
      val += (0x80 | ((c >> 6) & 0x3F));
      val += (0x80 | (c & 0x3F));
    }

    else if (c < 0xFFFF)
    {
      val += (0xF8 | (c >> 12));
      val += (0x80 | ((c >> 6) & 0x3F));
      val += (0x80 | (c & 0x3F));
    }
    else if (c < 0xFFFF)
    {
      val += (0xFC | (c >> 12));
      val += (0x80 | ((c >> 6) & 0x3F));
      val += (0x80 | (c & 0x3F));
    }
*/
  else
    throw ParameterException(ApplicationError, ulxr_i18n(ULXR_PCHAR("unicodeToUtf8(): unicode character too big: ")) + HtmlFormHandler::makeHexNumber(c));

  return val;
}

#if defined(HAVE_ICONV_H) || defined(HAVE_ICONV)

ULXR_API_IMPL(Cpp8BitString) convertEncoding(const Cpp8BitString &val,
                                          const char *to_encoding,
                                          const char *from_encoding)
{
  iconv_t con;
  if ((ssize_t) (con = iconv_open(to_encoding, from_encoding)) < 0)
    throw ParameterException(UnsupportedEncodingError,
                             ulxr_i18n(ULXR_PCHAR("Unsupported encoding (iconv_open() reported error)")));

  Cpp8BitString ret;
  try
  {
    ret = convertEncoding(val, con);
  }
  catch(...)
  {
    iconv_close(con);
    throw;
  }
  iconv_close(con);
  return ret;
}


ULXR_API_IMPL(Cpp8BitString) convertEncoding(const Cpp8BitString &val, iconv_t con)
{
  Cpp8BitString ret;
  char buffer[200];
  bool convert = true;
  unsigned in_offset = 0;
  while (convert && val.length() > in_offset)
  {
    size_t outbytes = sizeof(buffer);
    char *outbuf = buffer;
    const char *inbuf = const_cast<char*>(val.data())+in_offset;
    size_t inbytes = val.length()-in_offset;
    if ((int)iconv(con, (ICONV_CONST char**)&inbuf, &inbytes, &outbuf, &outbytes) <  0)
    {
      // a bit dangerous, output buffer must take at least a complete
      // character sequence, e.g. utf8 has at most 6 bytes.
      if (errno != E2BIG)
        throw ParameterException(SystemError,
                                 ulxr_i18n(ULXR_PCHAR("Error while converting string (iconv() reported error)")));
    }
    else
      convert = false;

    ret.append(buffer, sizeof(buffer)-outbytes);
    in_offset += (val.length()-in_offset)-inbytes;
  }
  return ret;
}


ULXR_API_IMPL(Cpp8BitString) encodingToUtf8(const Cpp8BitString &val, const char *encoding)
{
  return convertEncoding(val, "UTF-8", encoding);
}


ULXR_API_IMPL(Cpp8BitString) utf8ToEncoding(const Cpp8BitString &val, const char *encoding)
{
  return convertEncoding(val, encoding, "UTF-8");
}

#endif


ULXR_API_IMPL(void) makeLower( CppString &str)
{
  for (unsigned i= 0; i < str.length(); ++i)
#ifdef ULXR_UNICODE
    if (str[i] <= 0xff)   // TODO: handle correctly ??
#endif
      str[i] = ulxr_tolower(str[i]);
}


ULXR_API_IMPL(void) makeUpper( CppString &str)
{
  for (unsigned i= 0; i < str.length(); ++i)
#ifdef ULXR_UNICODE
    if (str[i] <= 0xff)   // TODO: handle correctly ??
#endif
      str[i] = ulxr_toupper(str[i]);
}


ULXR_API_IMPL(std::string) getWbXmlExtInt(long int i)
{
  std::string s;
  s = (char) WbXmlParser::wbxml_EXT_T_0;
  if (i <= (1 << 8))
    s += (unsigned char) i;
  else if (i <= (1 << (8+7)))
  {
    s += (unsigned char) ((i >> 7) & 0x7F) | 0x80;
    s += (unsigned char) i & 0x7F;
  }
  else if (i <= (1 << (8+7+7)))
  {
    s += ((unsigned char) (i >> (7+7)) & 0x7F) | 0x80;
    s += ((unsigned char) (i >> 7) & 0x7F) | 0x80;
    s += (unsigned char) i & 0x7F;
  }
  else if (i <= (1 << (8+7+7+7)))
  {
    s += ((unsigned char) (i >> (7+7+7)) & 0x7F) | 0x80;
    s += ((unsigned char) (i >> (7+7)) & 0x7F) | 0x80;
    s += ((unsigned char) (i >> 7) & 0x7F) | 0x80;
    s += (unsigned char) i & 0x7F;
  }
  else
  {
    s += ((unsigned char) (i >> (7+7+7+7)) & 0x7F) | 0x80;
    s += ((unsigned char) (i >> (7+7+7)) & 0x7F) | 0x80;
    s += ((unsigned char) (i >> (7+7)) & 0x7F) | 0x80;
    s += ((unsigned char) (i >> 7) & 0x7F) | 0x80;
    s += (unsigned char) i & 0x7F;
  }
  return s;
}


ULXR_API_IMPL(long int) wbXmlToInt(std::string &inp)
{
  if (inp.length() < 2)
    throw ParameterException(ApplicationError,
                             ulxr_i18n(ULXR_PCHAR("wbXmlToInt(): wbxml input string shorter than 2 characters")));

  if (inp[0] != (char) WbXmlParser::wbxml_EXT_T_0)
    throw ParameterException(ApplicationError,
                             ulxr_i18n(ULXR_PCHAR("wbXmlToInt(): token EXT_T_0 not found at the beginning")));

  unsigned idx = 1;
  bool cont = true;
  long int i = 0;
  while (cont) // at most 5 octets
  {
    if (idx >= 5+1) // 5 * 7 = 35 bits
      throw ParameterException(ApplicationError,
                               ulxr_i18n(ULXR_PCHAR("wbXmlToInt(): multibyte sequence is too long for an <int32>")));
    if (idx >= inp.length())
      throw ParameterException(ApplicationError,
                               ulxr_i18n(ULXR_PCHAR("wbXmlToInt(): end of multibyte sequence not found")));
    i <<= 7;
    unsigned val = (unsigned char) inp[idx];
    i |= 0x7f & val;
    ++idx;
    cont = (0x80 & val) != 0; // last byte: missing bit 7
  }
  inp.erase(0, idx);
  return i;
}


ULXR_API_IMPL(std::string) getWbXmlString(const CppString &inp)
{
  std::string s;
  s = (char) WbXmlParser::wbxml_STR_I;
// TODO: transform in case of '\0' in string
#ifdef ULXR_UNICODE
  s += unicodeToUtf8(inp) + '\0';
#else
  s += inp + '\0';
#endif
  return s;
}


ULXR_API_IMPL(CppString) wbXmlToString(std::string &inp)
{
  if (inp[0] != WbXmlParser::wbxml_STR_I)
    throw ParameterException(ApplicationError,
                             ulxr_i18n(ULXR_PCHAR("wbXmlToString(): token wbxml_STR_I not found at the beginning")));

  if (inp.length() < 2)
    throw ParameterException(ApplicationError,
                             ulxr_i18n(ULXR_PCHAR("wbXmlToString(): wbxml input string shorter than 2 characters")));

  std::string s;
  unsigned idx = 1;
  bool cont = true;
  while (cont)
  {
    if (idx >= inp.length())
      throw ParameterException(ApplicationError,
                               ulxr_i18n(ULXR_PCHAR("wbXmlToString(): end of inline string not found")));

    unsigned val = (unsigned char) inp[idx];
    if (val == 0)
      cont = false;
    else
      s += char(val);
    ++idx;
  }
  inp.erase(0, idx);

#ifdef ULXR_UNICODE
  return utf8ToUnicode(s);
#else
  return s;
#endif
}


ULXR_API_IMPL(CppString) binaryDebugOutput (const std::string &s)
{
  CppString ret;
  unsigned cnt = 0;
  for (unsigned i = 0; i < s.length(); ++i)
  {
    ret += ulxr::HtmlFormHandler::makeHexNumber((unsigned char) s[i]) + ULXR_PCHAR(" ");
    if (++cnt >= 20)
    {
      ret += ULXR_PCHAR("\n");
      cnt = 0;
    }
  }
  if (cnt != 0)
    ret += ULXR_PCHAR("\n");

  return ret;
}


#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
ULXR_API_IMPL(Mutex) randMutex;
ULXR_API_IMPL(Mutex) strerrorMutex;
ULXR_API_IMPL(Mutex) ctimeMutex;
ULXR_API_IMPL(Mutex) localtimeMutex;
ULXR_API_IMPL(Mutex) gethostbynameMutex;
ULXR_API_IMPL(Mutex) gethostbyaddrMutex;
#endif


ULXR_API_IMPL(int) getRand()
{
#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
  Mutex::Locker lock(randMutex);
#endif

  return std::rand();
}


ULXR_API_IMPL(CppString) getLastErrorString(unsigned errornum)
{
#ifndef ULXR_OMIT_REENTRANT_PROTECTOR
  Mutex::Locker lock(strerrorMutex);
#endif

  return ULXR_GET_STRING(std::strerror(errornum));
}


namespace {

std::auto_ptr<TcpIpConnection> l4jconn;
std::auto_ptr<Log4JSender>     l4j;

}


ULXR_API_IMPL(Log4JSender*) getLogger4J()
{
  return l4j.get();
}


ULXR_API_IMPL(void) intializeLog4J(const std::string &appname, const CppString &loghost)
{
  std::string s = appname;
  unsigned pos;
#ifdef __unix__
  while ((pos = s.find("/")) != std::string::npos)
#else
  while ((pos = s.find("\\")) != std::string::npos)
#endif
    s.erase(0, pos+1);
  l4jconn.reset(new TcpIpConnection(false, loghost, 4448));
  l4j.reset(new Log4JSender(ULXR_GET_STRING(s), *l4jconn.get()));
}


namespace
{
  bool pretty_xml  = false;
}


ULXR_API_IMPL(void) enableXmlPrettyPrint(bool pretty)
{
  pretty_xml = pretty;
}

namespace
{
  static const CppString empty_LF = ULXR_PCHAR("");
  static const CppString normal_LF = ULXR_PCHAR("\n");
  static const CppString empty_Indent = ULXR_PCHAR("");
}

ULXR_API_IMPL(CppString) getXmlLinefeed()
{
  if (pretty_xml)
    return normal_LF;

  else
    return empty_LF;
}


ULXR_API_IMPL(CppString) getXmlIndent(unsigned indent)
{
  if (pretty_xml)
    return CppString(indent, ' ');

  else
    return empty_Indent;
}


}  // namespace ulxr

