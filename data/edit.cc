/* edit.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <cctype>

#include <ncurses/pen.hh>

#include <ndk++/packer.hh>
#include <ndk++/edit.hh>
#include <ndk++/palette.hh>
#include <ndk++/application.hh>



using namespace ndk;

mask::mask(const std::string& str):
		pattern_(str)
{
	init_mapping();
}

void mask::init_mapping()
{
	const std::string::size_type len = real_len();
	std::string::size_type mask_pos = 0;
	while (mask_pos <= len)
	{
		mask_pos = pattern_.find_first_of("#", mask_pos); //find any special mask symbol
		if (mask_pos == std::string::npos) break;
		mapping_.push_back(mask_pos);
		++mask_pos; //go to the next symbol to allow next find
	}
	mapping_.push_back(len); //append fake value
}

unsigned int mask::size() const
{
	return mapping_.size();
}

char mask::space() const
{
	char sym = ' ';
	//user defined space symbol
	if ((pattern_.size() >= 2) && (*(pattern_.end() - 2) == ';'))
		sym = *(pattern_.end() - 1);
	return sym;
}

bool mask::validate(unsigned int n, char sym) const /**< check are symbol at possition valid */
{
	if (n >= mapping_.size()) return false;

	switch (pattern_[mapping_.at(n)])
	{
	case '#':
		return ::isdigit(sym);
	default:
		return false; //answer false on unknown mask symbols
	}
}

std::string mask::apply(const std::string& str) const
{
	std::string out(pattern_.substr(0, real_len()));

	const std::string::size_type len = str.length();
	for (std::string::size_type i = 0;i < len;++i) out[mapping_.at(i)] = str[i];
	return out;
}

unsigned int mask::real_pos(unsigned int n) const
{
	return mapping_.at(n);
}

unsigned int mask::real_len() const
{
	return (space() == ' ') ? pattern_.length() : (pattern_.length() - 2);
}


edit::edit(const std::string& str, panel *parent, bool accept):
		button(str, parent, accept),
		current_(0),
		begin_(0),
		mode_(normal),
		max_(0),
		mask_(0)
{
	events()->on_keyboard(slot(&edit::at_keyboard, this));
	events()->on_system(slot(&edit::at_system, this));
}

void edit::resize(unsigned int size)
{
	button::resize(size, 1);
}

void edit::set_text(const std::string& str)
{
	label::set_text((max_ > 0) ? str.substr(0, max_) : str);
	current_ = 0;
	draw();
}

void edit::set_mode(echo_mode mode)
{
	mode_ = mode;
}

void edit::change_color()
{
	ncurses::pen(*this).set_color(palette::instance()
				      [
					      (state_ == engine::enter_focus) ? colors::edit_active : colors::edit_inactive
				      ]);
}

void edit::draw()
{
	//first process flags
	std::string out;
	switch (mode_)
	{
	case normal:
		out.assign(text());
		break;
	case no_echo:
		out.assign("");
		break;
	case password:
		out.assign(std::string(text().size(), '*'));
		break;
	default:
		break;
	}

	if (mask_) out = mask_->apply(out); //then process masking
	label::draw(out, begin_); //and draw
	set_cursor(); //and finally update cursor position
	ncurses::pen(*this).commit(); //commit changes to the screen
}

event::result edit::at_system(const event& ev)
{
	event::result res = event::ignored;
	if (!ev.sender_)
	{
	    if (engine::enter_focus == ev.type_)
	    {
		    application::instance().cursor_on();
		    set_cursor(); //only set cursor position, don't need full redraw
		    res = event::accepted;
	    }
	    else if (engine::leave_focus == ev.type_)
	    {
		    application::instance().cursor_off();
		    res = event::accepted;
	    }
	}
	return res;
}

