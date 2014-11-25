/**************************************************************

callback.cpp (C-Munipack project)
Base class for objects which register callbacks
Copyright (C) 2008 David Motl, dmotl@volny.cz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

**************************************************************/

#include <stdlib.h>

#include "callback.h"

// Callback descriptor:
struct tCBData
{
	CCBObject::CBProc		*proc;
	void		*data;
	tCBData		*next;
};

// Default constructor
CCBObject::CCBObject(void): m_List(NULL)
{
}

// Destructor
CCBObject::~CCBObject(void)
{
	tCBData *ptr, *next;

	ptr = m_List;
	while (ptr) {
		next = ptr->next;
		delete ptr;
		ptr = next;
	}
	m_List = NULL;
}

// Register callback procedure
void CCBObject::RegisterCallback(CBProc *cb_proc, void* cb_data)
{
	if (!cb_proc)
		return;

	tCBData *ptr = new tCBData;
	if (ptr) {
		ptr->data = cb_data;
		ptr->proc = cb_proc;
		ptr->next = m_List;
		m_List = ptr;
	}
}

// Unregister callback procedure
void CCBObject::UnregisterCallback(CBProc *cb_proc)
{
	tCBData *ptr = m_List, *prev = NULL;
	while (ptr) {
		if (cb_proc == ptr->proc) {
			if (prev)
				prev->next = ptr->next;
			else
				m_List = ptr->next;
			delete ptr;
			break;
		}
		prev = ptr;
		ptr = ptr->next;
	}
}

// Broadcast message (synchronous)
void CCBObject::Callback(int message, int wparam, void* lparam)
{
	tCBData *ptr = m_List;
	while (ptr) {
		tCBData *next = ptr->next;
		ptr->proc(this, message, wparam, lparam, ptr->data);
		ptr = next;
	}
}
