/**
 *  Copyright 2009-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *  WARNING: this file is here for development / testing purposes only
 *  and its contents and input / output can change witout any prior
 *  notice.
 */
#define LAGOON_LAZY_IMPLEMENTATION 0
#define LAGOON_NO_NAMESPACE_MEMBERS 1
#define LAGOON_MT_WITH_MAKE_FACTORY 1

#include <lagoon/lagoon.hpp>

#include <mirror/mirror.hpp>

#include <mirror/utils/polymorph_factory.hpp>

#include <iostream>
#include <list>
#include <stdexcept>

#include "./test.hpp"

LAGOON_NAMESPACE_BEGIN

class my_factory_manager
 : public polymorph_factory_manager
{
private:
	std::list<std::string> ctr_labels;
public:
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

	int index(void)
	{
		std::cout << "Pick a constructor " << std::endl;
		int idx = 0;
		for(auto i=ctr_labels.begin(), e=ctr_labels.end(); i!=e; ++i)
			std::cout << idx++ << ": " << *i << std::endl;
		idx = 0;
		std::cin >> idx;
		return idx;
	}
};

class my_factory_composite
 : public polymorph_factory_composite
{
private:
	std::string label;
public:
	my_factory_composite(const shared<meta_class>& param)
	{
		label.append(param->base_name());
	}

	my_factory_composite(const shared<meta_parameter>& param)
	{
		label.append(param->type()->base_name());
		label.append(" ");
		label.append(param->base_name());
	}

	void on_create(void)
	{
		std::cout << "Creating " << label << std::endl;
	}
};

template <class Product>
class my_factory_manufacturer
 : public polymorph_factory_manufacturer<Product>
{
private:
	std::string label;

	template <typename Src, typename Dst>
	static void assign_val(Dst& dst, const Src& src)
	{
		dst = Dst(src);
	}

	static void assign_val(std::string& dst, const std::wstring& src)
	{
		//TODO:
	}

	static void assign_val(std::wstring& dst, const std::string& src)
	{
		//TODO:
	}

	template <typename X, typename T>
	static void do_get(T& val)
	{
		X x;
		std::cin >> x;
		assign_val(val, x);
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

	static void get(char& val)
	{
		do_get<char>(val);
	}

	static void get(wchar_t& val)
	{
		do_get<char>(val);
	}

	static void get(std::tm& val)
	{
		// TODO
	}
public:
	my_factory_manufacturer(const shared<meta_parameter>& param)
	{
		label.append(param->type()->base_name());
		label.append(" ");
		label.append(param->base_name());
	}

	Product create(void)
	{
		std::cout << "Enter " << label << ": " << std::flush;
		Product x = Product();
		get(x);
		return x;
	}
};

class my_factory_suppliers
 : public polymorph_factory_suppliers
{
public:
	my_factory_suppliers(const shared<meta_parameter>& param)
	{ }

	raw_ptr get(void)
	{
		throw std::runtime_error("No external sources available");
		return raw_ptr();
	}
};

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
		const shared<meta_class>& product,
		const polymorph_factory_context& context
	)
	{
		return new my_factory_composite(product);
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
};

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

typedef concrete_polymorph_factory_builder<
	my_factory_builder_base,
	my_factory_builder_unit
> my_factory_builder;

LAGOON_NAMESPACE_END

int main(void)
{
	using namespace lagoon;

	try
	{
		auto meta_person = reflected_class<test::person>();
		my_factory_builder builder;
		auto person_factory = meta_person->make_factory(builder, raw_ptr());
		meta_person->delete_(person_factory->new_());
	}
	catch(std::exception const& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	//
	return 0;
}

