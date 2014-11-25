/**
 *  @example lagoon/example/for_each_09.cpp
 *  This example shows the usage of the namespace reflection functions
 *  and range for-each utility with lambda functions and the extract
 *  and chain_mo adapters
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/range/extract.hpp>
#include <lagoon/range/chain.hpp>
#include <lagoon/range/for_each.hpp>
#include <iostream>
#include "classes.hpp"

int main(void)
{
	using namespace lagoon;
	typedef shared<meta_named_scoped_object> shared_mnso;
	typedef shared<meta_type> shared_mt;
	//
	// reflect the global scope
	auto meta_gs = reflected_global_scope();
	// and then get the std, boost and the test namespaces
	auto meta_std = meta_gs->namespace_("std");
	auto meta_boost = meta_gs->namespace_("boost");
	auto meta_test = meta_gs->namespace_("test");
	// from each of these three namespaces extract only the member
	// types, then chain the resulting ranges into a single range
	// and for each of its elements use the lambda function
	// to print its base name
	for_each(
		chain_mo<meta_type>(
			extract<meta_type>(meta_std->members()),
			extract<meta_type>(meta_boost->members()),
			extract<meta_type>(meta_test->members())
		),
		[](const shared<meta_type>& metatype)
		{
			std::cout << metatype->base_name() << std::endl;
		}
	);
	return 0;
}

/* Example of output:
string
wstring
tm
A
B
C
D
E
F
G
H
*/
