//  Copyright (C) 2001--2009  Petter Urkedal
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  As a special exception, you may use this file as part of a free
//  software library without restriction.  Specifically, if other files
//  instantiate templates or use macros or inline functions from this
//  file, or you compile this file and link it with other files to
//  produce an executable, this file does not by itself cause the
//  resulting executable to be covered by the GNU General Public
//  License.  This exception does not however invalidate any other
//  reasons why the executable file might be covered by the GNU General
//  Public License.


#include <more/sys/date.h>
#include <more/io/syncstream.h>
#include <sstream>

namespace more {
namespace sys {

  date::date(time_t t0, precision_type prec, timezone_type tz)
      : m_tz(tz), m_prec(prec)
  {
      std::time_t t = static_cast<std::time_t>(t0) + tz;
      m_tm = *gmtime(&t);
  }

  date::date(time_t t0, precision_type prec_)
      : m_prec(prec_)
  {
      std::time_t t = static_cast<std::time_t>(t0);
      m_tm = *localtime(&t);
      m_tz = ::timezone;
  }

  date::date(int year, int month, int day,
	     int h, int min, int s)
  {
      if (s == -1) {
	  s = 0;
	  if (min == -1) {
	      min = 0;
	      if (h == -1) {
		  h = 0;
		  if (day == -1) {
		      day = 1;
		      if (month == -1) {
			  month = 1;
			  m_prec = within_1_year;
		      }
		      else
			  m_prec = within_1_month;
		  }
		  else
		      m_prec = within_1_day;
	      }
	      else
		  m_prec = within_1_h;
	  }
	  else
	      m_prec = within_1_min;
      }
      else
	  m_prec = within_1_s;

      m_tm.tm_year = year - 1900;
      m_tm.tm_mon = month - 1;
      m_tm.tm_mday = day;
      m_tm.tm_hour = h;
      m_tm.tm_min = min;
      m_tm.tm_sec = s;

      mktime(&m_tm);
      ::time_t t_ignore;
      localtime(&t_ignore); // sets ::timezone
      m_tz = ::timezone;

#ifndef NDEBUG
      if (month < 1 || month > 12)
	  throw std::out_of_range("more::date::date: "
				  "Month is out of range.");
      if (day < 1 || day > 31)
	  throw std::out_of_range("more::date::date: "
				  "Day is out of range.");
      if (h < 0 || h > 23)
	  throw std::out_of_range("more::date::date: "
				  "Hour is out of range.");
      if (min < 0 || min >= 60)
	  throw std::out_of_range("more::date::date: "
				  "Minutes is out of range.");
      if (s < 0 || s > 62)
	  throw std::out_of_range("more::date::date: "
				  "Seconds is out of range.");
#endif
  }

  void
  date::sync(more::io::syncstream& sync)
  {
      if (sync.is_output()) {
	  double t = static_cast<double>(*this);
	  int p = static_cast<int>(m_prec);
	  sync | p | m_tz | t;
      }
      if (sync.is_input()) {
	  int p;
	  long z;
	  double t;
	  sync | p | z | t;
	  *this = date(t, static_cast<precision_type>(p), z);
      }
  }

  template<typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, date const& d)
    {
	typename std::basic_ostream<CharT, Traits>::sentry cerberus(os);
	if (cerberus) {
#if 1 // XXX remove when library supports time_put
	    char buf[80];
	    if (d.m_prec == date::undefined)
		os << "#[unknown date]";
	    else if (d.m_prec == date::within_1_year) {
		strftime(buf, 80, "%Y", &d.m_tm);
		os << buf;
	    }
	    else if (d.m_prec == date::within_1_month) {
		strftime(buf, 80, "%Y-%m", &d.m_tm);
		os << buf;
	    }
	    else {
		strftime(buf, 80, "%Y-%m-%d", &d.m_tm);
		os << buf;
		if (d.m_prec != date::within_1_day) {
		    // XXX there should be a 'T' before the time
		    // string, but this makes the date rather
		    // unreadable. Implement IO manipulators to select
		    // this separator or '/' as alternatives to ' ',
		    // and to select compressed date (without '-').
		    if (d.m_prec == date::within_1_s)
			strftime(buf, 80, " %H:%M:%S", &d.m_tm);
		    else
			strftime(buf, 80, " %H:%M", &d.m_tm);
		    os << buf << '+';
		    char savefill = os.fill();
		    os.fill('0');
		    os.width(2);
		    os << (-d.m_tz / 3600);
		    os.width(2);
		    os << (-d.m_tz / 60 % 60);
		    os.fill(savefill);
		}
	    }
#elif 0
	    std::use_facet
		< std::time_put
		    < CharT, std::ostreambuf_iterator<CharT, Traits> > >
		(os.getloc()).put(os, os, os.fill(), d.c_tm(), 'c');
#endif
// 	    os.setstate(err);
	}
	return os;
    }

