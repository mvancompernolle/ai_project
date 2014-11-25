/**
 *  @example mirror/example/meta_prog_08.cpp
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

// Helper functor class used by for_each_ii in print_virtual_bases_of
struct name_printer
{
	template <typename IterInfo>
	void operator()(IterInfo) const
	{
		typedef typename IterInfo::type MetaInheritance;
		// print the full name of the current base class
		std::cout << MetaInheritance::base_class::full_name();
		// if this is the last element in the traversal
		// then print the final dot or print a separating comma
		// otherwise
		if(IterInfo::is_last::value)
			std::cout << ".";
		else	std::cout << ", ";
	}
};

template <typename MetaClass>
void print_virtual_bases_of(void)
{
	using namespace mirror;
	std::cout << "Virtual base classes of the '"
		<< MetaClass::full_name()
		<< "' class: ";
	mp::for_each_ii<
		mp::only_if<
			base_classes<MetaClass>,
			std::is_same<
				inheritance_type<
					mp::arg<1>
				>,
				spec_virtual_tag
			>
		>
	>(name_printer());
	std::cout << std::endl;
}


MIRROR_NAMESPACE_END


int main(void)
{
	using namespace mirror;
	print_virtual_bases_of<
		reflected<test::A>::type
	>();
	print_virtual_bases_of<
		reflected<test::B>::type
	>();
	print_virtual_bases_of<
		reflected<test::C>::type
	>();
	print_virtual_bases_of<
		reflected<test::D>::type
	>();
	print_virtual_bases_of<
		reflected<test::E>::type
	>();
	print_virtual_bases_of<
		reflected<test::F>::type
	>();
	print_virtual_bases_of<
		reflected<test::G>::type
	>();
	print_virtual_bases_of<
		reflected<test::H>::type
	>();
	//
	return 0;
}

/* Example of output:
Virtual base classes of the 'test::A' class:
Virtual base classes of the 'test::B' class:
Virtual base classes of the 'test::C' class: test::A, test::B.
Virtual base classes of the 'test::D' class: test::A, test::B.
Virtual base classes of the 'test::E' class: test::C, test::D.
Virtual base classes of the 'test::F' class: test::C, test::D.
Virtual base classes of the 'test::G' class:
Virtual base classes of the 'test::H' class:
*/
