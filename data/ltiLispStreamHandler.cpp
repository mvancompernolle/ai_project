/*
 * Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */


/* -----------------------------------------------------------------------
 * project ....: LTI Digitale Bild/Signal Verarbeitungsbibliothek
 * file .......: ltiLispStreamHandler.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 7.12.2000
 * revisions ..: $Id: ltiLispStreamHandler.cpp,v 1.15 2008/08/17 22:20:12 alvarado Exp $
 */

#include "ltiLispStreamHandler.h"
#include "ltiException.h"
#include <ctype.h>
#include <cstdlib>

#ifdef _LTI_GNUC_2
namespace std {
  typedef ios ios_base;
}
#endif

#undef _LTI_DEBUG
// #define _LTI_DEBUG 4
#include "ltiDebug.h"

namespace lti {
  // --------------------------------------------------
  // lispStreamHandler
  // --------------------------------------------------
  const char lispStreamHandler::openChar = '(';
  const char lispStreamHandler::closeChar = ')';
  const char lispStreamHandler::separator = ' ';
  const char lispStreamHandler::commentChar = ';';
  const char lispStreamHandler::stringChar = '"';
  const char lispStreamHandler::quoteChar = '\'';

  const bool* lispStreamHandler::delimiters = 0;

  // more than 8kB seems to produce problems with some versions of std::string
  // so our buffer has just 8kB.
  const int lispStreamHandler::garbageThreshold   = 8192;  // 8kB buffer size

  // delimiters-LUT initialization
  void lispStreamHandler::initializeDelimiters() {
    if (isNull(delimiters)) {
      // the one and only real LUT
      static bool tmp[256];

      for (int i=0;i<256;++i) {
        tmp[i] = false;
      }
      tmp[0] = true; // end of a string must be considered as a delimiter too
      tmp[static_cast<int>(openChar)]    = true; // "("
      tmp[static_cast<int>(closeChar)]   = true; // ")"
      tmp[static_cast<int>(separator)]   = true; // " "
      tmp[static_cast<int>('\n')]        = true;
      tmp[static_cast<int>('\r')]        = true;
      tmp[static_cast<int>('\f')]        = true;
      tmp[static_cast<int>('\v')]        = true;
      tmp[static_cast<int>('\t')]        = true;
      tmp[static_cast<int>(commentChar)] = true; // ";"
      tmp[static_cast<int>(stringChar)]  = true; // '"'
      tmp[static_cast<int>('\\')]        = true;

      // from here on, a const bool*...
      delimiters = tmp;
    }
  }

  // default constructor
  lispStreamHandler::lispStreamHandler()
    : ioHandler(),inStream(0),inString(""),inStringPos(0),outStream(0),
      supressSpaces(false),tryEOL(false),buffer(0) {
    initializeDelimiters();
    buffer = new char[garbageThreshold];
  }

  /*
   * default constructor
   */
  lispStreamHandler::lispStreamHandler(std::istream& aStream)
    : ioHandler(),inStream(&aStream),inString(""),inStringPos(0),outStream(0),
      supressSpaces(false),tryEOL(false),buffer(0) {
    initializeDelimiters();
    buffer = new char[garbageThreshold];
  }

  /*
   * default constructor
   */
  lispStreamHandler::lispStreamHandler(std::ostream& aStream)
    : ioHandler(),inStream(0),inString(""),inStringPos(0),outStream(&aStream),
      supressSpaces(false),tryEOL(false),buffer(0) {
    initializeDelimiters();
    buffer = new char[garbageThreshold];
  }

  /*
   * default constructor
   */
  lispStreamHandler::lispStreamHandler(const char *in)
    : ioHandler(),inStream(0),inString(in),inStringPos(0),outStream(0),
      supressSpaces(false),tryEOL(false) {
    initializeDelimiters();

    buffer=new char[garbageThreshold];
  }



  // copy constructor
  lispStreamHandler::lispStreamHandler(const lispStreamHandler& other)
    : ioHandler()  {
    copy(other);
    buffer = new char[garbageThreshold];
  }

  // destructor
  lispStreamHandler::~lispStreamHandler() {
    inStream = 0;
    outStream = 0;
    delete[] buffer;
    buffer = 0;
  }

