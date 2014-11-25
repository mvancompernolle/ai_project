/**
 *  @example mirror/example/various_04.cpp
 *  This example shows the usage of Mirrors intrinsic meta-functions
 *
 *  More preciselly this example shows the usage of the reflection templates,
 *  inheritance reflection, intrinsic meta-functions and meta-programming tools.
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>
#include "./classes.hpp"

int main(void)
{
	using namespace mirror;
	//
	// print the inheritance type specifier for
	// for the first base of the test::F class
	std::cout <<
		inheritance_type<
			mp::at_c<
				base_classes<
					reflected<test::F>
				>,
				1
			>
		>::type::keyword()
	<< std::endl;
	// now print the full name of that base class
	std::cout <<
		base_class<
			mp::at_c<
				base_classes<
					reflected<test::F>
				>,
				1
			>
		>::type::full_name()
	<< std::endl;
	//
	return 0;
}

/* Example of output:
*/
