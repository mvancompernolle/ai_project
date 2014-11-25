/**
 *  @example mirror/example/meta_prog_14.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions for various meta-objects
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>
#include <memory>

#include "./factories/person.hpp"

namespace std {

template <typename T>
struct add_shared_ptr
{
	typedef shared_ptr<T> type;
};

} // namespace std

struct name_printer
{
	template <typename MetaNamedScopedObject>
	void operator()(MetaNamedScopedObject mo) const
	{
		std::cout << mo.full_name() << std::endl;
	}
};

int main(void)
{
	using namespace mirror;
	mp::for_each<
		mp::transform<
			mp::only_if<
				members<
					reflected<
						test::person
					>::type
				>,
				mp::is_a<
					mp::arg<1>,
					meta_member_variable_tag
				>
			>,
			apply_modifier<
				type_of<
					mp::arg<1>
				>,
				mp::protect<
					std::add_shared_ptr<
						mp::arg<1>
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
 | std::shared_ptr< std::string >
 | std::shared_ptr< std::string >
 | std::shared_ptr< std::string >
 | std::shared_ptr< std::tm >
 | std::shared_ptr< test::gender >
 | std::shared_ptr< float >
 | std::shared_ptr< float >
 */