  void lispStreamHandler::use(std::istream& aStream) {
    resetLevel();
    inStream = &aStream;
    inString = "";
    inStringPos = 0;
    stack.clear();
  }

  void lispStreamHandler::use(std::ostream& aStream) {
    resetLevel();
    outStream = &aStream;
    supressSpaces = false;
    tryEOL=false;
  }

  void lispStreamHandler::clear() {
    resetLevel();
    inString="";
    inStringPos=0;
    stack.clear();
    supressSpaces = false;
    tryEOL=false;
  }

  // returns the name of this type
  const char* lispStreamHandler::getTypeName() const {
    return "lispStreamHandler";
  }

  // copy member
  lispStreamHandler&
  lispStreamHandler::copy(const lispStreamHandler& other) {
    ioHandler::copy(other);
    inStream         = other.inStream;
    inString         = other.inString;
    inStringPos      = other.inStringPos;
    outStream        = other.outStream;
    supressSpaces    = other.supressSpaces;
    tryEOL           = other.tryEOL;
    stack            = other.stack;
    return (*this);
  }

  lispStreamHandler&
  lispStreamHandler::operator=(const lispStreamHandler& other) {
    return copy(other);
  }

  // clone member
  ioHandler* lispStreamHandler::clone() const {
    return new lispStreamHandler(*this);
  }

  /*
   * the begin token or tokens
   */
  bool lispStreamHandler::writeBegin() {
    if (tryEOL) {
      (*outStream) << std::endl;
      tryEOL = false;
    }
    writeSpaces(getLevel());
    (*outStream) << openChar;
    supressSpaces = true;
    ioHandler::writeBegin();
    return (outStream->good());
  }

  /*
   * the end token or tokens
   */
  bool lispStreamHandler::writeEnd() {
    supressSpaces = false;
    (*outStream) << closeChar;
    ioHandler::writeEnd();
    return (outStream->good());
  }

  /*
   * the begin token or tokens
   */
  bool lispStreamHandler::readBegin() {
    std::string str;

    if (stack.empty() || stack.front().level != (getLevel()+1) ||
        ((!stack.front().complete) && stack.front().cache.empty())) {
      if (getNextToken(str) == BeginToken) {
        return ioHandler::readBegin();
      } else {
        appendContextStatus();
        return false;
      }
    } else {
      // token already read (maybe!)
      return ioHandler::readBegin();
    }
  }

  /*
   * the end token or tokens
   */
  bool lispStreamHandler::readEnd() {
    std::string str;
    int level = getLevel();
    int actLevel = level;
    eTokenId tkId = EndToken;

    while ((tkId != ErrorToken) && (actLevel>=level)) {
      tkId = getNextToken(str);
      if (tkId == BeginToken) {
        actLevel++;
      } else if (tkId == EndToken) {
        actLevel--;
      }
    }

    if ((tkId == EndToken) && (actLevel < level)) {

      while (!stack.empty() && (stack.front().level > getLevel())) {
        // pop all unused data (these should already be processed!)
        stack.pop_front();
      }

      return ioHandler::readEnd();
    } else {
      appendContextStatus();
      return false;
    }
  }

  /*
   * write a std::string
   */
  bool lispStreamHandler::write(const std::string& data) {

    // strings always with quotes! except if they are a comment!
    if ((data.length() > 0) && (data[0] == ';')) {
      // its a comment!
      if (tryEOL) {
        (*outStream) << std::endl;
      }
      (*outStream) << data;
    } else {
      (*outStream) << stringChar << data;
      if ((data.length() > 0) && (data[data.length()-1] == '\\')) {
        // the last character is a backslash, and it is not allowed
        // that the combination '\"' the last thing in the string is.
        // so we end with the combination '\\"'.  That way the string can be
        // read back
        (*outStream) << '\\';
      }
      (*outStream) << stringChar;
    }
    return outStream->good();
  }

  /*
   * write a std::string
   */
  bool lispStreamHandler::write(const char* data) {
    return write(std::string(data));
  }

  /*
   * read a std::string
   */
  bool lispStreamHandler::readSymbol(std::string& data) {
    return read(data); // just read the string as defined before!
  }

