/**
 * \file test/namespaces_ct_01.cpp
 *
 * This file is part of the Mirror library testsuite.
 *
 * Testing namespaces registration and reflection.
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#include <boost/test/unit_test.hpp>

#include <mirror/meta_object_tags.hpp>
#include <mirror/meta_prog/is_a.hpp>

#include "./namespaces.hpp"

void test_global_scope(void)
{
	using namespace ::std;
	using namespace mirror;
	//
	typedef MIRRORED_GLOBAL_SCOPE() meta_gs;
	static_assert(
		mp::is_a<meta_gs, meta_namespace_tag>::value,
		"meta_gs must reflect a namespace"
	);
	static_assert(
		mp::is_a<meta_gs, meta_global_scope_tag>::value,
		"meta_gs must reflect the global scope"
	);

}

void test_foo(void)
{
	using namespace ::std;
	using namespace mirror;
	//
	typedef MIRRORED_NAMESPACE(foo) meta_foo;
	static_assert(
		mp::is_a<meta_foo, meta_namespace_tag>::value,
		"meta_foo must reflect a namespace"
	);
	static_assert(
		mp::is_not_a<meta_foo, meta_global_scope_tag>::value,
		"meta_foo must not reflect the global scope"
	);
	static_assert(
		mp::is_a<meta_foo::scope, meta_global_scope_tag>::value,
		"meta_foo's scope must reflect the global scope"
	);

}

void test_bar(void)
{
	using namespace ::std;
	using namespace mirror;
	//
	typedef MIRRORED_NAMESPACE(foo::bar) meta_bar;
	static_assert(
		mp::is_a<meta_bar, meta_namespace_tag>::value,
		"meta_bar must reflect a namespace"
	);
	static_assert(
		mp::is_not_a<meta_bar, meta_global_scope_tag>::value,
		"meta_bar must not reflect the global scope"
	);
	static_assert(
		mp::is_not_a<meta_bar::scope, meta_global_scope_tag>::value,
		"meta_bar's scope must not reflect the global scope"
	);
	static_assert(
		mp::is_a<meta_bar::scope::scope, meta_global_scope_tag>::value,
		"meta_bar scope's scope must reflect the global scope"
	);
}

void test_baz(void)
{
	using namespace ::std;
	using namespace mirror;
	//
	typedef MIRRORED_NAMESPACE(foo::bar::baz) meta_baz;
	static_assert(
		mp::is_a<meta_baz, meta_namespace_tag>::value,
		"meta_baz must reflect a namespace"
	);
	static_assert(
		mp::is_not_a<meta_baz, meta_global_scope_tag>::value,
		"meta_baz must not reflect the global scope"
	);
	static_assert(
		mp::is_a<meta_baz::scope, meta_namespace_tag>::value,
		"meta_baz's scope must reflect a namespace"
	);
	static_assert(
		mp::is_not_a<meta_baz::scope, meta_global_scope_tag>::value,
		"meta_baz's scope must not reflect the global scope"
	);
}

using namespace boost::unit_test;

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
    test_suite *test = BOOST_TEST_SUITE("Mirror: namespaces compile test 01");
    test->add(BOOST_TEST_CASE(&test_global_scope));
    test->add(BOOST_TEST_CASE(&test_foo));
    test->add(BOOST_TEST_CASE(&test_bar));
    test->add(BOOST_TEST_CASE(&test_baz));
    return test;
}


