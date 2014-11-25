/**
 *  @example mirror/example/meta_prog_09.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions for various meta-objects
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <type_traits>
#include <iostream>
#include "./classes.hpp"

MIRROR_NAMESPACE_BEGIN

// Helper functor class used by for_each_ii in print_static_members_of
struct name_printer
{
	template <typename IterInfo>
	void operator()(IterInfo) const
	{
		typedef typename IterInfo::type MetaMemberVariable;
		// print the full name of the current member variable
		std::cout << MetaMemberVariable::full_name();
		// if this is the last element in the traversal
		// then print the final dot or print a separating comma
		// otherwise
		if(IterInfo::is_last::value)
			std::cout << ".";
		else	std::cout << ", ";
	}
};

template <typename MetaClass>
void print_static_members_of(void)
{
	using namespace mirror;
	std::cout << "Static members of the '"
		<< MetaClass::full_name()
		<< "' class: ";
	mp::for_each_ii<
		mp::only_if<
			all_member_variables<MetaClass>,
			std::is_same<
				storage_class<
					mp::arg<1>
				>,
				spec_static_tag
			>
		>
	>(name_printer());
	std::cout << std::endl;
}


MIRROR_NAMESPACE_END


int main(void)
{
	using namespace mirror;
	print_static_members_of<
		reflected<test::H>::type
	>();
	print_static_members_of<
		reflected<test::I>::type
	>();
	print_static_members_of<
		reflected<test::J>::type
	>();
	//
	return 0;
}

/* Example of output:
Static members of the 'test::H' class:
Static members of the 'test::I' class: test::I::i.
Static members of the 'test::J' class: test::I::i.
*/
