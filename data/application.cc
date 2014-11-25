/* application.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/

#include <ncurses/color.hh>
#include <ndk++/palette.hh>
#include <ndk++/application.hh>
#include <ndk++/panel.hh>

using namespace ndk;

application_impl::application_impl():
		ncurses::application()
{
	palette::obj_type &p = palette::instance();

	p[colors::button_default] =
		p[colors::label] =
			p[colors::panel] = ncurses::color(COLOR_BLACK, COLOR_WHITE);

	p[colors::title] = p[colors::frame1] = ncurses::color(COLOR_WHITE, COLOR_WHITE, WA_BOLD);
	p[colors::frame2] = ncurses::color(COLOR_BLACK, COLOR_WHITE);

	p[colors::button_focused] = ncurses::color(COLOR_WHITE, COLOR_BLACK, WA_BOLD);
	p[colors::button_selected] = ncurses::color(COLOR_WHITE, COLOR_WHITE, WA_BOLD);
	p[colors::minibutton] = ncurses::color(COLOR_BLACK, COLOR_WHITE);
	p[colors::scrollbar_line] = ncurses::color(COLOR_WHITE, COLOR_BLACK);
	p[colors::textview_active] = ncurses::color(COLOR_WHITE, COLOR_WHITE, A_BOLD);
	p[colors::textview_inactive] = ncurses::color(COLOR_BLACK, COLOR_WHITE);

	p[colors::edit_active] = ncurses::color(COLOR_WHITE, COLOR_BLACK, A_BOLD);
	p[colors::edit_inactive] = ncurses::color(COLOR_WHITE, COLOR_BLACK);
}

engine *application_impl::events()
{
	return &events_;
}

void application_impl::exec()
{
	events_.exec();
}

void application_impl::quit()
{
	events_.quit();
}

void application_impl::in_focus(panel& other)
{
	events_.in_focus(other.events());
}
