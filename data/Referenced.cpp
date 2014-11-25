/*
librpn - A cross-platform Reverse Polish Notation C++ library
Copyright (C) 2005 Thibault Genessay
http://librpn.sourceforge.net/

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef _DEBUG
//#define _COUNT_INSTANCES
#endif

#ifdef LIBRPN_MULTITHREADED
#include <OpenThreads/ScopedLock>
#endif // LIBRPN_MULTITHREADED

#include <librpn/Referenced.h>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace rpn;
#ifdef LIBRPN_MULTITHREADED
//using namespace OpenThreads;
#endif // LIBRPN_MULTITHREADED


#ifdef _COUNT_INSTANCES
unsigned int Referenced::s_instancesCreated(0);
unsigned int Referenced::s_instancesDeleted(0);
#ifdef LIBRPN_MULTITHREADED
//OpenThreads::Mutex Referenced::s_instancesMutex;
#endif // LIBRPN_MULTITHREADED
#endif //_COUNT_INSTANCES

Referenced::Referenced() : _refCount(0)
{
#ifdef _COUNT_INSTANCES
#ifdef LIBRPN_MULTITHREADED
	OpenThreads::ScopedLock<OpenThreads::Mutex> slock(s_instancesMutex);
#endif // LIBRPN_MULTITHREADED
	++s_instancesCreated;
	cout << "CRE/DEL = " << (int)s_instancesCreated << "/" << (int)s_instancesDeleted << endl;
#endif //_COUNT_INSTANCES
}

Referenced::~Referenced()
{
	if (_refCount > 0)
	{
		cout << "Warning : deleting still referenced object at " << this << endl;
	}
#ifdef _COUNT_INSTANCES
	else{
#ifdef LIBRPN_MULTITHREADED
		OpenThreads::ScopedLock<OpenThreads::Mutex> slock(s_instancesMutex);
#endif // LIBRPN_MULTITHREADED
		++s_instancesDeleted;
		cout << "Deleting rpn::Referenced at " << this << "  CRE/DEL = " << (int)s_instancesCreated << "/" << (int)s_instancesDeleted << endl;
	}
#endif //_COUNT_INSTANCES
}

void Referenced::ref()
{
#ifdef LIBRPN_MULTITHREADED
	ScopedLock<Mutex> lock(_refCountMutex); 
#endif // LIBRPN_MULTITHREADED
	++_refCount;
}

void Referenced::unref()
{
	bool kill(false);
	{
#ifdef LIBRPN_MULTITHREADED
		ScopedLock<Mutex> lock(_refCountMutex); 
#endif // LIBRPN_MULTITHREADED
		if (--_refCount <= 0)
			kill = true;
	}
	if (kill)
		delete this;
}

#ifdef _COUNT_INSTANCES
std::pair<unsigned int, unsigned int> Referenced::GetInstancesCount()
{
#ifdef LIBRPN_MULTITHREADED
	ScopedLock<Mutex> lock(s_instancesMutex); 
#endif // LIBRPN_MULTITHREADED
	return std::pair<unsigned int, unsigned int>(s_instancesCreated, s_instancesDeleted);
}
#endif //_COUNT_INSTANCES
