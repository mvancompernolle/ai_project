/*
 * Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006
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
 * file .......: ltiBinaryStreamHandler.cpp
 * authors ....: Pablo Alvarado, Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 18.5.2001
 * revisions ..: $Id: ltiBinaryStreamHandler.cpp,v 1.7 2006/02/08 12:09:01 ltilib Exp $
 */

#include "ltiObject.h"
#include "ltiTypes.h"
#include "ltiBinaryStreamHandler.h"
#include <ctype.h>

#ifdef _LTI_GNUC_2
namespace std {
  typedef ios ios_base;
}
#endif

#define _OLD_METHOD_ 1

namespace lti {


  // default constructor
  binaryStreamHandler::binaryStreamHandler()
    : ioHandler(),inStream(0),inString(""),inStringPos(0),outStream(0),
      supressSpaces(false),tryEOL(false),buffer(0),bufSize(0) {
  }

  /*
   * default constructor
   */
  binaryStreamHandler::binaryStreamHandler(std::istream& aStream)
    : ioHandler(),inStream(&aStream),inString(""),inStringPos(0),outStream(0),
      supressSpaces(false),tryEOL(false),buffer(0),bufSize(0) {

    // alvarado: 20.10.2001
    // following line is wrong!  ios_base::binary is an "openmode" token
    // and not a format flag (set with setf)!
    // The stream given as parameters should have set this flag as it was
    // opened!

    //inStream->setf(std::ios_base::binary);
  }

  /*
   * default constructor
   */
  binaryStreamHandler::binaryStreamHandler(std::ostream& aStream)
    : ioHandler(),inStream(0),inString(""),inStringPos(0),outStream(&aStream),
      supressSpaces(false),tryEOL(false),buffer(0),bufSize(0) {

    // alvarado: 20.10.2001
    // following line is wrong!  ios_base::binary is an "openmode" token
    // and not a format flat (set with setf)!
    // The stream given as parameters should have set this flag when
    // opened!

    // outStream->setf(std::ios_base::binary);
  }

  // copy constructor
  binaryStreamHandler::binaryStreamHandler(const binaryStreamHandler& other)
    : ioHandler()  {
    copy(other);
  }

  // destructor
  binaryStreamHandler::~binaryStreamHandler() {
    inStream = 0;
    outStream = 0;
  }

  void binaryStreamHandler::use(std::istream& aStream) {
    inStream = &aStream;

    // alvarado: 20.10.2001
    // following line is wrong!  ios_base::binary is an "openmode" token
    // and not a format flat (set with setf)!
    // The stream given as parameters should have set this flag when
    // opened!

    // inStream->setf(std::ios_base::binary);
  }

  void binaryStreamHandler::use(std::ostream& aStream) {
    outStream = &aStream;

    // alvarado: 20.10.2001
    // following line is wrong!  ios_base::binary is an "openmode" token
    // and not a format flat (set with setf)!
    // The stream given as parameters should have set this flag when
    // opened!

    // outStream->setf(std::ios_base::binary);
  }

  // returns the name of this type
  const char* binaryStreamHandler::getTypeName() const {
    return "binaryStreamHandler";
  }

  // copy member
  binaryStreamHandler&
  binaryStreamHandler::copy(const binaryStreamHandler& other) {
    ioHandler::copy(other);
    inStream         = other.inStream;
    outStream        = other.outStream;
    bufSize = other.bufSize;
    buffer=new char[bufSize];
    return (*this);
  }

  // clone member
  ioHandler* binaryStreamHandler::clone() const {
    return new binaryStreamHandler(*this);
  }

  /*
   * the begin token or tokens
   */
  bool binaryStreamHandler::writeBegin() {
    outStream->put(char(BeginToken));
    return (outStream->good());
  }

  /*
   * the end token or tokens
   */
  bool binaryStreamHandler::writeEnd() {
    outStream->put(char(EndToken));
    return (outStream->good());
  }

