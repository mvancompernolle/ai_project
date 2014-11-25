/**
 *  Copyright 2009-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *  WARNING: this file is here for development / testing purposes only
 *  and its contents and input / output can change witout any prior
 *  notice.
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
#include <lagoon/utils/rapidxml_factory.hpp>

#include <cassert>
#include <iostream>
#include <stdexcept>

#include "./test.hpp"

MIRROR_NAMESPACE_BEGIN
namespace aux {

} // namespace aux

MIRROR_NAMESPACE_END

int main(void)
{
	try
	{
		//
		using namespace lagoon;
		rapidxml_factory_builder builder;
		rapidxml_factory_input in;
		auto data = in.data();
		//
		auto meta_person = reflected_class<test::person>();
		auto person_factory = meta_person->make_factory(
			builder,
			raw_ptr(&data)
		);
		const char* xml = "\
			<?xml version='1.0' encoding='utf-8' ?>\n\
			<persons> \
				<person	\
					title='mr' \
					first_name='Johnny' \
					middle_name='B.' \
					family_name='Goode' \
					birth_date='1910-01-01' \
					weight='70.4' \
					height='1.75e2' \
				/> \
				<person	\
					title='mr' \
					first_name='John' \
					family_name='Doe' \
					birth_date='1955-05-10' \
					weight='75.9' \
					height='182' \
				/> \
			</persons> \
		";
		//
		rapidxml::xml_document<> doc;
		doc.parse<rapidxml::parse_non_destructive>((char*)xml);
		rapidxml::xml_node<>* root = doc.first_node();
		rapidxml::xml_node<>* src = root->first_node("person");
		//
		in.print_handlers(std::cout);
		while(src != nullptr)
		{
			in.set(src);
			raw_ptr ppers = person_factory->new_();
			test::person& pers = *raw_cast<test::person*>(ppers);
			std::cout << mirror::stream::to_json::from(
				pers,[](std::ostream& out){ out << "pers"; }
			) << std::endl;
			meta_person->delete_(ppers);
			src = src->next_sibling();
		}
	}
	catch(std::exception const& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	//
	return 0;
}
