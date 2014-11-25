
//  Copyright (C) 1998--2009  Petter Urkedal
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


#include "directory.h"
#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>

namespace more {
namespace io {

    void
    directory::iterator::getnext()
    {
	if (dir) {
	    dirent *ent = readdir((DIR*)dir);
	    if (ent)
		entry = std::string(ent->d_name);
	    else {
		if (ic.is_unique())
		    closedir((DIR*)dir);
		dir = 0;
	    }
	}
    }

    directory::iterator::iterator(std::string name)
	: dir(opendir(name.c_str()))
    {
	getnext();
    }

    directory::iterator::~iterator()
    {
	if (dir != 0 && ic.is_unique()) closedir((DIR*)dir);
    }
}
}
