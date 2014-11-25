/**
 *  @example mirror/example/factories/person_pq.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a libpq-based data input interface implementation to generically
 *  construct instances of classes with non default constructors.
 *
 *  This example requires access to a local postgresql database named 'test'
 *  with a table named 'person' with (more or less) the following definition
 *  (containing some data):
 *
 *  @code
 *  CREATE TABLE person (
 *    first_name TEXT NOT NULL,
 *    middle_name TEXT.
 *    family_name TEXT NOT NULL,
 *    birth_date DATE,
 *    weight FLOAT,
 *    height FLOAT
 *  );
 *  @endcode
 *
 *  Copyright 2006-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <mirror/stream.hpp>
#include <mirror/utils/libpq_factory.hpp>
#include <iostream>
#include <stdexcept>

#include "./person.hpp"

int main(void)
{
	try
	{
		using namespace mirror;
		// connect to the database
		libpq_fact_db test_db("dbname=test");
		// query data about persons
		libpq_fact_result persons=test_db.query("SELECT * FROM person");
		// make a person object factory
		libpq_factory_maker::factory<test::person>::type
			person_factory(persons.data());
		// go through the rows of the data-set
		while(!persons.empty())
		{
			std::cout <<
				// make a person from the data
				// and write it to std output
				stream::to_json::from(
					person_factory(),
					[&persons](std::ostream& out)
					{out << "person_" << persons.position();}
				) << std::endl;
			persons.next();
		}
	}
	catch(const std::exception& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	//
	return 0;
}

