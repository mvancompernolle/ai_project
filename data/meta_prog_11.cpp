/**
 *  @example mirror/example/meta_prog_11.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions for various meta-objects
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <mirror/utils/quick_reg.hpp>
#include <type_traits>
#include <iostream>

bool x = false;
int y = 123;
double z = 456.789;

namespace test {

short w = 0;
bool x = true;
long y = 2345;
float z = 6.789;

} // namespace test

MIRROR_REG_BEGIN

MIRROR_QREG_GLOBAL_SCOPE_VARIABLES((x)(y)(z))

MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(test)
MIRROR_QREG_FREE_VARIABLES(test, (w)(x)(y)(z))

MIRROR_REG_END


MIRROR_NAMESPACE_BEGIN

// Helper functor class used by for_each_ii in print_free_variables_of
struct name_printer
{
	template <typename IterInfo>
	void operator()(IterInfo) const
	{
		typedef typename IterInfo::type MetaFreeVariable;
		// print the full name of the current member variable
		std::cout << MetaFreeVariable::full_name()
			<< " = "
			<< MetaFreeVariable::get();
		// if this is the last element in the traversal
		// then print the final dot or print a separating comma
		// otherwise
		if(IterInfo::is_last::value)
			std::cout << ".";
		else	std::cout << ", ";
	}
};

template <typename MetaNamespace>
void print_free_variables_of(void)
{
	using namespace mirror;
	std::cout << "Variables in the '"
		<< MetaNamespace::full_name()
		<< "' namespace: ";
	mp::for_each_ii<
		mp::only_if<
			members<MetaNamespace>,
			mirror::mp::is_a<
				mp::arg<1>,
				meta_free_variable_tag
			>
		>
	>(name_printer());
	std::cout << std::endl;
}


MIRROR_NAMESPACE_END


int main(void)
{
	using namespace mirror;
	print_free_variables_of<
		MIRRORED_GLOBAL_SCOPE()
	>();
	print_free_variables_of<
		MIRRORED_NAMESPACE(test)
	>();
	//
	return 0;
}

/* Example of output:
Variables in the '' namespace: x = 0, y = 123, z = 456.789.
Variables in the 'test' namespace: test::w = 0, test::x = 1, test::y = 2345, test::z = 6.789.
*/
