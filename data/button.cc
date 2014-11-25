/* button.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <ncurses/pen.hh>
#include <ndk++/button.hh>
#include <ndk++/palette.hh>
#include <ndk++/application.hh>

using namespace ndk;

namespace
{
	struct run_callback
	{ /**< functor to run callback functions */
		template <typename CallbackT>
		void operator()(CallbackT& callback)
		{
			callback();
		}
	};
}


mini_button::mini_button(const std::string& str, ndk::panel *parent, bool accept):
		label(str, parent)
{
	events()->on_mouse(slot(&mini_button::at_mouse, this));
	events()->on_keyboard(slot(&mini_button::at_keyboard, this));
	events()->set_accept(accept);
}

event::result mini_button::at_mouse(const mouse&)
{
	click();
	return event::accepted;
}

event::result mini_button::at_keyboard(const keyboard& ev)
{
	if ('\r' == ev.code_) //user pressed enter on the button
	{
		click(); //call default action first
		return event::accepted;
	}

	return event::ignored;
}


void mini_button::set_color(ncurses::pen& p)
{
	p.set_color(palette::instance()[colors::minibutton]);
}

void mini_button::on_click(const button_command& action, bool front)
{
	if (front)
		act_.push_front(action);
	else
		act_.push_back(action);
}

void mini_button::click()
{
	std::for_each(act_.begin(), act_.end(), run_callback());
	emit(clicked);
}

button::button(const std::string& str, panel *parent, bool accept):
		mini_button(str, parent, accept),
		selected_(false),
		state_(engine::leave_focus)
{
	events()->on_system(slot(&button::at_system, this));
}

void button::set_color(ncurses::pen& p)
{
	ncurses::color color;
	switch (state_)
	{
	case engine::enter_focus:
		color = palette::instance()[colors::button_focused];
		break;
	case engine::leave_focus:
		color = palette::instance()[(selected_ ?
					     colors::button_selected :
					     colors::button_default
					    )];
		break;
	default:
		color = palette::instance()[colors::button_default];
		break;
	};
	p.set_color(color);
}


//Note:
//There are following reason to move widget's panel to the top:
//ncurses has strange algorithm of moving cursor
//if widget on the top, it's cursor is active and visible on the screen
//I need this feature for the Line Edit
event::result button::at_system(const event& ev) /**< system event processor */
{
	event::result res = event::ignored;

	if (!ev.sender_)
	{
		state_ = ev.type_;
		res = event::accepted;
		draw();
	}

	return res;
}

void button::select()
{
	selected_ = true;
	draw();
}

void button::unselect()
{
	selected_ = false;
	draw();
}
