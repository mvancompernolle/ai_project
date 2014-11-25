/**
 *  @example mirror/example/meta_prog_12.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions for various meta-objects
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <mirror/ct_string.hpp>
#include <iostream>

struct name_printer
{
	template <typename MetaNamedObject>
	void operator()(MetaNamedObject mo) const
	{
		std::cout << mo.base_name() << std::endl;
	}
};

int main(void)
{
	using namespace mirror;
	std::cout << "Types having 'int' in their name:" << std::endl;
	mp::for_each<
		mp::only_if<
			mp::only_if<
				members<
					MIRRORED_GLOBAL_SCOPE()
				>,
				mp::is_a<
					mp::arg<1>,
					meta_type_tag
				>
			>,
			cts::contains<
				static_name<
					mp::arg<1>
				>,
				cts::string<'i','n','t'>
			>
		>
	>(name_printer());
	std::cout << std::endl;
	//
	std::cout << "Types having 'long' in their name:" << std::endl;
	mp::for_each<
		mp::only_if<
			members<
				MIRRORED_GLOBAL_SCOPE()
			>,
			mp::and_<
				mp::is_a<
					mp::arg<1>,
					meta_type_tag
				>,
				cts::contains<
					static_name<
						mp::arg<1>
					>,
					cts::string<'l','o','n','g'>
				>
			>
		>
	>(name_printer());
	//
	return 0;
}

/* Example of output:
 |
 | Types having 'int' in their name:
 | short int
 | int
 | long int
 | unsigned short int
 | unsigned int
 | unsigned long int
 |
 | Types having 'long' in their name:
 | long int
 | unsigned long int
 | long double
*/
