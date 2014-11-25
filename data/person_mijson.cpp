/**
 *  @example mirror/example/factories/person_mijson.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a JSON parsing plugin to generically construct instances
 *  of classes with non default constructors, from strings read from
 *  the standard input.
 *
 *  Copyright 2006-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <mirror/stream.hpp>
#include <mirror/utils/mijson_factory.hpp>
#include <iostream>
#include <stdexcept>

#include "./person.hpp"

int main(void)
{
	try
	{
		using namespace mirror;
		MIRROR_USING_NAMESPACE(test);
		//
		const char json[] =
			"["
			"	{"
			"		'first_name': '\\u0414\\u043C\\u0438\\u0301\\u0442\\u0440\\u0438\\u0439', "
			"		'middle_name': '\\u0418\\u0432\\u0430\\u0301\\u043D\\u043E\\u0432\\u0438\\u0446', "
			"		'family_name': '\\u041C\\u0435\\u043D\\u0414\\u0435\\u043B\\u0435\\u0301\\u0435\\u0432', "
			"		'birth_date': '1834-02-08', "
			"		'sex': 'male', "
			"		'weight': 75, "
			"		'height': 175"
			"	},"
			"	{"
			"		'first_name': 'Albert',"
			"		'family_name': 'Einstein',"
			"		'birth_date': '1879-03-14',"
			"		'sex': 'male', "
			"		'weight': 70,"
			"		'height': 170"
			"	},"
			"	{"
			"		'first_name': 'Marie',"
			"		'family_name': 'Sk\\u0142odowska-Curie',"
			"		'birth_date': '1867-11-07',"
			"		'sex': 'female', "
			"		'weight': 52,"
			"		'height': 165"
			"	}"
			"]";
		//
		mijson_factory_input in;
		mijson_factory_maker::factory<test::person>::type f = in.data();
		//
		mijson_fragment<const char*> fragment(
			json,
			json + sizeof(json) - 1
		);
		auto	i = fragment.elements().begin(),
			e = fragment.elements().end();
		int n = 0;
		while(i != e)
		{
			in.set(*i);
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
			++i;
			++n;
		}
	}
	catch(const std::exception& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	//
	return 0;
}

/* Example of output
 |  "person_0": {
 |      "first_name": "Дми́трий",
 |      "middle_name": "Ива́новиц",
 |      "family_name": "МенДеле́ев",
 |      "birth_date": {
 |          "tm_sec": 0,
 |          "tm_min": 0,
 |          "tm_hour": 0,
 |          "tm_mday": 8,
 |          "tm_mon": 1,
 |          "tm_year": -66,
 |          "tm_wday": 0,
 |          "tm_yday": 0,
 |          "tm_isdst": -1
 |      },
 |      "sex": "male",
 |      "weight": 75,
 |      "height": 175
 |  }
 |  "person_1": {
 |      "first_name": "Albert",
 |      "middle_name": "",
 |      "family_name": "Einstein",
 |      "birth_date": {
 |          "tm_sec": 0,
 |          "tm_min": 0,
 |          "tm_hour": 0,
 |          "tm_mday": 14,
 |          "tm_mon": 2,
 |          "tm_year": -21,
 |          "tm_wday": 0,
 |          "tm_yday": 0,
 |          "tm_isdst": -1
 |      },
 |      "sex": "male",
 |      "weight": 70,
 |      "height": 170
 |  }
 |  "person_2": {
 |      "first_name": "Marie",
 |      "middle_name": "",
 |      "family_name": "Skłodowska-Curie",
 |      "birth_date": {
 |          "tm_sec": 0,
 |          "tm_min": 0,
 |          "tm_hour": 0,
 |          "tm_mday": 7,
 |          "tm_mon": 10,
 |          "tm_year": -33,
 |          "tm_wday": 0,
 |          "tm_yday": 0,
 |          "tm_isdst": -1
 |      },
 |      "sex": "female",
 |      "weight": 52,
 |      "height": 165
 |  }
 */
