/* Copyright (C) 2005 to 2014 Chris Vine

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

#include <c++-gtk-utils/io_watch.h>
#include <c++-gtk-utils/emitter.h>
#include <c++-gtk-utils/thread.h>


struct WatchSource {
  GSource source;
  GPollFD poll_fd;
  GIOCondition watch_condition;
  void* func;
};

extern "C" {
  static gboolean cgu_io_watch_prepare_func(GSource*, gint*);
  static gboolean cgu_io_watch_check_func(GSource*);

  static gboolean cgu_io_watch_dispatch_func(GSource*, GSourceFunc, void*);
  static gboolean cgu_io_watch_dispatch_tracked_func(GSource*, GSourceFunc, void*);
  static gboolean cgu_io_watch_dispatch_condition_func(GSource*, GSourceFunc, void*);
  static gboolean cgu_io_watch_dispatch_tracked_condition_func(GSource*, GSourceFunc, void*);

  static void cgu_io_watch_finalize_func(GSource*);
  static void cgu_io_watch_finalize_tracked_func(GSource*);
  static void cgu_io_watch_finalize_condition_func(GSource*);
  static void cgu_io_watch_finalize_tracked_condition_func(GSource*);
}

// in the functions below, reinterpret_cast<>()ing to WatchSource* is
// guaranteed to give the correct here as the address of
// WatchSource::source must be the same as the address of the instance
// of the WatchSource struct of which it is the first member as both
// are PODSs

gboolean cgu_io_watch_prepare_func(GSource*, gint* timeout_p) {

  *timeout_p = -1;
  return false; // we want the file descriptor to be polled
}

gboolean cgu_io_watch_check_func(GSource* source) {
  WatchSource* watch_source = reinterpret_cast<WatchSource*>(source);

  // what we have got
  gushort poll_condition = watch_source->poll_fd.revents;
  // what we are looking for
  gushort watch_condition = watch_source->watch_condition;

  // return true if we have what we are looking for
  return (poll_condition & watch_condition); 
}

gboolean cgu_io_watch_dispatch_func(GSource* source, GSourceFunc, void*) {
  const Cgu::Callback::CallbackArg<bool&>* cb =
    static_cast<Cgu::Callback::CallbackArg<bool&>*>(reinterpret_cast<WatchSource*>(source)->func);

  // we are not interested in the GSourceFunc argument here as we have never
  // called g_source_set_callback()
  bool keep_source = true;
  // provide a CancelBlock here to make this function NPTL friendly,
  // as we have a catch-all without rethrowing
  Cgu::Thread::CancelBlock b;
  try {
    cb->dispatch(keep_source);
  }
  // we can't propagate exceptions from functions with C linkage.
  catch (...) {
    g_critical("Exception thrown in io_watch_dispatch_func()\n");
    return true;
  }
  return keep_source;
}

gboolean cgu_io_watch_dispatch_tracked_func(GSource* source, GSourceFunc, void*) {
  const Cgu::SafeEmitterArg<bool&>* emitter =
    static_cast<Cgu::SafeEmitterArg<bool&>*>(reinterpret_cast<WatchSource*>(source)->func);

  // we are not interested in the GSourceFunc argument here as we have never
  // called g_source_set_callback()
  bool keep_source = true;
  bool connected;
  // provide a CancelBlock here to make this function NPTL friendly,
  // as we have a catch-all without rethrowing
  Cgu::Thread::CancelBlock b;
  try {
    connected = emitter->test_emit(keep_source);
  }
  // we can't propagate exceptions from functions with C linkage.
  catch (...) {
    g_critical("Exception thrown in io_watch_dispatch_func()\n");
    return true;
  }
  return (keep_source && connected);
}

gboolean cgu_io_watch_dispatch_condition_func(GSource* source, GSourceFunc, void*) {
  const Cgu::Callback::CallbackArg<GIOCondition, bool&>* cb =
    static_cast<Cgu::Callback::CallbackArg<GIOCondition, bool&>*>(reinterpret_cast<WatchSource*>(source)->func);
  GIOCondition cond = GIOCondition(reinterpret_cast<WatchSource*>(source)->poll_fd.revents);

  // we are not interested in the GSourceFunc argument here as we have never
  // called g_source_set_callback()
  bool keep_source = true;
  // provide a CancelBlock here to make this function NPTL friendly,
  // as we have a catch-all without rethrowing
  Cgu::Thread::CancelBlock b;
  try {
    cb->dispatch(cond, keep_source);
  }
  // we can't propagate exceptions from functions with C linkage.
  catch (...) {
    g_critical("Exception thrown in io_watch_dispatch_func()\n");
    return true;
  }
  return keep_source;
}

gboolean cgu_io_watch_dispatch_tracked_condition_func(GSource* source, GSourceFunc, void*) {
  const Cgu::SafeEmitterArg<GIOCondition, bool&>* emitter =
    static_cast<Cgu::SafeEmitterArg<GIOCondition, bool&>*>(reinterpret_cast<WatchSource*>(source)->func);
  GIOCondition cond = GIOCondition(reinterpret_cast<WatchSource*>(source)->poll_fd.revents);

  // we are not interested in the GSourceFunc argument here as we have never
  // called g_source_set_callback()
  bool keep_source = true;
  bool connected;
  // provide a CancelBlock here to make this function NPTL friendly,
  // as we have a catch-all without rethrowing
  Cgu::Thread::CancelBlock b;
  try {
    connected = emitter->test_emit(cond, keep_source);
  }
  // we can't propagate exceptions from functions with C linkage.
  catch (...) {
    g_critical("Exception thrown in io_watch_dispatch_func()\n");
    return true;
  }
  return (keep_source && connected);
}

void cgu_io_watch_finalize_func(GSource* source) {
  WatchSource* watch_source = reinterpret_cast<WatchSource*>(source);
  delete static_cast<Cgu::Callback::CallbackArg<bool&>*>(watch_source->func);
  watch_source->func = 0;
}

void cgu_io_watch_finalize_tracked_func(GSource* source) {
  WatchSource* watch_source = reinterpret_cast<WatchSource*>(source);
  delete static_cast<Cgu::SafeEmitterArg<bool&>*>(watch_source->func);
  watch_source->func = 0;
}

void cgu_io_watch_finalize_condition_func(GSource* source) {
  WatchSource* watch_source = reinterpret_cast<WatchSource*>(source);
  delete static_cast<Cgu::Callback::CallbackArg<GIOCondition, bool&>*>(watch_source->func);
  watch_source->func = 0;
}

void cgu_io_watch_finalize_tracked_condition_func(GSource* source) {
  WatchSource* watch_source = reinterpret_cast<WatchSource*>(source);
  delete static_cast<Cgu::SafeEmitterArg<GIOCondition, bool&>*>(watch_source->func);
  watch_source->func = 0;
}

static GSourceFuncs cgu_io_watch_source_funcs = {
  cgu_io_watch_prepare_func,
  cgu_io_watch_check_func,
  cgu_io_watch_dispatch_func,
  cgu_io_watch_finalize_func
};

static GSourceFuncs cgu_io_watch_source_tracked_funcs = {
  cgu_io_watch_prepare_func,
  cgu_io_watch_check_func,
  cgu_io_watch_dispatch_tracked_func,
  cgu_io_watch_finalize_tracked_func
};

static GSourceFuncs cgu_io_watch_source_condition_funcs = {
  cgu_io_watch_prepare_func,
  cgu_io_watch_check_func,
  cgu_io_watch_dispatch_condition_func,
  cgu_io_watch_finalize_condition_func
};

static GSourceFuncs cgu_io_watch_source_tracked_condition_funcs = {
  cgu_io_watch_prepare_func,
  cgu_io_watch_check_func,
  cgu_io_watch_dispatch_tracked_condition_func,
  cgu_io_watch_finalize_tracked_condition_func
};

namespace Cgu {

guint start_iowatch(int fd, const Callback::CallbackArg<bool&>* cb,
		    GIOCondition io_condition, gint priority,
		    GMainContext* context_p) {

  // context_p has a default value of NULL which will create the watch
  // in the default program main context

  GSource* source = g_source_new(&cgu_io_watch_source_funcs, sizeof(WatchSource));
  // reinterpret_cast<>() is guaranteed to give the correct result here as the
  // address of WatchSource::source must be the same as the address of the
  // instance of the WatchSource struct of which it is the first member as
  // both are PODSs
  WatchSource* watch_source = reinterpret_cast<WatchSource*>(source);
  watch_source->poll_fd.fd = fd;
  watch_source->poll_fd.events = io_condition;
  watch_source->poll_fd.revents = 0;
  watch_source->watch_condition = io_condition;
  watch_source->func = const_cast<void*>(static_cast<const void*>(cb));

  g_source_set_priority(source, priority);

  // connect the source object to its polling object  
  g_source_add_poll(source, &watch_source->poll_fd);

  // attach the source to the relevant main context
  guint id = g_source_attach(source, context_p);

  // g_source_attach() will add a reference count to the GSource object
  // so we unreference it here so that the callback returning false or
  // calling g_source_remove() on the return value of this function will
  // finalize/destroy the GSource object
  g_source_unref(source);

  return id;
}

guint start_iowatch(int fd, const Callback::CallbackArg<bool&>* cb, Releaser& r,
		    GIOCondition io_condition, gint priority,
		    GMainContext* context_p) {

  // context_p has a default value of NULL which will create the watch
  // in the default program main context

  Callback::SafeFunctorArg<bool&> f{cb};   // take ownership

  GSource* source = g_source_new(&cgu_io_watch_source_tracked_funcs, sizeof(WatchSource));
  // reinterpret_cast<>() is guaranteed to give the correct result here as the
  // address of WatchSource::source must be the same as the address of the
  // instance of the WatchSource struct of which it is the first member as
  // both are PODSs
  WatchSource* watch_source = reinterpret_cast<WatchSource*>(source);
  watch_source->poll_fd.fd = fd;
  watch_source->poll_fd.events = io_condition;
  watch_source->poll_fd.revents = 0;
  watch_source->watch_condition = io_condition;
  SafeEmitterArg<bool&>* emitter = 0;
  try {
    emitter = new SafeEmitterArg<bool&>;
    emitter->connect(f, r);
  }
  catch (...) {
    delete emitter; // either NULL or object allocated
    g_source_unref(source);
    throw;
  }
  watch_source->func = emitter;

  g_source_set_priority(source, priority);

  // connect the source object to its polling object  
  g_source_add_poll(source, &watch_source->poll_fd);

  // attach the source to the relevant main context
  guint id = g_source_attach(source, context_p);

  // g_source_attach() will add a reference count to the GSource object
  // so we unreference it here so that the callback returning false or
  // calling g_source_remove() on the return value of this function will
  // finalize/destroy the GSource object
  g_source_unref(source);

  return id;
}

guint start_iowatch(int fd, const Callback::CallbackArg<GIOCondition, bool&>* cb,
		    GIOCondition io_condition, gint priority,
		    GMainContext* context_p) {

  // context_p has a default value of NULL which will create the watch
  // in the default program main context

  GSource* source = g_source_new(&cgu_io_watch_source_condition_funcs, sizeof(WatchSource));
  // reinterpret_cast<>() is guaranteed to give the correct result here as the
  // address of WatchSource::source must be the same as the address of the
  // instance of the WatchSource struct of which it is the first member as
  // both are PODSs
  WatchSource* watch_source = reinterpret_cast<WatchSource*>(source);
  watch_source->poll_fd.fd = fd;
  watch_source->poll_fd.events = io_condition;
  watch_source->poll_fd.revents = 0;
  watch_source->watch_condition = io_condition;
  watch_source->func = const_cast<void*>(static_cast<const void*>(cb));

  g_source_set_priority(source, priority);

  // connect the source object to its polling object  
  g_source_add_poll(source, &watch_source->poll_fd);

  // attach the source to the relevant main context
  guint id = g_source_attach(source, context_p);

  // g_source_attach() will add a reference count to the GSource object
  // so we unreference it here so that the callback returning false or
  // calling g_source_remove() on the return value of this function will
  // finalize/destroy the GSource object
  g_source_unref(source);

  return id;
}

guint start_iowatch(int fd, const Callback::CallbackArg<GIOCondition, bool&>* cb,
		    Releaser& r, GIOCondition io_condition, gint priority,
		    GMainContext* context_p) {

  // context_p has a default value of NULL which will create the watch
  // in the default program main context

  Callback::SafeFunctorArg<GIOCondition, bool&> f{cb};   // take ownership

  GSource* source = g_source_new(&cgu_io_watch_source_tracked_condition_funcs, sizeof(WatchSource));
  // reinterpret_cast<>() is guaranteed to give the correct result here as the
  // address of WatchSource::source must be the same as the address of the
  // instance of the WatchSource struct of which it is the first member as
  // both are PODSs
  WatchSource* watch_source = reinterpret_cast<WatchSource*>(source);
  watch_source->poll_fd.fd = fd;
  watch_source->poll_fd.events = io_condition;
  watch_source->poll_fd.revents = 0;
  watch_source->watch_condition = io_condition;
  SafeEmitterArg<GIOCondition, bool&>* emitter = 0;
  try {
    emitter = new SafeEmitterArg<GIOCondition, bool&>;
    emitter->connect(f, r);
  }
  catch (...) {
    delete emitter; // either NULL or object allocated
    g_source_unref(source);
    throw;
  }
  watch_source->func = emitter;

  g_source_set_priority(source, priority);

  // connect the source object to its polling object  
  g_source_add_poll(source, &watch_source->poll_fd);

  // attach the source to the relevant main context
  guint id = g_source_attach(source, context_p);

  // g_source_attach() will add a reference count to the GSource object
  // so we unreference it here so that the callback returning false or
  // calling g_source_remove() on the return value of this function will
  // finalize/destroy the GSource object
  g_source_unref(source);

  return id;
}

} // namespace Cgu
