/**
 *  @example lagoon/example/std_range_01.cpp
 *  This example shows the usage of the namespace reflection functions
 *  and std range adaptor together with algorithms from the standard library.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/range/std_range.hpp>

#include <iostream>
#include <vector>
#include <algorithm>

int main(void)
{
	using namespace lagoon;
	// an adapted std range of meta objects
	auto mosr = make_std_range(reflected_global_scope()->members());
	typedef shared<meta_named_scoped_object> shared_mnso;
	// a vector of meta-objects
	std::vector<shared_mnso> mov;
	// copy the items from the range to the vector
	// using the standard copy algorithm
	std::copy(
		mosr.begin(),
		mosr.end(),
		std::insert_iterator<
			std::vector<shared_mnso>
		>(mov, mov.begin())
	);
	// sort the vector by base name of the meta-object
	// using the standard sort algorithm
	std::sort(
		mov.begin(),
		mov.end(),
		[](const shared_mnso& a, const shared_mnso& b)
		{
			return a->base_name() < b->base_name();
		}
	);
	// print the base names of the meta-objects
	// using the standard for_each algorithm and
	// a lambda functor
	std::for_each(
		mov.begin(),
		mov.end(),
		[](const shared_mnso& mo)
		{
			std::cout << mo->base_name() << std::endl;
		}
	);
	return 0;
}

/* Example of output:
bool
boost
char
double
float
int
long double
long int
short int
std
unsigned char
unsigned int
unsigned long int
unsigned short int
void
wchar_t
*/
