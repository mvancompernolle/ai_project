/**
 *  @example mirror/example/meta_prog_07.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the reflection and compile-time and run-time traversals of the values
 *  of enumerated types
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>
#include <iomanip>
#include "./classes.hpp"

MIRROR_NAMESPACE_BEGIN

// Helper functor class used by the for_each_ii algorithm
// in the print_public_types_in<>() function
struct name_printer
{
	template <typename IterInfo>
	void operator()(IterInfo) const
	{
		typedef typename IterInfo::type MetaObject;
		// print the base name of the current element
		std::cout << MetaObject::base_name();
		std::cout << " = 0x";
		std::cout << std::setw(2)
			<< std::setfill('0')
			<< std::hex
			<< int(MetaObject::value::value);
		// if this is the last element in the traversal
		// then print the final dot or print a separating comma
		// otherwise
		if(IterInfo::is_last::value)
			std::cout << ".";
		else	std::cout << ", ";
	}
};

template <typename MetaEnum>
void print_enumeration_ct(void)
{
	using namespace mirror;
	std::cout << "Enum '"
		<< MetaEnum::full_name()
		<< "': ";
	mirror::mp::for_each_ii<members<MetaEnum> >(name_printer());
	std::cout << std::endl;
}

template <typename MetaEnum>
void print_enumeration_rt(void)
{
	using namespace mirror;
	std::cout << "Enum '"
		<< MetaEnum::full_name()
		<< "': ";
	int i=0;
	while(i != MetaEnum::size::value)
	{
		std::cout << MetaEnum::value_name(i);
		std::cout << " = 0x";
		std::cout << std::setw(2)
			<< std::setfill('0')
			<< std::hex
			<< int(MetaEnum::value(i));
		if(++i == MetaEnum::size::value)
			std::cout << ".";
		else	std::cout << ", ";
	}
	std::cout << std::endl;
}


MIRROR_NAMESPACE_END


int main(void)
{
	using namespace mirror;
	print_enumeration_ct<
		MIRRORED_ENUM(test::Ch)
	>();
	print_enumeration_rt<
		MIRRORED_ENUM(test::Ch)
	>();
	//
	return 0;
}

/* Example of output:
Enum 'test::Ch': null = 0x00, bell = 0x07, linefeed = 0x0a, carriage_return = 0x0d, horizontal_tab = 0x09, formfeed = 0x0c.
Enum 'test::Ch': null = 0x00, bell = 0x07, linefeed = 0x0a, carriage_return = 0x0d, horizontal_tab = 0x09, formfeed = 0x0c.
*/
