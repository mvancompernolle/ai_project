/**
 *  @example mirror/example/factories/si_prefixes.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a JSON parsing plugin to generically construct instances
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
#include <mirror/utils/mijson_factory.hpp>
#include <iostream>

int main(void)
{
	using namespace mirror;
	//
	typedef std::map<int, std::vector<std::string> > prefix_map;
	mijson_factory_input in;
	mijson_factory_maker::factory<prefix_map>::type f = in.data();
	//
	const char json[] = "[ \
		{'first':  24, 'second': ['yotta', 'Y']}, \
		{'first':  21, 'second': ['zetta', 'Z']}, \
		{'first':  18, 'second': ['exa', 'E']}, \
		{'first':  15, 'second': ['peta', 'P']}, \
		{'first':  12, 'second': ['tera', 'T']}, \
		{'first':   9, 'second': ['giga', 'G']}, \
		{'first':   6, 'second': ['mega', 'M']}, \
		{'first':   3, 'second': ['kilo', 'k']}, \
		{'first':   2, 'second': ['hecto', 'h']}, \
		{'first':   1, 'second': ['deca', 'da']}, \
		{'first':  -1, 'second': ['deci', 'd']}, \
		{'first':  -2, 'second': ['centi', 'c']}, \
		{'first':  -3, 'second': ['milli', 'm']}, \
		{'first':  -6, 'second': ['micro', '\\u03BC']}, \
		{'first':  -9, 'second': ['nano', 'n']}, \
		{'first': -12, 'second': ['pico', 'p']}, \
		{'first': -15, 'second': ['femto', 'f']}, \
		{'first': -18, 'second': ['atto', 'a']}, \
		{'first': -21, 'second': ['zepto', 'z']}, \
		{'first': -24, 'second': ['yocto', 'y']} \
	]";
	mijson_fragment<const char*> input(
		json,
		json + sizeof(json) - 1
	);
	in.set(input);
	prefix_map prefixes(f());
	for(auto i = prefixes.begin(), e = prefixes.end(); i != e; ++i)
	{
		std::cout <<
			i->second[0] <<
			" (" <<
			i->second[1] <<
			") = 10e" <<
			i->first <<
			std::endl;
	}
	//
	return 0;
}

/* Example of output:
yocto (y) = 10e-24
zepto (z) = 10e-21
atto (a) = 10e-18
femto (f) = 10e-15
pico (p) = 10e-12
nano (n) = 10e-9
micro (μ) = 10e-6
milli (m) = 10e-3
centi (c) = 10e-2
deci (d) = 10e-1
deca (da) = 10e1
hecto (h) = 10e2
kilo (k) = 10e3
mega (M) = 10e6
giga (G) = 10e9
tera (T) = 10e12
peta (P) = 10e15
exa (E) = 10e18
zetta (Z) = 10e21
yotta (Y) = 10e24
 */
