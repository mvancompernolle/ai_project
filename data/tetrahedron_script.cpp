/**
 *  @example mirror/example/factories/tetrahedron_script.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a script parsing plugin to generically construct instances
 *  of classes with non default constructors, from input strings.
 *
 *  Copyright 2006-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#include <mirror/factory.hpp>
#include <mirror/meta_type.hpp>
#include <mirror/utils/script_factory.hpp>
#include <iostream>

#include "./tetrahedron.hpp"

int main(void)
{
	using namespace mirror;
	using namespace test;
	MIRROR_USING_NAMESPACE(test);
	//
	// make the input object for the factory
	script_factory_input in;
	// create a factory plugged with the script parser
	script_factory_maker::factory<tetrahedron>::type f = in.data();
	//
	const char* inputs[] = {
		"tetrahedron( \
			triangle( \
				vector(1, 0, 0), \
				vector(0, 1, 0), \
				vector() \
			), \
			vector(0, 0, 6) \
		)",
		"tetrahedron( \
			triangle( \
				vector(2, 0, 0), \
				vector(0, 2, 0), \
				vector(0.0) \
			), \
			vector(0, 0, 3) \
		)",
		"tetrahedron( \
			vector(0, 0, 0), \
			vector(2, 0, 0), \
			vector(0, 2, 0), \
			vector(0, 0, 3) \
		)",
		"{{{1,0,0},{0,2,0},{0,0,0}},{0,0,3}}",
		"{{3,0,0},{0,4,0},{0,0,0},{0,0,1}}"
	};
	const char** cur = inputs;
	const char** end = inputs + sizeof(inputs) / sizeof(inputs[0]);
	while(cur != end)
	{
		std::string src(*cur++);
		in.set(src);
		// use the factory to construct a tetrahedron and calculate
		// its volume and base area
		tetrahedron t(f());
		// ... and print them out
		std::cout << "the volume is " << t.volume() << " " << std::flush;
		std::cout << "the area of the base is " << t.base.area() << std::endl;
	}
	//
	return 0;
}

/* Example of output:
the volume is 1 the area of the base is 0.5
the volume is 2 the area of the base is 2
the volume is 2 the area of the base is 2
the volume is 1 the area of the base is 1
the volume is 2 the area of the base is 6
 */
