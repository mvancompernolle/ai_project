/*
 * Copyright (C) 2003, 2004, 2005, 2006
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
 * file .......: ltiColors.cpp
 * authors ....: Jochen Wickel
 * organization: LTI, RWTH Aachen
 * creation ...: 12.02.03
 * revisions ..: $Id: ltiColors.cpp,v 1.11 2009/06/10 15:32:56 alvarado Exp $
 */

#include "ltiMacroSymbols.h"
#ifdef _LTI_MSC_6
// a weird warning from MSVC++ is a false alarm!
#pragma warning(disable:4660)
#endif

#include "ltiColors.h"
#include <cstdio>
#include <cstring>
#include <fstream>

/// namespace lti
namespace lti {

  bool rgbColor::parseHTML(const std::string& s, bool acceptAlpha) {
    int n=s.length();
    // reject definitions with the wrong length
    if (n != 7 && !(acceptAlpha && n == 9)) {
      return false;
    }
    const char *cname=s.c_str();
    if (*cname == '#') {
      unsigned int r,g,b,a=0;
      cname++;
      char cdef[3];
      cdef[2]='\0';
      strncpy(cdef,cname,2);
      sscanf(cdef,"%x",&r);
      strncpy(cdef,cname+2,2);
      sscanf(cdef,"%x",&g);
      strncpy(cdef,cname+4,2);
      sscanf(cdef,"%x",&b);
      if (acceptAlpha && n > 7) {
        // we have an alpha channel
        strncpy(cdef,cname+6,2);
        sscanf(cdef,"%x",&a);
      }
      setRed(r);
      setGreen(g);
      setBlue(b);
      setDummy(a);
      return true;
    } else {
      return false;
    }
  }

  std::string rgbColor::makeHTML(bool getAlpha) const {
    char buf[16];

    if (getAlpha) {
      sprintf(buf,"#%02x%02x%02x%02x",
                   getRed(),getGreen(),getBlue(),getDummy());
    } else {
      sprintf(buf,"#%02x%02x%02x",getRed(),getGreen(),getBlue());
    }
    return std::string(buf);
  }

  bool rgbColor::parseName(const std::string& s) {
#if defined(_LTI_LINUX) || defined(_LTI_CYGWIN) || defined(_LTI_MACOSX)
    std::ifstream rgbDefs("/usr/X11R6/lib/X11/rgb.txt");

    bool notFound=true;
    int r,g,b,a=0;

    while (!rgbDefs.eof() && notFound) {
      std::string tmp;
      std::getline(rgbDefs,tmp);
      const char* t=tmp.c_str();
      if (*t == '!') {
        // comment line
        continue;
      }
      const char* cdef=t;
      // search the beginning of the color name
      while (isspace(*t) || isdigit(*t)) t++;
      if (s == t) {
        sscanf(cdef,"%d %d %d",&r,&g,&b);
        notFound=false;
        setRed(r);
        setGreen(g);
        setBlue(b);
        setAlpha(a);
      }
    }
    return !notFound;
#endif
#ifdef _LTI_WIN32
    return false;
#endif
  }

}
