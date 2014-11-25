/**
 *  @example mirror/example/meta_prog_13.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions for various meta-objects
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
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
	mp::for_each<
		mp::transform<
			mp::only_if<
				members<
					MIRRORED_GLOBAL_SCOPE()
				>,
				mp::is_a<
					mp::arg<1>,
					meta_type_tag
				>
			>,
			apply_modifier<
				mp::arg<1>,
				mp::protect<
					std::add_pointer<
						std::add_const<
							mp::arg<1>
						>
					>
				>
			>
		>
	>(name_printer());
	std::cout << std::endl;
	return 0;
}

/* Example of output:
 |
 | void const *
 | bool const *
 | char const *
 | unsigned char const *
 | wchar_t const *
 | short int const *
 | int const *
 | long int const *
 | unsigned short int const *
 | unsigned int const *
 | unsigned long int const *
 | float const *
 | double const *
 | long double const *
 */
