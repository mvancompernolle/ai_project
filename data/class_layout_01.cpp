/**
 *  @example lagoon/example/class_layout_01.cpp
 *  This example shows the usage of the reflection functions
 *  and meta-class member traversal.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <iostream>
#include "./classes.hpp"

LAGOON_NAMESPACE_BEGIN

template <class Range>
void print_range_info(Range r)
{
	std::cout << r.size() << std::endl;
	// for each member in the range
	while(!r.empty())
	{
		// print the 'kind' of the meta-object
		// and its full name including the nested name specifier
		std::cout << r.front()->self()->construct_name()
			<< ": "
			<< r.front()->full_name()
			<< std::endl;
		r.step_front();
	}
}

template <class MetaClass>
void print_info(MetaClass meta_X)
{
	if(bool(meta_X))
	{
		// first print the 'kind' of the meta-object (type, class, ...)
		// and its base name (without the nested name specifier)
		std::cout
			<< "Layout of "
			<< meta_X->self()->construct_name()
			<< " "
			<< meta_X->base_name()
			<< ": "
			<< std::endl;
		// then get a range of class member variables in the order
		// in which they are layed-out in the class
		// and use the function above to print the range
		print_range_info(meta_X->class_layout());
		std::cout << std::endl;
	}
}

LAGOON_NAMESPACE_END

int main(void)
{
	using namespace lagoon;
	//
	// reflect the global scope
	auto meta_gs(reflected_global_scope());
	// reflect the test namespace
	auto meta_test(meta_gs->namespace_("test"));
	// go through the meta_classes
	for(auto r = meta_test->members(); !r.empty(); r.step_front())
	{
		// and print information about it
		print_info(r.front().as<meta_class>());
	}
	//
	return 0;
}

