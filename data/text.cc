/* text.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <ncurses/pen.hh>

#include <ndk++/text.hh>
#include <ndk++/palette.hh>

using namespace ndk;

text::text(const std::string& str, ndk::panel *parent, wrap_type wrapping):
		ndk::panel(parent),
		x_(0),
		y_(0),
		wrapping_(wrapping)
{
	ncurses::pen	pen(pad_);
	if (!str.empty())
	{
		int h = string_height(str);
		if (*(str.end() - 1) != '\n') ++h; //extra hack only for the first time
		const int w = string_width(str);
		resize(w, h);
		pen.write(str); //add next text to the pad
	}
	pen.fill(palette::instance()[colors::label]);

	events()->on_keyboard(slot(&text::at_keyboard, this));
}

void text::add_text(const std::string& str)
{
	//calculate new height
	pad_.resize(
		(wrapping_ == wrap) ? width() : std::max(pad_.width(), string_width(str) + 1),
		std::max(height(), ncurses::pen(pad_).y() + string_height(str)));
	ncurses::pen(pad_).write(str); //add next text to the pad
}

void text::draw() /**< draw frame on the panel */
{
	pad_.draw(*this, x_, y_); //draw pad content in the viewport
}

void text::right()
{
	if (x_ + width() < pad_.width() - 1 )
	{
		++x_;
		draw();

	}
}

void text::left()
{
	if (x_ > 0)
	{
		--x_;
		draw();
	}
}

void text::down()
{
	if (y_ + height() < pad_.height())
	{
		++y_;
		draw();
	}
}

void text::up()
{
	if (y_ > 0)
	{
		--y_;
		draw();
	}
}


event::result text::at_keyboard(const keyboard& ev)
{
	event::result res = event::ignored;
	switch (ev.code_)
	{
	case KEY_RIGHT:
		right();
		res = event::accepted;
		break;
	case KEY_LEFT:
		left();
		res = event::accepted;
		break;
	case KEY_UP:
		up();
		res = event::accepted;
		break;
	case KEY_DOWN:
		down();
		res = event::accepted;
		break;
	default:
		break;
	};
	if (event::accepted == res)
	{
		ncurses::pen(*this).commit();
		emit(changed);
	}

	return res;
}

unsigned int text::string_height(const std::string& str) /**< @return number of strings */
{
	unsigned int count = 0;
	if (wrapping_ == wrap)
		//in wrap mode we need additional recalculations of the string height
		count = str.length() / this->width() + ((str.length() % this->width() != 0) ? 1 : 0);
	else
		//not wrap mode: only calculate number of '\n'
		count = std::count(str.begin(), str.end(), '\n');

	//we also need an additional space below if line have '\n' at the end
	if (*(str.end() - 1) == '\n') ++count; //grow width if nesessary
	return count;
}

unsigned int text::string_width(const std::string& str) /**< @return maximum string length */
{
	std::string::const_iterator beg = str.begin();
	const std::string::const_iterator end = str.end();
	unsigned int	max_len = 0;
	bool	need_fix = true;
	do
	{
		std::string::const_iterator curr(std::find(beg, str.end(), '\n'));
		max_len = std::max(max_len,
				   std::distance(beg, curr) + ((need_fix) ? ncurses::pen(pad_).x() : 0));
		beg = (curr == end) ? curr : (curr + 1);
		need_fix = false;
	}
	while (beg != end);
	return max_len;
}
void text::resize(unsigned int w, unsigned int h)
{
	panel::resize(w, h);
	//grow size of the text if we need it
	pad_.resize(std::max(pad_.width(), w + 1), std::max(pad_.height(), h));
}

void text::clear()
{
	ncurses::pen(pad_).clear();
	x_ = 0;
	y_ = 0;
	//truncate buffer to viewport size
	pad_.resize(this->width(), this->height());
	draw();
}

void text::set_text(const std::string& str)
{
	clear();
	add_text(str);
	draw();
}

unsigned int text::length() const
{
	return pad_.height();
}

unsigned int text::last_line() const
{
	return (y_ + height());
}

text_view::text_view(const std::string& title, ndk::panel *parent):
		ndk::panel(parent),
		text_("", this, text::wrap),
		bar_(0, this),
		title_(title),
		active_(false)
{
	events()->on_system(slot(&text_view::at_system, this));
	events()->on_mouse(slot(&text_view::at_mouse, this));
	in_focus(text_);
}

void text_view::add_text(const std::string& str)
{
	text_.add_text(str);
	bar_.set_max(text_.length());
}

void text_view::set_text(const std::string& str)
{
	text_.set_text(str);
	bar_.set_max(text_.length());
}

void text_view::draw()
{
	//pack widgets
	packer<pack::horizontal, pack::no_resizer> pack(this, align::center);
	pack.add(&text_);
	pack.add(&bar_);
	pack.pack();

	panel::draw();

	ncurses::pen pen(*this);

	pen.set_color(palette::instance()[(active_) ? colors::textview_active : colors::textview_inactive]);
	//draw border and title
	pen.frame();

	if (!title_.empty())
	{
		std::string out(title_);

		//if we have enough width made title more beautiful
		const int len = title_.length();
		const int wid = width();
		if (len < wid)
		{
			if (len + 6 < wid) out = " " + out + " ";
			pen.move(align::pos(out.size(), wid, align::center), 0);
		}
		else
		{
			out = out.substr(0, wid);
			pen.move(0, 0);
		}
		pen.write(out);
	}
}

int text_view::padding() const
{ //save frame during packing
	return 1;
}

void text_view::resize(unsigned int w, unsigned int h)
{
	panel::resize(w, h);
	if ((w > 2) && (h > 2))
	{
		w -= 2;
		h -= 2;
		text_.resize(w, h);
		bar_.resize(h);
	}
}

event::result text_view::at_system(const event& ev)
{
	event::result res = event::ignored;
	if (!ev.sender_)
	{
		active_ = (engine::enter_focus == ev.type_);
		prepare(); //redraw
		res = event::accepted;
	}
	else if (text_.events() == ev.sender_ && text::changed == ev.type_)
	{
			bar_.set(text_.last_line());
			res = event::accepted;	    
	}
	else if (bar_.events() == ev.sender_)
	{
		if (ev.type_ == v_scroll_bar::go_up)
		    text_.up();
		else if (ev.type_ == v_scroll_bar::go_down)
		    text_.down();
		res = event::accepted;
	}
	return res;
}

event::result text_view::at_mouse(const mouse& ev)
{
	if (mouse::over(ev, bar_)) bar_.events()->rat(ev);
	return event::accepted;
}
