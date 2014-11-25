/**
 *  Copyright 2009-2010 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *  WARNING: this file is here for development / testing purposes only
 *  and its contents and input / output can change witout any prior
 *  notice.
 */

#include <mirror/mirror.hpp>
#include <iostream>

MIRROR_NAMESPACE_BEGIN
namespace aux {

template <
	template <class, class> class Handler,
	class HandlerTraits,
	class Subject,
	class MetaInheritSeq,
	class InheritIdxSeq,
	class MetaMemVarSeq,
	class MemVarIdxSeq
>
class base_manip
{
private:
public:
	template <typename Param>
	base_manip(Param param)
	{ }

	void operator()(Subject& subject) const
	{
	}
};

} // namespace aux

template <
	template <class, class> class Handler,
	class HandlerTraits,
	class Subject
>
class manipulator : public aux::base_manip<
	Handler,
	HandlerTraits,
	Subject,
	typename base_classes< meta_class<Subject> >::type,
	typename mp::make_index_seq<
		typename mp::size<
			base_classes< meta_class<Subject> >
		>::type
	>::type,
	typename member_variables< meta_class<Subject> >::type,
	typename mp::make_index_seq<
		typename mp::size<
			member_variables< meta_class<Subject> >
		>::type
	>::type
>
{
private:
	typedef aux::base_manip<
		Handler,
		HandlerTraits,
		Subject,
		typename base_classes< meta_class<Subject> >::type,
		typename mp::make_index_seq<
			typename mp::size<
				base_classes< meta_class<Subject> >
			>::type
		>::type,
		typename member_variables< meta_class<Subject> >::type,
		typename mp::make_index_seq<
			typename mp::size<
				member_variables< meta_class<Subject> >
			>::type
		>::type
	> base_class;
public:
	// TODO: this can be removed when inherited constructors
	// are suppoted by the compilers
	template <typename Param>
	manipulator(Param param)
	 : base_class(param)
	{ }
};

template <
	template <class, class> class Handler,
	class HandlerTraits
> struct manipulator_maker
{
	/// This meta-function creates a manipulator handling the @c Subject
	/** This meta-function returns a manipulator class which can handle
	 *  instances of the @c Subject type.
	 *
	 *  @tparam Subject the type instances of which are to be manipulated
	 *  by the resulting manipulator.
	 */
	template <typename Subject>
	struct manipulator
	{
		typedef mirror::manipulator<
			Handler,
			HandlerTraits,
			Subject
		> type;
	};
};

template <class Subject, class Traits>
class my_handler
{
public:
};

MIRROR_NAMESPACE_END

#include "./test.hpp"
int main(void)
{
	using namespace mirror;
	typedef manipulator_maker<my_handler, void> make;
	make::manipulator<test::person>::type manip(0);
	test::person johnny("Johnny", "B.", "Goode", std::tm(), 75, 170);
	//
	manip(johnny);
	//
	return 0;
}

