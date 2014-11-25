/**
 *  @example mirror/example/typenames_03.cpp
 *  This example shows the usage of the reflection macros
 *  the typename getter functions (base_name/full_name/local_name)
 *  and the MIRROR_USING_NAMESPACE macro.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>

namespace test {
namespace foo {
namespace bar {

struct baz
{
	int x;
};

} // namespace bar
} // namespace foo
} // namespace test

MIRROR_REG_BEGIN

MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(test)
MIRROR_QREG_NESTED_NAMESPACE(test, foo)
MIRROR_QREG_NESTED_NESTED_NAMESPACE(test, foo, bar)

MIRROR_REG_TYPE(test::foo::bar, baz)

MIRROR_REG_END

int main(void)
{
	using namespace mirror;
	//
	typedef std::vector<test::foo::bar::baz> (*t)(std::allocator<test::foo::bar::baz> volatile * const&);
	typedef MIRRORED_TYPE(t) meta_t;
	//
	// print the base name of the type (without the nested
	// name specifiers)
	std::cout << meta_t::base_name() << std::endl;
	//
	// print the full name of the type (with all the nested
	// name specifiers)
	std::cout << meta_t::full_name() << std::endl;
	//
	// print the local_name of the type (with the nested name
	// specifiers without the parts "used" with the
	// MIRROR_USING_NAMESPACE macro)
	//
	{
		// here the local_name strips the leading std::
		// from all full typenames from the std namespace
		MIRROR_USING_NAMESPACE(std);
		std::cout << meta_t::local_name() << std::endl;
	}
	//
	{
		// here the local_name strips the leading test::
		// from all full typenames from the test namespace
		MIRROR_USING_NAMESPACE(test);
		std::cout << meta_t::local_name() << std::endl;
	}
	//
	{
		// here the local_name strips the leading std:: and test::
		// from all full typenames from the std and test namespaces
		MIRROR_USING_NAMESPACE(std);
		MIRROR_USING_NAMESPACE(test);
		std::cout << meta_t::local_name() << std::endl;
	}
	//
	{
		// strips test::foo:: from full typenames
		MIRROR_USING_NESTED_NAMESPACE(_, test::foo);
		std::cout << meta_t::local_name() << std::endl;
	}
	//
	{
		// strips test::foo::bar:: from full typenames
		MIRROR_USING_NAMESPACE_(test::foo::bar);
		std::cout << meta_t::local_name() << std::endl;
	}
	//
	return 0;
}

/* Example of output:
vector< baz, allocator< baz > > ( *)(allocator< baz > volatile * const &)
std::vector< test::foo::bar::baz, std::allocator< test::foo::bar::baz > > ( *)(std::allocator< test::foo::bar::baz > volatile * const &)
vector< test::foo::bar::baz, allocator< test::foo::bar::baz > > ( *)(allocator< test::foo::bar::baz > volatile * const &)
std::vector< foo::bar::baz, std::allocator< foo::bar::baz > > ( *)(std::allocator< foo::bar::baz > volatile * const &)
vector< foo::bar::baz, allocator< foo::bar::baz > > ( *)(allocator< foo::bar::baz > volatile * const &)
std::vector< bar::baz, std::allocator< bar::baz > > ( *)(std::allocator< bar::baz > volatile * const &)
std::vector< baz, std::allocator< baz > > ( *)(std::allocator< baz > volatile * const &)
*/
