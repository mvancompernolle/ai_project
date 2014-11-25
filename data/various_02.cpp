/**
 *  @example mirror/example/various_02.cpp
 *  This example shows the usage of multiple utilities provided by Mirror.
 *
 *  More preciselly this example shows the usage of the reflection macros,
 *  base and full name getter functions, the scope member typedef of
 *  MetaScopedObject(s) and the meta-meta-objects
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>

MIRROR_NAMESPACE_BEGIN

template <typename MetaObject>
void print_info(void)
{
	std::cout
		<< MIRRORED_META_OBJECT(MetaObject)::construct_name()
		<< " "
		<< MetaObject::base_name()
		<< std::endl;
}

MIRROR_NAMESPACE_END

int main(void)
{
	using namespace mirror;
	//
	print_info<MIRRORED_NAMESPACE(std)>();
	print_info<MIRRORED_NAMESPACE(boost)>();
	print_info<MIRRORED_NAMESPACE(mirror)>();
	//
	print_info<MIRRORED_TYPE(int)>();
	print_info<MIRRORED_TYPE(int)::scope>();
	print_info<MIRRORED_TYPE(std::wstring)>();
	print_info<MIRRORED_TYPE(std::wstring)::scope>();
	//
	print_info<MIRRORED_TEMPLATE(std::pair)>();
	print_info<MIRRORED_TEMPLATE(std::tuple)>();
	print_info<MIRRORED_TEMPLATE(std::tuple)::scope>();
	//
	return 0;
}

/* Example of output:
namespace std
namespace boost
namespace mirror
type int
global scope
type wstring
namespace std
template pair
template tuple
namespace std
*/
