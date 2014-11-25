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
#include <lagoon/utils/soci_factory.hpp>
#include <soci-postgresql.h>
#include <iostream>
#include <stdexcept>

#include "./test.hpp"
int main(void)
{
	try
	{
		using namespace lagoon;
		using namespace soci;
		session test_db(postgresql, "dbname=test");
		rowset<row> dataset(test_db.prepare << "SELECT * FROM person");
		auto r = dataset.begin(), e = dataset.end();
		int i = 0;
		soci_factory_builder builder;
		soci_fact_data data(r);
		auto meta_person = reflected_class<test::person>();
		auto person_factory = meta_person->make_factory(
			builder,
			raw_ptr(&data)
		);
		// make a tetrahedron object factory
		while(r != e)
		{
			raw_ptr ppers = person_factory->new_();
			test::person& pers = *raw_cast<test::person*>(ppers);
			std::cout << mirror::stream::to_json::from(
				pers,
				[&i](std::ostream& out)
				{ out << "person_" << i; }
			) << std::endl;
			meta_person->delete_(ppers);
			++r;
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

