/* ndk_menu.cc
* This file is part of NDK++ library
* Copyright (c) 2003 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <stdexcept>

#include <ndk++/application.hh>
#include <ndk++/dialog_box.hh>
#include <ndk++/menu.hh>


int main(int, char **)
{
	try
	{
		ndk::application::obj_type &app = ndk::application::instance();

		ndk::popup_menu	single_popup;
		single_popup.move(10, 10);
		single_popup.add("s Item 1", ndk::message("Item #1 called"));
		single_popup.add("s Item 2", ndk::message("Item #2 called"));


		ndk::popup_menu	popup;
		popup.add("Item 1", ndk::message("Item #1 called"));
		popup.add("Item 2", ndk::message("Item #2 called"));
		popup.add("Item 3", ndk::message("Item #3 called"));
		popup.add("Item 4", ndk::message("Item #4 called"));
		popup.move(3, 3);


		ndk::menu_bar top_menu;
		top_menu.add("MsgBox", ndk::message("Hello World"));
		top_menu.add("Other", ndk::message("Other Hello World"));
		top_menu.add("Popup", &popup);

		top_menu.visualize();
		single_popup.visualize();
		popup.prepare();

		app.in_focus(top_menu);
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