  /*
   * read a std::string
   */
  bool lispStreamHandler::read(std::string& data) {
    bool result;
    eTokenId tkid = getNextToken(data);

    result = ((tkid == StringToken) ||
              (tkid == SymbolToken));

    return result;
  }

  /*
   * try to read a symbol
   */
  bool lispStreamHandler::trySymbol(const std::string& data) {
    _lti_debug("Try symbol " << data << std::endl);

#if defined _LTI_DEBUG && _LTI_DEBUG >= 2
    _lti_debug("Stack contents: " << std::endl);
    
    std::list<stackElement>::const_iterator sit;
    cacheType::const_iterator it;

    for (sit=stack.begin();sit!=stack.end();++sit) {
      const stackElement& e = (*sit);
      _lti_debug("  Level: " << e.level << std::endl);
      _lti_debug("  Compl: " << (e.complete ? "true" : "false") << std::endl);
      for (it=e.cache.begin();it!=e.cache.end();++it) {
        _lti_debug("    " << (*it).first << " -> " << (*it).second << "\n");
      }
    }
    
#endif


    // assuming that before this symbol, there was a beginToken
    // and after this symbol there is still data that can be used...

    // first read the next token, that should be a symbol
    std::string symb,value;

    // was the stack for this level already initialized?
    while (!stack.empty() && stack.front().level > getLevel()) {
      // pop all unused data (these should already be processed!)
      stack.pop_front();
    }

    if (stack.empty() || stack.front().level < getLevel()) {
      // no stack for this level there... create one:
      stackElement elem;
      elem.level = getLevel();
      stack.push_front(elem);
    } else {
      cacheType::iterator it;
      it = stack.front().cache.find(data);
      if (it != stack.front().cache.end()) {
        // already read
        // insert the value in the input string
        if (inStringPos >= inString.length()) {
          inString.append((*it).second);
        } else {
          inString.insert(inStringPos+1,(*it).second); // changed 0311301107
        }
        // delete this "recovered" data
        stack.front().cache.erase(it);
        //
        return true;
      } else if (stack.front().complete) {
        // stack completed and the symbol is not there!
        return false;
      }

      if (!stack.front().cache.empty() || stack.front().complete) {
        getNextToken(symb);  // read the begin token (this wasn't really read
                             // by the last readBegin(), due to the supposition
                             // that the data could already be in the cache)
      }
    }

    // try to read next symbol
    eTokenId tkId;

    tkId = getNextToken(symb); // read the next symbol

    while ((symb != data) && !stack.front().complete) {
      // wrong symbol read... save for later use...
      completeLevel(value);
      stack.front().cache[symb]=value;

      // try to get the next symbol/value pair
      tkId = getNextToken(symb,true); // just try
      if (tkId == BeginToken) {
        // everything ok! read the next pair...
        getNextToken(symb);  // read the begin token, but this time do it!
        tkId = getNextToken(symb); // read the next symbol
      } else {
        // token not expected... the level is done...
        stack.front().complete = true;
      }
    }

#if defined _LTI_DEBUG && _LTI_DEBUG >= 2
    if (symb==data) {
      _lti_debug("done." << std::endl);
    } else {
      _lti_debug("failed." << std::endl);
    }
#endif

    return (symb == data);
  }

  /*
   * try to read a the end token
   */
  bool lispStreamHandler::tryEnd() {
    std::string tmp;
    // try to read the End Token
    if (getNextToken(tmp,true) == EndToken) {
      return readEnd(); // it is there... just read it as usual...
    } else {
      return false;    // is not there!
    }
  }

  /*
   * try to read a the begin token
   */
  bool lispStreamHandler::tryBegin() {
    std::string tmp;
    // try to read the Begin Token
    if (getNextToken(tmp,true) == BeginToken) {
      return readBegin(); // it is there... just read it as usual...
    } else {
      return false;    // is not there!
    }
  }


  bool lispStreamHandler::write(const double& data) {
    outStream->precision(16);
    outStream->setf(std::ios_base::fmtflags(0),std::ios_base::floatfield);

    (*outStream) << data;
    return (outStream->good());
  }

