/* frame.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/

#include <ncurses/pen.hh>
#include <ndk++/frame.hh>
#include <ndk++/palette.hh>


ndk::frame::frame(panel *parent, style st):
		panel(parent),
		style_(st)
{}

void ndk::frame::draw()
{
	panel::draw();

	ncurses::pen p(*this);
	p.clear();

	//draw first angle
	p.set_color(palette::instance()[(style_ == raised) ? colors::frame1 : colors::frame2]);
	p.write(ncurses::pen::acs::ulcorner());

	if (width() > 2) p.h_line(width() - 2);
	p.move(0, 1);
	if (height() > 2) p.v_line(height() - 2);
	p.move(0, height() - 1);
	p.write(ncurses::pen::acs::llcorner());

	//draw second angle
	p.set_color(palette::instance()[(style_ == raised) ? colors::frame2 : colors::frame1]);
	if (width() > 2) p.h_line(width() - 2);
	p.move(width() - 1, height() - 1);
	p.write(ncurses::pen::acs::lrcorner());
	p.move(width() - 1, 0);
	p.write(ncurses::pen::acs::urcorner());
	p.move(width() - 1, 1);
	if (height() > 2) p.v_line(height() - 2);
}

int ndk::frame::padding() const
{
	return 1;
}
