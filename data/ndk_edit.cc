/* ndk_edit.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <locale.h>

#include <ndk++/application.hh>
#include <ndk++/packer.hh>
#include <ndk++/edit.hh>
#include <ndk++/push_button.hh>

/*
#include <ndk++/dialog_box.h>
 
 
class view_texts
{
public:
	view_texts(NDK::Input& i1, NDK::Input& i2, NDK::Input& i3, NDK::Input& i4, NDK::Input& i5):
			i1_(i1),
			i2_(i2),
			i3_(i3),
			i4_(i4),
			i5_(i5)
	{}
	void operator()()
	{
		std::ostringstream ost;
		ost << "Edit1:" << i1_.text() << std::endl;
		ost << "Edit2:" << i2_.text() << std::endl;
		ost << "Edit3:" << i3_.text() << std::endl;
		ost << "Edit4:" << i4_.text() << std::endl;
		ost << "Edit5:" << i5_.text() << std::endl;
		NDK::Message(ost.str())();
		NDK::Application::instance().quit();
	}
private:
	NDK::Input& i1_;
	NDK::Input& i2_;
	NDK::Input& i3_;
	NDK::Input& i4_;
	NDK::Input& i5_;
};
*/

int main(int, char **)
{
	try
	{
		setlocale(LC_ALL, "");

		ndk::application::obj_type &app = ndk::application::instance();
		ndk::panel panel;

		ndk::input edit("Common Edit:", &panel);
		ndk::input passwd_edit("Edit with password echo mode:", &panel);
		ndk::input noecho_edit("Edit with no_echo mode:", &panel);
		ndk::input max_edit("with max text size:", &panel);
		ndk::input mask_edit("edit with mask:", &panel);
		ndk::mask phone("(###)-###-##-##;0");
		passwd_edit.set_mode(ndk::edit::password);
		noecho_edit.set_mode(ndk::edit::no_echo);
		max_edit.set_max(10);
		mask_edit.set_mask(&phone);


		edit.set_text("1234567890 this is a very big,big text very very long text, I say long");
		max_edit.set_text("1234567890 this is a text");


		ndk::push_button button("Some Button", &panel);
		edit.resize(60);
		passwd_edit.resize(60);
		noecho_edit.resize(60);
		max_edit.resize(60);
		mask_edit.resize(60);

		ndk::vertical_packer pack(&panel, ndk::align::center);
		pack.add(&edit);
		pack.add(&passwd_edit);
		pack.add(&noecho_edit);
		pack.add(&max_edit);
		pack.add(&mask_edit);

		pack.add(&button);
		pack.pack();
		/*
				button.on_click(view_texts(edit,
							   passwd_edit,
							   noecho_edit,
							   max_edit,
							   mask_edit));
		*/

		ndk::align::widget(&panel);
		panel.visualize();

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
