/* ndk_scroll.cc
* This file is part of NDK++ library
* Copyright (c) 2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <stdexcept>

#include <ndk++/application.hh>
#include <ndk++/scroll_bar.hh>
#include <ndk++/frame.hh>
#include <ndk++/label.hh>

int main(int, char **)
{
	try
	{
		ndk::application::obj_type &app = ndk::application::instance();

		ndk::frame frame;
		frame.resize(60, 20);

		ndk::label label("Scroll bars demo", &frame);
		ndk::v_scroll_bar vbar(20, &frame);
		ndk::h_scroll_bar hbar(20, &frame);
		vbar.resize(15);
		hbar.resize(15);
		vbar.move(1, 1);
		hbar.move(2, 10);
		label.move(10, 1);

		//special for demo made this widget active
		vbar.events()->set_accept(true);
		hbar.events()->set_accept(true);

		frame.visualize();

		app.in_focus(frame);
		app.exec();
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
