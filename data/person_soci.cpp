/**
 *  @example mirror/example/factories/person_soci.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a SOCI-based data input interface implementation to generically
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
#include <mirror/utils/soci_factory.hpp>
#include <soci-postgresql.h>
#include <iostream>
#include <stdexcept>

#include "./person.hpp"

int main(void)
{
	try
	{
		using namespace mirror;
		using namespace soci;
		// connect to the 'test' database
		session test_db(soci::postgresql, "dbname=test");
		// select data from the 'person' table
		rowset<row> data(test_db.prepare << "SELECT * FROM person");
		auto r = data.begin(), e = data.end();
		int i = 0;
		// make a person object factory
		soci_factory_maker::factory<test::person>::type
			person_factory = soci_fact_data(r);
		// go through the rows of the resulting dataset
		while(r != e)
		{
			std::cout <<
				// make a person from the data
				// and write it to std output
				stream::to_json::from(
					person_factory(),
					[&i](std::ostream& out)
					{ out << "person_" << i; }
				) << std::endl;
			++r;
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

/* Example of output:
 |  "person_0": {
 |      "first_name": "Johnny",
 |      "middle_name": "B",
 |      "family_name": "Goode",
 |      "birth_date": {
 |          "tm_sec": 0,
 |          "tm_min": 0,
 |          "tm_hour": 0,
 |          "tm_mday": 1,
 |          "tm_mon": 0,
 |          "tm_year": 10,
 |          "tm_wday": 6,
 |          "tm_yday": 0,
 |          "tm_isdst": 0
 |      },
 |      "sex": "male",
 |      "weight": 80.1,
 |      "height": 180
 |  }
 */
