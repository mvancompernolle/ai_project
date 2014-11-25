/* ndk_listbox.cc
* This file is part of NDK++ library
* Copyright (c) 2003 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <ndk++/application.hh>
#include <ndk++/switch_button.hh>
#include <ndk++/push_button.hh>
#include <ndk++/edit.hh>
#include <ndk++/list_box.hh>

int main(int, char **)
{
	try
	{
		ndk::application::obj_type &app = ndk::application::instance();
		ndk::panel	panel;

		ndk::list_box	listbox(&panel);
		ndk::push_button	button("Exit", &panel);

		button.on_click(ndk::slot(&ndk::application::obj_type::quit, &app));
		listbox.resize(app.width() / 3, app.height() / 3);

		for (int i = 0;i <= 40;++i)
		{
			std::ostringstream ost;
			ost << "button #" << i;
			listbox.add(ost.str());
		}

		//add special buttons to the list
		listbox.add<ndk::check_button>("button #c");
		listbox.add<ndk::check_button>("button #d");
		listbox.add<ndk::edit>("line edit");
		listbox.add<ndk::push_button>("push button");

		listbox.remove(3); //remove test
		listbox.set_current(13); //set current test

		ndk::vertical_packer pack(&panel, ndk::align::center);
		pack.add(&listbox);
		pack.add(&button);
		pack.pack();

		panel.visualize();
		ndk::align::widget(&panel);

		panel.in_focus(listbox);
		app.in_focus(panel);

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
