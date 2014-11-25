/* packer.cc
* This file is part of NDK++ library
* Copyright (c) 2003 by Stanislav Ievlev
*
* This file is covered by the GNU Library General Public License,
* which should be included with libndk++ as the file COPYING.
*/
#include <ndk++/align.hh>
#include <ndk++/panel.hh>
#include <ndk++/application.hh>

using namespace ndk;

unsigned int align::pos(unsigned int internal_size, unsigned int external_size, align::type align)
{
	switch (align)
	{
	case left:
		return 0;
	case right:
		return external_size - internal_size;
	case center:
		return (external_size - internal_size) / 2;
	default:
		return 0;
	}
}


void align::widget(panel* p, type horizontal, type vertical)
{
	p->move(pos(p->width(), application::instance().width(), horizontal),
		pos(p->height(), application::instance().height(), vertical));
}
