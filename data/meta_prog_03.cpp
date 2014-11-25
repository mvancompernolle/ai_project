/**
 *  @example mirror/example/meta_prog_03.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions for various meta-objects
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>

MIRROR_NAMESPACE_BEGIN

struct info_printer
{
	template <typename MetaObject>
	void operator()(MetaObject mo) const
	{
		MIRRORED_META_OBJECT(MetaObject) mmo;
		std::cout
			<< mmo.construct_name()
			<< ": "
			<< mo.full_name()
			<< std::endl;
	}
};

MIRROR_NAMESPACE_END


int main(void)
{
	using namespace mirror;
	// print the name of each of the members of the global scope
	mirror::mp::for_each<
		members<
			MIRRORED_GLOBAL_SCOPE()
		>
	>(info_printer());
	// print the name of each of the members of the std namespace
	mp::for_each<
		members<
			MIRRORED_NAMESPACE(std)
		>
	>(info_printer());
	//
	return 0;
}

/* Example of output:
namespace: std
namespace: boost
type: void
type: bool
type: char
type: unsigned char
type: wchar_t
type: short int
type: int
type: long int
type: unsigned short int
type: unsigned int
type: unsigned long int
type: float
type: double
type: long double
class: std::string
class: std::wstring
class: std::tm
template: std::pair
template: std::tuple
template: std::allocator
template: std::equal_to
template: std::not_equal_to
template: std::less
template: std::greater
template: std::less_equal
template: std::greater_equal
template: std::vector
template: std::list
template: std::deque
template: std::map
template: std::set
*/
