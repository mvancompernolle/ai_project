/**
 *  @example mirror/example/stream_01.cpp
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
#include "./factories/tetrahedron.hpp"

int main(void)
{
	using namespace mirror;
	test::tetrahedron t(
		test::triangle(
			test::vector(),
			test::vector(1.0, 0.0, 0.0),
			test::vector(0.0, 1.0, 0.0)
		),
		test::vector(0.0, 0.0, 1.0)
	);
	auto name_maker = [](std::ostream& out){out << "a_tetrahedron";};
	// write in JSON format
	std::cout << "to_json:" << std::endl;
	std::cout << stream::to_json::from(t, name_maker) << std::endl;
	// write in XML format
	std::cout << "to_xml:" << std::endl;
	std::cout << stream::to_xml::from(t, name_maker) << std::endl;
	// write in "meta" XML format
	std::cout << "to_meta_xml:" << std::endl;
	std::cout << stream::to_meta_xml::from(t, name_maker) << std::endl;
	//
	return 0;
}

/* Example of output:
 |  to_json:
 |  "a_tetrahedron": {
 |      "base": {
 |          "a": {
 |              "x": 0,
 |              "y": 0,
 |              "z": 0
 |          },
 |          "b": {
 |              "x": 1,
 |              "y": 0,
 |              "z": 0
 |          },
 |          "c": {
 |              "x": 0,
 |              "y": 1,
 |              "z": 0
 |          }
 |      },
 |      "apex": {
 |          "x": 0,
 |          "y": 0,
 |          "z": 1
 |      }
 |  }
 |  to_xml:
 |  <a_tetrahedron type='test::tetrahedron'>
 |      <base type='test::triangle'>
 |          <a type='test::vector'>
 |              <x type='double'>0<x/>
 |              <y type='double'>0<y/>
 |              <z type='double'>0<z/>
 |          <a/>
 |          <b type='test::vector'>
 |              <x type='double'>1<x/>
 |              <y type='double'>0<y/>
 |              <z type='double'>0<z/>
 |          <b/>
 |          <c type='test::vector'>
 |              <x type='double'>0<x/>
 |              <y type='double'>1<y/>
 |              <z type='double'>0<z/>
 |          <c/>
 |      <base/>
 |      <apex type='test::vector'>
 |          <x type='double'>0<x/>
 |          <y type='double'>0<y/>
 |          <z type='double'>1<z/>
 |      <apex/>
 |  <a_tetrahedron/>
 |  to_meta_xml:
 |  <object name='a_tetrahedron' type='test::tetrahedron'>
 |      <member name='base' type='test::triangle'>
 |          <member name='a' type='test::vector'>
 |              <member name='x' type='double'>0<member/>
 |              <member name='y' type='double'>0<member/>
 |              <member name='z' type='double'>0<member/>
 |          <member/>
 |          <member name='b' type='test::vector'>
 |              <member name='x' type='double'>1<member/>
 |              <member name='y' type='double'>0<member/>
 |              <member name='z' type='double'>0<member/>
 |          <member/>
 |          <member name='c' type='test::vector'>
 |              <member name='x' type='double'>0<member/>
 |              <member name='y' type='double'>1<member/>
 |              <member name='z' type='double'>0<member/>
 |          <member/>
 |      <member/>
 |      <member name='apex' type='test::vector'>
 |          <member name='x' type='double'>0<member/>
 |          <member name='y' type='double'>0<member/>
 |          <member name='z' type='double'>1<member/>
 |      <member/>
 |  <object/>
 |
 */
