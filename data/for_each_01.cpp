/**
 *  @example rubber/example/for_each_01.cpp
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
		mirror::members<
			MIRRORED_GLOBAL_SCOPE()
		>
	>(
		[](const rubber::meta_named_scoped_object& member)
		{
			std::cout << member.full_name() << std::endl;
		}
	);
	return 0;
}

/* Example of output:
std
boost
void
bool
char
unsigned char
wchar_t
short int
int
long int
unsigned short int
unsigned int
unsigned long int
float
double
long double
*/

