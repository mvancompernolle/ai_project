/**
 * \file test/namespaces_ct_02.cpp
 *
 * This file is part of the Mirror library testsuite.
 *
 * Testing namespaces registration and reflection.
 *
 *  Copyright 2008 - 2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#include <boost/test/unit_test.hpp>
#include <mirror/concept_check.hpp>

#include "./namespaces.hpp"

void test_main(void)
{
	using namespace ::std;
	using namespace mirror;
	//
	MIRROR_ASSERT_CONCEPT(
		MIRRORED_GLOBAL_SCOPE(),
		meta_namespace_tag
	);
	MIRROR_ASSERT_CONCEPT(
		MIRRORED_GLOBAL_SCOPE(),
		meta_global_scope_tag
	);
	MIRROR_ASSERT_CONCEPT(
		MIRRORED_NAMESPACE(foo),
		meta_namespace_tag
	);
	MIRROR_ASSERT_CONCEPT(
		MIRRORED_NAMESPACE(foo)::scope,
		meta_global_scope_tag
	);
	MIRROR_ASSERT_CONCEPT(
		MIRRORED_NAMESPACE(foo::bar),
		meta_namespace_tag
	);
	MIRROR_ASSERT_CONCEPT(
		MIRRORED_NAMESPACE(foo::bar)::scope,
		meta_namespace_tag
	);
	MIRROR_ASSERT_CONCEPT(
		MIRRORED_NAMESPACE(foo::bar::baz),
		meta_namespace_tag
	);
	MIRROR_ASSERT_CONCEPT(
		MIRRORED_NAMESPACE(foo::bar::baz::foobar),
		meta_namespace_tag
	);
}

using namespace boost::unit_test;

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
    test_suite *test = BOOST_TEST_SUITE("Mirror: namespaces compile test 02");
    test->add(BOOST_TEST_CASE(&test_main));
    return test;
}


