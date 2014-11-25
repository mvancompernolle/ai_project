/**
 *  @example mirror/example/factories/physical_units.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a xml parsing plugin to generically construct instances
 *  of classes with non default constructors, from input strings.
 *
 *  Copyright 2006-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#include <mirror/mirror_base.hpp>
#include <mirror/pre_registered/type/native.hpp>
#include <mirror/pre_registered/type/std/string.hpp>
#include <mirror/pre_registered/class/std/list.hpp>
#include <mirror/pre_registered/class/std/map.hpp>
#include <mirror/utils/rapidxml_factory.hpp>
#include <iostream>

int main(void)
{
	using namespace mirror;
	//
	// make the input object for the factory
	rapidxml_factory_input in;
	// create a factory plugged with the rapidxml parser
	typedef std::map<std::string, std::list<std::string> > unit_map;
	rapidxml_factory_maker::factory<unit_map>::type f = in.data();
	//
	const char xml[] = " \
		<?xml version='1.0' encoding='utf-8' ?> \
		<data> \
		  <quantity first='length'> \
		    <second> \
		      <unit name='meter'/> \
		      <unit name='inch'/> \
		      <unit name='foot'/> \
		      <unit name='yard'/> \
		      <unit name='mile'/> \
		      <unit name='furlong'/> \
		      <unit name='parsec'/> \
		      <unit name='lightyear'/> \
		    </second> \
		  </quantity> \
		  <quantity first='mass'> \
		    <second> \
		      <unit name='kilogram'/> \
		      <unit name='tonne'/> \
		      <unit name='pound'/> \
		    </second> \
		  </quantity> \
		  <quantity first='time'> \
		    <second> \
		      <unit name='second'/> \
		      <unit name='minute'/> \
		      <unit name='hour'/> \
		      <unit name='week'/> \
		      <unit name='fortnight'/> \
		      <unit name='month'/> \
		      <unit name='year'/> \
		    </second> \
		  </quantity> \
		  <quantity first='electric current'> \
		    <second> \
		      <unit name='ampere'/> \
		    </second> \
		  </quantity> \
		  <quantity first='temperature'> \
		    <second> \
		      <unit name='Kelvin'/> \
		      <unit name='degrees Celsius'/> \
		      <unit name='degrees Fahrenheit'/> \
		      <unit name='degrees Rankine'/> \
		    </second> \
		  </quantity> \
		</data> \
	";

	rapidxml::xml_document<> doc;
	doc.parse<rapidxml::parse_non_destructive>((char*)xml);
	in.set(doc.first_node("data"));
	unit_map units(f());
	//
	for(auto i = units.begin(), e = units.end(); i != e; ++i)
	{
		std::cout << "Some units for '" << i->first << "': '";
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
Some units for 'electric current': 'ampere'.
Some units for 'length': 'meter', 'inch', 'foot', 'yard', 'mile', 'furlong', 'parsec', 'lightyear'.
Some units for 'mass': 'kilogram', 'tonne', 'pound'.
Some units for 'temperature': 'Kelvin', 'degrees Celsius', 'degrees Fahrenheit', 'degrees Rankine'.
Some units for 'time': 'second', 'minute', 'hour', 'week', 'fortnight', 'month', 'year'.
 */
