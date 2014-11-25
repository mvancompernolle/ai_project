/**
 * \file test/pre_reg_ct_01.cpp
 *
 * This file is part of the Mirror library testsuite.
 *
 * Testing the reflection and meta-object categorization
 * of the pre-registered native types.
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>
#include <boost/test/unit_test.hpp>

#include <mirror/meta_type.hpp>
#include <mirror/pre_registered/types.hpp>
#include <mirror/meta_object_tags.hpp>
#include <mirror/meta_prog/is_a.hpp>

template <class MetaObject>
void do_test(void)
{
	using namespace mirror;
	static_assert(
		mp::is_a<MetaObject, meta_type_tag>::value,
		"MetaObject must be a MetaType"
	);
}

void test_main(void)
{
	do_test<MIRRORED_TYPE(bool)>();
	do_test<MIRRORED_TYPE(char)>();
	do_test<MIRRORED_TYPE(unsigned char)>();
	do_test<MIRRORED_TYPE(wchar_t)>();
	do_test<MIRRORED_TYPE(short)>();
	do_test<MIRRORED_TYPE(int)>();
	do_test<MIRRORED_TYPE(long)>();
	do_test<MIRRORED_TYPE(unsigned short)>();
	do_test<MIRRORED_TYPE(unsigned int)>();
	do_test<MIRRORED_TYPE(unsigned long)>();
	do_test<MIRRORED_TYPE(float)>();
	do_test<MIRRORED_TYPE(double)>();
	do_test<MIRRORED_TYPE(std::string)>();
	do_test<MIRRORED_TYPE(std::wstring)>();
	//do_test<MIRRORED_TYPEDEF(mirror, mstring)>();
}

using namespace boost::unit_test;

test_suite* init_unit_test_suite( int argc, char* argv[] )
{
	test_suite *test = BOOST_TEST_SUITE(
		"Mirror: Pre-registered types compile test 01"
	);
	test->add(BOOST_TEST_CASE(&test_main));
	return test;
}


