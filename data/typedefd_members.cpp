/**
 *  @example  mirror/example/typedefd_members.cpp
 *  This example shows the registering of member variables with typedefined
 *  types and the use of the member_variables template meta-function.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <mirror/using_directive.hpp>
#include <iostream>

namespace test {

// use typedef to define some types
typedef double kilogram;
typedef size_t centimeter;
typedef size_t year;

// a test class using typedefs for member variable types
struct person
{
	std::string first_name;
	std::string middle_name;
	std::string family_name;

	kilogram weight;
	centimeter height;
	year age;
};

} // namespace test

MIRROR_REG_BEGIN

// register the test namespace
MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(test)

// register the typedefs with Mirror
MIRROR_REG_TYPEDEF(test, kilogram)
MIRROR_REG_TYPEDEF(test, centimeter)
MIRROR_REG_TYPEDEF(test, year)

// get the typedefs
typedef MIRROR_TYPEDEF(test, kilogram) _typedef_kilogram;
typedef MIRROR_TYPEDEF(test, centimeter) _typedef_centimeter;
typedef MIRROR_TYPEDEF(test, year) _typedef_year;

// register the person class
MIRROR_REG_CLASS_BEGIN(struct, test, person)
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR( _, _, _, first_name)
	MIRROR_REG_CLASS_MEM_VAR( _, _, _, middle_name)
	MIRROR_REG_CLASS_MEM_VAR( _, _, _, family_name)
	MIRROR_REG_CLASS_MEM_VAR( _, _, _typedef_kilogram, weight)
	MIRROR_REG_CLASS_MEM_VAR( _, _, _typedef_centimeter, height)
	MIRROR_REG_CLASS_MEM_VAR( _, _, _typedef_year, age)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END


MIRROR_REG_END

struct info_printer
{
	template <class IterInfo>
	void operator()(IterInfo)
	{
		using namespace mirror;
		if(IterInfo::is_first::value)
			std::cout << "(" << std::endl;
		std::cout << "\t"
			<< IterInfo::type::type::local_name()
			<< " "
			<< IterInfo::type::base_name();
		if(IterInfo::is_last::value)
			std::cout << std::endl << ")" << std::endl;
		else std::cout << ", " << std::endl;
	}
};

template <class Class>
void print_info(void)
{
	using namespace mirror;
	typedef MIRRORED_CLASS(Class) meta_X;
	std::cout << "Members of "
		<< meta_X::local_name()
		<< ": ";
	mp::for_each_ii< all_member_variables<meta_X> >(info_printer());
}

int main(void)
{
	using namespace mirror;
	//
	// print with the full names
	print_info<test::person>();
	//
	// print with the test:: name specifier stripped
	{
		MIRROR_USING_NAMESPACE(test);
		print_info<test::person>();
	}
	//
	// print with the test:: and the std::
	// name specifiers stripped
	{
		MIRROR_USING_NAMESPACE(test);
		MIRROR_USING_NAMESPACE(std);
		print_info<test::person>();
	}
	//
	return 0;
}

/* Example of output:
 |  Members of test::person: (
 |          std::string first_name,
 |          std::string middle_name,
 |          std::string family_name,
 |          test::kilogram weight,
 |          test::centimeter height,
 |          test::year age
 |  )
 |  Members of person: (
 |          std::string first_name,
 |          std::string middle_name,
 |          std::string family_name,
 |          kilogram weight,
 |          centimeter height,
 |          year age
 |  )
 |  Members of person: (
 |          std::string first_name,
 |          std::string middle_name,
 |          std::string family_name,
 |          kilogram weight,
 |          centimeter height,
 |          year age
 |  )
 */
