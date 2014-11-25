//  Copyright (C) 2002--2009  Petter Urkedal
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


#include "ios_storage.h"

namespace more {
namespace io {

  namespace bits {

    ios_storage::ios_storage()
	: tslang(tslang_none),
	  style(0),
	  origin_indicator_prefix("("),
	  origin_indicator_suffix(")"),
	  origin_indicator_separator(","),
	  origin_experiment_indicator("ex"),
	  origin_calculation_indicator("ca"),
	  origin_systematics_indicator("sy"),
	  origin_other_indicator("oth"),
	  origin_unknown_indicator("un") {}

    ios_storage*
    storage(std::ios_base& iosb)
    {
	static int i = -1;
	if (i == -1)
	    i = std::ios_base::xalloc();
	void*& ptr = iosb.pword(i);
	if (ptr == 0)
	    ptr = new ios_storage();
	return static_cast<ios_storage*>(ptr);
    }

  }

}}
