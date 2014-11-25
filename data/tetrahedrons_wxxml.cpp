/**
 *  @example mirror/example/factories/tetrahedrons_wxxml.cpp
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
#include <mirror/pre_registered/class/std/list.hpp>
#include <mirror/utils/wxxml_factory.hpp>
#include <iostream>

#include <wx/mstream.h>

#include "./tetrahedron.hpp"

int main(void)
{
	using namespace mirror;
	using namespace test;
	MIRROR_USING_NAMESPACE(test);
	//
	// make the input object for the factory
	wxxml_factory_input in;
	// create a factory plugged with the wxXml parser
	wxxml_factory_maker::factory<
		std::list<tetrahedron>
	>::type f = in.data();
	//
	const char xml[] =
		"<?xml version='1.0' encoding='utf-8' ?>\n"
		"<data>\n"
		"	<t1>\n"
		"		<base>\n"
		"			<a x='1' y='0' z='0'/>\n"
		"			<b x='0' y='1' z='0'/>\n"
		"			<c x='0' y='0' z='0'/>\n"
		"		</base>\n"
		"		<apex x='0' y='0' z='6'/>\n"
		"	</t1>\n"
		"	<t2>\n"
		"		<base>\n"
		"			<a x='1' y='0' z='0'/>\n"
		"			<b x='0' y='2' z='0'/>\n"
		"			<c w='0'/>\n"
		"		</base>\n"
		"		<apex x='0' y='0' z='6'/>\n"
		"	</t2>\n"
		"	<t3>\n"
		"		<base>\n"
		"			<a x='2' y='0' z='0'/>\n"
		"			<b x='0' y='2' z='0'/>\n"
		"			<c/>\n"
		"		</base>\n"
		"		<apex x='0' y='0' z='6'/>\n"
		"	</t3>\n"
		"	<t4>\n"
		"		<a x='2' y='0' z='0'/>\n"
		"		<b x='0' y='2' z='0'/>\n"
		"		<c/>\n"
		"		<d x='0' y='0' z='12'/>\n"
		"	</t4>\n"
		"</data>\n";

	wxMemoryInputStream xml_stream(xml, sizeof(xml) - 1);
	wxXmlDocument doc(xml_stream);
	in.set(doc.GetRoot());
	// use the factory to create a list of tetrahedrons
	std::list<tetrahedron> tv(f());
	for(auto i = tv.begin(), e = tv.end(); i != e; ++i)
	{
		std::cout << "the volume is " << i->volume() << " " << std::flush;
		std::cout << "the area of the base is " << i->base.area() << std::endl;
	}
	return 0;
}

/* Example of output:
the volume is 1 the area of the base is 0.5
the volume is 2 the area of the base is 1
the volume is 4 the area of the base is 2
the volume is 8 the area of the base is 2
 */
