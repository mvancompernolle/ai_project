/* list_box.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <ndk++/list_box.hh>

using namespace ndk;

list_box::list_box(panel *parent):
		panel(parent),
		lister_(this),
		bar_(0, this)
{
	events()->on_system(slot(&list_box::at_system, this));
	events()->on_mouse(slot(&list_box::at_mouse, this));
	in_focus(lister_);
}

unsigned int list_box::size()
{
	return lister_.size();
}

void list_box::set_current(unsigned int num)
{
	lister_.set_current(num);
}

void list_box::resize(unsigned int w, unsigned int h)
{
	panel::resize(w, h); //first resize itself
	--w;
	if (w > 0)
	{
		lister_.resize(w, h);
		bar_.resize(h);
	}
}

void list_box::draw()
{ //Note: we don't need call Panel::draw() 'cause lister+bar cover general panel
	//set values
	bar_.set_max((lister_.size() > 2) ? (lister_.size() - 1) : 1);
	bar_.set((lister_.size() > 2) ? lister_.current() : 1);

	//pack widgets
	packer<pack::horizontal, pack::no_resizer> pack(this, align::center);
	pack.add(&lister_);
	pack.add(&bar_);
	pack.pack();

	//redraw subwidgets:packer with no resize policy not made it
	lister_.draw();
	bar_.draw();
}

event::result list_box::at_system(const event& ev)
{
	event::result res = event::ignored;

	if (!lister_.size() && (vertical_lister::changed != ev.type_)) return res;

	if (lister_.events() == ev.sender_ && vertical_lister::changed == ev.type_)
	{
		bar_.set(lister_.current());
		emit(changed); //re-emit
		res = event::accepted;
	}
	else if (bar_.events() == ev.sender_)
	{
	    if (v_scroll_bar::go_up == ev.type_)
		lister_.up();
	    else if (v_scroll_bar::go_down == ev.type_)
		lister_.down();
	    res = event::accepted;
	}

	return res;
}


event::result list_box::at_mouse(const mouse& ev)
{
	if (mouse::over(ev, bar_)) bar_.events()->rat(ev);
	return event::accepted;
}

unsigned int list_box::current()
{
	return lister_.current();
}

button *list_box::current_item()
{
	return lister_.current_item();
}

void list_box::remove(unsigned int num)
{
	lister_.remove(num); //then remove from the lister
}
