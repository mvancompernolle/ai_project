/**
 *  @example lagoon/example/for_each_05.cpp
 *  This example shows the usage of the namespace reflection functions
 *  and range for-each utility with lambda functions and the limit
 *  and offset adapters
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/range/for_each.hpp>
#include <lagoon/range/limit.hpp>
#include <lagoon/range/offset.hpp>
#include <iostream>

int main(void)
{
	using namespace lagoon;
	//
	// get the members of the global scope, skip (offset) the first
	// four then limit the rest only to the first five members
	// and for each of them call the lambda function.
	for_each(
		limit(offset(reflected_global_scope()->members(), 4), 5),
		[](shared<meta_named_scoped_object> member)
		{
			std::cout
				<< member->self()->construct_name()
				<< " "
				<< member->base_name()
				<< std::endl;
		}
	);
	return 0;
}

/* Example of output:
type char
type unsigned char
type wchar_t
type short int
type int
*/
