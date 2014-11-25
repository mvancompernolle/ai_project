/**
 *  @example mirror/example/factories/tetrahedron_io.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a iostream-based user input interface implementation to generically
 *  construct instances of classes with non default constructors.
 *
 *  Copyright 2006-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#include <mirror/invoker.hpp>
#include <mirror/factory.hpp>
#include <mirror/meta_type.hpp>
#include <iostream>

#include "./input_ui.hpp"
#include "./tetrahedron.hpp"

int main(void)
{
	using namespace mirror;
	// create a factory plugged with the input ui
	::test::input_ui_factory_maker::factory<
		::test::tetrahedron
	>::type f(0);
	// use the factory to construct a tetrahedron and calculate
	// its volume and base area
	::test::tetrahedron t(f());
	// ... and print them out
	std::cout << "the volume is " << t.volume() << std::endl;
	std::cout << "the area of the base is " << t.base.area() << std::endl;
	//
	::test::input_ui_invoker_maker::invoker<
		MIRRORED_MEMBER_FUNCTION(test::tetrahedron, reset_apex, 1)
	>::type i(0);
	i.call_on(t);
	//
	// ... and print the volume again
	std::cout << "the volume is " << t.volume() << std::endl;
	//
	return 0;
}