  bool lispStreamHandler::write(const float& data) {
    outStream->precision(9);
    outStream->setf(std::ios_base::fmtflags(0),std::ios_base::floatfield);

    (*outStream) << data;
    return (outStream->good());
  }

  bool lispStreamHandler::write(const int& data) {
    outStream->precision(10);
    outStream->setf(std::ios_base::dec,std::ios_base::basefield);
    outStream->setf(std::ios_base::fixed,std::ios_base::floatfield);

    (*outStream) << data;
    return (outStream->good());
  }

  bool lispStreamHandler::write(const unsigned int& data) {
    outStream->precision(10);
    outStream->setf(std::ios_base::dec,std::ios_base::basefield);
    outStream->setf(std::ios_base::fixed,std::ios_base::floatfield);

    (*outStream) << data;
    return (outStream->good());
  }

  bool lispStreamHandler::write(const char& data) {
    if ((data >= ' ') && (data <= '~')) {
      (*outStream) << "'" << data << "'";
    } else {
      outStream->precision(4);
      outStream->setf(std::ios_base::dec,std::ios_base::basefield);
      outStream->setf(std::ios_base::fixed,std::ios_base::floatfield);
      (*outStream) << static_cast<int>(data);
    }

    return (outStream->good());
  }

  bool lispStreamHandler::write(const byte& data) {
    outStream->precision(4);
    outStream->setf(std::ios_base::dec,std::ios_base::basefield);
    outStream->setf(std::ios_base::fixed,std::ios_base::floatfield);

    (*outStream) << static_cast<int>(data);
    return (outStream->good());
  }

  bool lispStreamHandler::write(const ubyte& data) {
    outStream->precision(4);
    outStream->setf(std::ios_base::dec,std::ios_base::basefield);
    outStream->setf(std::ios_base::fixed,std::ios_base::floatfield);

    (*outStream) << static_cast<int>(data);
    return (outStream->good());
  }

  bool lispStreamHandler::write(const bool& data) {
    if (data) {
      return writeSymbol(std::string("#t"));
    } else {
      return writeSymbol(std::string("#f"));
    }
  }

  bool lispStreamHandler::write(const long& data) {
    outStream->precision(20);
    outStream->setf(std::ios_base::dec,std::ios_base::basefield);
    outStream->setf(std::ios_base::fixed,std::ios_base::floatfield);

    (*outStream) << data;
    return (outStream->good());
  }

  bool lispStreamHandler::write(const unsigned long& data) {
    outStream->precision(20);
    outStream->setf(std::ios_base::dec,std::ios_base::basefield);
    outStream->setf(std::ios_base::fixed,std::ios_base::floatfield);

    (*outStream) << data;
    return (outStream->good());
  }

  bool lispStreamHandler::write(const short& data) {
    outStream->precision(10);
    outStream->setf(std::ios_base::dec,std::ios_base::basefield);
    outStream->setf(std::ios_base::fixed,std::ios_base::floatfield);

    (*outStream) << data;
    return (outStream->good());
  }

  bool lispStreamHandler::write(const unsigned short& data) {
    outStream->precision(10);
    outStream->setf(std::ios_base::dec,std::ios_base::basefield);
    outStream->setf(std::ios_base::fixed,std::ios_base::floatfield);

    (*outStream) << data;
    return (outStream->good());
  }


  /*
   * write a std::string
   */
  bool lispStreamHandler::writeSymbol(const std::string& data) {
    if ((data == "") || (data.find_first_of(" \t") != std::string::npos)) {
      // there are some spaces in the string, use quotes!
      (*outStream) << stringChar << data;
      if ((data.length() > 0) && (data[data.length()-1] == '\\')) {
        // the last character is a backslash, and it is not allowed
        // that the combination '\"' the last thing in the string is.
        // so we end with the combination '\\"'.  That way the string can be
        // read back
        (*outStream) << '\\';
      }
      (*outStream) << stringChar;
    } else {
      (*outStream) << data;
    }

    supressSpaces = false;

    return outStream->good();
  }

