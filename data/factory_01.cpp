/**
 *  @example lagoon/example/factories/factory_01.cpp
 *  This example shows a sample implementation of the polymorphic
 *  factory builder interfaces and the usage of a polymorphic factory.
 *
 *  Copyright 2008-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

//
// We want to use Lagoon's polymorphic factories
#define LAGOON_MT_WITH_MAKE_FACTORY 1
//
// We don't need to traverse through namespace members.
// Using this CT switch can greatly improve compile times
// and the resulting executable size if namespace member
// traversal is not needed
#define LAGOON_NO_NAMESPACE_MEMBERS 1

// include stuff from both Lagoon and Mirror
#include <lagoon/lagoon.hpp>
#include <mirror/mirror.hpp>

#include <iostream>
#include <stdexcept>
#include <list>

// use the tetrahedron, triangle and vector classes
#include "./tetrahedron.hpp"

LAGOON_NAMESPACE_BEGIN

// Implementation of the polymorphic factory manager interface
/* A factory manager is responsible for the choosing of the constructor
 * that is to be used to create an instance of a Product type.
 * The information about the constructors available are passed to
 * the manager by the framework by the means of the  param member
 * function, which gets called once per every available constructor.
 *
 * The manager should implement the index function which must return
 * the index of the constructor to be used.
 */
class my_factory_manager
 : public polymorph_factory_manager
{
private:
	std::list<std::string> ctr_labels;
public:
	// Informs the manager about a constructor
	// The constructors can be iterated in normal
	// or reverse order
	raw_ptr add_constructor(
		raw_ptr parent_data,
		const shared<meta_constructor>& constructor,
		const polymorph_factory_context& context,
		bool backward_iteration
	)
	{
		std::string label(constructor->base_name());
		label.append("(");
		auto params = constructor->parameters();
		while(!params.empty())
		{
			label.append(params.front()->type()->base_name());
			label.append(" ");
			label.append(params.front()->base_name());
			params.step_front();
			if(!params.empty())
				label.append(",");
		}
		label.append(")");
		if(backward_iteration)
			ctr_labels.push_front(label);
		else ctr_labels.push_back(label);
		return raw_ptr();
	}

	// Returns the index of the constructor to be used
	int index(void)
	{
		std::cout << "Pick a constructor " << std::endl;
		int idx = 0;
		for(auto i=ctr_labels.begin(), e=ctr_labels.end(); i!=e; ++i)
			std::cout << "  " << idx++ << ": " << *i << std::endl;
		idx = 0;
		std::cin >> idx;
		return idx;
	}
};

// Implementation of the polymorphic factory composite
/* A composite contains native type manufacturers or
 * other composites recursivelly. One composite is created
 * per every non-native-typed constructor parameter.
 *
 * Container can do special handling just before the creation
 * of the parameter value in the on_create member function.
 */
class my_factory_composite
 : public polymorph_factory_composite
{
private:
	std::string label;
public:
	// uses the provided meta-data to create an info string
	my_factory_composite(const shared<meta_type>& product)
	{
		label.append("a/an ");
		label.append(product->base_name());
	}

	// uses the provided meta-data to create an info string
	my_factory_composite(const shared<meta_parameter>& param)
	{
		label.append(param->type()->base_name());
		label.append(" ");
		label.append(param->base_name());
	}

	// print the info that we are about to create an elaborated object
	void on_create(void)
	{
		std::cout << "Creating " << label << std::endl;
	}
};

// Implementation of the polymorphic factory arrayer
/* An arrayer handles the construction of containers and contains
 * composites, or manufacturers recursivelly. An arrayer is created
 * for initializer list constructors of a class.
 */
class my_factory_arrayer
 : public polymorph_factory_arrayer
{
private:
	std::string label;

	typedef typename polymorph_factory_arrayer::element_producer
		element_producer;
public:
	// uses the provided meta-data to create an info string
	my_factory_arrayer(const shared<meta_type>& product)
	{
		label.append("a/an ");
		label.append(product->base_name());
	}

	// print the info that we are about to create an elaborated object
	void create(element_producer&)
	{
		//TODO: add support for containers
	}
};

// Implementation of the manufacturer template interface
/* Manufacturers are responsible for the "construction" of parameters
 * with native-types.
 */
template <class Product>
class my_factory_manufacturer
 : public polymorph_factory_manufacturer<Product>
{
private:
	std::string label;

	template <typename X, typename T>
	static void do_get(T& val)
	{
		X x;
		std::cin >> x;
		val = T(x);
	}

	template <typename T>
	static void get(T& val)
	{
		std::cin >> val;
	}

	static void get(bool& val)
	{
		std::string x;
		std::cin >> x;
		val = (x == "true") || (x == "t") || (x != "0");
	}

	static void get(unsigned char& val)
	{
		do_get<unsigned long>(val);
	}

	static void get(wchar_t& val)
	{
		do_get<char>(val);
	}

	static void get(std::tm& val)
	{
		int year, month;
		std::cout << std::endl;
		std::cout << "year: " << std::flush;
		std::cin >> year;
		val.tm_year = year - 1900;
		std::cout << "month (1-12): " << std::flush;
		std::cin >> month;
		val.tm_mon = month - 1;
		std::cout << "day: " << std::flush;
		std::cin >> val.tm_mday;
		std::cout << "hour: " << std::flush;
		std::cin >> val.tm_hour;
		std::cout << "minutes: " << std::flush;
		std::cin >> val.tm_min;
		std::cout << "second: " << std::flush;
		std::cin >> val.tm_sec;
	}
public:
	// use the meta-data to create an informational string label
	my_factory_manufacturer(const shared<meta_parameter>& param)
	{
		label.append(param->type()->base_name());
		label.append(" ");
		label.append(param->base_name());
	}

	// "creates" an instance of the Product type
	Product create(void)
	{
		// in this case read the value from the standard input
		std::cout << "Enter " << label << ": " << std::flush;
		Product x = Product();
		get(x);
		return x;
	}
};

