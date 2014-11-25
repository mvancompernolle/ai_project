/*
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
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


/*----------------------------------------------------------------
 * project ....: LTI Digital Image/Signal Processing Library
 * file .......: ltiClassName.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 01.04.99
 * revisions ..: $Id: ltiClassName.cpp,v 1.11 2008/08/17 22:19:06 alvarado Exp $
 */

#include <typeinfo>
#include <iostream>
#include <cctype>
#include <cstdlib> // for free
#include <cstring> // for strcpy
#include "ltiClassName.h"

#ifdef _LTI_GNUC_3
#include <cxxabi.h>
#endif

namespace lti {

  className::className() {
    buffer=0;
  }

  className::~className() {
    if (buffer != 0) {

#ifdef _LTI_GNUC_3
      // the in cxxabi defined interface indicates the use of free
      // instead of delete. (Thanks Valgrind for the tip!)
      free(buffer);
      buffer=0;
#else
      delete[] buffer;
#endif

    }

  }


#ifdef _LTI_GNUC_2

  // implementation for gcc 2.x.y

  // in gcc, classes are encoded by strange names:
  // 'Q' introduces a class name which includes a double colon '::'; the number of
  // parts follows immediately after that, after that the parts themselves follow
  // (Q2xxyy)
  // A class name is always prefixed by its length in characters (3xxx)
  // 't' denotes a template class; after the class description, the
  // number of template arguments follows. After that, the template arguments
  // themselves are found, each one introduced by 'Z', if they are classes,
  // of the primitive type key, if they are value templates (currently only 'bool'
  // is supported)
  // primitive types are encoded by one lowercase character, maybe prefixed by
  // a capital 'U' for 'unsigned' types.
  void className::decode(char* dest, int& dpos, const char* src, int& spos) const {
    // check if we have a composed name or a template or something else
    switch (src[spos]) {
      case 'Q': {
        spos++;
        int parts=src[spos++]-'0';
        for (int i=0; i<parts; i++) {
          decode(dest,dpos,src,spos);
          if (i<parts-1) {
            dest[dpos++]=':';
            dest[dpos++]=':';
          }
        }
        break;
      }
      case 't': {
        spos++;
        decode(dest,dpos,src,spos);
        dest[dpos++]='<';
        int ntemps=src[spos++]-'0';
        for (int i=0; i<ntemps; i++) {
          switch(src[spos++]){
            case 'Z': {
              decode(dest,dpos,src,spos);
              break;
            }
            case 'b': {
              if(src[spos++] == '1'){
                strcpy(dest+dpos,"true");
                dpos += 4;
              }
              else {
                strcpy(dest+dpos,"false");
                dpos += 5;
              }
              break;
            }
              // TODO: add support for other primitive template arguments.
            default: {
              cerr << "Fatal error: Illegal internal class name: " << src[spos] << "\n";
            }
          }
          if (i<ntemps-1) {
            dest[dpos++]=',';
          }
        }
        dest[dpos++]='>';
        break;
      }
      case 'd':
        strcpy(dest+dpos,"double");
        dpos+=6;
        spos++;
        break;
      case 'i':
        strcpy(dest+dpos,"int");
        dpos+=3;
        spos++;
        break;
      case 'f':
        strcpy(dest+dpos,"float");
        dpos+=5;
        spos++;
        break;
      case 'b':
        strcpy(dest+dpos,"bool");
        dpos+=4;
        spos++;
        break;
      case 'l':
        strcpy(dest+dpos,"long");
        dpos+=4;
        spos++;
        break;
      case 'c':
        strcpy(dest+dpos,"char");
        spos++;
        dpos+=4;
        break;
      case 'U':
        strcpy(dest+dpos,"unsigned ");
        dpos+=9;
        spos++;
        decode(dest,dpos,src,spos);
        break;
      default: {
        int len=0;
        while (isdigit(src[spos])) {
          len=len*10+src[spos++]-'0';
        }
        for (int i=0; i<len; i++) {
          dest[dpos++]=src[spos++];
        }
        break;
        }
    }
    dest[dpos]=0;
  }

#elif defined(_LTI_MSC_VER)

  // implementation for VisualC x.y
  // the encoding is straightforward: each class name is prepended with "class "
  void className::decode(char* dest, int& dpos, const char* src, int& spos) const {
    char *dptr=dest+dpos;
    const char *sptr=src+spos;
    const char *r=strstr(sptr,"class ");
    if (r != 0) {
      int s=r-sptr;
      strncpy(dptr,sptr,s);
      dpos+=s;
      spos+=s+6;
      decode(dest,dpos,src,spos);
    } else {
      strcpy(dptr,sptr);
    }
  }

#else

  // this is the default implementation
  void className::decode(char* dest, int& dpos, const char* src, int& spos) const {
    strcpy(dest,src);
  }
#endif

  const char* className::get(const object* o) const {
#ifdef _LTI_GNUC_3
    if (buffer != 0) {
      free(buffer); // need to be "free" due to interface definition
                    // of the demangle function
      buffer=0;
    }

    const char* tmp=typeid(*o).name();
    __SIZE_TYPE__ size;
    int error;
    buffer=::abi::__cxa_demangle(tmp,0,&size,&error);
    if (error == 0) {
      return buffer;
    } else {
      return tmp;
    }
#else
    if (buffer != 0) {
      delete[] buffer;
    }
    const char* tmp=typeid(*o).name();
    buffer=new char[2*strlen(tmp)];
    int spos=0;
    int dpos=0;
    decode(buffer,dpos,tmp,spos);
    return buffer;
#endif
  }

  // This is a rather quick and dirty hack
  std::string className::decode(const std::string& name) const {
#ifdef _LTI_GNUC_3
    if (buffer != 0) {
      free(buffer); // need to be "free" due to interface definition
                    // of the demangle function
      buffer=0;
    }

    __SIZE_TYPE__ size;
    int error;
    buffer=::abi::__cxa_demangle(name.c_str(),0,&size,&error);
    if (error == 0) {
      return buffer;
    } else {
      return name;
    }
#else
    if (buffer != 0) {
      delete[] buffer;
    }
    buffer=new char[2*name.size()];
    int spos=0;
    int dpos=0;
    decode(buffer,dpos,name.c_str(),spos);
    return buffer;
#endif
  }
  
  
}
