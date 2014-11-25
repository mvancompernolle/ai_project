/**
 *  @example mirror/example/typenames_01.cpp
 *  This example shows the usage of the reflection macros
 *  and the full_name member function of MetaNamedScopedObject(s)
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>

/**
 */

MIRROR_NAMESPACE_BEGIN

template <typename MetaType>
void print_info(void)
{
	using ::std::cout;
	using ::std::endl;
	cout << MetaType::full_name() << endl;
}

MIRROR_NAMESPACE_END

int main(void)
{
	using namespace mirror;
	//
	// print info about individual types
	print_info<MIRRORED_TYPE(::std::wstring)>();
	print_info<MIRRORED_TYPE(long double)>();
	print_info<MIRRORED_TYPE(long double****)>();
	print_info<MIRRORED_TYPE(long double&&)>();
	print_info<MIRRORED_TYPE(const ::std::wstring&)>();
	print_info<MIRRORED_TYPE(const long double* const *)>();
	print_info<MIRRORED_TYPE(int volatile * const * [][9][8][7][6][5][4][3][2][1])>();
	print_info<MIRRORED_TYPE(double * const (*)(bool, char, int))>();
	//
	// cooperation with decltype
	print_info<MIRRORED_TYPE(decltype(1+1))>();
	print_info<MIRRORED_TYPE(decltype(1.0+1.0))>();
	print_info<MIRRORED_TYPE(decltype(std::string("1")+std::string("1")))>();
	//
	return 0;
}

/* Example of output:
std::wstring
long double
long double * * * *
long double &&
std::wstring const &
long double const * const *
int volatile * const * [][9][8][7][6][5][4][3][2][1]
double * const ( *)(bool, char, int)
int
double
std::string
*/
