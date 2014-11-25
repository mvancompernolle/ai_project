/**
 *  @example rubber/example/for_each_04.cpp
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
	mirror::mp::for_each<
		mirror::mp::only_if<
			mirror::members<
				MIRRORED_GLOBAL_SCOPE()
			>,
			mirror::mp::is_a<
				mirror::mp::arg<1>,
				mirror::meta_type_tag
			>
		>
	>(
		[](const rubber::meta_type& type)
		{
			std::cout <<
				type.base_name() <<
				", size = " <<
				type.sizeof_() <<
				" B" <<
				std::endl;
		}
	);
	return 0;
}

/* Example of output:
void, size = 0 B
bool, size = 1 B
char, size = 1 B
unsigned char, size = 1 B
wchar_t, size = 4 B
short int, size = 2 B
int, size = 4 B
long int, size = 8 B
unsigned short int, size = 2 B
unsigned int, size = 4 B
unsigned long int, size = 8 B
float, size = 4 B
double, size = 8 B
long double, size = 16 B
*/

