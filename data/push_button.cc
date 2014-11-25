/* push_button.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <ncurses/pen.hh>
#include <ndk++/align.hh>
#include <ndk++/push_button.hh>

using namespace ndk;

push_button::push_button(const std::string& str, ndk::panel *parent):
		button(str, parent)
{
	if (!str.empty()) resize(str.length() + 4, 1);
}

void push_button::draw()
{
	ncurses::pen p(*this);
	set_color(p);

	//enchance button text if we need it
	if ((text_.length() + 4) < width())
	{
		const unsigned int pos = align::pos(text_.length(), width() - 4, align::center);
		text_.insert(text_.begin(), pos, ' ');
		if (width() > text_.length() + 4) text_.append(width() - 4 - text_.length(), ' ');
	}

	p.move(0, 0);
	p.write("[ ");
	p.write(text_.substr(0, width() - 4));
	p.write(" ]");
}
