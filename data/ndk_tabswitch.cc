/* ndk_tabswitch.cc
* This file is part of NDK++ library
* Copyright (c) 2003 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <stdexcept>

#include <ndk++/application.h>
#include <ndk++/dialog_box.h>
#include <ndk++/menu.h>
#include <ndk++/list_box.h>

int main(int, char **)
{
	try
	{
		NDK::Application &app = NDK::Application::instance();

		NDK::Frame	panel;
		NDK::Button	b1("Button 1", &panel), b2("Button 2", &panel);
		NDK::Frame	subpanel(&panel);
		NDK::PushButton	b3("Button 3", &subpanel), b4("Button 4", &subpanel);
		NDK::ListBox	b5(&subpanel);

		b5.add("zerg");
		b5.add("zerg1");
		b5.resize(30, 4);

		NDK::Label	label("Some Label", &panel);
		NDK::VerticalPacker packer(&subpanel, NDK::Align::center);
		packer.add(&b3), packer.add(&b4), packer.add(&b5), packer.pack();

		panel.resize(70, 10);
		b1.move(2, 2);
		label.move(2, 3);
		b2.move(2, 4);
		subpanel.move(20, 1);

		NDK::Align::make(&panel);

		panel.visualize();

		app.in_focus(&panel);

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
