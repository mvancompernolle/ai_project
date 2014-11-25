/**
 *  @example lagoon/example/factories/tetrahedron_mijson.cpp
 *  This example shows the usage of the JSON-parsing automatically
 *  generated polymorphic factory to create instances of the test tetrahedron
 *  class.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

//
// We want to use Lagoon's polymorphic factories
#define LAGOON_MT_WITH_MAKE_FACTORY 1
//
// We don't need to traverse through namespace members.
// Using this CT switch can greatly improve compile times
// and the resulting executable size if namespace member
// traversal is not needed
#define LAGOON_NO_NAMESPACE_MEMBERS 1

#include <mirror/mirror.hpp>
#include <mirror/stream.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/utils/mijson_factory.hpp>
#include <iostream>
#include <stdexcept>

#include "./tetrahedron.hpp"

int main(void)
{
	try
	{
		using namespace lagoon;
		mijson_factory_builder builder;
		mijson_factory_input in;
		auto data = in.data();
		//
		auto meta_th = reflected_class<test::tetrahedron>();
		auto th_factory = meta_th->make_factory(
			builder,
			raw_ptr(&data)
		);
		//
		const char json[] =
			"["
			"	{"
			"		'base' : {"
			"			'a' : {'x': 1, 'y': 0, 'z': 0},"
			"			'b' : {'x': 0, 'y': 1, 'z': 0},"
			"			'c' : {'x': 0, 'y': 0, 'z': 0} "
			"		},"
			"		'apex' : {'x': 0, 'y': 0, 'z': 6} "
			"	},"
			"	{"
			"		'base' : {"
			"			'a' : {'x': 1, 'y': 0, 'z': 0},"
			"			'b' : {'x': 0, 'y': 2, 'z': 0},"
			"			'c' : {'w': 0} "
			"		},"
			"		'apex' : {'x': 0, 'y': 0, 'z': 6} "
			"	},"
			"	{"
			"		'base' : {"
			"			'a' : {'x': 2, 'y': 0, 'z': 0},"
			"			'b' : {'x': 0, 'y': 2, 'z': 0},"
			"			'c' : null "
			"		},"
			"		'apex' : {'x': 0, 'y': 0, 'z': 6} "
			"	},"
			"	{"
			"		'a' : {'x': 2, 'y': 0, 'z': 0},"
			"		'b' : {'x': 0, 'y': 2, 'z': 0},"
			"		'c' : { },"
			"		'd' : {'x': 0, 'y': 0, 'z': 12} "
			"	}"
			"]";
		//
		mirror::mijson_fragment<const char*> fragment(
			json,
			json + sizeof(json) - 1
		);
		auto	i = fragment.elements().begin(),
			e = fragment.elements().end();
		while(i != e)
		{
			in.set(*i);
			// use the factory to construct a tetrahedron and calculate
			// its volume and base area
			raw_ptr pt = th_factory->new_();
			test::tetrahedron& t = *raw_cast<test::tetrahedron*>(pt);
			// ... and print them out
			std::cout << "the volume is " << t.volume() << " " << std::flush;
			std::cout << "the area of the base is " << t.base.area() << std::endl;
			meta_th->delete_(pt);
			++i;
		}
	}
	catch(std::exception const& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	//
	return 0;
}

/* Example of output:
the volume is 1 the area of the base is 0.5
the volume is 2 the area of the base is 1
the volume is 4 the area of the base is 2
the volume is 8 the area of the base is 2
*/
