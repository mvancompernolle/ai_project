/*
cpplint - the C++ lint
Copyright (C) 2005 Daniel S. Day

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Please see the file LICENSE, which should have been distributed with the package,
to see the terms and conditions under the GNU General Public License.
*/

#include "CForward.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

extern std::ifstream fin;
std::vector<std::string> variables;
char last_char_read;

extern "C" void report_error( const char *str )
{
	extern int on_line;
	std::cerr << on_line << " " << str << std::endl;
}

extern "C" char get_next_char( void )
{
	/*char c;
	fin >> c;
	return c;*/
	int c = fin.get();
	last_char_read = static_cast<char>(c);
	return static_cast<char>(c);
}

extern "C" int that_was_eof( void )
{
	return fin.eof() ? 1 : 0;
}

extern "C" void register_variable( const char *var )
{
	//std::cerr << var << std::endl;
	variables.push_back(std::string(var));
}

extern "C" void register_function( const char *var )
{
}

