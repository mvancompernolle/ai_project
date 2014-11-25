/**
 *  @example puddle/example/introspect_person.cpp
 *  This example shows the usage of the Puddle compile-time layer.
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <mirror/utils/quick_reg.hpp>
#include <puddle/puddle.hpp>

#include <type_traits>
#include <iostream>
#include <cassert>

namespace sample {

struct address
{
	std::string street_name;
	std::string street_number;
	std::string postal_code;
	std::string city;
	std::string state;
	std::string country;
};

enum class gender {female, male};
enum class marital_status {single, married, divorced, widowed};

struct person
{
	std::string given_name;
	std::string middle_name;
	std::string family_name;
	std::tm birth_date;
	gender sex;
	marital_status status;
	address residence;
};

} // namespace sample

MIRROR_REG_BEGIN

MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(sample)

MIRROR_QREG_POD_CLASS(
	struct, sample, address,
	(street_name)
	(street_number)
	(postal_code)
	(city)
	(state)
	(country)
)

MIRROR_QREG_ENUM(sample, gender, (female)(male))
MIRROR_QREG_ENUM(sample, marital_status, (single)(married)(divorced)(widowed))

MIRROR_QREG_POD_CLASS(
	struct, sample, person,
	(given_name)
	(middle_name)
	(family_name)
	(birth_date)
	(sex)
	(status)
	(residence)
)

MIRROR_REG_END

struct print_names
{
	template <typename MetaObject>
	void operator ()(MetaObject obj, bool first, bool last)
	{
		std::cout << obj.base_name();
		if(last) std::cout << "." << std::endl;
		else std::cout << ", ";
	}
};

int main(void)
{
	auto meta_person = puddle::reflected_type<sample::person>();
	//
	assert(meta_person.is_class());
	assert(meta_person.is_type());
	assert(meta_person.base_name() == "person");
	assert(meta_person.elaborated_type().is_struct());
	assert(meta_person.elaborated_type().keyword() == "struct");
	assert(!meta_person.member_variables().empty());
	assert(meta_person.scope().base_name() == "sample");
	assert(meta_person.scope().is_namespace());
	assert(!meta_person.scope().is_global_scope());
	assert(!meta_person.scope().members().empty());
	assert(meta_person.scope().members().size() >= 1);
	assert(meta_person.scope().scope().is_global_scope());
	assert(meta_person.scope().scope().is_namespace());
	assert(meta_person.member_variables().at_c<0>().access_type().is_public());
	assert(meta_person->birth_date().access_type().is_public());
	assert(meta_person.member_variables().at_c<1>() == meta_person->middle_name());
	assert(meta_person.member_variables().at_c<1>() != meta_person->family_name());
	assert(meta_person->residence().type()->country().type().scope().base_name() == "std");
	assert(meta_person->residence().type()->postal_code().type().base_name() == "string");
	//
	assert(meta_person.apply<std::is_class>());
	assert(meta_person.apply<std::is_compound>());
	assert(!meta_person.apply<std::is_fundamental>());
	assert(meta_person->sex().type().apply<std::is_enum>());
	//
	auto meta_address = meta_person->residence().type();
	assert(meta_address.is_class());
	assert(!meta_address.elaborated_type().is_class());
	assert(meta_address.elaborated_type().is_struct());
	assert(meta_address.scope() == meta_person.scope());
	assert(meta_address.scope() == meta_person->status().type().scope());
	assert(meta_address.scope() != meta_person->given_name().type().scope());
	//
	auto meta_status = meta_person->status().type();
	assert(meta_status.is_enum());
	assert(meta_status.is_scope());
	assert(!meta_status.is_namespace());
	assert(meta_status.scope() == meta_person.scope());
	assert(!meta_status.enum_values().empty());
	assert(meta_status.enum_values().size() > 2);
	//
	meta_status.enum_values().for_each(print_names());
	meta_person->sex().type().enum_values().for_each(print_names());
	meta_person.member_variables().for_each(print_names());
	meta_address.member_variables().for_each(print_names());
	//
	sample::person p = {
		"Johnny", "B.", "Goode",
		{0, 30, 6, 1, 2, 1910},
		sample::gender::male, sample::marital_status::single,
		{"Willow st.", "1", "70080", "Paradis", "Louisiana", "U.S.A."}
	};
	//
	std::cout
		<< meta_person->given_name().get(p)
		<< " from "
		<< meta_address->state().get(meta_person->residence().get(p))
		<< std::endl;
	return 0;
}

/* Example of output:
single, married, divorced, widowed.
female, male.
given_name, middle_name, family_name, birth_date, sex, status, residence.
street_name, street_number, postal_code, country.
Johnny from Louisiana
 */
