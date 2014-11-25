/* Copyright (C) 2009 Chris Vine

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

#include <c++-gtk-utils/timeout.h>
#include <c++-gtk-utils/emitter.h>
#include <c++-gtk-utils/thread.h>

extern "C" {
  static gboolean cgu_timeout_wrapper(void*);
  static gboolean cgu_tracked_timeout_wrapper(void*);
  static void cgu_timeout_destroy_func(void*);
  static void cgu_tracked_timeout_destroy_func(void*);
}

gboolean cgu_timeout_wrapper(void* data) {
  const Cgu::Callback::CallbackArg<bool&>* cb = static_cast<Cgu::Callback::CallbackArg<bool&>*>(data);

  bool keep_source = true;
  // provide a CancelBlock here to make this function NPTL friendly,
  // as we have a catch-all without rethrowing
  Cgu::Thread::CancelBlock b;
  try {
    cb->dispatch(keep_source);
  }
  catch (...) {
    g_critical("Exception thrown in timeout_wrapper() for timeout function\n");
  }
  return keep_source;
}

gboolean cgu_tracked_timeout_wrapper(void* data) {
  const Cgu::SafeEmitterArg<bool&>* e = static_cast<Cgu::SafeEmitterArg<bool&>*>(data);

  bool keep_source = true;
  bool connected;
  // provide a CancelBlock here to make this function NPTL friendly,
  // as we have a catch-all without rethrowing
  Cgu::Thread::CancelBlock b;
  try {
    connected = e->test_emit(keep_source);
  }
  catch (...) {
    g_critical("Exception thrown in tracked_timeout_wrapper() for timeout function\n");
    return true;
  }
  return (keep_source && connected);
}

void cgu_timeout_destroy_func(void* data) {
  delete static_cast<Cgu::Callback::CallbackArg<bool&>*>(data);
}

void cgu_tracked_timeout_destroy_func(void* data) {
  delete static_cast<Cgu::SafeEmitterArg<bool&>*>(data);
}

namespace Cgu {

guint start_timeout(guint millisec, const Callback::CallbackArg<bool&>* cb,
		    gint priority, GMainContext* context) {
  // context has a default value of NULL which will attach the timeout source
  // to the default program main context

  GSource* source_p = g_timeout_source_new(millisec);
  if (priority != G_PRIORITY_DEFAULT)
    g_source_set_priority(source_p, priority);
  g_source_set_callback(source_p, cgu_timeout_wrapper,
			const_cast<Callback::CallbackArg<bool&>*>(cb), cgu_timeout_destroy_func);

  guint id = g_source_attach(source_p, context);
  g_source_unref(source_p);

  return id;
}

guint start_timeout(guint millisec, const Callback::CallbackArg<bool&>* cb,
		    Releaser& r, gint priority, GMainContext* context) {
  // context has a default value of NULL which will attach the timeout source
  // to the default program main context

  Callback::SafeFunctorArg<bool&> f(cb);   // take ownership
  SafeEmitterArg<bool&>* e = new SafeEmitterArg<bool&>;
  try {
    e->connect(f, r);
  }
  catch (...) {
    delete e;
    throw;
  }
  
  GSource* source_p = g_timeout_source_new(millisec);
  if (priority != G_PRIORITY_DEFAULT)
    g_source_set_priority(source_p, priority);
  g_source_set_callback(source_p, cgu_tracked_timeout_wrapper,
			e, cgu_tracked_timeout_destroy_func);

  guint id = g_source_attach(source_p, context);
  g_source_unref(source_p);

  return id;
}

#if GLIB_CHECK_VERSION(2,14,0)
guint start_timeout_seconds(guint sec, const Callback::CallbackArg<bool&>* cb,
			    gint priority, GMainContext* context) {
  // context has a default value of NULL which will attach the timeout source
  // to the default program main context

  GSource* source_p = g_timeout_source_new_seconds(sec);
  if (priority != G_PRIORITY_DEFAULT)
    g_source_set_priority(source_p, priority);
  g_source_set_callback(source_p, cgu_timeout_wrapper,
			const_cast<Callback::CallbackArg<bool&>*>(cb), cgu_timeout_destroy_func);

  guint id = g_source_attach(source_p, context);
  g_source_unref(source_p);

  return id;
}

guint start_timeout_seconds(guint sec, const Callback::CallbackArg<bool&>* cb,
			    Releaser& r, gint priority, GMainContext* context) {
  // context has a default value of NULL which will attach the timeout source
  // to the default program main context

  Callback::SafeFunctorArg<bool&> f(cb);   // take ownership
  SafeEmitterArg<bool&>* e = new SafeEmitterArg<bool&>;
  try {
    e->connect(f, r);
  }
  catch (...) {
    delete e;
    throw;
  }
  
  GSource* source_p = g_timeout_source_new_seconds(sec);
  if (priority != G_PRIORITY_DEFAULT)
    g_source_set_priority(source_p, priority);
  g_source_set_callback(source_p, cgu_tracked_timeout_wrapper,
			e, cgu_tracked_timeout_destroy_func);

  guint id = g_source_attach(source_p, context);
  g_source_unref(source_p);

  return id;
}
#endif // GLIB_CHECK_VERSION

} // namespace Cgu
