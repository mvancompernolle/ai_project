/**
 *  @example lagoon/example/factories/morse_code.cpp
 *
 *  This example shows how to use a factory created by the factory generator
 *  with a script parsing plugin to generically construct instances
 *  of classes with non default constructors, from input strings.
 *
 *  Copyright 2006-2011 Matus Chochlik. Distributed under the Boost
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

#include <mirror/mirror_base.hpp>
#include <mirror/pre_registered/basic.hpp>
#include <mirror/pre_registered/class/std/vector.hpp>
#include <mirror/pre_registered/class/std/map.hpp>
#include <mirror/utils/quick_reg.hpp>
#include <lagoon/lagoon.hpp>
#include <lagoon/utils/script_factory.hpp>
#include <iostream>

namespace morse {

enum class signal { dash = '-', dot = '.' };
typedef std::vector<signal> sequence;
typedef std::map<char, sequence> code;

} // namespace morse

MIRROR_REG_BEGIN

MIRROR_QREG_GLOBAL_SCOPE_NAMESPACE(morse)
MIRROR_QREG_ENUM(morse, signal, (dash)(dot))

MIRROR_REG_END

int main(void)
{
	try
	{
		using namespace lagoon;
		c_str_script_factory_builder builder;
		c_str_script_factory_input in;
		auto data = in.data();
		//
		auto meta_morse_code = reflected_class<morse::code>();
		auto morse_code_factory = meta_morse_code->make_factory(
			builder,
			raw_ptr(&data)
		);
		// TODO: fix the rest of the codes
		const char input[] = "{ \
			{'A', {dot, dash}}, \
			{'B', {dash, dot, dot, dot}}, \
			{'C', {dash, dot, dash, dot}}, \
			{'D', {dash, dot, dot}}, \
			{'E', {dot}}, \
			{'F', {dot, dot, dash, dot}}, \
			{'G', {dash, dash, dot}}, \
			{'H', {dot, dot, dot, dot}}, \
			{'I', {dot, dot}}, \
			{'J', {dot, dash, dash, dash}}, \
			{'K', {dash, dot, dash}}, \
			{'L', {dot, dash, dot, dot}}, \
			{'M', {dash, dash}}, \
			{'N', {dash, dot}}, \
			{'O', {dash, dash, dash}}, \
			{'P', {dot, dash, dash, dot}}, \
			{'Q', {dash, dash, dot, dash}}, \
			{'R', {dot, dash, dot}}, \
			{'S', {dot, dot, dot}}, \
			{'T', {dash}}, \
			{'U', {dot, dot, dash}}, \
			{'V', {dot, dot, dot, dash}}, \
			{'W', {dot, dash, dash}}, \
			{'X', {dash, dot, dot, dash}}, \
			{'Y', {dash, dot, dash, dash}}, \
			{'Z', {dash, dash, dot, dot}}, \
			{'1', {dot, dash, dash, dash, dash}}, \
			{'2', {dot, dot, dash, dash, dash}}, \
			{'3', {dot, dot, dot, dash, dash}}, \
			{'4', {dot, dot, dot, dot, dash}}, \
			{'5', {dot, dot, dot, dot, dot}}, \
			{'6', {dash, dot, dot, dot, dot}}, \
			{'7', {dash, dash, dot, dot, dot}}, \
			{'8', {dash, dash, dash, dot, dot}}, \
			{'9', {dash, dash, dash, dash, dot}}, \
			{'0', {dash, dash, dash, dash, dash}} \
		}";
		in.set(input, input+sizeof(input));
		raw_ptr pmc = morse_code_factory->new_();
		morse::code& mc = *raw_cast<morse::code*>(pmc);
		for(auto i = mc.begin(), e = mc.end(); i != e; ++i)
		{
			std::cout << "Morse code for '" << i->first << "': ";
			auto j = i->second.begin(), f = i->second.end();
			while(j != f)
			{
				std::cout << char(*j);
				++j;
			}
			std::cout << std::endl;
		}
		meta_morse_code->delete_(pmc);
	}
	catch(std::exception const& error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
	}
	//
	return 0;
}

/* Example of output:
Morse code for '0': -----
Morse code for '1': .----
Morse code for '2': ..---
Morse code for '3': ...--
Morse code for '4': ....-
Morse code for '5': .....
Morse code for '6': -....
Morse code for '7': --...
Morse code for '8': ---..
Morse code for '9': ----.
Morse code for 'A': .-
Morse code for 'B': -...
Morse code for 'C': -.-.
Morse code for 'D': -..
Morse code for 'E': .
Morse code for 'F': ..-.
Morse code for 'G': --.
Morse code for 'H': ....
Morse code for 'I': ..
Morse code for 'J': .---
Morse code for 'K': -.-
Morse code for 'L': .-..
Morse code for 'M': --
Morse code for 'N': -.
Morse code for 'O': ---
Morse code for 'P': .--.
Morse code for 'Q': --.-
Morse code for 'R': .-.
Morse code for 'S': ...
Morse code for 'T': -
Morse code for 'U': ..-
Morse code for 'V': ...-
Morse code for 'W': .--
Morse code for 'X': -..-
Morse code for 'Y': -.--
Morse code for 'Z': --..
 */
