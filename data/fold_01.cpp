/**
 *  @example lagoon/example/fold_01.cpp
 *  This example shows the usage of the fold range utility
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/range/extract.hpp>
#include <lagoon/range/fold.hpp>
#include <iostream>

int main(void)
{
	using namespace lagoon;
	//
	typedef shared<meta_named_scoped_object> shared_mnso;
	// initialize a string
	std::string str("Types in the global scope: ");
	fold_ref(
		// get members of the global scope and extract only meta-types
		extract<meta_type>(reflected_global_scope()->members()),
		// use the initialized string as the initial value for fold
		str,
		// use this lambda function to append the name
		// of the first member (type) of the global scope
		[](std::string& str, const shared_mnso& member)
		{ str.append(member->base_name()); },
		// use this for every other namespace member except the first
		// and the last
		[](std::string& str, const shared_mnso& member)
		{ str.append(", "); str.append(member->base_name()); },
		// and use this lambda function to append the name
		// of the last namespace member
		[](std::string& str, const shared_mnso& member)
		{ str.append(" and "); str.append(member->base_name()); }
	).append(".");
	std::cout << str << std::endl;
	//
	return 0;
}

/* Example of output:
Types in the global scope: void, bool, char, unsigned char, wchar_t, short int, int, long int, unsigned short int, unsigned int, unsigned long int, float, double and long double.
 */
