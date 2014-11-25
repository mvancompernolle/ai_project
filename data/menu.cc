/* menu.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <ndk++/menu.hh>
#include <ndk++/application.hh>

using namespace ndk;

popup_menu::popup_menu(panel *parent):
		base_type(parent),
		up_(0),
		plus_(0)
{
	events()->on_keyboard(slot(&popup_menu::at_keyboard, this));
	events()->on_system(slot(&popup_menu::at_system, this));
}

event::result popup_menu::at_keyboard(const keyboard& ev)
{
	event::result res = event::ignored;
	if ((27 == ev.code_) && up_)
	{
		events()->switch_to(up_->events());
		res = event::accepted;
	}
	return res;
}

event::result popup_menu::at_system(const event& ev)
{
	event::result res = event::ignored;
	if (!ev.sender_)
	{
	    if (engine::enter_focus == ev.type_)
	    {
		    show(); //enter_focus it's a signal to work
		    res = event::accepted;
	    }
	    else if (engine::leave_focus == ev.type_)
	    {
		    if (up_ && !hidden()) //protection from infinite loops
		    {
			hide();
			//make sure we are switching focus to up level
			events()->switch_to(up_->events());
			res = event::accepted;
		    }
	    }
	}
	return res;
}

//Note: default height is 1,'cause ncurses interprets 0 as a Application::height()
void popup_menu::adjust_size(button *item)
{
	menu<list::vertical>::resize(std::max(width(), item->width()),
				     height() + plus_);
	plus_ = 1;
}

void popup_menu::at_run()
{
	//if submenu pass control to up-level
	if (up_) events()->switch_to(up_->events());
}

void popup_menu::up_menu(panel* up)
{
	up_ = up;
}

menu_bar::menu_bar(panel *parent):
		base_type(parent)
{
	resize((parent) ? parent->width() : application::instance().width(), 1);
}

void menu_bar::add(const std::string& name, popup_menu* popup)
{
	//prepare special functor for the popup menu and insert into the button
	popup->up_menu(this); //bind with up level menu
	add(name, slot(&menu_bar::submenu, this, popup));
}


void menu_bar::submenu(popup_menu* popup)
{
	//recalculate new position of popup
	//note: I will need to made more complicated algo here
	int new_x = 0, new_y = 0;
	if ((application::instance().width() - current_item()->x()) >= popup->width())
		new_x = current_item()->x();
	else
		new_x = current_item()->x() + current_item()->width() - popup->width();

	if ((application::instance().height() - y() - 1 ) > popup->height())
		new_y = y() + 1;
	else
		new_y = y() - popup->height();

	popup->move(new_x, new_y, ncurses::absolute);
	events()->switch_to(popup->events()); //pass control to submenu
}
