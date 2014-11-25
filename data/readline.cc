
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


#include <more/io/readline.h>

#ifdef MORE_MORE_CONF_HAVE_READLINE

#include <stdio.h> // readline needs it

#undef __P // XXX bug in lib.

#ifdef MORE_CONF_HAVE_READLINE_READLINE_H
#  include <readline/readline.h>
#elif defined(MORE_CONF_HAVE_READLINE_H)
#  include <readline.h>
#endif
#ifdef MORE_CONF_HAVE_READLINE_HISTORY_H
#  include <readline/history.h>
#elif defined(MORE_CONF_HAVE_HISTORY_H)
#  include <history.h>
#endif

namespace more {
namespace io {

  char*
  readline(char* prompt)
  {
      return ::readline(prompt);
  }

  void
  add_history(char* ln)
  {
      ::add_history(ln);
  }
}
}

#else

#include <iostream>

namespace more {
namespace io {

  char*
  readline(char* prompt)
  {
      static const unsigned int n = 2048;
      static char ln[n];
      std::cout << prompt;
      std::cin.getline(ln, n);
      return ln;
  }

  void
  add_history(char* ln) {}

}
}

#endif
