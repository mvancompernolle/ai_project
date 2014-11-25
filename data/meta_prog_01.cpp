/**
 *  @example mirror/example/meta_prog_01.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions for various meta-objects
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>

MIRROR_NAMESPACE_BEGIN

template <typename MetaNamespace>
void count_integral_types_in(void)
{
	using namespace mirror;
	// print to the output
	std::cout <<
		// the count of elements in a range such that it ...
		mp::size<
			// ...contains only meta-types reflecting integral types
			mp::only_if<
				// from a range of all members of the namespace
				// such that they satisfy the MetaType concept
				mp::only_if<
					members<MetaNamespace>,
					mp::is_a<
						mp::arg<1>,
						meta_type_tag
					>
				>,
				std::is_integral<
					original_type<
						mp::arg<1>
					>
				>
			>
		>::value
	<< " integral types in namespace '"
	<< MetaNamespace::full_name()
	<< "'"
	<< std::endl;
}


MIRROR_NAMESPACE_END


int main(void)
{
	using namespace mirror;
	count_integral_types_in<
		MIRRORED_GLOBAL_SCOPE()
	>();
	count_integral_types_in<
		MIRRORED_NAMESPACE(std)
	>();
	//
	return 0;
}

/* Example of output:
10 integral types in namespace ''
0 integral types in namespace 'std'
*/