  /**
   * write comment writes the input data without any preprocessing,
   * just ensuring that the comment format is given
   */
  bool lispStreamHandler::writeComment(const std::string& data) {
    if (tryEOL) {
      (*outStream) << std::endl;
      tryEOL = false;
    }
    (*outStream) << ";; " << data << std::endl; // must end with EOL!
    return outStream->good();
  }

  /**
   * write comment writes the input data without any preprocessing,
   * just ensuring that the comment format is given
   */
  bool lispStreamHandler::writeComment(const char* data) {
    if (tryEOL) {
      (*outStream) << std::endl;
      tryEOL = false;
    }
    (*outStream) << ";; " << data << std::endl;  // must end with EOL
    return outStream->good();
  }

  /*
   * write white spaces
   */
  bool lispStreamHandler::writeSpaces(const int& i) {
    if (!supressSpaces && i>0) {
      std::string spc;
      spc.assign(i,' ');
      (*outStream) << spc;
    } else {
      supressSpaces = false;
    }

    return outStream->good();
  }

  bool lispStreamHandler::writeKeyValueSeparator() {
    supressSpaces = true;
    (*outStream) << separator;
    return outStream->good();
  }

  bool lispStreamHandler::writeDataSeparator() {
    supressSpaces = false;
    (*outStream) << separator;
    return outStream->good();
  }

  bool lispStreamHandler::readKeyValueSeparator() {
    return true;
  }

  bool lispStreamHandler::readDataSeparator() {
    return true;
  }

  /*
   * write eol
   */
  bool lispStreamHandler::writeEOL() {
    tryEOL = true;
    return outStream->good();
  }

  /*
   * check for eof()
   */
  bool lispStreamHandler::eof() {
    if (isNull(inStream)) {
      return (inStringPos>=inString.length());
    } else {
      return inStream->eof();
    }
  }

  /*
   * restore all the information in the handler taken in the actual
   * level.  If "complete" is true, the begin-token is also restored
   */
  bool lispStreamHandler::restoreLevel() {
    return true;
  }

  /*
   * read a double value
   */
  bool lispStreamHandler::read(double& data) {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      data = atof(str.c_str());
    }

