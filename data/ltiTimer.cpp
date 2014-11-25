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
 * file .......: ltiTimer.cpp
 * authors ....: Pablo Alvarado
 * organization: LTI, RWTH Aachen
 * creation ...: 07.04.99
 * revisions ..: $Id: ltiTimer.cpp,v 1.7 2008/08/17 22:20:13 alvarado Exp $
 */

#include "ltiTimer.h"
#include <iostream>

#ifndef _LTI_WIN32
#  include <sys/time.h>
#  include <unistd.h>
#  include <cstring>
#else
#  include <windows.h>
#endif

namespace lti {

  // replacement for Sleep/usleep
  void passiveWait(const int& usTime) {
#   ifdef _LTI_WIN32
    Sleep(usTime/1000);
#   else
    usleep(long(usTime));
#   endif
  }

#ifdef  _LTI_WIN32
  const double timer::max32bit = 4294967296.0; // 2^32;
#endif

  timer::timer() {
    started = false;
    startTime = 0.0;
    endTime = 0.0;

#   ifdef _LTI_WIN32
    LARGE_INTEGER tmp;
    QueryPerformanceFrequency(&tmp);
    freq = double(tmp.LowPart) + max32bit*double(tmp.HighPart);
#   endif

  }

  void timer::start() {
    started = true;
    startTime = getActualTime();
  }

  void timer::stop() {
    if (started) {
      endTime = getActualTime();
      started = false;
    } else {
      endTime = startTime;
    }
  }

  double timer::getTime() const {
    double t=0.0;
    if (!started) {
      t = endTime-startTime;
    } else {
      t = getActualTime() - startTime;
    }

    // started before midnight?
    if (t<=-1) {
      t += (60.0*60.0*24.0*1000000.0);
    }

    return (t);
  }

  double timer::getActualTime() const {
    double t=0.0;
#ifndef _LTI_WIN32
    timeval tv;

    if (gettimeofday(&tv,0) == 0) {
      t = double(tv.tv_sec)*1000000.0 + double(tv.tv_usec);
    }

#else
    // MS VC++ Implementation
    LARGE_INTEGER counter;
    if (QueryPerformanceCounter(&counter) != 0) {
      t = double(counter.LowPart) +
          max32bit*double(counter.HighPart);
      t/=freq; // seconds
      t*=1000000.0; // microseconds
    }
#endif
    return t;
  }

  const char* timer::getTypeName() const {
    return "timer";
  }

  std::string timer::getDateAndTime() {
#ifndef _LTI_WIN32
    time_t t=time(&t);
    char buf[256];
    strcpy(buf,ctime(&t));
    char *tmp=buf;
    while (*tmp != '\n' && *tmp != 0) {
      tmp++;
    }
    if (*tmp == '\n') {
      *tmp='\0';
    }
    return buf;
#else

  std::string buf,sTemp;
  char s[33];
  SYSTEMTIME sysTime;
  GetLocalTime(&sysTime);

  switch(sysTime.wDayOfWeek){
    case 0: sTemp="Sun";break;
    case 1: sTemp="Mon";break;
    case 2: sTemp="Tue";break;
    case 3: sTemp="Wed";break;
    case 4: sTemp="Thu";break;
    case 5: sTemp="Fri";break;
    case 6: sTemp="Sat";break;
    default: sTemp="error";
  }
  buf.append(sTemp);
  buf.append(" ");

  switch(sysTime.wMonth){
    case 1: sTemp="Jan";break;
    case 2: sTemp="Feb";break;
    case 3: sTemp="Mar";break;
    case 4: sTemp="Apr";break;
    case 5: sTemp="May";break;
    case 6: sTemp="Jun";break;
    case 7: sTemp="Jul";break;
    case 8: sTemp="Aug";break;
    case 9: sTemp="Sep";break;
    case 10: sTemp="Oct";break;
    case 11: sTemp="Nov";break;
    case 12: sTemp="Dez";break;
    default: sTemp="error";
  }
  buf.append(sTemp);
  buf.append(" ");
  sprintf(s,"%i",sysTime.wDay);
  buf.append(s);
  buf.append(" ");
  sprintf(s,"%i",sysTime.wHour);
  buf.append(s);
  buf.append(":");
  sprintf(s,"%i",sysTime.wMinute);
  buf.append(s);
  buf.append(":");
  sprintf(s,"%i",sysTime.wSecond);
  buf.append(s);
  buf.append(" ");
  sprintf(s,"%i",sysTime.wYear);
  buf.append(s);
  return buf;

#endif
  }

}