event::result edit::at_keyboard(const keyboard& ev)
{
	event::result res = event::ignored;
	unsigned int pos = current_;
	switch (ev.code_)
	{
	case KEY_RIGHT:
		(scroll_right()) ? draw() : set_cursor();
		res = event::accepted;
		break;
	case KEY_LEFT:
		(scroll_left()) ? draw() : set_cursor();
		res = event::accepted;
		break;
	case KEY_BACKSPACE:
		--pos; //try to go to previous position
		if (pos < text_.length())
		{
			erase(pos);
			scroll_left(); //scroll all view to the left
			draw(); //we always need full redraw
			res = event::accepted;
		}
		break;
	case KEY_DC:
		if (pos < text_.length())
		{
			erase(pos);
			draw(); //we always need full redraw
			res = event::accepted;
		}
		break;
	case KEY_HOME:
		begin_ = 0;
		current_ = 0;
		draw();
		res = event::accepted;
		break;
	case KEY_END:
		current_ = text_.length();
		(recalibrate()) ? draw() : set_cursor();
		res = event::accepted;
		break;
	case 9:                             //don't beep on tab
		break;
	default:                                                            //enter key to the string
		if (std::isprint(ev.code_))
		{ //printable symbol
			if (((max_ == 0) || (text_.length() < max_)) &&                         //check for maximum
			    ((mask_ == 0) || mask_->validate(pos, ev.code_))) //validation in mask
			{
				insert(pos, ev.code_);
				scroll_right();
				draw(); //we always need full redraw
				res = event::accepted;
			}
		}
		else
			::beep();
		break;
	};

	return res;
}

bool edit::scroll_right()
{
	if (current_ < text_.length()) ++current_; //increase current
	//recalibrate begin, decide are we need to redraw
	return recalibrate();
}

bool edit::scroll_left()
{
	if (current_ > 0) --current_; //decrease current
	//recalibrate begin, decide are we need to redraw
	return recalibrate();
}

bool edit::recalibrate()
{
	bool res = false;
	const unsigned int cur_pos = (mask_) ? mask_->real_pos(current_) : current_; //real cursor position
	if ((begin_ < cur_pos) && ((cur_pos - begin_) > (width() - 1)))
	{ //may be cursor is over(after) edit screen?
		begin_ = cur_pos - width() + 1;
		res = true;
	}
	else if (begin_ > cur_pos)
	{ //may be cursor is over(before) edit screen?
		begin_ = cur_pos;
		res = true;
	}
	return res;
}

void edit::set_cursor()
{	//always move to zero position in no_echo mode
	const unsigned int cur_pos = (mask_) ? mask_->real_pos(current_) : current_; //real cursor position
	ncurses::pen(*this).move((mode_ != no_echo) ? (cur_pos - begin_) : 0, 0);
	ncurses::pen(*this).commit();
}

void edit::insert(unsigned int pos, char sym)
{
	if (!mask_)
		text_.insert(text_.begin() + pos, sym); //default mode is insert mode
	else //in the mask mode we use overwrite; don't insert the latest symbol: it's fake
		if (pos != max_) text_[pos] = sym;
}

void edit::erase(unsigned int pos)
{
	if (!mask_) //default mode is erase
		text_.erase(text_.begin() + pos);
	else //in the mask mode we replace symbol with default; don't work the latest symbol: it's fake
		if (pos != max_) text_[pos] = mask_->space();
}

void edit::set_max(unsigned int max)
{
	max_ = max;
}

void edit::set_mask(mask *m)
{
	mask_ = m;
	text_.assign(m->size() - 1, m->space()); //reset text to default 'space' values, without latest fake value
	max_ = m->size();
}


input::input(const std::string& label, panel *parent):
		panel(parent),
		label_(label, this),
		edit_("", this)
{
	events()->on_system(slot(&input::at_system, this));
}

void input::resize(unsigned int size)
{
	panel::resize(size, 1);
	size -= label_.width() + 1;
	if (size > 0) edit_.resize(size);
}

void input::draw()
{
	packer<pack::horizontal, pack::no_resizer> pack(this, align::center, 1);
	pack.add(&label_);
	pack.add(&edit_);
	pack.pack();
	panel::draw();
}

void input::set_text(const std::string& str)
{
	edit_.set_text(str);
}

void input::set_mode(edit::echo_mode mode)
{
	edit_.set_mode(mode);
}

void input::set_max(unsigned int max)
{
	edit_.set_max(max);
}

void input::set_mask(mask *m)
{
	edit_.set_mask(m);
}

const std::string& input::text() const
{
	return edit_.text();
}

event::result input::at_system(const event& ev)
{
	event::result res = event::ignored;

	if (!ev.sender_ && engine::enter_focus == ev.type_)
		edit_.events()->system(ev);
	return res;
}
