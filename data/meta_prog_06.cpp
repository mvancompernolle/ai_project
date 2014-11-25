/**
 *  @example mirror/example/meta_prog_06.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions and tagging with members
 *  of a namespace.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>
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
		// print the full name of the current element
		std::cout << MetaObject::full_name();
		// if this is the last element in the traversal
		// then print the final dot or print a separating comma
		// otherwise
		if(IterInfo::is_last::value)
			std::cout << ".";
		else	std::cout << ", ";
	}
};

template <typename MetaNamespace>
void print_public_types_in(void)
{
	using namespace mirror;
	std::cout << "Public types in the '"
		<< MetaNamespace::full_name()
		<< "' namespace: ";
	// Get the members of the passed MetaNamespace, filter only
	// meta-types and then filter only those that lack the 'hidden' tag
	mirror::mp::for_each_ii<
		mp::only_if<
			mp::only_if<
				members<MetaNamespace>,
				mp::is_a<
					mp::arg<1>,
					meta_type_tag
				>
			>,
			mp::lacks<
				tags<
					mp::arg<1>
				>,
				MIRROR_OBJECT_TAG(hidden)
			>
		>
	>(name_printer());
	std::cout << std::endl;
}


MIRROR_NAMESPACE_END


int main(void)
{
	using namespace mirror;
	print_public_types_in<
		MIRRORED_NAMESPACE(std)
	>();
	print_public_types_in<
		MIRRORED_NAMESPACE(test)
	>();
	//
	return 0;
}

/* Example of output:
Public types in the 'std' namespace: std::string, std::wstring, std::tm.
Public types in the 'test' namespace: test::C, test::D, test::E, test::F, test::G, test::H, test::Ch.
*/
