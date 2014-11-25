/**
 *  @example lagoon/example/std_range_04.cpp
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
#include <lagoon/range/only_if.hpp>
#include <lagoon/range/transform.hpp>

#include <iostream>
#include <algorithm>

int main(void)
{
	using namespace lagoon;
	typedef shared<meta_named_scoped_object> shared_mnso;
	typedef shared<meta_type> shared_mt;
	// get the members of the global scope, extract only types
	// and adapt the range to have the standard begin()/end()
	// member functions returning standard conforming iterators
	auto mosr = make_std_range(
		transform(
			only_if(
				reflected_global_scope()->members(),
				[](const shared_mnso& member)
				{
					return member->is_type();
				}
			),
			[](const shared_mnso& member)
			{
				return member.as<meta_type>();
			}
		)
	);
	// now use the standard count_if algorithm with a lambda
	// unary predicate counting only types whose size is 4 bytes
	std::cout << std::count_if(
		mosr.begin(),
		mosr.end(),
		[](const shared_mt& metatype) -> bool
		{
			return metatype->size_of() == 4;
		}
	) << std::endl;
	return 0;
}

/* Example of output:
6
*/
