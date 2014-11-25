/**
 *  @example lagoon/example/members_01.cpp
 *  This example shows the usage of the reflection functions
 *  and meta-scope / meta-namespace member traversal.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <lagoon/lagoon.hpp>
#include <iostream>

LAGOON_NAMESPACE_BEGIN

template <class Range>
void print_info(Range r)
{
	// print the number of elements in the range
	std::cout << r.size() << std::endl;
	// traverse the elements of the range
	while(!r.empty())
	{
		// and for each print its 'kind' (type, class, template, ...)
		// and full name with the nested name specifier
		std::cout << r.front()->self()->construct_name()
			<< ": "
			<< r.front()->full_name()
			<< std::endl;
		r.step_front();
	}
}

LAGOON_NAMESPACE_END

int main(void)
{
	using namespace lagoon;
	//
	// reflect the global scope
	auto meta_gs(reflected_global_scope());
	// and print the list of its members
	std::cout << "the global scope:" << std::endl;
	print_info(meta_gs->members());
	//
	// get the meta-namespace reflecting the std namespace
	auto meta_std(meta_gs->namespace_("std"));
	// and print the list of its members
	std::cout << "namespace "
		<< meta_std->base_name()
		<< ": "
		<< std::endl;
	print_info(meta_std->members());
	//
	// get the meta-namespace for the boost namespace
	auto meta_boost(meta_gs->namespace_("boost"));
	std::cout << "namespace "
		<< meta_boost->base_name()
		<< ": "
		<< std::endl;
	// and print the list of its members
	print_info(meta_boost->members());
	//
	return 0;
}

/* Example of output:
the global scope:
16
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
namespace std:
17
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
*/
