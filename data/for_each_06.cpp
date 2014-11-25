/**
 *  @example lagoon/example/for_each_06.cpp
 *  This example shows the usage of the namespace reflection functions
 *  and range for-each utility with lambda functions and the transform
 *  and only_if adapters
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/range/for_each.hpp>
#include <lagoon/range/transform.hpp>
#include <lagoon/range/only_if.hpp>
#include <iostream>

int main(void)
{
	using namespace lagoon;
	// get the members of the global scope and filter only those
	// that have the meta_type interface and cast the individual
	// members of the resulting range to meta_type
	// and then print the base name of the type and its size
	for_each(
		transform(
			only_if(
				reflected_global_scope()->members(),
				[](shared<meta_named_scoped_object> mo)
				{
					return mo.is<meta_type>();
				}
			),
			[](shared<meta_named_scoped_object> mo)
			{
				return mo.as<meta_type>();
			}
		),
		[](shared<meta_type> mt)
		{
			std::cout
				<< mt->base_name()
				<< ": "
				<< mt->size_of()
				<< std::endl;
		}
	);
	return 0;
}

/* Example of output:
void: 0
bool: 1
char: 1
unsigned char: 1
wchar_t: 4
short int: 2
int: 4
long int: 4
unsigned short int: 2
unsigned int: 4
unsigned long int: 4
float: 4
double: 8
long double: 12
*/
