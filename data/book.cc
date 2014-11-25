#include <sstream>
#include <iostream>
#include <stdexcept>

#include <ndk++/application.hh>
#include <ndk++/menu.hh>
#include <ndk++/dialog_box.hh>

#include "user_list.hh"

int main()
{
	try
	{
		ndk::application::obj_type &app = ndk::application::instance();

		user_list list;

		ndk::menu_bar main_menu;

		ndk::popup_menu	file_menu;
		file_menu.add("Exit", ndk::question("Are you really want to exit this application?",
						    "Confirm", 
						    ndk::slot(&ndk::application::obj_type::quit,&app)));

		ndk::popup_menu	action_menu;
		action_menu.add("Add user", ndk::slot(&user_list::add,&list));
		action_menu.add("Delete user", ndk::slot(&user_list::del,&list));
		action_menu.add("Edit user", ndk::message("Not implemented yet"));

		main_menu.add("File", &file_menu);
		main_menu.add("Actions", &action_menu);

		main_menu.visualize();
		list.visualize();
		file_menu.prepare();
		action_menu.prepare();

		ndk::align::widget(&list);

		app.in_focus(main_menu);

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
