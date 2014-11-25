/**
 *  @example lagoon/example/for_each_08.cpp
 *  This example shows the usage of the namespace reflection functions
 *  and range for-each utility with lambda functions and the only_if
 *  and until adapters
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/range/extract.hpp>
#include <lagoon/range/until.hpp>
#include <lagoon/range/for_each.hpp>
#include <iostream>

int main(void)
{
	using namespace lagoon;
	typedef shared<meta_named_scoped_object> shared_mnso;
	typedef shared<meta_type> shared_mt;
	// Get the members of the global scope, start extracting only
	// meta_types and stop when a type with the size greater than
	// 4 bytes is found and for each of them use the second
	// lambda function to print their full names
	for_each(
		until(
			extract<meta_type>(
				reflected_global_scope()->members()
			),
			[](const shared_mt& a)
			{
				return a->size_of() > 4;
			}
		),
		[](const shared_mt& member)
		{
			std::cout << member->full_name() << std::endl;
		}
	);
	return 0;
}

/* Example of output:
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
*/
