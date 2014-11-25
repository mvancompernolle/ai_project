/* switch_button.cc
* This file is part of NDK++ library
* Copyright (c) 2003,2004 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <functional>

#include <ndk++/switch_button.hh>


using namespace ndk;

//setup general default values
std::string check::on = "[x]"; /**< default view of CheckButton if this button on*/
std::string check::off = "[ ]"; /**< default view of CheckButton if this button off*/

std::string radio::on = "(*)"; /**< default view of RadioButton if this button on*/
std::string radio::off = "( )"; /**< default view of RadioButton if this button off*/

void radio_manager::add(radio_button *item)
{
	buttons_.push_back(item);
}

void radio_manager::del(radio_button *item)
{
	buttons_.erase(std::remove(buttons_.begin(),
					   buttons_.end(), item), buttons_.end());
}

void radio_manager::activate(radio_button *item)
{
	//find previous active item
	radio_vector::const_iterator it =
		std::find_if(buttons_.begin(), buttons_.end(), std::mem_fun(&radio_button::state));
	if (it != buttons_.end()) (*it)->off(); //and turn off if found
	item->on(); //then activate this item
}


radio_button::radio_button(const std::string& str, radio_manager& group, panel *parent):
		switch_button<radio>(str, parent),
		group_(group)
{
	group_.add(this);
}

radio_button::~radio_button()
{
	group_.del(this);
}

void radio_button::do_toggle()
{
	group_.activate(this);
}