  /*
   * the begin token or tokens
   */
  bool binaryStreamHandler::readBegin() {
    //cerr << "Reading begin, ";
    if (getNextToken() == BeginToken) {
      //cerr << "found!" << endl;
      return ioHandler::readBegin();
    } else {
      //cerr << "but found " << lastToken << endl;
      return false;
    }
  }

  /*
   * the end token or tokens
   */
  bool binaryStreamHandler::readEnd() {
    //cerr << "Reading end, ";
    if (getNextToken() == EndToken) {
      //cerr << "found!" << endl;
      return ioHandler::readEnd();
    } else {
      //cerr << "but found " << lastToken << endl;
      return false;
    }
  }

  /*
   * ensures that buffer contains at least newSize characters.
   */
  void binaryStreamHandler::checkBuffer(int newSize) {
    if (bufSize > newSize) {
      return;
    }
    delete[] buffer;
    buffer=new char[newSize];
    bufSize=newSize;
  }

  /*
   * write a std::string
   */
  bool binaryStreamHandler::write(const std::string& data) {
    outStream->put(StringToken);
    int32 k=data.length();
    outStream->write(reinterpret_cast<char *>(&k),sizeof(k));
    outStream->write(data.c_str(),k);
    return outStream->good();
  }


  /*
   * write a std::string
   */
  bool binaryStreamHandler::write(const char* data) {
    return write(std::string(data));
  }

  /*
   * read a std::string
   */
  bool binaryStreamHandler::readSymbol(std::string& data) {
    //cerr << "Reading symbol: ";
    if (getNextToken() == SymbolToken) {
      //cerr << lastString << "\n";
      data=lastString;
      return inStream->good();
    } else {
      //cerr << "but found: " << lastToken << endl;
      return false;
    }
  }

  /*
   * read a std::string
   */
  bool binaryStreamHandler::read(std::string& data) {
    eTokenId tkid = getNextToken();

    //cerr << "Reading string: ";
    if (tkid == StringToken || tkid == SymbolToken) {
      //cerr << lastString << "\n";
      data=lastString;
      return inStream->good();
    } else {
      //cerr << "but found: " << lastToken << endl;
      return false;
    }
  }

  /*
   * try to read a symbol
   */
  bool binaryStreamHandler::trySymbol(const std::string& data) {

    // assuming that before this symbol, there was a beginToken
    // and after this symbol there is still data that can be used...

    //cerr << "Trying symbol " << data;
    eTokenId tkid=getNextToken(true);

    if ((tkid == StringToken) || (tkid == SymbolToken)) {
      getNextToken(); // now get the token (not in try mode)
      if (lastString == data) {
        return true;
      } else {
        // there was a symbol there, but not the one we were looking for
        putBackupToken(); // put it back
      }
    }
    return false;
  }

  /*
   * try to read a the begin token
   */
  bool binaryStreamHandler::tryBegin() {
    std::string tmp;
    // try to read the BeginToken
    if (getNextToken(true) == BeginToken) {
      getNextToken();
      return ioHandler::readBegin(); // it is there... just read it as usual...
    } 
    return false;    // is not there!
  }

  /*
   * try to read a the end token
   */
  bool binaryStreamHandler::tryEnd() {
    std::string tmp;
    // try to read the End Token
    if (getNextToken(true) == EndToken) {
      getNextToken();
      return ioHandler::readEnd(); // it is there... just read it as usual...
    } 
    return false;    // is not there!    
  }


  bool binaryStreamHandler::write(const double& data) {
    outStream->write(reinterpret_cast<const char*>(&data),sizeof(data));
    return outStream->good();
  }

  bool binaryStreamHandler::write(const float& data) {
    outStream->write(reinterpret_cast<const char*>(&data),sizeof(data));
    return outStream->good();
  }

  bool binaryStreamHandler::write(const int& data) {
    outStream->write(reinterpret_cast<const char*>(&data),sizeof(data));
    return outStream->good();
  }

