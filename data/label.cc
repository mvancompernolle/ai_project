/* label.cc
* This file is part of NDK++ library
* Copyright (c) 2003 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <ndk++/label.hh>
#include <ndk++/palette.hh>

using namespace ndk;

label::label(const std::string& str, ndk::panel *parent):
		ndk::panel(parent),
		text_(str)
{
	if (!str.empty()) panel::resize(str.length(), 1);
	events()->set_accept(false); //this widget doesn't accept focus
}

void label::set_text(const std::string& str)
{
	text_ = str;
}

const std::string& label::text() const
{
	return text_;
}

void label::draw()
{
	draw(this->text_, 0); //simple drawing of the text
}

void label::draw(const std::string& str, std::string::size_type begin)
{
	ncurses::pen p(*this);
	set_color(p);
	p.move(0, 0);
	std::string out(str.substr(begin, this->width()));
	if (out.length() < this->width()) out.append(this->width() - out.length(), ' ');
	p.write(out);
}

void label::set_color(ncurses::pen& p)
{
	p.set_color(palette::instance()[colors::label]);
}
