/*
 *  Copyright (C) 2005 Krzysztof Rzymkowski <rzymek@users.sourceforge.net>
 *  Copyright (C) 2006 Ilya Volynets <ilya@total-knowledge.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "dso_code.h"
#include <servlet/IOError.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef DEBUG
#include <iostream> //dbg
#endif

using namespace std;

dso_code::dso_code(const string& a_filename) : filename(a_filename)
{
	load();
	try {
		mtime = file_mtime();
	} catch (...) {
		unload();
		throw;
	}
}

time_t dso_code::file_mtime()
{
	return 0;
/*	struct stat finfo;
	int status = stat(filename.c_str(),&finfo);
	if (status == -1)
		throw servlet::IOError(filename + ": Cannot stat the file");
	return finfo.st_mtime;*/
}

void dso_code::load()
{
	handle = ::dlopen(filename.c_str(), RTLD_LAZY|RTLD_GLOBAL);
	if (!handle) {
		throw std::runtime_error(filename+": Unable to load the library: "+dlerror());
	}

}
void dso_code::unload()
{
	if (dlclose(handle) != 0)
		throw std::runtime_error(filename+": unable to dlclose: "+dlerror());
}

bool dso_code::reload_if_needed(time_t mtime)
{
	if (this->mtime != mtime) {
		unload();
		load();
		this->mtime = mtime;
		return true;
	}
	return false;
}

bool dso_code::reload_if_needed()
{
	time_t mtime = file_mtime();
	return reload_if_needed(mtime);
}

void* dso_code::call_(const std::string& name)
{
	typedef void*(*dso_func_t)();
	dso_func_t sym = (dso_func_t)dlsym(handle, name.c_str());
	if (!sym)
		throw runtime_error(filename+": "+name+": Error getting the symbol: "+dlerror());
	return sym();
}

void* dso_code::func_(const std::string& name)
{
	void* sym = dlsym(handle, name.c_str());
	if (!sym)
		throw runtime_error(filename+": "+name+": Error getting the symbol: "+dlerror());
	return sym;
}

