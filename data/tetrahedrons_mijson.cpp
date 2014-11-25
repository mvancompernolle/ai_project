/**
 *  @example lagoon/example/factories/tetrahedrons_mijson.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a JSON parsing plugin to generically construct instances
 *  of classes with non default constructors, from input strings.
 *
 *  Copyright 2006-2011 Matus Chochlik. Distributed under the Boost
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

#include <mirror/mirror_base.hpp>
#include <mirror/pre_registered/basic.hpp>
#include <mirror/pre_registered/class/std/list.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/utils/mijson_factory.hpp>
#include <iostream>

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
		typedef std::list<test::tetrahedron> tetrahedron_list;
		auto meta_tl = reflected_class<tetrahedron_list>();
		auto tl_factory = meta_tl->make_factory(
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
		in.set(fragment);
		// use the factory to create a list of tetrahedrons
		raw_ptr ptl = tl_factory->new_();
		tetrahedron_list& tl = *raw_cast<tetrahedron_list*>(ptl);
		for(auto i = tl.begin(), e = tl.end(); i != e; ++i)
		{
			std::cout << "the volume is " << i->volume() << " " << std::flush;
			std::cout << "the area of the base is " << i->base.area() << std::endl;
		}
		meta_tl->delete_(ptl);
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
