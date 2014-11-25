/**
 *  @example lagoon/example/std_range_03.cpp
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
#include <algorithm>

void find_first_by_name(const std::string& srch)
{
	using namespace lagoon;

	// adapt the range of members to have a standard begin()/end()
	// member functions returning standard conforming iterators
	auto mosr = make_std_range(reflected_global_scope()->members());
	typedef shared<meta_named_scoped_object> shared_mnso;
	// use the standard find_if algorithm with a lambda unary
	// predicate checking if the base name contains the searched string
	auto pos = std::find_if(
		mosr.begin(),
		mosr.end(),
		[&srch](const shared_mnso& meta_obj)
		{
			return meta_obj->base_name().find(srch) !=
				std::string::npos;
		}
	);
	if(pos != mosr.end())
		std::cout << (*pos)->full_name() << std::endl;
	else std::cerr << "Not found!" << std::endl;
}

int main(void)
{
	find_first_by_name("int");
	find_first_by_name("bool");
	find_first_by_name("char");
	find_first_by_name("unknown");
	return 0;
}

/* Example of output:
short int
bool
char
Not found!
*/

