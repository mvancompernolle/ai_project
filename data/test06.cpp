/**
 *  Copyright 2009-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *  WARNING: this file is here for development / testing purposes only
 *  and its contents and input / output can change witout any prior
 *  notice.
 */

#include <mirror/mirror.hpp>
#include <mirror/stream.hpp>
#include <mirror/utils/quick_reg.hpp>
#include <iostream>
//

namespace test {

class foo
{
public:
	int a;
	int b;
	char c;
};

struct bar
{
	double d, e;
	float f;
};

class foobar : public foo, public bar
{
private:
	MIRROR_FRIENDLY_CLASS(foobar);
	short g,h;
public:
	foobar(void)
	 : g(0)
	 , h(h)
	{
		a = b = 0;
		c = '\0';
		d = e = 0.0;
		f = 0.0f;
	}
};

} // namespace test

MIRROR_NAMESPACE_BEGIN

/*
*/


MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(test)

MIRROR_QREG_POD_CLASS(class,  test, foo, (a)(b)(c))
MIRROR_QREG_POD_CLASS(struct, test, bar, (d)(e)(f))

MIRROR_QREG_CLASS(class, test, foobar, (
	(test::foo)(test::bar),
	(g)(h)
))

MIRROR_NAMESPACE_END

struct name_printer
{
	template <typename MetaObject>
	void operator()(MetaObject mo) const
	{
		std::cout << mo.base_name() << std::endl;
	}
};

int main(void)
{
	try
	{
		using namespace mirror;
		//
		static_assert(
			mp::size<member_variables<
				MIRRORED_CLASS(test::foo)
			> >::value == 3, "test::foo reg error"
		);
		static_assert(
			mp::size<member_variables<
				MIRRORED_CLASS(test::bar)
			> >::value == 3, "test::bar reg error"
		);
		static_assert(
			mp::size<base_classes<
				MIRRORED_CLASS(test::foobar)
			> >::value == 2, "test::foobar reg error"
		);
		//
		std::cout << reflected<mp::apply<
			std::add_pointer<mp::arg<1> >,
			original_type<MIRRORED_CLASS(test::foobar)>::type
		>::type>::type::base_name() << std::endl;
		//
		std::cout << apply_modifier<
			MIRRORED_CLASS(test::bar),
			std::add_pointer<std::add_const<std::add_pointer<mp::arg<1>>>>
		>::type::base_name() << std::endl;
		//
		mp::for_each<
			mp::transform<
				mp::only_if<
					members<MIRRORED_GLOBAL_SCOPE()>,
					mp::is_a<
						mp::arg<1>,
						meta_type_tag
					>
				>,
				apply_modifier<
					mp::arg<1>,
					mp::protect<std::add_pointer<mp::arg<1>>>
				>
			>
		>(name_printer());
	}
	catch(const std::exception& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	//
	return 0;
}

