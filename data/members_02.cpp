/**
 *  @example lagoon/example/members_02.cpp
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
	while(!r.empty())
	{
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
	// if the passed meta-class is not nil
	if(bool(meta_X))
	{
		// print the 'kind' and the base name of the class
		std::cout << "Members of "
			<< meta_X->self()->construct_name()
			<< " "
			<< meta_X->base_name()
			<< ": "
			<< std::endl;
		// print the members of the reflected class
		print_range_info(meta_X->members());
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
	// go through the members of the test namespace
	for(auto r = meta_test->members(); !r.empty(); r.step_front())
	{
		// cast the member's interface to meta_class
		// and print the information about the reflected class
		print_info(r.front().as<meta_class>());
	}
	//
	return 0;
}

/* Example of output:
Members of class A:
0

Members of class B:
0

Members of class C:
1
member variable: test::C::c

Members of class D:
1
member variable: test::D::d

Members of class E:
3
member variable: test::C::c
member variable: test::D::d
member variable: test::E::e

Members of class F:
4
member variable: test::C::c
member variable: test::D::d
member variable: test::E::e
member variable: test::F::f

Members of class G:
1
member variable: test::G::g

Members of class H:
6
member variable: test::C::c
member variable: test::D::d
member variable: test::E::e
member variable: test::F::f
member variable: test::G::g
member variable: test::H::h

*/
