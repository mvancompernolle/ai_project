/* ndk_packer.cc
* This file is part of NDK++ library
* Copyright (c) 2003 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <stdexcept>

#include <ndk++/application.hh>
#include <ndk++/panel.hh>
#include <ndk++/frame.hh>
#include <ndk++/label.hh>
#include <ndk++/button.hh>
#include <ndk++/push_button.hh>
#include <ndk++/switch_button.hh>
#include <ndk++/packer.hh>

int main(int, char **)
{
	try
	{
		ndk::application::obj_type &app = ndk::application::instance();

		//big frame
		ndk::frame big;
		big.move(1, 1);

		//first frame with packing
		ndk::frame frame1(&big);
		ndk::label label1("#1", &frame1);
		ndk::label label2("llabel #2 xxxx", &frame1);
		ndk::label label3_1("label #3.1 xxxxx", &frame1);
		ndk::label label3("label #3 xxxx", &frame1);
		ndk::label label4("label #4", &frame1);
		ndk::label label4_1("#4", &frame1);

		{
			ndk::vertical_packer pack(&frame1, ndk::align::center, 1);
			pack.add(&label1);
			pack.add(&label2);
			pack.add(&label3);
			pack.add(&label3_1);
			pack.add(&label4);
			pack.add(&label4_1);
			pack.pack();
		}

		//second pack
		ndk::frame frame2(&big);

		ndk::check_button check1("Check #1", &frame2);
		ndk::check_button check2("Other Check #1", &frame2);

		{
			ndk::vertical_packer pack(&frame2, ndk::align::right);
			pack.add(&check1);
			pack.add(&check2);
			pack.pack();
		}

		//pack small panels to the big
		{
			ndk::horizontal_packer pack(&big, ndk::align::center);
			pack.add(&frame1);
			pack.add(&frame2);
			pack.pack();
		}
		big.visualize();

		app.in_focus(big);
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
