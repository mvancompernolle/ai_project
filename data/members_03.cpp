/**
 *  @example lagoon/example/members_03.cpp
 *  This example shows the usage of the reflection functions
 *  and templated meta-class member traversal.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror_base.hpp>
#include <mirror/pre_registered/type/native.hpp>
#include <mirror/pre_registered/type/std/string.hpp>
#include <mirror/pre_registered/class/std/pair.hpp>
#include <lagoon/lagoon.hpp>
#include <iostream>

LAGOON_NAMESPACE_BEGIN

template <class Range>
void print_range_info(Range r)
{
	std::cout << r.size() << std::endl;
	while(!r.empty())
	{
		auto member = r.front().template as<meta_variable>();
		// print the name of the kind of meta-variable
		std::cout << member->self()->construct_name()
			<< ": "
			// print the full name of the type of the variable
			<< member->type()->full_name()
			<< " "
			// and its base name
			<< member->base_name()
			<< std::endl;
		r.step_front();
	}
}

template <class MetaClass>
void print_info(MetaClass meta_X)
{
	// check if we've got a non-null pointer
	if(bool(meta_X))
	{
		// print the members of the passed meta-class
		std::cout
			<< "Members of "
			// first print the kind of the class
			<< meta_X->self()->construct_name()
			<< " "
			// print its full name
			<< meta_X->full_name()
			<< ": "
			<< std::endl;
		// and now print the info about its members
		print_range_info(meta_X->members());
		std::cout << std::endl;
	}
}

LAGOON_NAMESPACE_END

int main(void)
{
	using namespace lagoon;
	//
	print_info(reflected_class<std::pair< char, wchar_t> >());
	print_info(reflected_class<std::pair< double, long> >());
	print_info(reflected_class<std::pair< unsigned, std::string> >());
	//
	return 0;
}

/* Example of output:
Members of template class std::pair< char, wchar_t >:
2
member variable: char first
member variable: wchar_t second

Members of template class std::pair< double, long int >:
2
member variable: double first
member variable: long int second

Members of template class std::pair< unsigned int, std::string >:
2
member variable: unsigned int first
member variable: std::string second

*/
