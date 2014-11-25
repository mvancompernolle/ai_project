/**
 *  @example mirror/example/factories/person_boostfs.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a filesystem-based plugin to generically construct instances
 *  of classes with non default constructors, from strings read from
 *  files in a directory hierarchy.
 *
 *  Copyright 2006-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <mirror/stream.hpp>
#include <mirror/utils/boostfs_factory.hpp>
#include <iostream>
#include <stdexcept>

#include "./person.hpp"

int main(int argc, const char* argv[])
{
	try
	{
		using namespace mirror;
		MIRROR_USING_NAMESPACE(test);
		//
		namespace fs = boost::filesystem;
		boostfs_factory_input in;
		boostfs_factory_maker::factory<test::person>::type f = in.data();
		//
		fs::path root(argc > 1 ? argv[1] : ".");
		auto	i = fs::directory_iterator(root),
			e = fs::directory_iterator();
		int n = 0;
		while(i != e)
		{
			if(fs::is_directory(*i))
			{
				in.set(fs::path(*i));
				// make a person from the data
				test::person pers(f());
				std::cout <<
					// and write it to std output
					stream::to_json::from(
						pers,
						[&n](std::ostream& out)
						{ out << "person_" << n; }
					) << std::endl;
				test::person p(f());
				++n;
			}
			++i;
		}
	}
	catch(const std::exception& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	//
	return 0;
}

