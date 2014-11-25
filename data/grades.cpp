/**
 *  @example mirror/example/factories/grades.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a script parsing plugin to generically construct instances
 *  of classes with non default constructors, from input strings.
 *
 *  Copyright 2006-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <mirror/mirror_base.hpp>
#include <mirror/pre_registered/type/native.hpp>
#include <mirror/pre_registered/type/std/string.hpp>
#include <mirror/pre_registered/class/std/vector.hpp>
#include <mirror/utils/script_factory.hpp>
#include <mirror/utils/quick_reg.hpp>
#include <iostream>

namespace testing {

struct exam
{
	std::string name;
	enum class grade
	{
		A, B, C, D, E, Fx
	} result;
};

struct student
{
	std::string first_name;
	std::string middle_name;
	std::string family_name;

	std::vector<exam> exams;
};

} //

MIRROR_REG_BEGIN

MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(testing)

MIRROR_QREG_POD_CLASS(struct, testing, exam, (name)(result))

MIRROR_QREG_NESTED_ENUM(testing::exam, grade, (A)(B)(C)(D)(E)(Fx))

MIRROR_QREG_POD_CLASS(
	struct, testing, student,
	(first_name)(middle_name)(family_name)(exams)
)

MIRROR_REG_END

int main(void)
{
	using namespace mirror;
	//
	// make the input object for the factory
	script_factory_input in;
	// create a factory plugged with the script parser
	typedef std::vector<testing::student> student_list;
	script_factory_maker::factory<student_list>::type f = in.data();
	//
	const std::string input = "{ \
		{'Johnny', 'B.', 'Goode', {{'Reading', E}, {'Writing', Fx}, {'Playing guitar', A}}}, \
		{'Emmett', 'Lathrop', 'Brown', {{'Physics', A}, {'Plutonium trade', C}}}, \
		{'Martin', '', 'McFly', {{'Playing guitar', A}, {'4-dimensional thinking', Fx}}}, \
		{'Jack', '', 'Sparrow', {{'Vessel comandeering', A}, {'Swordplay', A}}}, \
		{'Pai', '', 'Mei', {{'Kung fu', A}, {'Good manners', D}}}, \
		{'Sheldon', 'Jeffrey', 'Sands', {{'Shooting', B}, {'Persuasion', A}}}, \
		{'Jane', 'X.', 'Doe', {}} \
	}";
	in.set(input);
	student_list students(f());
	reflected<testing::exam::grade>::type meta_grade;
	for(auto i = students.begin(), e = students.end(); i != e; ++i)
	{
		std::cout << "Results of " <<
			i->family_name << ", " <<
			i->first_name << (i->middle_name.empty() ? "": " ") <<
			i->middle_name <<
			": " << std::endl;
		auto j = i->exams.begin(), f = i->exams.end();
		while(j != f)
		{
			std::cout << "	'" << j->name << "': " <<
				meta_grade.name_by_value(j->result);
			++j;
			if(j == f) std::cout << ".";
			else std::cout << ", ";
			std::cout << std::endl;
		}
	}
	//
	return 0;
}

/* Example of output:
 |  Results of Goode, Johnny B.:
 |          'Reading': E,
 |          'Writing': Fx,
 |          'Playing guitar': A.
 |  Results of Brown, Emmett Lathrop:
 |          'Physics': A,
 |          'Plutonium trade': C.
 |  Results of McFly, Martin:
 |          'Playing guitar': A,
 |          '4-dimensional thinking': Fx.
 |  Results of Sparrow, Jack:
 |          'Vessel comandeering': A,
 |          'Swordplay': A.
 |  Results of Mei, Pai:
 |          'Kung fu': A,
 |          'Good manners': D.
 |  Results of Sands, Sheldon Jeffrey:
 |          'Shooting': B,
 |          'Persuasion': A.
 |  Results of Doe, Jane X.:
 */