  bool binaryStreamHandler::write(const unsigned int& data) {
    outStream->write(reinterpret_cast<const char*>(&data),sizeof(data));
    return outStream->good();
  }

  bool binaryStreamHandler::write(const char& data) {
    outStream->put(data);
    return outStream->good();
  }

  bool binaryStreamHandler::write(const byte& data) {
    outStream->put(data);
    return outStream->good();
  }

  bool binaryStreamHandler::write(const ubyte& data) {
    outStream->put(data);
    return outStream->good();
  }

  bool binaryStreamHandler::write(const bool& data) {
    if (data) {
      return write('T');
    } else {
      return write('F');
    }
  }

  bool binaryStreamHandler::write(const long& data) {
    outStream->write(reinterpret_cast<const char*>(&data),sizeof(data));
    return outStream->good();
  }

  bool binaryStreamHandler::write(const unsigned long& data) {
    outStream->write(reinterpret_cast<const char*>(&data),sizeof(data));
    return outStream->good();
  }

  bool binaryStreamHandler::write(const short& data) {
    outStream->write(reinterpret_cast<const char*>(&data),sizeof(data));
    return outStream->good();
  }

  bool binaryStreamHandler::write(const unsigned short& data) {
    outStream->write(reinterpret_cast<const char*>(&data),sizeof(data));
    return outStream->good();
  }


  /*
   * write a std::string
   */
  bool binaryStreamHandler::writeSymbol(const std::string& data) {
    outStream->put(SymbolToken);
    int32 k=data.length();
    outStream->write(reinterpret_cast<const char*>(&k),sizeof(k));
    outStream->write(data.c_str(),k);
    return outStream->good();
  }

  /**
   * write comment writes the input data without any preprocessing,
   * just ensuring that the comment format is given
   */
  bool binaryStreamHandler::writeComment(const std::string& data) {
    return true;
  }

  /**
   * write comment writes the input data without any preprocessing,
   * just ensuring that the comment format is given
   */
  bool binaryStreamHandler::writeComment(const char* data) {
    return true;
  }

  /*
   * write white spaces
   */
  bool binaryStreamHandler::writeSpaces(const int& i) {
    return true;
  }

  bool binaryStreamHandler::writeKeyValueSeparator() {
    return true;
  }

  bool binaryStreamHandler::writeDataSeparator() {
    return true;
  }

  bool binaryStreamHandler::readKeyValueSeparator() {
    return true;
  }

  bool binaryStreamHandler::readDataSeparator() {
    return true;
  }

  /*
   * write eol
   */
  bool binaryStreamHandler::writeEOL() {
    return true;
  }

  /*
   * check for eof()
   */
  bool binaryStreamHandler::eof() {
    return inStream->eof();
 }

  /*
   * restore all the information in the handler taken in the actual
   * level.  If "complete" is true, the begin-token is also restored
   */
  bool binaryStreamHandler::restoreLevel() {
    return true;
  }

  /*
   * read a double value
   */
  bool binaryStreamHandler::read(double& data) {
    inStream->read(reinterpret_cast<char*>(&data),sizeof(data));
    //cerr << "Reading double: " << data << endl;
    return inStream->good();
  };

  /*
   * read a float value
   */
  bool binaryStreamHandler::read(float& data)  {
    inStream->read(reinterpret_cast<char*>(&data),sizeof(data));
    //cerr << "Reading float: " << data << endl;
    return inStream->good();
  };

  /*
   * read an integer value
   */
  bool binaryStreamHandler::read(int& data)  {
    inStream->read(reinterpret_cast<char*>(&data),sizeof(data));
    //cerr << "Reading int: " << data << endl;
    return inStream->good();
  };

  /*
   * read an unsigned int value
   */
  bool binaryStreamHandler::read(unsigned int& data)  {
    inStream->read(reinterpret_cast<char*>(&data),sizeof(data));
    //cerr << "Reading uint: " << data << endl;
    return inStream->good();
  };

