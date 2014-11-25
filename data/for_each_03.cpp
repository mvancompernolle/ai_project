/**
 *  @example rubber/example/for_each_03.cpp
 *  This example shows the usage of the type erasures for compile-time
 *  Mirror's meta-object with the for-each algorithm and lambda functions
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <rubber/rubber.hpp>
#include <iostream>

int main(void)
{
	auto gs = rubber::erase_type<MIRRORED_GLOBAL_SCOPE()>();
	gs.members().for_each(
		[](const rubber::meta_named_scoped_object& member)
		{
			// print the full name of the reflected object
			std::cout <<
				member.self().construct_name() <<
				" " <<
				member.base_name() <<
				std::endl;
		}
	);
	return 0;
}

/* Example of output:
namespace std
namespace boost
namespace mirror
type void
type bool
type char
type unsigned char
type wchar_t
type short int
type int
type long int
type unsigned short int
type unsigned int
type unsigned long int
type float
type double
type long double
*/

