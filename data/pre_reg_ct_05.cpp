/**
 * \file test/pre_reg_ct_05.cpp
 *
 * This file is part of the Mirror library testsuite.
 *
 * Testing the reflection and meta-object categorization
 * of the pre-registered native types, and some of the meta-programming tools.
 *
 *  Copyright 2008 - 2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#include <boost/test/unit_test.hpp>

#include <mirror/meta_namespace.hpp>
#include <mirror/meta_type.hpp>
#include <mirror/pre_registered/namespaces.hpp>
#include <mirror/concept_check.hpp>
#include <mirror/meta_prog/only_if.hpp>
#include <mirror/meta_prog/for_each.hpp>

struct tester
{
	template <class MetaObject>
	void operator()(MetaObject) const
	{
		using namespace mirror;
		MIRROR_ASSERT_CONCEPT(
			MetaObject,
			meta_namespace_tag
		);
	}
};

template <class MetaNamespace>
void test_namespace(void)
{
	using namespace mirror;
	MIRROR_ASSERT_CONCEPT(MetaNamespace, meta_namespace_tag);
	// get the types from the reflected namespace
	mp::for_each<
		mp::only_if<
			members<MetaNamespace>,
			mp::is_a<
				mp::arg<1>,
				meta_namespace_tag
			>
		>
	>(tester());
}

void test_main(void)
{
	using namespace mirror;
	test_namespace<MIRRORED_GLOBAL_SCOPE()>();
	test_namespace<MIRRORED_NAMESPACE(boost)>();
}

using namespace boost::unit_test;

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
	test_suite *test = BOOST_TEST_SUITE(
		"Mirror: Pre-registered namespaces compile test 05"
	);
	test->add(BOOST_TEST_CASE(&test_main));
	return test;
}