// Implementation of the enumerator template interface
/* Enumerators are responsible for the "construction" of parameters
 * with enumerated types
 */
class my_factory_enumerator
 : public polymorph_factory_enumerator
{
private:
	shared<meta_enum> enumeration;
	std::string label;
public:
	// use the meta-data to create an informational string label
	my_factory_enumerator(const shared<meta_parameter>& param)
	 : enumeration(param->type().as<meta_enum>())
	{
		label.append(param->type()->base_name());
		label.append(" ");
		label.append(param->base_name());
		label.append(" {");
		for (int i=0, n=enumeration->size(); i!=n; ++i)
		{
			if(i != 0) label.append(", ");
			label.append(enumeration->value_name(i));
		}
		label.append("}");
	}

	// "creates" an instance of the enum type
	int create(void)
	{
		std::cout << "Enter " << label << ": " << std::flush;
		std::string x;
		while(std::cin.good())
		{
			std::cin >> x;
			if(enumeration->has_value_name(x))
				return enumeration->value_by_name(x);
		}
		return 0;
	}
};

// Implementation of the suppliers interface
/* Suppliers are responsible for getting existing instances for the parameter
 * described by the meta-data passed to the constructor.
 *
 * This simple implementation does not support copy construction and always
 * returns a null pointer.
 */
class my_factory_suppliers
 : public polymorph_factory_suppliers
{
public:
	my_factory_suppliers(const shared<meta_parameter>& param)
	{ }

	// we don't support copy construction in this implementation
	raw_ptr get(void)
	{
		throw std::runtime_error(
			"No external sources available. "\
			"(copy construction not supported for this type)"
		);
		return raw_ptr();
	}
};

// Partial implementation of the factory builder
/* This class instantiates the managers, composites and suppliers
 * during the creation of a concrete factory.
 *
 * The make_* member functions are called by the polymorphic factory
 * framework as required for a factory constructing a concrete Product.
 *
 * This class is plugged together with the my_factory_builder_unit into
 * the concrete_polymorph_factory_builder template to create a full
 * implementation of a factory builder.
 */
class my_factory_builder_base
{
public:
	my_factory_manager* make_manager(
		raw_ptr parent_data,
		const polymorph_factory_context& context
	)
	{
		return new my_factory_manager();
	}

	my_factory_composite* make_composite(
		raw_ptr parent_data,
		const shared<meta_type>& product,
		const polymorph_factory_context& context
	)
	{
		return new my_factory_composite(product);
	}

	my_factory_arrayer* make_arrayer(
		raw_ptr parent_data,
		const shared<meta_type>& product,
		const polymorph_factory_context& context
	)
	{
		return new my_factory_arrayer(product);
	}

	my_factory_composite* make_composite(
		raw_ptr parent_data,
		const shared<meta_parameter>& param,
		const polymorph_factory_context& context
	)
	{
		return new my_factory_composite(param);
	}

	my_factory_suppliers* make_suppliers(
		raw_ptr parent_data,
		const shared<meta_parameter>& param,
		const polymorph_factory_context& context
	)
	{
		return new my_factory_suppliers(param);
	}

	my_factory_enumerator* make_enumerator(
		raw_ptr parent_data,
		const shared<meta_parameter>& param,
		const polymorph_factory_context& context
	)
	{
		return new my_factory_enumerator(param);
	}
};


// Partial implementation of the factory builder
/* This class instantiates manufacturers for a concrete product.
 *
 * The make_manufacturer member function is called by the polymorphic factory
 * framework as required for a factory constructing a concrete Product.
 *
 *
 * This class is plugged together with the my_factory_builder_base into
 * the concrete_polymorph_factory_builder template to create a full
 * implementation of a factory builder.
 */
template <class Product>
class my_factory_builder_unit
{
public:
	my_factory_manufacturer<Product>* make_manufacturer(
		raw_ptr parent_data,
		const shared<meta_parameter>& param,
		const polymorph_factory_context& context
	)
	{
		return new my_factory_manufacturer<Product>(param);
	}
};

/* Here we are using the concrete_polymorph_factory_builder helper template
 * to create a complete factory builder from the base parts implemented above.
 *
 * The resulting builder can be used together with the meta_type's
 * make_factory() member function to build a factory which can be used to
 * create instances of the type reflected by the meta-type.
 */
typedef concrete_polymorph_factory_builder<
	my_factory_builder_base,
	my_factory_builder_unit
> my_factory_builder;

LAGOON_NAMESPACE_END

int main(void)
{
	try
	{
		using namespace lagoon;
		//
		// we'll need a concrete factory builder
		my_factory_builder builder;
		// reflect the tetrahedron class
		auto meta_th = reflected_class<test::tetrahedron>();
		// let the meta-object and the builder to make a factory
		// constructing tetrahedrons
		auto th_factory = meta_th->make_factory(builder, raw_ptr());
		//
		// use the factory to create an instance of tetrahedron
		auto rth = th_factory->new_();
		// and cast the raw pointer to a pointer to a concrete type
		auto th = raw_cast<test::tetrahedron*>(rth);
		// print out the volume of the created tetrahedron
		std::cout << "The volume is " << th->volume() << std::endl;
		// delete the object
		meta_th->delete_(rth);
	}
	catch(std::exception const& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	return 0;
}

