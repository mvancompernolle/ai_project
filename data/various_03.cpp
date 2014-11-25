/**
 *  @example mirror/example/various_03.cpp
 *  This example shows the usage of the registering macros, reflection
 *  macros, meta-programming utilities and various parts of the meta-objects'
 *  interface.
 *
 *  Copyright 2008-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror.hpp>
#include <iostream>


namespace test {
namespace foo {

struct A
{
	int a0;
	static int a1;
	mutable volatile int a2;
};

class B
{
MIRROR_FRIENDLY_CLASS(B);
	long b0;
	long* b1;
	const long * b2;
protected:
	long (*b3)(int);
	decltype(b3) (*b4)(bool, char, wchar_t);
	long * volatile * b5;
public:
	long b6;
	long b7;
	long b8;
};

struct C : A, B
{
	char c0;
	char c1;
	char c2;
	char c3;
};

} // namespace foo
} // namespace test

MIRROR_REG_BEGIN

// register the global scope 'test' namespace
MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(test)
// register the nested 'test::foo' namespace
MIRROR_QREG_NESTED_NAMESPACE(test, foo)

// register the test::foo::A struct
MIRROR_REG_CLASS_BEGIN(struct, test::foo, A)
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, a0)
	MIRROR_REG_CLASS_MEM_VAR(_, static, _, a1)
	MIRROR_REG_CLASS_MEM_VAR(_, mutable, _, a2)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

// register the test::foo::B class
MIRROR_REG_CLASS_BEGIN(class, test::foo, B)
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, b0)
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, b1)
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, b2)
	MIRROR_REG_CLASS_MEM_VAR(protected, _, _, b3)
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, b4)
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, b5)
	MIRROR_REG_CLASS_MEM_VAR(public, _, _, b6)
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, b7)
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, b8)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END


// register the test::foo::C class
MIRROR_REG_CLASS_BEGIN(struct, test::foo, C)
MIRROR_REG_BASE_CLASSES_BEGIN
	MIRROR_REG_BASE_CLASS(_, _, test::foo::A),
	MIRROR_REG_BASE_CLASS(_, _, test::foo::B)
MIRROR_REG_BASE_CLASSES_END
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, char, c0)
	MIRROR_REG_CLASS_MEM_VAR(_, _, char, c1)
	MIRROR_REG_CLASS_MEM_VAR(_, _, char, c2)
	MIRROR_REG_CLASS_MEM_VAR(_, _, char, c3)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

template <typename MetaObject>
void print_info(MetaObject mo);

template <typename MetaObject>
void print_info(MetaObject, meta_member_variable_tag)
{
	std::cout
		<< MetaObject::access_type::keyword()
		<< " "
		<< MetaObject::storage_class::keyword()
		<< " "
		<< MetaObject::type::local_name()
		<< " "
		<< MetaObject::local_name()
		<< std::endl;
}

template <typename MetaObject>
void print_info(MetaObject, meta_inheritance_tag)
{
	std::cout
		<< MetaObject::inheritance_type::keyword()
		<< " "
		<< MetaObject::access_type::keyword()
		<< " "
		<< MetaObject::base_class::local_name()
		<< std::endl;
}

struct member_printer
{
	template <class MetaObject>
	void operator()(MetaObject mo) const
	{
		print_info(mo);
	}
};

template <typename ... MetaObject>
void print_info(mp::range<MetaObject...> rng)
{
	// call the member_printer on each member of the range
	mp::for_each<decltype(rng)>(member_printer());

}

template <typename MetaClass>
void print_info(MetaClass, meta_class_tag)
{
	std::cout
		<< MIRRORED_META_OBJECT(MetaClass)::construct_name()
		<< " "
		<< MetaClass::local_name()
		<< ": "
		<< std::endl
		<< "base classes: "
		<< std::endl;
	print_info(typename base_classes<MetaClass>::type());
	std::cout << "members: "
		<< std::endl;
	print_info(typename member_variables<MetaClass>::type());
	std::cout << std::endl;
}

template <typename MetaObject>
void print_info(MetaObject mo)
{
	std::cout
		<< MIRRORED_META_OBJECT(MetaObject)::construct_name()
		<< " ";
	print_info(mo, categorize_meta_object(mo));
}

MIRROR_REG_END


int main(void)
{
	using namespace mirror;
	//
	print_info(MIRRORED_CLASS( test::foo::A )());
	print_info(MIRRORED_CLASS( test::foo::B )());
	print_info(MIRRORED_CLASS( test::foo::C )());
	//
	return 0;
}

/* Example of output:
class class test::foo::A:
base classes:
members:
member variable public  int test::foo::A::a0
member variable public static int test::foo::A::a1
member variable public mutable int volatile test::foo::A::a2

class class test::foo::B:
base classes:
members:
member variable private  long int test::foo::B::b0
member variable private  long int * test::foo::B::b1
member variable private  long int const * test::foo::B::b2
member variable protected  long int ( *)(int) test::foo::B::b3
member variable protected  long int ( * ( *)(bool, char, wchar_t))(int) test::foo::B::b4
member variable protected  long int * volatile * test::foo::B::b5
member variable public  long int test::foo::B::b6
member variable public  long int test::foo::B::b7
member variable public  long int test::foo::B::b8

class class test::foo::C:
base classes:
base class  public test::foo::A
base class  public test::foo::B
members:
member variable public  char test::foo::C::c0
member variable public  char test::foo::C::c1
member variable public  char test::foo::C::c2
member variable public  char test::foo::C::c3
*/
