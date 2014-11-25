/* ndk_application.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <stdexcept>

#include <ndk++/application.hh>
#include <ndk++/frame.hh>
#include <ndk++/label.hh>


int main(int, char **)
{
	try
	{

		ndk::application::obj_type &app = ndk::application::instance();

		ndk::frame panel1;
		panel1.move(1, 1);
		panel1.resize(60, 15);
		ndk::label	label("Panel #1", &panel1);
		label.move(1, 1);

		panel1.visualize();

		ndk::frame panel2(0, ndk::frame::sunken);
		panel2.move(7, 7);
		panel2.resize(60, 15);
		ndk::label	label1("Panel #2", &panel2);
		label1.move(1, 1);

		panel2.visualize();

		app.in_focus(panel2);

		app.exec();
		//		panel2.exec(); //test modal
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
