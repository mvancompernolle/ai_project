/**
 *  @example mirror/example/factories/tetrahedron_soci.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a soci database library-based implementation to generically
 *  construct instances of classes with non default constructors
 *  from data obtained by a SQL database query.
 *
 *  This example requires access to a local postgresql database named 'test'
 *  with a tables named 'vector', 'triangle' and 'tetrahedron'
 *  with (more or less) the following definition (containing some data):
 *
 *  @code
 *  CREATE TABLE vector(
 *      vector_id INTEGER PRIMARY KEY.
 *      x DOUBLE PRECISION NOT NULL,
 *      y DOUBLE PRECISION NOT NULL,
 *      z  DOUBLE PRECISION NOT NULL
 *  );
 *
 *  CREATE TABLE triangle (
 *      triangle_id INTEGER PRIMARY KEY,
 *      a_vector_id INTEGER NOT NULL,
 *      b_vector_id INTEGER NOT NULL,
 *      c_vector_id INTEGER NOT NULL
 *  );
 *  ALTER TABLE triangle ADD FOREIGN KEY (a_vector_id) REFERENCES vector;
 *  ALTER TABLE triangle ADD FOREIGN KEY (b_vector_id) REFERENCES vector;
 *  ALTER TABLE triangle ADD FOREIGN KEY (c_vector_id) REFERENCES vector;
 *
 *  CREATE TABLE tetrahedron (
 *      tetrahedron_id INTEGER PRIMARY KEY,
 *      base_triangle_id INTEGER NOT NULL,
 *      apex_vector_id INTEGER NOT NULL
 *  );
 *  ALTER TABLE tetrahedron ADD FOREIGN KEY (base_triangle_id) REFERENCES triangle;
 *  ALTER TABLE tetrahedron ADD FOREIGN KEY (apex_vector_id) REFERENCES vector;
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

#include "./tetrahedron.hpp"

int main(void)
{
	try
	{
		using namespace mirror;
		using namespace soci;
		session test_db(postgresql, "dbname=test");
		rowset<row> data(
			test_db.prepare <<
			"SELECT "\
				"vba.x AS base__a__x, "\
				"vba.y AS base__a__y, "\
				"vba.z AS base__a__z, "\
				"vbb.x AS base__b__x, "\
				"vbb.y AS base__b__y, "\
				"vbb.z AS base__b__z, "\
				"vbc.x AS base__c__x, "\
				"vbc.y AS base__c__y, "\
				"vbc.z AS base__c__z, "\
				"va.x AS apex__x, "\
				"va.y AS apex__y, "\
				"va.z AS apex__z  "\
			"FROM tetrahedron t "\
			"JOIN triangle b ON(t.base_triangle_id = b.triangle_id) "\
			"JOIN vector va ON(t.apex_vector_id = va.vector_id) "\
			"JOIN vector vba ON(b.a_vector_id = vba.vector_id) "\
			"JOIN vector vbb ON(b.b_vector_id = vbb.vector_id) "\
			"JOIN vector vbc ON(b.c_vector_id = vbc.vector_id) "
		);
		auto r = data.begin(), e = data.end();
		int i = 0;
		// make a tetrahedron object factory
		soci_quick_factory_maker::factory<test::tetrahedron>::type
			tetrahedron_factory = soci_fact_data(r);
		while(r != e)
		{
			// make a tetrahedron from the data
			test::tetrahedron t = tetrahedron_factory();
			std::cout <<
				// and write it to std output
				stream::to_json::from(
					t,
					[&i](std::ostream& out)
					{ out << "tetrahedron_" << i; }
				) << std::endl;
			// and calculate its volume and base area
			// ... and print them out
			std::cout <<
				"the volume is " <<
				t.volume() <<
				std::endl;
			std::cout <<
				"the area of the base is " <<
				t.base.area() <<
				std::endl;
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

/* Sample data:
 |
 |  test=> SELECT * FROM vector;
 |   vector_id | x | y | z
 |  -----------+---+---+----
 |           0 | 0 | 0 |  0
 |           1 | 1 | 0 |  0
 |           2 | 0 | 1 |  0
 |           3 | 0 | 0 |  6
 |           4 | 0 | 0 | 24
 |  (5 rows)
 |
 |  test=> SELECT * FROM triangle ;
 |   triangle_id | a_vector_id | b_vector_id | c_vector_id
 |  -------------+-------------+-------------+-------------
 |             0 |           0 |           1 |           2
 |  (1 row)
 |
 |  test=> SELECT * FROM tetrahedron ;
 |   tetrahedron_id | base_triangle_id | apex_vector_id
 |  ----------------+------------------+----------------
 |                0 |                0 |              3
 |                1 |                0 |              4
 |  (2 rows)
 */

/* Example of output:
 |  "tetrahedron_0": {
 |      "base": {
 |          "a": {
 |              "x": 0,
 |              "y": 0,
 |              "z": 0
 |          },
 |          "b": {
 |              "x": 1,
 |              "y": 0,
 |              "z": 0
 |          },
 |          "c": {
 |              "x": 0,
 |              "y": 1,
 |              "z": 0
 |          }
 |      },
 |      "apex": {
 |          "x": 0,
 |          "y": 0,
 |          "z": 6
 |      }
 |  }
 |  the volume is 1
 |  the area of the base is 0.5
 |  "tetrahedron_1": {
 |      "base": {
 |          "a": {
 |              "x": 0,
 |              "y": 0,
 |              "z": 0
 |          },
 |          "b": {
 |              "x": 1,
 |              "y": 0,
 |              "z": 0
 |          },
 |          "c": {
 |              "x": 0,
 |              "y": 1,
 |              "z": 0
 |          }
 |      },
 |      "apex": {
 |          "x": 0,
 |          "y": 0,
 |          "z": 24
 |      }
 |  }
 |  the volume is 4
 |  the area of the base is 0.5
 */
