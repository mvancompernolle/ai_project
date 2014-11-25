/* Copyright (C) 2009 and 2011 Chris Vine

The library comprised in this file or of which this file is part is
distributed by Chris Vine under the GNU Lesser General Public
License as follows:

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License, version 2.1, for more details.

   You should have received a copy of the GNU Lesser General Public
   License, version 2.1, along with this library (see the file LGPL.TXT
   which came with this source code package in the c++-gtk-utils
   sub-directory); if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include <c++-gtk-utils/lib_defs.h>

#include <c++-gtk-utils/emitter.h>

#include <glib.h>

namespace Cgu {

Releaser::~Releaser(void) {
  Thread::Mutex::Lock lock{mutex};
  for(const auto& f: disconnect_set) {f();};
}

Releaser& Releaser::operator=(const Releaser&) {

  // first disconnect ourselves from any existing emitters protected
  // by this object and then vacate disconnect_set - effectively, on
  // assignment we drop our old identity and become a blank sheet,
  // since our parent cannot be assigned any emitter connections in
  // its new identity - the assignee retains them - and it may not be
  // safe for it to keep its pre-assignment emitter connections.
  // (Note, EmitterArg and SafeEmitterArg objects can't be copied)

  Thread::Mutex::Lock lock{mutex};
  for(const auto& f: disconnect_set) {f();};
  disconnect_set.clear();
  return *this;
}

void Releaser::add(const Callback::SafeFunctor& f) {
  Thread::Mutex::Lock lock{mutex};
  if (!disconnect_set.insert(f).second) { // this shouldn't ever happen but is
                                          // kinder than an assert
    g_critical("Attempt to insert duplicate value into disconnect_set "
	       "in Cgu::Releaser::add()\n");
  }
}

void Releaser::remove(const Callback::SafeFunctor& f) {
  Thread::Mutex::Lock lock{mutex};
  disconnect_set.erase(f);
}

// this method is called instead of Releaser::remove() when called by
// a SafeEmitterArg object which has locked its mutex, so as to avoid
// out of order locking deadlocks.  It is primarily intended to cover
// a case where both the Releaser and SafeEmitterArg destructors are
// operating and will cause the SafeEmitterArg destructor to spin
// until the Releaser destructor has done its business, although it is
// also used in SafeEmitterArg::disconnect()
void Releaser::try_remove(const Callback::SafeFunctor& f, int* result_p) {

  if (!(*result_p = mutex.trylock())) {
    Thread::Mutex::Lock lock{mutex, Thread::locked};
    disconnect_set.erase(f);
  }
}

} // namespace Cgu
