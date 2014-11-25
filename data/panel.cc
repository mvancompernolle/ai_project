/* panel.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <ncurses/pen.hh>
#include <ndk++/palette.hh>

#include <ndk++/panel.hh>


ndk::panel::panel(panel *parent):
		ncurses::panel(parent),
		events_(parent ? parent->events() : 0, this)
{
	events()->on_system(ndk::slot(&panel::at_system, this));
}

/**
 * @todo add support for palette
 */
void ndk::panel::draw()
{
	ncurses::pen(*this).fill(palette::instance()[colors::panel]);
}

int ndk::panel::padding() const
{
	return 0;
}

void ndk::panel::exec()
{
	show();
	::update_panels();
	events_.exec();
	hide();
}


void ndk::panel::emit(int type)
{
    events_.emit(type);
}

ndk::engine *ndk::panel::events()
{
	return &events_;
}

void ndk::panel::in_focus(panel& other)
{
	events_.in_focus(other.events());
}

ndk::event::result ndk::panel::at_system(const event& ev)
{
	if (!ev.sender_ && engine::enter_focus == ev.type_)
	{
		top();
		return event::accepted;
	}
	return event::ignored;
}
