/**
 * \file test/pre_reg_ct_02.cpp
 *
 * This file is part of the Mirror library testsuite.
 *
 * Testing the reflection and meta-object categorization
 * of the pre-registered native types, and some of the meta-programming tools.
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#include <boost/test/unit_test.hpp>

#include <mirror/meta_namespace.hpp>
#include <mirror/meta_type.hpp>
#include <mirror/pre_registered/types.hpp>
#include <mirror/meta_object_tags.hpp>
#include <mirror/meta_prog/is_a.hpp>
#include <mirror/meta_prog/only_if.hpp>
#include <mirror/meta_prog/for_each.hpp>

struct tester
{
	template <class MetaObject>
	void operator()(MetaObject) const
	{
		using namespace mirror;
		static_assert(
			mp::is_a<MetaObject, meta_type_tag>::value,
			"MetaObject must be a MetaType"
		);
	}
};

template <class MetaNamespace>
void test_namespace(void)
{
	using namespace mirror;
	static_assert(
		mp::is_a<MetaNamespace, meta_namespace_tag>::value,
		"This test requires a MetaNamespace"
	);
	// get the types from the reflected namespace
	mp::for_each<
		mp::only_if<
			members<MetaNamespace>,
			mp::is_a<
				mp::arg<1>,
				meta_type_tag
			>
		>
	>(tester());
}

void test_main(void)
{
	using namespace mirror;
	test_namespace<MIRRORED_GLOBAL_SCOPE()>();
	test_namespace<MIRRORED_NAMESPACE(std)>();
}

using namespace boost::unit_test;

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
	test_suite *test = BOOST_TEST_SUITE(
		"Mirror: Pre-registered types compile test 02"
	);
	test->add(BOOST_TEST_CASE(&test_main));
	return test;
}