  template<typename CharT, typename Traits>
    std::basic_istream<CharT, Traits>&
    operator>>(std::basic_istream<CharT, Traits>& is, date& d)
    {
#if 1
	if (is.peek() == '#') {
	    static char const* str_unknown_date = "#[unknown date]";
	    for (int i = 0; str_unknown_date[i]; ++i) {
		if (is.get() != str_unknown_date[i]) {
		    is.setstate(std::ios_base::failbit);
		    return is;
		}
	    }
	    d = date();
	    return is;
	}
	char c;
	date::precision_type prec;
	date::timezone_type tz = 0;
	int year, month = 1, day = 1, h = 0, min = 0, s = 0;
	prec = date::within_1_day;

	is >> year;
	if (is.fail())
	    return is;

	c = is.get();
	if (c != '-') {
	    if (!is.fail())
		is.putback(c);
	    else if (is.eof())
		is.clear();
	    prec = date::within_1_year;
	    goto ok;
	}

	is >> month;
	if (is.fail())
	    return is;

	c = is.get();
	if (c != '-') {
	    if (!is.fail())
		is.putback(c);
	    else if (is.eof())
		is.clear();
	    prec = date::within_1_month;
	    goto ok;
	}

	is >> day;
	if (is.fail())
	    return is;

	c = is.get();
	if (is.fail()) {
	    if (is.eof()) {
		is.clear();
		prec = date::within_1_day;
		goto ok;
	    }
	    else
		return is;
	}
	if (std::isspace(c, is.getloc())) {
	    do c = is.get(); while (std::isspace(c, is.getloc()));
	    is.putback(c);
	    if (!std::isdigit(c))
		goto ok;
	}
	else if (c != 'T' && c != 't' && c != '/') {
	    is.putback(c);
	    goto ok;
	}
	is >> h;
	if (is.fail())
	    return is;
	if ((c = is.get()) != ':') {
	    is.putback(c);
	    goto fail;
	}
	is >> min;
	if (is.fail())
	    return is;
	if ((c = is.get()) != ':') {
	    if (is.eof())
		is.clear();
	    else
		is.putback(c);
	    prec = date::within_1_min;
	}
	else {
	    is >> s;
	    if (is.fail())
		goto fail;
	    prec = date::within_1_s;
	}
	if ((c = is.get()) == '+') {
	    static const int mult[4] = { 600, 60, 10, 1 };
	    tz = 0;
	    for (int i = 0; i < 4; ++i) {
		c = is.get();
		if (!std::isdigit(c))
		    goto fail;
		tz -= (c - '0')*mult[i];
	    }
	    tz *= 60;
	}
	else if (c == 'Z')
	    tz = 0;
	else {
	    ::time_t t_ignore = 0;
	    if (is.eof())
		is.clear();
	    else
		is.putback(c);
	    localtime(&t_ignore); // sets ::timezone
	    tz = ::timezone;
	}
      ok:
	d.m_tm.tm_year = year - 1900;
	d.m_tm.tm_mon = month - 1;
	d.m_tm.tm_mday = day;
	d.m_tm.tm_hour = h;
	d.m_tm.tm_min = min;
	d.m_tm.tm_sec = s;
	d.m_prec = prec;
	d.m_tz = tz;
	std::mktime(&d.m_tm);
	is.clear();
	return is;
      fail:
	return is.setstate(std::ios_base::failbit), is;
#else
	typename std::basic_istream<CharT, Traits>::sentry Cerberus(is);
	if (Cerberus) {
	    std::ios_base::iostate err;
	    std::use_facet
		< std::time_get
		    < CharT, std::istreambuf_iterator<CharT, Traits> > >
		(is.getloc()).get_time(is, is, is, err, &d.m_tm);
	    is.setstate(err);
	}
	return is;
#endif
    }

  template
    std::basic_ostream< char, std::char_traits<char> >&
    operator<<(std::basic_ostream< char, std::char_traits<char> >&,
	       date const&);

  template
    std::basic_istream< char, std::char_traits<char> >&
    operator>>(std::basic_istream< char, std::char_traits<char> >&, date&);

}} // more::sys
