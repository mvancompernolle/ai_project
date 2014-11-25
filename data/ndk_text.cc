/* ndk_text.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <ndk++/text.hh>
#include <ndk++/push_button.hh>
#include <ndk++/packer.hh>
#include <ndk++/application.hh>
#include <ndk++/dialog_box.hh>

int main(int, char **)
{
	try
	{
		ndk::application::obj_type &app = ndk::application::instance();
		const std::string filename("../README");
		ndk::panel	panel;

		ndk::text_view	view(filename, &panel);
		view.resize((app.width()*2) / 3, (app.height()*2) / 3);

		std::ifstream in(filename.c_str()); //!!!if (!in) NDK::Message...
		if (!in)
		{
			ndk::message("Cannot open file: " + filename)();
			return EXIT_FAILURE;
		}

		std::string str;
		while (std::getline(in, str)) view.add_text(str + "\n");

		ndk::push_button	button("Exit", &panel);
		button.on_click(ndk::slot(&ndk::application::obj_type::quit, &app));

		ndk::vertical_packer pack(&panel, ndk::align::center);
		pack.add(&view);
		pack.add(&button);
		pack.pack();

		panel.visualize();
		ndk::align::widget(&panel);

		panel.in_focus(view);
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
