/**
 *  @example mirror/example/various_01.cpp
 *  This example shows the usage of multiple utilities provided
 *  by Mirror.
 *
 *  More preciselly this example shows the usage of the reflection macros,
 *  the meta-object classification and tag dispatch facility,
 *  base name getter functions, the members<> meta-function and
 *  the mp::for_each run-time algorithm.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>

namespace std {

// helper class for easy indenting in standard streams
struct indent
{
	size_t amount;
	size_t spaces;

	indent(size_t a)
	 : amount(a)
	 , spaces(2)
	{ }
};

// the overload of the << operator for indentation
template <typename Out>
Out& operator << (Out& out, indent ind)
{
	size_t n = ind.amount * ind.spaces;
	while(n--) out << ' ';
	return out;
}

} // namespace std

MIRROR_NAMESPACE_BEGIN

template <typename MetaObject>
void print_info(size_t);

// Helper functor which is used together with mp::for_each
struct info_printer
{
	size_t indent;

	info_printer(size_t ind)
	 : indent(ind)
	{ }

	template <class MetaObject>
	void operator()(MetaObject) const
	{
		print_info<MetaObject>(indent);
	}
};

// Helper function printing the info about the members of a scope
template <class MetaScope>
void print_members_info(size_t indent)
{
	// get the members of the scope
	typedef typename members<MetaScope>::type mems;
	// call the info_printer on each member of the MetaScope
	mp::for_each<mems>(info_printer(indent+1));
}

// fallback function for printing info
template <class MetaObject>
void print_info(size_t indent, meta_object_tag)
{
	std::cout << std::indent(indent) << "unknown object" << ::std::endl;
}

// overload printing info about a type
template <class MetaType>
void print_info(size_t indent, meta_type_tag)
{
	std::cout << std::indent(indent) << "type "
		<< MetaType::base_name() << ::std::endl;
}

// overload printing info about a class
template <class MetaClass>
void print_info(size_t indent, meta_class_tag)
{
	std::cout << std::indent(indent) << "class "
		<< MetaClass::base_name() << ::std::endl;
}

// overload printing info about a type
template <class MetaTypedef>
void print_info(size_t indent, meta_typedef_tag)
{
	std::cout << std::indent(indent) << "typedef "
		<< MetaTypedef::base_name() << ::std::endl;
}

// overload printing info about a template type
template <class MetaType>
void print_info(size_t indent, meta_type_template_tag)
{
	std::cout << std::indent(indent) << "template "
		<< MetaType::base_name() << ::std::endl;
}

// overload printing info about a namespace
template <class MetaScope>
void print_info(size_t indent, meta_namespace_tag)
{
	std::cout << std::indent(indent) << "namespace "
		<< MetaScope::base_name() << ::std::endl
		<< std::indent(indent) << "{" << ::std::endl;
	// print information about the members of the namespace
	print_members_info<MetaScope>(indent);
	std::cout << std::indent(indent) << "}" << ::std::endl;
}

// overload printing info about the global scope
template <class MetaObject>
void print_info(size_t indent, meta_global_scope_tag)
{
	std::cout << std::indent(indent) << "the global scope" << ::std::endl
		<< std::indent(indent) << "{" << ::std::endl;
	// print information about the members of the global scope
	print_members_info<MetaObject>(indent);
	std::cout << std::indent(indent) << "}" << ::std::endl;
}

// This version of print_info calls the proper overload based on
// MetaObject's category
template <typename MetaObject>
void print_info(size_t indent)
{
	// use the classification function to get the meta-object
	// category tag and call the proper overload of the print_info
	// function
	print_info<MetaObject>(
		indent,
		categorize_meta_object(MetaObject())
	);
}

MIRROR_NAMESPACE_END

int main(void)
{
	using namespace mirror;
	//
	// reflect the global scope
	typedef MIRRORED_GLOBAL_SCOPE() meta_gs;
	// and print the info about it
	print_info<meta_gs>(0);
	//
	return 0;
}

/* Example of output:
the global scope
{
  namespace std
  {
    class string
    class wstring
    class tm
    template pair
    template tuple
    template allocator
    template equal_to
    template not_equal_to
    template less
    template greater
    template less_equal
    template greater_equal
    template vector
    template list
    template deque
    template map
    template set
  }
  namespace boost
  {
  }
  namespace mirror
  {
  }
  type void
  type bool
  type char
  type unsigned char
  type wchar_t
  type short int
  type int
  type long int
  type unsigned short int
  type unsigned int
  type unsigned long int
  type float
  type double
  type long double
}
*/
