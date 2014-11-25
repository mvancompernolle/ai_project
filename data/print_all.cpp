/**
 *  @example rubber/example/print_all.cpp
 *  This example shows the usage of the type erasures for compile-time
 *  Mirror's meta-object with the for-each algorithm and lambda functions
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <rubber/rubber.hpp>
#include <iostream>
#include <vector>

int main(void)
{
	using namespace rubber;
	mirror::mp::for_each<
		mirror::members<
			MIRRORED_GLOBAL_SCOPE()
		>
	>(
		eraser<meta_scope, meta_type, meta_named_object>(
			[](const meta_scope& scope)
			{
				std::cout <<
					scope.self().construct_name() <<
					" '" <<
					scope.base_name() <<
					"', number of members = " <<
					scope.members().size() <<
					std::endl;

			},
			[](const meta_type& type)
			{
				std::cout <<
					type.self().construct_name() <<
					" '" <<
					type.base_name() <<
					"', size in bytes = " <<
					type.sizeof_() <<
					std::endl;
			},
			[](const meta_named_object& named)
			{
				std::cout <<
					named.self().construct_name() <<
					" '" <<
					named.base_name() <<
					"'" <<
					std::endl;
			}
		)
	);
	return 0;
}

/* Example of output:
namespace 'std', number of members = 20
namespace 'boost', number of members = 0
namespace 'mirror', number of members = 0
type 'void', size in bytes = 0
type 'bool', size in bytes = 1
type 'char', size in bytes = 1
type 'unsigned char', size in bytes = 1
type 'wchar_t', size in bytes = 4
type 'short int', size in bytes = 2
type 'int', size in bytes = 4
type 'long int', size in bytes = 8
type 'unsigned short int', size in bytes = 2
type 'unsigned int', size in bytes = 4
type 'unsigned long int', size in bytes = 8
type 'float', size in bytes = 4
type 'double', size in bytes = 8
type 'long double', size in bytes = 16
*/