    return result;
  };

  /*
   * read a float value
   */
  bool lispStreamHandler::read(float& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      data = static_cast<float>(atof(str.c_str()));
    }

    return result;
  };

  /*
   * read an integer value
   */
  bool lispStreamHandler::read(int& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      // use of atof to allow reading "valid" integers
      // in float format (e.g. 1.2e+6)
      data = static_cast<int>(atof(str.c_str()));
    }

    return result;
  };

  /*
   * read an unsigned int value
   */
  bool lispStreamHandler::read(unsigned int& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      // use of atof to allow reading "valid" integers
      // in float format (e.g. 1.2e+6)
      data = static_cast<unsigned int>(atof(str.c_str()));
    }

    return result;
  };

  /*
   * read a char value
   */
  bool lispStreamHandler::read(char& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      // since the write(const char& x) writes the character enclosed in
      // single quotes or in numeric format, and the getNextToken parses the
      // LISP single quote away, then the only possible way to know if the
      // char is stored as simple ASCII is checking if the second quote is
      // somewhere around
      if (str.find('\'') != std::string::npos) {
        data = str[0];  // get the char "as is"
      } else {
        data = atoi(str.c_str()); // asume a numerical representation
      }
    }

    return result;
  };

  /*
   * read a char value
   */
  bool lispStreamHandler::read(byte& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      data = atoi(str.c_str());
    }

    return result;
  };

  /*
   * read an ubyte value
   */
  bool lispStreamHandler::read(ubyte& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      data = atoi(str.c_str());
    }

    return result;
  };


  /*
   * read a boolean value
   */
  bool lispStreamHandler::read(bool& data) {
    std::string tmp;
    bool b;
    b = readSymbol(tmp);
    data = (tmp == "#t");
    return b;
  }

  /*
   * read a long value
   */
  bool lispStreamHandler::read(long& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      data = atol(str.c_str());
    }

    return result;
  };

  /*
   * read an unsigned long value
   */
  bool lispStreamHandler::read(unsigned long& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      data = atol(str.c_str());
    }

    return result;
  };

  /*
   * read a long value
   */
  bool lispStreamHandler::read(short& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      data = static_cast<short>(atol(str.c_str()));
    }

    return result;
  };

  /*
   * read an unsigned long value
   */
  bool lispStreamHandler::read(unsigned short& data)  {
    bool result;

    std::string str;

    result = (getNextToken(str) != ErrorToken);

    if (result) {
      data = static_cast<unsigned short>(atol(str.c_str()));
    }

    return result;
  };

  /*
   * read next token
   * this member gets the next token into the given string
   *
   * Following tokens are recognized:
   *
   * beginToken := '('
   * endToken   := ')'
   * symbolToken := alpha {alpha | digit}
   * stringToken := quote {alpha | digit | other | escape quote} quote
   * NumberToken := float | integer
   * ErrorToken
   *
   * where
   *
   * alpha = "A" | "B" | ... "Z" | "a" | "b" | ... "z"
   * digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" |
   * other = "!" | "," | "$" | "%" | "&" | "/" | "[" | "]" | "{" | "}" |
   *         "?" | "'" | "#" | "+" | "*" | "-" | "." | "," | ":" | ";"
   * quote    = '"'
   * escape   = '\'
   * integer  = digit {digit}
   * float    = ['+' | '-'] {digit} '.' {digit} [exponent]
   * exponent = eid ['+' | '-'] digit {digit}
   * eid      = 'E' | 'e'
   *
   * comments will be ignored!  They begin with a ';'
   * The ErrorToken is not a real token.  This is the way to signalize that
   * no other token could be read!
   */
  lispStreamHandler::eTokenId
  lispStreamHandler::getNextToken(std::string& token,
                                  const bool justTry) {

    std::string::size_type pos = std::string::npos;

    commentFilter();

    // check if the token should be read from the input string or
    // the input stream...
    if ((inString.length() == 0) ||
        (inStringPos>=inString.length()) ||
        (inString.find_first_not_of(" \t",inStringPos) == std::string::npos)) {
      // read from stream

      std::string newline;

      // read the next line in the stream...
      // empty lines and comment lines should be ignored!
      do {
        newline = "";
        getNextLine(newline);
      } while (!newline.empty() &&
               ((((pos = newline.find_first_not_of(" \t"))==std::string::npos)
                 || (newline[pos] == ';')) &&
                !eof()));

      if (newline.empty() || (pos == std::string::npos)) {
        return ErrorToken;
      }

      inString += newline.substr(pos);
    }

    // read from input string
    eTokenId tkId;

    tkId = getNextTokenFromString(inString,inStringPos,token,justTry);

    if ((tkId == ErrorToken) && (!eof())) {
      // error happend!  maybe the inString was empty...
      // this usually happens due to a comment at the end of the last line...
      tkId = getNextToken(token,justTry);
    }

    return tkId;
  }

  void lispStreamHandler::getNextLine(std::string& newline) {
    char c;
    int count;

    newline="";

    // read a new line from the stream

    if (isNull(inStream) || inStream->eof() || !inStream->good()) {
      // we don't have a stream
      return;
    }

    count = -1;

    // read all empty lines, until some data is found or the end of file
    // is reached
    do {
      // the get() method set the fail flag of the stream if the
      // next char is the terminal char, so let's get rid of them first

      do {
        c=inStream->peek();
        if (c == '\n') {
          c = inStream->get();
        }
      } while (c == '\n');

      inStream->get(buffer,garbageThreshold-1,'\n');      
      count = inStream->gcount(); // number of read chars

      if (count > 0) {
        newline=buffer;
      }

      c = inStream->get(); // this MUST be done, or the next get(buffer...)
                           // can set the fail bit if the next char is a \n

    } while (!inStream->eof() && (count <= 0));

    // the last read char was not a new line only if the 
    if (count >= garbageThreshold-2) {
      // buffer full!
      // add chars until a delimiter is found
      do {
        newline+=c;
        c=inStream->get();
      } while (!isTokenDelimiter(c));

      if ((c != '\n') && (c != '\r') && (c != '\f')) {
        // if the delimiter found was not a new line, save it back, because
        // it can be an important token, like begin or end.
        inStream->putback(c);
        newline+=' '; // space to avoid confusion in reading symbol!
      }
    } 
  }

  bool lispStreamHandler::commentFilter() {
    std::string::size_type pos,pos2;

    while ((inString.length()>inStringPos) &&
           ((pos = inString.find_first_not_of(" \t\r\n\f",inStringPos))
            != std::string::npos) &&
           (inString[pos] == ';')) {
      // comment begin found
      // search for end of comment at the end of line (eol)
      // potential bug fixed: search from inStringPos and not from the
      // beginning of the inString
      //                                     |||||||||||
      //                                     VVVVVVVVVVV
      pos2 = inString.find_first_of("\r\n\f",inStringPos);
      if (pos2 == std::string::npos) {
        // no eol found -> delete everything
        inString.erase(inStringPos);
      } else {
        // delete until the eol
        inString.erase(inStringPos,pos2-inStringPos+1);
      }
    }
    return true;
  }

  inline bool lispStreamHandler::isTokenDelimiter(const char& c) {
    return delimiters[static_cast<unsigned char>(c)];
  }

  /*
   * read next token
   * this member gets the next token into the given string
   *
   * Following tokens are recognized:
   *
   * beginToken := '('
   * endToken   := ')'
   * symbolToken := alpha {alpha | digit}
   * stringToken := quote {alpha | digit | other | escape quote} quote
   * NumberToken := float | integer
   * ErrorToken
   *
   * where
   *
   * alpha = "A" | "B" | ... "Z" | "a" | "b" | ... "z"
   * digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" |
   * other = "!" | "," | "$" | "%" | "&" | "/" | "[" | "]" | "{" | "}" |
   *         "?" | "'" | "#" | "+" | "*" | "-" | "." | "," | ":" | ";"
   * quote    = '"'
   * escape   = '\'
   * integer  = digit {digit}
   * float    = ['+' | '-'] {digit} '.' {digit} [exponent]
   * exponent = eid ['+' | '-'] digit {digit}
   * eid      = 'E' | 'e'
   *
   * comments will be ignored!  They begin with a ';'
   * The ErrorToken is not a real token.  This is the way to signalized that
   * no other token could be read!
   */
  lispStreamHandler::eTokenId
  lispStreamHandler::getNextTokenFromString(std::string& src,
                                            std::string::size_type& srcPos,
                                            std::string& token,
                                            const bool justTry) {

    std::string::size_type pos;

    if ((src.length() == 0) ||
        (srcPos>=src.length()) ||
        (src.find_first_not_of(" \t",srcPos) == std::string::npos)) {

      _lti_debug3("ERR" << std::endl);

      return ErrorToken;
    }

    // read from input string

    // parsing with an automat...

    // possible states
    static const int stStart = 0;
    static const int stEnd   = 1;
    static const int stReadingSymbol = 2;
    static const int stReadingString = 3;

    int state = stStart;
    char c;
    token = "";
    pos = srcPos;
    eTokenId theToken = ErrorToken;

    while ((state != stEnd) && (pos < src.length())) {
      c = src[pos];

      switch (state) {
      case stStart: {
        if (c == openChar) {
          token += c;
          theToken = BeginToken;
          state = stEnd;
        } else if (c == closeChar) {
          token += c;
          theToken = EndToken;
          state = stEnd;
        } else if (c == stringChar) {
          theToken = StringToken;
          state = stReadingString;
        } else if (c == commentChar) {
          std::string::size_type til;
          std::string* theSrc;
          std::string tmp;

          if (justTry) {
            tmp = src;
            theSrc = &tmp;
          } else {
            theSrc = &src;
          }

          // delete comment until next newline
          til = theSrc->find_first_of("\r\n\f",pos);
          if (til == std::string::npos) {
            // delete until the end of the string
            theSrc->erase(pos);
          } else {
            til = theSrc->find_first_not_of("\r\n\f",til);
            theSrc->erase(pos,til-pos);
          }
          return getNextTokenFromString(*theSrc,srcPos,token,justTry);

        } else if (!isspace(c)) {
          if (c!=quoteChar) {
            token += c;
          }
          theToken = SymbolToken;
          state = stReadingSymbol;
        }
      } break;
      case stReadingSymbol: {
        while (state != stEnd) {
          while (!isTokenDelimiter(c)) {
            token += c;
            pos++;
            if (pos < src.length()) {
              c = src[pos];
            } else {
              c = 0;
            }
          }
          if (c == '\\') {
            pos++;
            if (pos < src.length()) {
              token += src[pos];
            } else {
              state = stEnd; // flag to exit
            }
          } else  {
            state = stEnd; // flag to exit
          }
        }
        pos--;
      } break;
      case stReadingString: {

        std::string::size_type pos2,porig;
        porig = pos;
        std::string sub;
        char cc;

        // search for end of the symbol (Delimiter chars).

        // Backslash will be used as escape character only if the
        // next character is a quote '"' or a backslash, i.e. if the string
        // contains a sequence '\"' or '\\', it will be replaced with '"' or
        // '\' respectivelly, otherwise the backslash will remain unchanged!

        // search for end of string
        pos2=src.find_first_of("\"\\",porig);
        while ((pos2 != std::string::npos) && (src[pos2] == '\\')) {
          sub+=src.substr(porig,pos2-porig);
          cc = src[pos2+1];
          if ((cc != '"') && (cc != '\\')) {
            sub+='\\';
          }
          sub+=src[pos2+1];
          porig = pos2+2;
          pos2=src.find_first_of("\"\\",porig);
        }
        sub += src.substr(porig,pos2-porig);
        token+= sub;
        pos = pos2;
        state = stEnd;

      } break;
      default: {
        throw
          exception("lispStreamHandler::getNextToken::unknown automat state!");
      } // default
      } // switch

      pos++;
    }

    if (theToken != ErrorToken) {
      if (!justTry) {        
        if (static_cast<int>(pos)>garbageThreshold) {
          // too much garbage! delete it!
          src.erase(0,pos);
          srcPos = 0;
        } else {
          // not many things to delete: just move the pointer:
          srcPos = pos;
        }
      }
    } else {
      // error
      if (state == stStart) {
        srcPos = pos; // report position change (maybe a space at the end)
      }
    }
    
    _lti_debug3(theToken << '<' << token << '>');

    return theToken;
  }

  /*
   * complete actual level reads the data from the input string and
   * input stream until the actual level has been read.
   */
  bool lispStreamHandler::completeLevel(std::string& restOfLevel) {
    restOfLevel = ""; // clean the result string...

    // try to get the data from the input string first...
    char c='E';
    int i = static_cast<int>(inStringPos);
    int theLevel,actLevel,lvl;
    int length = static_cast<int>(inString.length());

    bool fromString;

    theLevel = actLevel = getLevel();
    lvl = theLevel-1;

    // get all data until the corresponding endToken is found
    fromString = (length>i);

    do {

      if (fromString) {
        c = inString[i];
      } else {
        if (notNull(inStream)) {
          inStream->get(c);
        } else {
          c=' ';
        }
      }

      restOfLevel += c;
      if (c == closeChar) {
        actLevel--;
      } else if (c == openChar) {
        actLevel++;
      }
      ++i;
      fromString = (length>i);
    } while ((fromString || !eof()) &&
             (c!=closeChar || (actLevel>lvl)));

    // if the last char was the close token of lvl, it must
    // be restored!
    if (fromString) {
      inString = inString.erase(inStringPos,i-inStringPos);
    } else {
      inString.erase(inStringPos);
    }

    return !eof();
  }

  // default constructor for a stack element
  lispStreamHandler::stackElement::stackElement() {
    complete = false;
    level = 0;

    if (!cache.empty()) {
      cache.clear();
    }
  }

  // append some information to find where an error occur
  void lispStreamHandler::appendContextStatus() const {
    // FIXME: This is a very bad context information, but this is the general
    // idea!
    // find a context after the error until the next newline
    std::string::size_type posn = inString.find('\n',inStringPos);
    if ((posn == std::string::npos) || ((posn-inStringPos) > 30)) {
      appendStatusString(">>> " + inString.substr(inStringPos,40));
    } else {
      appendStatusString(">>> " + inString.substr(inStringPos,
                                                  posn-inStringPos+1));
    }
  }
}
