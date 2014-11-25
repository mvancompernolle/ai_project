/**
 *  @example mirror/example/factories/script_parsers.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a script parsing plugin to generically construct instances
 *  of classes with non default constructors, from input strings.
 *
 *  Copyright 2006-2011 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */


#include <mirror/invoker.hpp>
#include <mirror/factory.hpp>
#include <mirror/utils/script_factory.hpp>
#include <iostream>

#include "./tetrahedron.hpp"

int main(void)
{
	using namespace mirror;
	using namespace test;
	MIRROR_USING_NAMESPACE(test);
	//
	// make the input object for the factory
	script_factory_input in;
	// create a few factories plugged with the script parser plugin
	script_factory_maker::factory<vector>::type f_ve = in.data();
	script_factory_maker::factory<triangle>::type f_tr = in.data();
	script_factory_maker::factory<tetrahedron>::type f_th = in.data();
	in.print_parsers(std::cout);
	//
	return 0;
}

/* Example of output:
+-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector) OR triangle(vector, vector, vector) OR triangle() OR triangle(triangle) OR tetrahedron(triangle, vector) OR tetrahedron(vector, vector, vector, vector) OR tetrahedron(tetrahedron)}-(0xbfe0c51c)
  |
  +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3c040)
  | |
  | +-+-{vector(double, double, double)}-(0x9f3c230)
  | | |
  | | +---[double literal]-(0x9f3c3a0)
  | | |
  | | +---[double literal]-(0x9f3c358)
  | | |
  | | +---[double literal]-(0x9f3c310)
  | |
  | +-+-{vector(double)}-(0x9f3c1b0)
  | | |
  | | +---[double literal]-(0x9f3c2b8)
  | |
  | +---[vector()]-(0x9f3c118)
  | |
  | +-+-{vector(vector)}-(0x9f3c0c8)
  |   |
  |   +---[-]-(0x9f3c280)
  |
  +-+-{triangle(vector, vector, vector) OR triangle() OR triangle(triangle)}-(0x9f3c438)
  | |
  | +-+-{triangle(vector, vector, vector)}-(0x9f3c5c8)
  | | |
  | | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3ce78)
  | | | |
  | | | +-+-{vector(double, double, double)}-(0x9f3d068)
  | | | | |
  | | | | +---[double literal]-(0x9f3d1c8)
  | | | | |
  | | | | +---[double literal]-(0x9f3d180)
  | | | | |
  | | | | +---[double literal]-(0x9f3d138)
  | | | |
  | | | +-+-{vector(double)}-(0x9f3cfe8)
  | | | | |
  | | | | +---[double literal]-(0x9f3d0e0)
  | | | |
  | | | +---[vector()]-(0x9f3cf50)
  | | | |
  | | | +-+-{vector(vector)}-(0x9f3cf00)
  | | |   |
  | | |   +---[-]-(0x9f3ce20)
  | | |
  | | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3ca78)
  | | | |
  | | | +-+-{vector(double, double, double)}-(0x9f3cc68)
  | | | | |
  | | | | +---[double literal]-(0x9f3cdc8)
  | | | | |
  | | | | +---[double literal]-(0x9f3cd80)
  | | | | |
  | | | | +---[double literal]-(0x9f3cd38)
  | | | |
  | | | +-+-{vector(double)}-(0x9f3cbe8)
  | | | | |
  | | | | +---[double literal]-(0x9f3cce0)
  | | | |
  | | | +---[vector()]-(0x9f3cb50)
  | | | |
  | | | +-+-{vector(vector)}-(0x9f3cb00)
  | | |   |
  | | |   +---[-]-(0x9f3ca20)
  | | |
  | | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3c668)
  | |   |
  | |   +-+-{vector(double, double, double)}-(0x9f3c858)
  | |   | |
  | |   | +---[double literal]-(0x9f3c9c8)
  | |   | |
  | |   | +---[double literal]-(0x9f3c980)
  | |   | |
  | |   | +---[double literal]-(0x9f3c938)
  | |   |
  | |   +-+-{vector(double)}-(0x9f3c7d8)
  | |   | |
  | |   | +---[double literal]-(0x9f3c8e0)
  | |   |
  | |   +---[vector()]-(0x9f3c740)
  | |   |
  | |   +-+-{vector(vector)}-(0x9f3c6f0)
  | |     |
  | |     +---[-]-(0x9f3c8a8)
  | |
  | +---[triangle()]-(0x9f3c528)
  | |
  | +-+-{triangle(triangle)}-(0x9f3c498)
  |   |
  |   +---[-]-(0x9f3c3f8)
  |
  +-+-{tetrahedron(triangle, vector) OR tetrahedron(vector, vector, vector, vector) OR tetrahedron(tetrahedron)}-(0x9f3d380)
    |
    +-+-{tetrahedron(triangle, vector)}-(0x9f3d500)
    | |
    | +-+-{triangle(vector, vector, vector) OR triangle() OR triangle(triangle)}-(0x9f3e918)
    | | |
    | | +-+-{triangle(vector, vector, vector)}-(0x9f3eaa8)
    | | | |
    | | | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3f358)
    | | | | |
    | | | | +-+-{vector(double, double, double)}-(0x9f3f548)
    | | | | | |
    | | | | | +---[double literal]-(0x9f3f6a8)
    | | | | | |
    | | | | | +---[double literal]-(0x9f3f660)
    | | | | | |
    | | | | | +---[double literal]-(0x9f3f618)
    | | | | |
    | | | | +-+-{vector(double)}-(0x9f3f4c8)
    | | | | | |
    | | | | | +---[double literal]-(0x9f3f5c0)
    | | | | |
    | | | | +---[vector()]-(0x9f3f430)
    | | | | |
    | | | | +-+-{vector(vector)}-(0x9f3f3e0)
    | | | |   |
    | | | |   +---[-]-(0x9f3f300)
    | | | |
    | | | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3ef58)
    | | | | |
    | | | | +-+-{vector(double, double, double)}-(0x9f3f148)
    | | | | | |
    | | | | | +---[double literal]-(0x9f3f2a8)
    | | | | | |
    | | | | | +---[double literal]-(0x9f3f260)
    | | | | | |
    | | | | | +---[double literal]-(0x9f3f218)
    | | | | |
    | | | | +-+-{vector(double)}-(0x9f3f0c8)
    | | | | | |
    | | | | | +---[double literal]-(0x9f3f1c0)
    | | | | |
    | | | | +---[vector()]-(0x9f3f030)
    | | | | |
    | | | | +-+-{vector(vector)}-(0x9f3efe0)
    | | | |   |
    | | | |   +---[-]-(0x9f3ef00)
    | | | |
    | | | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3eb48)
    | | |   |
    | | |   +-+-{vector(double, double, double)}-(0x9f3ed38)
    | | |   | |
    | | |   | +---[double literal]-(0x9f3eea8)
    | | |   | |
    | | |   | +---[double literal]-(0x9f3ee60)
    | | |   | |
    | | |   | +---[double literal]-(0x9f3ee18)
    | | |   |
    | | |   +-+-{vector(double)}-(0x9f3ecb8)
    | | |   | |
    | | |   | +---[double literal]-(0x9f3edc0)
    | | |   |
    | | |   +---[vector()]-(0x9f3ec20)
    | | |   |
    | | |   +-+-{vector(vector)}-(0x9f3ebd0)
    | | |     |
    | | |     +---[-]-(0x9f3ed88)
    | | |
    | | +---[triangle()]-(0x9f3ea08)
    | | |
    | | +-+-{triangle(triangle)}-(0x9f3e978)
    | |   |
    | |   +---[-]-(0x9f3e8c0)
    | |
    | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3e668)
    |   |
    |   +-+-{vector(double, double, double)}-(0x9f3e708)
    |   | |
    |   | +---[double literal]-(0x9f3e868)
    |   | |
    |   | +---[double literal]-(0x9f3e820)
    |   | |
    |   | +---[double literal]-(0x9f3e7d8)
    |   |
    |   +-+-{vector(double)}-(0x9f3e6d0)
    |   | |
    |   | +---[double literal]-(0x9f3e780)
    |   |
    |   +---[vector()]-(0x9f3e6b0)
    |   |
    |   +-+-{vector(vector)}-(0x9f3e690)
    |     |
    |     +---[-]-(0x9f3e498)
    |
    +-+-{tetrahedron(vector, vector, vector, vector)}-(0x9f3d428)
    | |
    | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3e100)
    | | |
    | | +-+-{vector(double, double, double)}-(0x9f3e2f0)
    | | | |
    | | | +---[double literal]-(0x9f3e450)
    | | | |
    | | | +---[double literal]-(0x9f3e408)
    | | | |
    | | | +---[double literal]-(0x9f3e3c0)
    | | |
    | | +-+-{vector(double)}-(0x9f3e270)
    | | | |
    | | | +---[double literal]-(0x9f3e368)
    | | |
    | | +---[vector()]-(0x9f3e1d8)
    | | |
    | | +-+-{vector(vector)}-(0x9f3e188)
    | |   |
    | |   +---[-]-(0x9f3e0a8)
    | |
    | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3dd00)
    | | |
    | | +-+-{vector(double, double, double)}-(0x9f3def0)
    | | | |
    | | | +---[double literal]-(0x9f3e050)
    | | | |
    | | | +---[double literal]-(0x9f3e008)
    | | | |
    | | | +---[double literal]-(0x9f3dfc0)
    | | |
    | | +-+-{vector(double)}-(0x9f3de70)
    | | | |
    | | | +---[double literal]-(0x9f3df68)
    | | |
    | | +---[vector()]-(0x9f3ddd8)
    | | |
    | | +-+-{vector(vector)}-(0x9f3dd88)
    | |   |
    | |   +---[-]-(0x9f3dca8)
    | |
    | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3d900)
    | | |
    | | +-+-{vector(double, double, double)}-(0x9f3daf0)
    | | | |
    | | | +---[double literal]-(0x9f3dc50)
    | | | |
    | | | +---[double literal]-(0x9f3dc08)
    | | | |
    | | | +---[double literal]-(0x9f3dbc0)
    | | |
    | | +-+-{vector(double)}-(0x9f3da70)
    | | | |
    | | | +---[double literal]-(0x9f3db68)
    | | |
    | | +---[vector()]-(0x9f3d9d8)
    | | |
    | | +-+-{vector(vector)}-(0x9f3d988)
    | |   |
    | |   +---[-]-(0x9f3d8a8)
    | |
    | +-+-{vector(double, double, double) OR vector(double) OR vector() OR vector(vector)}-(0x9f3d558)
    |   |
    |   +-+-{vector(double, double, double)}-(0x9f3d710)
    |   | |
    |   | +---[double literal]-(0x9f3d850)
    |   | |
    |   | +---[double literal]-(0x9f3d808)
    |   | |
    |   | +---[double literal]-(0x9f3d7c0)
    |   |
    |   +-+-{vector(double)}-(0x9f3d690)
    |   | |
    |   | +---[double literal]-(0x9f3d768)
    |   |
    |   +---[vector()]-(0x9f3d5f8)
    |   |
    |   +-+-{vector(vector)}-(0x9f3d4e0)
    |     |
    |     +---[-]-(0x9f3d730)
    |
    +-+-{tetrahedron(tetrahedron)}-(0x9f3d3c8)
      |
      +---[-]-(0x9f3c3e8)
 */
