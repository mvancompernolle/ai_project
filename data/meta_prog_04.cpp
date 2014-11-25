/**
 *  @example mirror/example/meta_prog_04.cpp
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

// see also mirror/example/meta_prog_01.cpp for an alternate implementation
template <typename MetaNamespace>
void count_integral_types_in(void)
{
	using namespace mirror;
	// Get the members of the MetaNamespace, filter only those which
	// are meta-types and get the type reflected by each meta-type
	// and then filter only those types that are integral
	// finally get the size of the resulting range
	std::cout <<
		mp::size<
			mp::only_if<
				mp::transform<
					mp::only_if<
						members<MetaNamespace>,
						mp::is_a<
							mp::arg<1>,
							meta_type_tag
						>
					>,
					original_type<
						mp::arg<1>
					>
				>,
				std::is_integral< mp::arg<1> >
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
