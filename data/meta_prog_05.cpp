/**
 *  @example mirror/example/meta_prog_05.cpp
 *  This example shows the usage of the meta-programming utilities
 *  and the intrinsic meta-functions for various meta-objects
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>

MIRROR_NAMESPACE_BEGIN

// Helper functor clas used by for_each_ii in print_integral_types_in
struct name_printer
{
	template <typename IterInfo>
	void operator()(IterInfo) const
	{
		typedef typename IterInfo::type MetaObject;
		// print the full name of the current element
		std::cout << MetaObject::full_name();
		// if this is the last element in the traversal
		// then print the final dot or print a separating comma
		// otherwise
		if(IterInfo::is_last::value)
			std::cout << ".";
		else	std::cout << ", ";
	}
};

template <typename MetaNamespace>
void print_integral_types_in(void)
{
	using namespace mirror;
	std::cout << "Integers in the '"
		<< MetaNamespace::full_name()
		<< "' namespace: ";
	// get the members of the passed MetaNamespace, filter only
	// the meta_types and then filter only those that reflect
	// integral types and finally for each of such meta-types
	// execute the name_printer functor
	mp::for_each_ii<
		mp::only_if<
			mp::only_if<
				members<MetaNamespace>,
				mp::is_a<
					mp::arg<1>,
					meta_type_tag
				>
			>,
			std::is_integral<
				original_type<
					mp::arg<1>
				>
			>
		>
	>(name_printer());
	std::cout << std::endl;
}


MIRROR_NAMESPACE_END


int main(void)
{
	using namespace mirror;
	print_integral_types_in<
		MIRRORED_GLOBAL_SCOPE()
	>();
	print_integral_types_in<
		MIRRORED_NAMESPACE(std)
	>();
	//
	return 0;
}

/* Example of output:
Integers in the '' namespace: bool, char, unsigned char, wchar_t, short int, int, long int, unsigned short int, unsigned int, unsigned long int.
Integers in the 'std' namespace:
*/
