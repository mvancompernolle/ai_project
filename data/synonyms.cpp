/**
 *  @example mirror/example/factories/synonyms.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a script parsing plugin to generically construct instances
 *  of classes with non default constructors, from input strings.
 *
 *  Copyright 2006-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror_base.hpp>
#include <mirror/pre_registered/type/native.hpp>
#include <mirror/pre_registered/type/std/string.hpp>
#include <mirror/pre_registered/class/std/vector.hpp>
#include <mirror/pre_registered/class/std/map.hpp>
#include <mirror/utils/script_factory.hpp>
#include <iostream>

int main(void)
{
	using namespace mirror;
	//
	// make the input object for the factory
	script_factory_input in;
	// create a factory plugged with the script parser
	typedef std::map<std::string, std::vector<std::string> > word_map;
	script_factory_maker::factory<word_map>::type f = in.data();
	//
	const std::string input = "{ \
		{'mirror', {'looking glass', 'speculum', 'reflector'}}, \
		{'reflection', {'introspection', 'representation', 'observation'}}, \
		{'library', {'repository', 'bibliotheca'}} \
	}";
	in.set(input);
	word_map synonyms(f());
	for(auto i = synonyms.begin(), e = synonyms.end(); i != e; ++i)
	{
		std::cout << "Synonyms for '" << i->first << "': '";
		auto j = i->second.begin(), f = i->second.end();
		while(j != f)
		{
			std::cout << *j;
			++j;
			if(j == f) std::cout << "'.";
			else std::cout << "', '";
		}
		std::cout << std::endl;
	}
	//
	return 0;
}

/* Example of output:
Synonyms for 'library': 'repository', 'bibliotheca'.
Synonyms for 'mirror': 'looking glass', 'speculum', 'reflector'.
Synonyms for 'reflection': 'introspection', 'representation', 'observation'.
 */
