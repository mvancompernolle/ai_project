/**
 *  Copyright 2009-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *  WARNING: this file is here for development / testing purposes only
 *  and its contents and input / output can change witout any prior
 *  notice.
 */
#define LAGOON_NO_NAMESPACE_MEMBERS 1
#define LAGOON_MT_WITH_MAKE_SHARED 1
#define LAGOON_MT_WITH_MAKE_UNIQUE 1
#define LAGOON_MT_WITH_NEW_COPY 1
#define LAGOON_LAZY_IMPLEMENTATION 0

#include <lagoon/lagoon.hpp>

#include <mirror/mirror.hpp>

#include <type_traits>
#include <tuple>

#include <vector>
#include <iostream>

#include <mirror/raw_ptr/raw_ptr.hpp>

namespace test {

typedef int an_int_type;

struct A
{
	an_int_type a;
};

struct B
{
	bool b;
};

struct C
 : virtual public A
 , virtual public B
{
	char c;
};

struct D
 : virtual public A
 , virtual public B
{
	double d;
};

struct E
 : virtual public C
 , virtual public D
{
	static double e;
};

struct F
 : public E
 , virtual public C
 , virtual public D
{
	float f;
};

struct G
{
	short g;

	G(void){ }
	G(const G&){ }
	G(int x){ }
	G(int a, int b, double d){ }

	operator E(void)
	{
		return E();
	}

	operator F(void)
	{
		return F();
	}

	double f(int a, int b);
	int f(double x);

	static void h(bool b);
};

struct H
 : public F
 , public G
{
	long h;
};

template <typename First, typename Second>
struct P
{
	First first;
	Second second;
};

} // namespace test

namespace first {
namespace second {
namespace third {

}
}
}

MIRROR_REG_BEGIN

template <typename Type, Type Value>
struct meta_constant
{
	typedef meta_type<Type> type;
	typedef std::integral_constant<Type, Value> constant;
};


MIRROR_REG_NAMESPACE_BEGIN                    // begin the registering of namespaces
MIRROR_REG_NAMESPACE(first)             // register the namespace defined in the global scope
namespace first {
MIRROR_REG_NAMESPACE(second)            // register the second-level nested namespace
namespace second{
MIRROR_REG_NAMESPACE(third)             // register the second-level nested namespace
} // namespace second
} // namespace first
MIRROR_REG_NAMESPACE_END                       // finish the registering of namespaces
MIRROR_INIT_NAMESPACE_MEMBER_LIST(first) // initialize the list of namespace members
MIRROR_INIT_NAMESPACE_MEMBER_LIST(first::second) // initialize the list of namespace members
MIRROR_INIT_NAMESPACE_MEMBER_LIST(first::second::third) // initialize the list of namespace members


MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(test)

MIRROR_REG_TYPEDEF(test, an_int_type)
MIRROR_REG_GLOBAL_SCOPE_TYPEDEF(size_t)


MIRROR_REG_CLASS_BEGIN(struct, test, A)
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, MIRROR_TYPEDEF(test, an_int_type), a)
	//MIRROR_REG_CLASS_MEM_VAR(_, _, MIRROR_TYPEDEF_GLOBAL_SCOPE(size_t), a)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

MIRROR_REG_CLASS_BEGIN(struct, test, B)
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, b)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END


MIRROR_REG_CLASS_BEGIN(struct, test, C)
// register base classes
MIRROR_REG_BASE_CLASSES_BEGIN
	MIRROR_REG_BASE_CLASS(virtual, _, test::A),
	MIRROR_REG_BASE_CLASS(virtual, _, test::B)
MIRROR_REG_BASE_CLASSES_END
// register member variables
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, c)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

MIRROR_REG_CLASS_BEGIN(struct, test, D)
// register base classes
MIRROR_REG_BASE_CLASSES_BEGIN
	MIRROR_REG_BASE_CLASS(virtual, _, test::A),
	MIRROR_REG_BASE_CLASS(virtual, _, test::B)
MIRROR_REG_BASE_CLASSES_END
// register member variables
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, d)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

MIRROR_REG_CLASS_BEGIN(struct, test, E)
// register base classes
MIRROR_REG_BASE_CLASSES_BEGIN
	MIRROR_REG_BASE_CLASS(virtual, _, test::C),
	MIRROR_REG_BASE_CLASS(virtual, _, test::D)
MIRROR_REG_BASE_CLASSES_END
// register member variables
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(public, static, double, e)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

MIRROR_REG_CLASS_BEGIN(struct, test, F)
// register base classes
MIRROR_REG_BASE_CLASSES_BEGIN
	MIRROR_REG_BASE_CLASS(_, _, test::E),
	MIRROR_REG_BASE_CLASS(virtual, _, test::C),
	MIRROR_REG_BASE_CLASS(virtual, _, test::D)
