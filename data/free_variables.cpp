/**
 *  @example rubber/example/free_variables.cpp
 *  This example shows the usage of the type erasures for compile-time
 *  Mirror's meta-object with the for-each algorithm and lambda functions
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>

#define RUBBER_META_TYPE_OPERATIONS write_to_cout
#include <rubber/operation/write_to_ostream.hpp>

#include <rubber/rubber.hpp>
#include <iostream>
#include <vector>

// Total count of instances of test ever created
unsigned total_count = 0;
// Count of instances of test currently in existence
unsigned alive_count = 0;

struct test
{
	test(void)
	{
		++total_count;
		++alive_count;
	}

	test(const test&)
	{
		++total_count;
		++alive_count;
	}

	~test(void)
	{
		--alive_count;
	}
};

MIRROR_REG_BEGIN

MIRROR_REG_GLOBAL_SCOPE_VARIABLE(_, total_count)
MIRROR_REG_GLOBAL_SCOPE_VARIABLE(_, alive_count)

MIRROR_REG_END

void print_stats(int tag)
{
	using namespace rubber;
	std::cout << "Checkpoint " << tag << std::endl;
	std::cout << "--------------" << std::endl;
	mirror::mp::for_each<
		mirror::free_variables<
			MIRRORED_GLOBAL_SCOPE()
		>
	>(
		[](const meta_free_variable& var)
		{
			std::cout << var.base_name() << " = ";
			var.type().examine_by<write_to_cout>(var.get());
			std::cout << std::endl;
		}
	);
	std::cout << "--------------" << std::endl;
}

int main(void)
{
	int tag = 0;
	//
	print_stats(++tag);
	std::vector<test> v;
	print_stats(++tag);
	{
		std::vector<test> v1(10);
		print_stats(++tag);
		std::vector<test> v2(10);
		print_stats(++tag);
		std::vector<test> v3(10);
		print_stats(++tag);
		v.swap(v1);
		print_stats(++tag);
		v.insert(v.end(), v2.begin(), v2.end());
		print_stats(++tag);
		v.insert(v.begin(), v3.begin(), v3.end());
		print_stats(++tag);
	}
	print_stats(++tag);
	v.clear();
	print_stats(++tag);
	return 0;
}

/* Example of output:
Checkpoint 1
--------------
total_count = 0
alive_count = 0
--------------
Checkpoint 2
--------------
total_count = 0
alive_count = 0
--------------
Checkpoint 3
--------------
total_count = 10
alive_count = 10
--------------
Checkpoint 4
--------------
total_count = 20
alive_count = 20
--------------
Checkpoint 5
--------------
total_count = 30
alive_count = 30
--------------
Checkpoint 6
--------------
total_count = 30
alive_count = 30
--------------
Checkpoint 7
--------------
total_count = 50
alive_count = 40
--------------
Checkpoint 8
--------------
total_count = 80
alive_count = 50
--------------
Checkpoint 9
--------------
total_count = 80
alive_count = 30
--------------
Checkpoint 10
--------------
total_count = 80
alive_count = 0
--------------
*/

