/**
 *  Copyright 2009-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *  WARNING: this file is here for development / testing purposes only
 *  and its contents and input / output can change witout any prior
 *  notice.
 */

#include <mirror/mirror.hpp>
#include <mirror/stream.hpp>
#include <mirror/factory.hpp>
#include <mirror/using_directive.hpp>
#include <mirror/utils/wxxml_factory.hpp>

#include <wx/mstream.h>

#include <cassert>
#include <iostream>
#include <stdexcept>

#include "./test.hpp"

MIRROR_NAMESPACE_BEGIN

MIRROR_NAMESPACE_END

int main(void)
{
	try
	{
		using namespace mirror;
		MIRROR_USING_NAMESPACE(test);
		//
		const char xml[] =
			"<?xml version='1.0' encoding='utf-8' ?>\n"
			"<persons>\n"
			"	<person	\n"
			"		title='mr'\n"
			"		first_name='Johnny'\n"
			"		middle_name='B.'\n"
			"		family_name='Goode'\n"
			"		birth_date='1910-01-01'\n"
			"		weight='70.4'\n"
			"		height='1.75e2'\n"
			"	/>\n"
			"	<person	\n"
			"		title='mr'\n"
			"		first_name='John'\n"
			"		family_name='Doe'\n"
			"		birth_date='1955-05-10'\n"
			"		weight='75.9'\n"
			"		height='182'\n"
			"	/>\n"
			"</persons>\n";
		//
		wxMemoryInputStream xml_stream(xml, sizeof(xml) - 1);
		wxXmlDocument doc(xml_stream);
		wxXmlNode* root = doc.GetRoot();
		wxXmlNode* src = root->GetChildren();
		//
		wxxml_factory_input in;
		wxxml_factory_maker::factory<test::person>::type f = in.data();
		in.print_handlers(std::cout);
		while(src != nullptr)
		{
			in.set(src);
			test::person p(f());
			std::cout << stream::to_json::from(
				p,[](std::ostream& out){ out << "p"; }
			) << std::endl;
			src = src->GetNext();
		}
	}
	catch(std::exception const& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	//
	return 0;
}
