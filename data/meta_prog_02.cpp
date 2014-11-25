/**
 *  @example mirror/example/meta_prog_02.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions for various meta-objects
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>

MIRROR_NAMESPACE_BEGIN

template <typename MetaNamespace>
void count_templates_in(void)
{
	using namespace mirror;
	// print to the output
	std::cout <<
		// the count of elements in a range such that it ...
		mp::size<
			// ...contains only MetaClassTemplates
			mp::only_if<
				members<MetaNamespace>,
				mp::is_a<
					mp::arg<1>,
					meta_type_template_tag
				>
			>
		>::value
	<< " templates in namespace '"
	<< MetaNamespace::full_name()
	<< "'"
	<< std::endl;
}


MIRROR_NAMESPACE_END


int main(void)
{
	using namespace mirror;
	count_templates_in<
		MIRRORED_GLOBAL_SCOPE()
	>();
	count_templates_in<
		MIRRORED_NAMESPACE(std)
	>();
	count_templates_in<
		MIRRORED_NAMESPACE(boost)
	>();
	//
	return 0;
}

/* Example of output:
0 templates in namespace ''
14 templates in namespace 'std'
0 templates in namespace 'boost'
*/
