/**
 *  @example lagoon/example/std_range_02.cpp
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

int main(void)
{
	using namespace lagoon;
	// an adapted std range of meta objects
	auto mosr = make_std_range(reflected_global_scope()->members());
	typedef shared<meta_named_scoped_object> shared_mnso;
	// print the count of namespaces in the global scope
	// using the standard count_if algorithm
	std::cout << std::count_if(
		mosr.begin(),
		mosr.end(),
		[](const shared_mnso& meta_obj)
		{
			return meta_obj->is_namespace();
		}
	) << std::endl;
	return 0;
}

/* Example of output:
2
*/