MIRROR_REG_BASE_CLASSES_END
// register member variables
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, f)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

MIRROR_REG_CLASS_BEGIN(struct, test, G)
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, g)
MIRROR_REG_CLASS_MEM_VARS_END

MIRROR_REG_CONSTRUCTORS_BEGIN
	MIRROR_REG_DEFAULT_CONSTRUCTOR(public)
	MIRROR_REG_COPY_CONSTRUCTOR(public)

	MIRROR_REG_CONSTRUCTOR_BEGIN(public, 1)
		MIRROR_REG_CONSTRUCTOR_PARAM(int, x)
	MIRROR_REG_CONSTRUCTOR_END(1)

	MIRROR_REG_CONSTRUCTOR_BEGIN(public, 2)
		MIRROR_REG_CONSTRUCTOR_PARAM(int, a)
		MIRROR_REG_CONSTRUCTOR_PARAM(int, b)
		MIRROR_REG_CONSTRUCTOR_PARAM(double, d)
	MIRROR_REG_CONSTRUCTOR_END(2)
MIRROR_REG_CONSTRUCTORS_END

MIRROR_REG_MEM_FUNCTIONS_BEGIN
	MIRROR_REG_MEM_FUNCTION_BEGIN(public, _, double, f, ab)
		MIRROR_REG_MEM_FUNCTION_PARAM(int, a)
		MIRROR_REG_MEM_FUNCTION_PARAM(int, b)
	MIRROR_REG_MEM_FUNCTION_END(f, ab, _)
	MIRROR_REG_MEM_FUNCTION_BEGIN(public, _, int, f, x)
		MIRROR_REG_MEM_FUNCTION_PARAM(double, x)
	MIRROR_REG_MEM_FUNCTION_END(f, x, _)
	MIRROR_REG_MEM_FUNCTION_BEGIN(public, static, void, g, _)
		MIRROR_REG_MEM_FUNCTION_PARAM(bool, b)
	MIRROR_REG_MEM_FUNCTION_END(g, _, _)
MIRROR_REG_MEM_FUNCTIONS_END

MIRROR_REG_CLASS_CONVERSIONS((::test::E, ::test::F))

MIRROR_REG_CLASS_END

MIRROR_REG_CLASS_BEGIN(struct, test, H)
// register base classes
MIRROR_REG_BASE_CLASSES_BEGIN
	MIRROR_REG_BASE_CLASS(_, _, test::F),
	MIRROR_REG_BASE_CLASS(_, _, test::G)
MIRROR_REG_BASE_CLASSES_END
// register member variables
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, h)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

MIRROR_REG_CLASS_TEMPLATE_BEGIN(
	(typename First, typename Second),
	struct, test, P, (First, Second)
)
MIRROR_REG_CLASS_MEM_VARS_BEGIN
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, first)
	MIRROR_REG_CLASS_MEM_VAR(_, _, _, second)
MIRROR_REG_CLASS_MEM_VARS_END
MIRROR_REG_CLASS_END

MIRROR_REG_OBJECT_TAG(unique)
MIRROR_REG_OBJECT_TAG(optional)
MIRROR_REG_OBJECT_TAG(hidden)

MIRROR_TAG_TYPE( ::test::A, (unique, hidden))

MIRROR_REG_END

template <class Range>
void print_dyn_meta_inheritance(Range r)
{
	using namespace mirror;
	std::cout << r.size() << std::endl;
	while(!r.empty())
	{
		std::cout << r.front()->self()->construct_name()
			<< ": "
			<< r.front()->inheritance_type()->keyword()
			<< " "
			<< r.front()->access_type()->keyword()
			<< " "
			<< r.front()->base_class()->elaborated_type()->keyword()
			<< " "
			<< r.front()->base_class()->local_name()
			<< std::endl;
		r.step_front();
	}
}

template <class Range>
void print_dyn_meta_mem_var(Range r)
{
	using namespace mirror;
	std::cout << r.size() << std::endl;
	while(!r.empty())
	{
		std::cout << r.front()->self()->construct_name()
			<< ": "
			<< r.front()->access_type()->keyword()
			<< " "
			<< r.front()->storage_class()->keyword()
			<< " "
			<< r.front()->type()->local_name()
			<< " "
			<< r.front()->local_name()
			<< std::endl;
		r.step_front();
	}
}

struct static_name_printer
{
	template <class IterInfo>
	void operator()(IterInfo)
	{
		typedef typename IterInfo::type meta_X;
		std::cout
			<< meta_X::type::full_name()
			<< " "
			<< meta_X::full_name();
		if(IterInfo::is_last::value)
			std::cout << std::endl;
		else std::cout << ", ";
	}
};


struct test_class
{
	//test_class(void);
	test_class(int);
};


template <typename T>
struct default_creator
{
	template <typename MF, typename CI, typename PI>
	default_creator(int, MF, CI, PI)
	{ }

