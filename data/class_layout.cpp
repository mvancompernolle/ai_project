/**
 *  @example  mirror/example/class_layout.cpp
 *  This example shows the usage of the class_layout meta-function
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>
#include "./classes.hpp"

struct info_printer
{
	template <class IterInfo>
	void operator()(IterInfo)
	{
		using namespace mirror;
		// if this is the first element in the range traversal
		// print the opening parenthesis
		if(IterInfo::is_first::value)
			std::cout << "(";
		// print the full name of the current element
		std::cout << IterInfo::type::full_name();
		// if this is the last element if the range traversal
		// print the closing parenthesis or a separating comma
		// otherwise
		if(IterInfo::is_last::value)
			std::cout << ")" << std::endl;
		else std::cout << ", ";
	}
};

template <class Class>
void print_info(void)
{
	using namespace mirror;
	typedef MIRRORED_CLASS(Class) meta_X;
	std::cout << "Layout of "
		<< meta_X::full_name()
		<< ": ";
	// use the info_printer functor on each of the member variables
	// of the class reflected by the meta_X meta-class
	mp::for_each_ii< class_layout<meta_X> >(info_printer());
}

int main(void)
{
	using namespace mirror;
	//
	print_info<test::A>();
	print_info<test::B>();
	print_info<test::C>();
	print_info<test::D>();
	print_info<test::E>();
	print_info<test::F>();
	print_info<test::G>();
	print_info<test::H>();
	//
	return 0;
}

/* Example of output:
Layout of test::A: (test::A)
Layout of test::B: (test::B)
Layout of test::C: (test::A, test::B, test::C)
Layout of test::D: (test::A, test::B, test::D)
Layout of test::E: (test::A, test::B, test::C, test::D, test::E)
Layout of test::F: (test::A, test::B, test::C, test::D, test::E, test::F)
Layout of test::G: (test::G)
Layout of test::H: (test::A, test::B, test::C, test::D, test::E, test::F, test::G, test::H)
*/
