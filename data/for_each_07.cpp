/**
 *  @example lagoon/example/for_each_07.cpp
 *  This example shows the usage of the namespace reflection functions
 *  and range for-each utility with lambda functions and the transform
 *  only_if and sort adapters
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/range/extract.hpp>
#include <lagoon/range/sort.hpp>
#include <lagoon/range/for_each.hpp>
#include <iostream>

int main(void)
{
	using namespace lagoon;
	typedef shared<meta_named_scoped_object> shared_mnso;
	typedef shared<meta_type> shared_mt;
	//
	// get the members of the global scope and extract only
	// those having the meta_type interface, sort the resulting
	// range by the binary functor (the first lambda function,
	// comparing the sizes of the types) and then for each
	// if them use the second lambda functor to print their full names
	for_each(
		sort(
			extract<meta_type>(
				reflected_global_scope()->members()
			),
			[](const shared_mt& a, const shared_mt& b)
			{
				return a->size_of() < b->size_of();
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
short int
unsigned short int
wchar_t
int
long int
unsigned int
unsigned long int
float
double
long double
*/
