/**
 * \file test/pre_reg_ct_07.cpp
 *
 * This file is part of the Mirror library testsuite.
 *
 * Testing the reflection and meta-object categorization
 * of the pre-registered native types.
 *
 *  Copyright 2008 - 2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#include <boost/test/unit_test.hpp>

#include <mirror/pre_registered/types.hpp>
#include <mirror/meta_typedef.hpp>
#include <mirror/concept_check.hpp>

template <class MetaObject>
void do_test(void)
{
	using namespace mirror;
	MIRROR_ASSERT_CONCEPT(
		MetaObject,
		meta_typedef_tag
	);
}

void test_main(void)
{
	//TODO
	//do_test<MIRRORED_TYPEDEF(mirror, mstring)>();
}

using namespace boost::unit_test;

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
	test_suite *test = BOOST_TEST_SUITE(
		"Mirror: Pre-registered typedef compile test 07"
	);
	test->add(BOOST_TEST_CASE(&test_main));
	return test;
}


