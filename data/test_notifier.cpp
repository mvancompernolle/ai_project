/* Copyright (C) 2012 Chris Vine

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

#include <glib.h>

#include <c++-gtk-utils/notifier.h>
#include <c++-gtk-utils/callback.h>
#include <c++-gtk-utils/thread.h>
#include <c++-gtk-utils/emitter.h>

using namespace Cgu;

class Test {
  static int count;
public:
  Releaser releaser;
  static void notifier_func(Test*);
  static void inc() {++count;}
  static int get_count() {return count;}
};

int Test::count = 0;

Notifier event_notifier;
Notifier quit_notifier;

// this executes in the main thread
void Test::notifier_func(Test* self) {
  inc();
  g_assert_cmpint(get_count(), <=, 3);
  
  if (get_count() == 3) delete self; // Releaser will remove the notifier callback
                                     // so we won't get the last two notifications
}

void thread_func() {
  for (int i = 0; i < 5; ++i) {
    event_notifier.emit();
  }
  g_usleep(10000);
  quit_notifier();
}

// provide a function with C++ linkage specification
void loop_quit(GMainLoop* loop) {
  g_main_loop_quit(loop);
}

extern "C" {
static void test_notifier() {

  Test* test = new Test;
  GMainLoop* loop = g_main_loop_new(0, false);

  event_notifier.connect(Callback::make(&Test::notifier_func, test),
			 test->releaser);
  quit_notifier.connect([loop] () {loop_quit(loop);});

  Thread::JoinableHandle t(Thread::Thread::start(Callback::make(thread_func), true),
                           Thread::JoinableHandle::join_on_exit);

  g_main_loop_run(loop);
  t.join();

  g_assert_cmpint(Test::get_count(), ==, 3);
}
} // extern "C"


int main (int argc, char* argv[]) {
#if !(GLIB_CHECK_VERSION(2,32,0))
  g_thread_init(0);
#endif
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/notifier/notifier", test_notifier); 

  return g_test_run();
}
