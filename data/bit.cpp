//////////////////////////////////////////////////////////////////////////////////
// This file is distributed as part of the libLCS library.
// libLCS is C++ Logic Circuit Simulation library.
//
// Copyright (c) 2006-2007, B. R. Siva Chandra
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// In case you would like to contact the author, use the following e-mail
// address: sivachandra_br@yahoo.com
//////////////////////////////////////////////////////////////////////////////////

#include <lcs/bit.h>
#include <lcs/contassmod.h>

#ifndef NULL
#define NULL 0
#endif // NULL

using namespace lcs;

Bit::Bit(void)
{
    state_ = new LineState;
    *state_ = UNKNOWN;

    refcount_ = new int;
    *refcount_ = 1;
}

Bit::Bit(const Bit &bit)
    : state_(bit.state_), refcount_(bit.refcount_), modList(bit.modList)
{
    (*refcount_)++;
}

Bit::~Bit()
{
    if (*refcount_ <= 1)
    {
        delete refcount_;
        delete state_;
    }
    else
    {
        (*refcount_)--;
        refcount_ = NULL;
        state_ = NULL;
    }
}

void Bit::notify(Module *mod)
{
    if (mod != NULL && !modList.isPresent(mod))
        modList.append(mod);
}

void Bit::stopNotification(Module *mod)
{
    if (mod != NULL)
        modList.removeFirstMatch(mod);
}

Bit& Bit::operator=(const Bit& rhs)
{
    if (*refcount_ <= 1)
    {
        delete refcount_;
        delete state_;

        refcount_ = rhs.refcount_;
        state_ = rhs.state_;
        modList = rhs.modList;

        (*refcount_)++;
    }
    else
    {
        (*refcount_)--;

        refcount_ = rhs.refcount_;
        state_ = rhs.state_;
        modList = rhs.modList;

        (*refcount_)++;
    }

    return *this;
}

void Bit::operator=(const LineState &rhs)
{
    *state_ = rhs;

    ListIterator<Module*> iter = modList.getListIterator();
    iter.reset();
    while (iter.hasNext())
    {
        Module *mod = iter.next();
        mod->onStateChange(0);
    }
}
