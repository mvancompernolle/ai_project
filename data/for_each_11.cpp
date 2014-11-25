/**
 *  @example lagoon/example/for_each_11.cpp
 *  This example shows the usage of the namespace reflection functions
 *  and range for-each utility with the flatten function.
 *
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/range/flatten.hpp>
#include <lagoon/range/for_each.hpp>
#include <iostream>

int main(void)
{
	using namespace lagoon;
	//
	// Use the flatten(...) function to get all members of the global scope
	// including the nested ones and print their 'kinds' and full names
	// by the lambda function
	for_each(
		flatten(reflected_global_scope()),
		[](const shared<meta_named_scoped_object>& member)
		{
			std::cout <<
				member->self()->construct_name() <<
				": " <<
				member->full_name() <<
				std::endl;
		}
	);
	return 0;
}

/* Example of output:
namespace: std
namespace: boost
type: void
type: bool
type: char
type: unsigned char
type: wchar_t
type: short int
type: int
type: long int
type: unsigned short int
type: unsigned int
type: unsigned long int
type: float
type: double
type: long double
class: std::string
class: std::wstring
class: std::tm
template: std::pair
template: std::tuple
template: std::allocator
template: std::equal_to
template: std::not_equal_to
template: std::less
template: std::greater
template: std::less_equal
template: std::greater_equal
template: std::vector
template: std::list
template: std::deque
template: std::map
template: std::set
member variable: std::tm::tm_sec
member variable: std::tm::tm_min
member variable: std::tm::tm_hour
member variable: std::tm::tm_mday
member variable: std::tm::tm_mon
member variable: std::tm::tm_year
member variable: std::tm::tm_wday
member variable: std::tm::tm_yday
member variable: std::tm::tm_isdst
namespace: mirror
*/
