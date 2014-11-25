/**
 *  @example mirror/example/stream_02.cpp
 *  This example shows the usage of some of the reflection-based
 *  wrapper classes which allow to write instances of arbitrary
 *  reflectible class to standard output streams
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <mirror/stream.hpp>
#include <iostream>
#include "./factories/person.hpp"

int main(void)
{
	using namespace mirror;
	test::person p(
		"Johnny",
		"B.",
		"Goode",
		{0,0,12,15,7,10},
		test::gender::male,
		75,
		170
	);
	auto name_maker = [](std::ostream& out){out << "a_person";};
	// write in JSON format
	std::cout << "to_json:" << std::endl;
	std::cout << stream::to_json::from(p, name_maker) << std::endl;
	// write in XML format
	std::cout << "to_xml:" << std::endl;
	std::cout << stream::to_xml::from(p, name_maker) << std::endl;
	// write in "meta" XML format
	std::cout << "to_meta_xml:" << std::endl;
	std::cout << stream::to_meta_xml::from(p, name_maker) << std::endl;
	//
	return 0;
}

/* Example of output:
 |  to_json:
 |  "a_person": {
 |      "first_name": "Johnny",
 |      "middle_name": "B.",
 |      "family_name": "Goode",
 |      "birth_date": {
 |          "tm_sec": 0,
 |          "tm_min": 0,
 |          "tm_hour": 12,
 |          "tm_mday": 15,
 |          "tm_mon": 7,
 |          "tm_year": 10,
 |          "tm_wday": 0,
 |          "tm_yday": 0,
 |          "tm_isdst": 0
 |      },
 |      "sex": "male",
 |      "weight": 75,
 |      "height": 170
 |  }
 |  to_xml:
 |  <a_person type='test::person'>
 |      <first_name type='std::string'>Johnny<first_name/>
 |      <middle_name type='std::string'>B.<middle_name/>
 |      <family_name type='std::string'>Goode<family_name/>
 |      <birth_date type='std::tm'>
 |          <tm_sec type='int'>0<tm_sec/>
 |          <tm_min type='int'>0<tm_min/>
 |          <tm_hour type='int'>12<tm_hour/>
 |          <tm_mday type='int'>15<tm_mday/>
 |          <tm_mon type='int'>7<tm_mon/>
 |          <tm_year type='int'>10<tm_year/>
 |          <tm_wday type='int'>0<tm_wday/>
 |          <tm_yday type='int'>0<tm_yday/>
 |          <tm_isdst type='int'>0<tm_isdst/>
 |      <birth_date/>
 |      <sex type='test::gender'>male<sex/>
 |      <weight type='float'>75<weight/>
 |      <height type='float'>170<height/>
 |  <a_person/>
 |  to_meta_xml:
 |  <object name='a_person' type='test::person'>
 |      <member name='first_name' type='std::string'>Johnny<member/>
 |      <member name='middle_name' type='std::string'>B.<member/>
 |      <member name='family_name' type='std::string'>Goode<member/>
 |      <member name='birth_date' type='std::tm'>
 |          <member name='tm_sec' type='int'>0<member/>
 |          <member name='tm_min' type='int'>0<member/>
 |          <member name='tm_hour' type='int'>12<member/>
 |          <member name='tm_mday' type='int'>15<member/>
 |          <member name='tm_mon' type='int'>7<member/>
 |          <member name='tm_year' type='int'>10<member/>
 |          <member name='tm_wday' type='int'>0<member/>
 |          <member name='tm_yday' type='int'>0<member/>
 |          <member name='tm_isdst' type='int'>0<member/>
 |      <member/>
 |      <member name='sex' type='test::gender'>male<member/>
 |      <member name='weight' type='float'>75<member/>
 |      <member name='height' type='float'>170<member/>
 |  <object/>
 */
