// see	http://hfdpcpp.wordpress.com/white-gold/ for comments & discussions

	#include <SDKDDKVer.h>

	#include <stdio.h>
	#include <tchar.h>

	#include <cassert>
	#include <string>
	#include <memory>

	#include <iostream>
	#include <ostream>
	#include <sstream>

	#include "../Objectville/Hfdp.h"
	#include "../Objectville/Object.hpp"
	#include "../Objectville/Singleton.hpp";
	#include "../Objectville/Flight.hpp";
	#include "../Objectville/Speech.hpp";
	#include "../Objectville/Fowl.hpp";
	#include "../Objectville/Fowl_factory.hpp"

//
//	Table of Contents; comment-out zero or more of the following #define declarations
//	to exclude its corresponding block (in _tmain) from execution.
//
	#define	_hfdp_cpp_objectville_	// Comment-out this line to exclude 'Objectville' (core)
	#define	_hfdp_cpp_chapter_1_	// Comment-out this line to exclude chapter one (1)

	using namespace std;
	using namespace Hfdp;

	/**
	 *	Animates the specified fowl, by serializing it properties and executing
	 *	its behavior, which is captured and serialized to an output stream
	 */
		void animate(ostream& stream, const auto_ptr<Fowl>& fowl)
		{	
			stringstream string;

			string << fowl->get_weight();
			stream << endl;
			stream << fowl->to_string() << " @ " << string.str() << " lbs.";
			stream << endl;
			stream << fowl->fly();
			stream << endl;
			stream << fowl->speak();
			stream << endl;
			stream << fowl->swim();
			stream << endl;
		}

	/**
	 *	Main entry point into this program; essentially a large, sequential
	 *	test suite. In contrast to previous versions, which contained numerous
	 *	projects, one for each pattern, all examples are executed sequentially. 
	 *	More so, I didn't want to introduce a 3rd-party unit test library. After
	 *	all, the intent is to illustrate C++ principles and best practices and
	 *	patterns from the book, not support a full SDLC (Software Development
	 *	Life Cycle).
	 */
		int _tmain(int argc, _TCHAR* argv[])
		{

	#if defined(_hfdp_cpp_objectville_)

			Object& object = *(new Object());
			Object& object2 = *(new Object());

			assert(object != object2);
	#endif

	/**
	 *	Simulates the 'SimUDuck' application whereby a large variety of duck
	 *	(or fowl) species swim, fly or otherwise make quacking sounds. Exhibits 
	 *	Singleton, Strategy, Factory, Abstract Factory & Template patterns.
	 *
	 *	Book:	Several types of birds and behaviors are mentioned; however, four
	 *			types are sufficient to illustrate the pattern(s); specifically, 
	 *			Wild turkey, Mallard, Rubber and Decoy ducks. Other fowl and
	 *			behaviors were omitted for brevity and clarity.
	 */

	#if defined(_hfdp_cpp_chapter_1_)
			
			Fowl_factory& fowl_factory =
				Fowl_factory::instance();		// create a single(ton), abstract fowl factory

			auto_ptr<Fowl> mallard_duck = 
				fowl_factory.make<Fowl_factory::Mallard_duck>();

			assert(mallard_duck.get() != 0);	// ensure we got real, live mallard duck

			auto_ptr<Fowl> wild_turkey = 
				fowl_factory.make<Fowl_factory::Wild_turkey>();

			assert(wild_turkey.get() != 0);		// ensure we got a real, live turkey

			auto_ptr<Fowl> decoy_duck = 
				fowl_factory.make<Fowl_factory::Decoy_duck>();

			assert(decoy_duck.get() != 0);		// ensure we got a real, fake decoy duck

			auto_ptr<Fowl> rubber_duck = 
				fowl_factory.make<Fowl_factory::Rubber_duck>();

			assert(decoy_duck.get() != 0);		// ensure we got a real, fake rubber duck

			animate(cout, mallard_duck);
			animate(cout, decoy_duck);
			animate(cout, rubber_duck);
			animate(cout, wild_turkey);
	#endif
			return 0;	// end of simulation
		}