	default_creator(int)
	{ }

	T operator()(void) const
	{
		return T();
	}
};

template <>
struct default_creator<void>
{
	default_creator(int)
	{ }

	template <typename CI>
	static int add_constructor(int, CI)
	{
		return 0;
	}

	static int index(void)
	{
		return 0;
	}
};

/*
template <typename Type>
struct manufacturer

struct parameter_manufacturer
{
	virtual ~parameter_manufacturer(void)
	{ }

	virtual parameter_manufacturer* structure_maker
};
*/

int main(void)
{
	using namespace mirror;
	//
	MIRROR_OBJECT_TAG(optional) ta;
	MIRROR_OBJECT_TAG(unique)   tb;
	std::cout << ta.id() << ", " << tb.id() << std::endl;
	std::cout << ta.name() << ", " << tb.name() << std::endl;
	assert(ta != tb);
	std::cout << mp::size<tags<MIRRORED_CLASS(test::A)> >::value << std::endl;
	std::cout << mp::size<tags<MIRRORED_CLASS(test::B)> >::value << std::endl;
	//
#ifdef NEVER_DEFINED
	typedef mp::at_c<constructors< MIRRORED_CLASS(test::G) >, 3>::type meta_ctr_G;
	std::cout << meta_ctr_G::full_name() << std::endl;
	typedef mp::at_c<parameters<meta_ctr_G>, 2>::type meta_ctr_G_param;
	std::cout << meta_ctr_G_param::type::full_name() << std::endl;
	std::cout << meta_ctr_G_param::base_name() << std::endl;
	std::cout << meta_ctr_G_param::full_name() << std::endl;
	//
	typedef mp::at_c<member_functions< MIRRORED_CLASS(test::G) >, 0>::type meta_ctr_G_f;
	std::cout << meta_ctr_G_f::full_name() << std::endl;
	typedef mp::at_c<parameters<meta_ctr_G_f>, 0>::type meta_ctr_G_f_param;
	std::cout << meta_ctr_G_f_param::type::full_name() << std::endl;
	std::cout << meta_ctr_G_f_param::base_name() << std::endl;
	std::cout << meta_ctr_G_f_param::full_name() << std::endl;
	//
	//
	std::cout << mp::size< conversions< MIRRORED_CLASS(test::G) > >::value << std::endl;
	//
	typedef test::P<int, double> P;
	mp::for_each_ii< all_member_variables< MIRRORED_CLASS(P) > >(static_name_printer());
	//
	MIRROR_USING_NAMESPACE(test);
	MIRROR_USING_NAMESPACE(boost);
	//
	auto meta_x(lagoon::reflected_class<test::G>());
	print_dyn_meta_inheritance(meta_x->base_classes());
	print_dyn_meta_mem_var(meta_x->all_member_variables());
	//
	std::cout << meta_x->constructors().at(3)->parameters().at(2)->full_name() << std::endl;
	std::cout << lagoon::reflected_global_scope()->type_("long double")->constructors().at(1)->parameters().at(0)->full_name() << std::endl;
	std::cout << meta_x->member_functions().at(0)->parameters().at(1)->full_name() << std::endl;
	//
	typedef mp::at_c< member_variables< MIRRORED_CLASS(test::C) >, 0>::type
		meta_C_c;
	//
	typedef factory_maker<default_creator, default_creator> def_maker;
	typedef def_maker::factory<test::G>::type fact_G;
	fact_G _g(0);
	_g();
	//
#endif
	double x = 123.456;
	//
	raw_ptr px(&x);
	raw_ptr px2 = px;
	raw_ptr px3;
	px3 = px2;
	std::cout << *raw_cast<const double*>(px3) << std::endl;
	//
	raw_ptr rp;
	{
		std::string s("blah");
		std::cout << s << std::endl;
		rp = lagoon::reflected_type<std::string>()->new_copy(raw_ptr(&s));
	}
	{
		auto ps(raw_cast<const std::string*>(rp));
		std::cout << *ps << std::endl;
	}
	shared_raw_ptr srp;
	unique_raw_ptr urp;
	{
		auto meta_str = lagoon::reflected_type<std::string>();
		srp = meta_str->make_shared(meta_str->new_copy(rp));
		urp = meta_str->make_unique(meta_str->new_copy(rp));
	}
	{
		lagoon::reflected_type<std::string>()->delete_(rp);
	}
	{
		auto sps(raw_cast<std::shared_ptr<std::string> >(srp));
		std::cout << *sps << std::endl;
	}
	{
		auto sps(raw_cast<std::shared_ptr<const std::string> >(srp));
		std::cout << *sps << std::endl;
	}
	std::unique_ptr<std::string> ups;
	{
		ups = raw_cast<std::unique_ptr<std::string> >(std::move(urp));
		std::cout << *ups << std::endl;
	}
	return 0;
}