  /*
   * read a char value
   */
  bool binaryStreamHandler::read(char& data)  {
    data=inStream->get();
    //cerr << "Reading char: " << data << endl;
    return inStream->good();
  };

  /*
   * read a char value
   */
  bool binaryStreamHandler::read(byte& data)  {
    data=inStream->get();
    //cerr << "Reading char: " << data << endl;
    return inStream->good();
  };

  /*
   * read an ubyte value
   */
  bool binaryStreamHandler::read(ubyte& data)  {
    data=inStream->get();
    //cerr << "Reading uchar: " << data << endl;
    return inStream->good();
  };

  /*
   * read a boolean value
   */
  bool binaryStreamHandler::read(bool& data) {
    char tmp=inStream->get();
    if (tmp == 'T' || tmp == 'F') {
      data=(tmp == 'T');
      //cerr << "Reading bool: " << data << endl;
      return inStream->good();
    } else {
      return false;
    }
  }

  /*
   * read a long value
   */
  bool binaryStreamHandler::read(long& data)  {
    inStream->read(reinterpret_cast<char*>(&data),sizeof(data));
    //cerr << "Reading long: " << data << endl;
    return inStream->good();
  };

  /*
   * read an unsigned long value
   */
  bool binaryStreamHandler::read(unsigned long& data)  {
    inStream->read(reinterpret_cast<char*>(&data),sizeof(data));
    //cerr << "Reading ulong: " << data << endl;
    return inStream->good();
  };

  /*
   * read a long value
   */
  bool binaryStreamHandler::read(short& data)  {
    inStream->read(reinterpret_cast<char*>(&data),sizeof(data));
    //cerr << "Reading short: " << data << endl;
    return inStream->good();
  };

  /*
   * read an unsigned long value
   */
  bool binaryStreamHandler::read(unsigned short& data)  {
    inStream->read(reinterpret_cast<char*>(&data),sizeof(data));
    //cerr << "Reading ushort: " << data << endl;
    return inStream->good();
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
  binaryStreamHandler::eTokenId
  binaryStreamHandler::getNextToken(const bool justTry) {
    if (justTry) {
      if (!backupTokens.empty()) {
        // we have a backup token hanging around
        return backupTokens.top();
      } else {
        // since we are just trying, do not remove anything
        // BUG: found by alvarado at 2004-02-03
        // It is easy to see that if any data type that follows has one of 
        // the token values then a token will be incorrectly detected.
        // This bug requires a completely new format for the binary stream
        // handlers, and will be done for the release 2.0 of the library.
        // At this point it will be kept this way since in most cases followed
        // works, and there are many data around saved with this format.
        return eTokenId(inStream->peek());        
      }
    } else {
      if (!backupTokens.empty()) {
        // we have a backup token hanging around
        lastToken=backupTokens.top();
        backupTokens.pop();
        if (lastToken == SymbolToken || lastToken == StringToken) {
          const std::string* tmp=backupStrings.top();
          backupStrings.pop();
          lastString=*tmp;
          delete tmp;
        }
      } else {
        lastToken=eTokenId(inStream->get());
        switch (lastToken) {
          case BeginToken: break;
          case EndToken: break;
          case SymbolToken: case StringToken:
            int32 k;
            inStream->read(reinterpret_cast<char*>(&k),sizeof(k));
            checkBuffer(k+1);
            inStream->read(buffer,k);
            buffer[k]='\0';
            lastString=buffer;
            break;
          default:
            lastToken=ErrorToken;
            break;
        }
      }
    }
    //cerr << "Returning token: " << lastToken << "\n";
    return lastToken;
  }

  void binaryStreamHandler::putBackupToken() {
    backupTokens.push(lastToken);
    if (lastToken == SymbolToken || lastToken == StringToken) {
      backupStrings.push(new std::string(lastString));
    }
  }

}
