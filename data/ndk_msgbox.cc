/* ndk_msgbox.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <stdexcept>

#include <ndk++/application.hh>
#include <ndk++/dialog_box.hh>


int main(int, char **)
{
	try
	{
		ndk::application::obj_type &app = ndk::application::instance();

		ndk::message_box<>("Simple message:Hello NDK++ World!")();
		ndk::message_box<>("Multi-line message:\nHello NDK++ World again!")();
		if (ndk::select_box<>("This is a select box (first type)", "Question")())
			ndk::message_box<>("Answer was a true")();
		else
			ndk::message_box<>("Answer was a false")();

	}
	catch (std::runtime_error e)
	{
		std::cerr << "catch: runtime error " << e.what() << std::endl;
	}
	catch (std::exception e)
	{
		std::cerr << "catch: stl exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "unknown exception\n";
	}

}
