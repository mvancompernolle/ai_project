/* ndk_buttons.cc
* This file is part of NDK++ library
* Copyright (c) 2003 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <iostream>
#include <stdexcept>

#include <ndk++/application.hh>
#include <ndk++/frame.hh>
#include <ndk++/label.hh>
#include <ndk++/button.hh>
#include <ndk++/push_button.hh>
#include <ndk++/switch_button.hh>
#include <ndk++/button_group.hh>


class my_panel: public ndk::frame
{
public:
	my_panel(ndk::panel *parent = 0):
			ndk::frame(parent),
			button1("Button #1", this),
			button2("Button #2", this),
			pbutton1("  OK  ", this),
			pbutton2("Push Button", this),
			label("Some Label", this)
	{
		resize(60, 20);
		button1.move(2, 2);
		button2.move(2, 4);
		pbutton1.move(2, 7);
		pbutton2.move(2, 9);
		label.move(2, 11);

		pbutton1.on_click(ndk::slot(&my_panel::change_text, this, "OK clicked"));
		pbutton2.on_click(ndk::slot(&my_panel::change_text, this, "Cancel clicked"));
	}

	virtual ~my_panel()
	{}

private:
	void change_text(const char* txt) //slot to change text of the label
	{
		label.set_text(txt);
		label.resize(strlen(txt), 1);
		touch_line(11);
		label.draw();
		::update_panels();
	}

	ndk::button button1, button2;
	ndk::push_button pbutton1, pbutton2;
	ndk::label	label;
};

int main(int, char **)
{
	try
	{
		ndk::application::obj_type &app = ndk::application::instance();

		my_panel panel1;
		panel1.move(3, 3);
		panel1.show();

		//add second panel to check activation/deactivation features
		ndk::frame panel2;
		panel2.move(7, 7);
		panel2.resize(60, 20);

		//Check Buttons
		ndk::check_button check1("Check #1", &panel2);
		ndk::check_button check2("Check #2", &panel2);
		check1.move(2, 2);
		check2.move(2, 3);
		check1.on();

		//Radio Buttons
		ndk::radio_manager	gr; //logic object: group of the buttons
		ndk::radio_button	radio1("Radio #1", gr, &panel2);
		ndk::radio_button	radio2("Radio #2", gr, &panel2);
		ndk::radio_button	radio3("Radio #3", gr, &panel2);
		radio1.move(20, 2);
		radio2.move(20, 3);
		radio3.move(20, 4);


		//Group of special buttons
		ndk::check_group checks(&panel2);
		checks.add("check #1");
		checks.add("check #2");
		checks.add("check #3");
		checks.resize(15, 3);
		checks.move(2, 6);

		ndk::radio_group radios(&panel2);
		radios.add("radio #1");
		radios.add("radio #2");
		radios.add("radio #3");
		radios.resize(15, 3);
		radios.move(20, 6);
		gr.activate(&radio1);

		panel1.visualize();
		panel2.visualize();
		app.in_focus(panel2);
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
