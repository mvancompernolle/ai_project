/**
 *  @example mirror/example/typenames_02.cpp
 *  This example shows the usage of the reflection macros
 *  and the full_name member function of MetaNamedScopedObject(s)
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>


int main(void)
{
	using namespace mirror;
	//
	typedef MIRRORED_TEMPLATE(std::pair) meta_pair;
	typedef meta_pair::reflected_template<
		std::string,
		std::wstring
	>::type p;
	typedef MIRRORED_TYPE(p) meta_p;
	//
	std::cout << meta_pair::base_name() << ::std::endl;
	std::cout << meta_pair::full_name() << ::std::endl;
	//
	std::cout << meta_p::base_name() << ::std::endl;
	std::cout << meta_p::full_name() << ::std::endl;
	//
	typedef MIRRORED_TEMPLATE(std::tuple) meta_tuple;
	typedef meta_tuple::reflected_template<
		std::string,
		std::wstring,
		bool,
		long,
		double,
		void * const
	>::type t;
	typedef MIRRORED_TYPE(t) meta_t;
	typedef MIRRORED_TYPE(t volatile * const *) meta_t_pp;
	//
	std::cout << meta_tuple::base_name() << ::std::endl;
	std::cout << meta_tuple::full_name() << ::std::endl;
	//
	std::cout << meta_t::base_name() << ::std::endl;
	std::cout << meta_t::full_name() << ::std::endl;
	//
	std::cout << meta_t_pp::base_name() << ::std::endl;
	std::cout << meta_t_pp::full_name() << ::std::endl;
	//
	return 0;
}

/* Example of output:
pair
std::pair
pair< string, wstring >
std::pair< std::string, std::wstring >
tuple
std::tuple
tuple< string, wstring, bool, long int, double, void * const >
std::tuple< std::string, std::wstring, bool, long int, double, void * const >
tuple< string, wstring, bool, long int, double, void * const > volatile * const *
std::tuple< std::string, std::wstring, bool, long int, double, void * const > volatile * const *
*/
