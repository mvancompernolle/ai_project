/* Copyright (C) 2012 and 2013 Chris Vine

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

#include <c++-gtk-utils/timeout.h>
#include <c++-gtk-utils/callback.h>
#include <c++-gtk-utils/emitter.h>

using namespace Cgu;

class Test {
  static int count;
public:
  Releaser releaser;
  static void timeout_func(bool&);
  static int get_count() {return count;}
  Test() {count = 0;}
};

int Test::count;

Test* test;
GMainLoop* loop;

void Test::timeout_func(bool& keep) {
  ++count;
  g_assert_cmpint(count, <=, 3);
  if (count == 3) delete test; // Releaser will remove the timeout callback
                               // so we will only receive three timeouts
}

void quit_func(bool& keep) {
  if (Test::get_count() < 3) return;
  g_main_loop_quit(loop);
  keep = false;
}

extern "C" {
static void test_timeout() {

  test = new Test;
  loop = g_main_loop_new(0, false);

  start_timeout(1, Callback::make(&Test::timeout_func),
		test->releaser);
  start_timeout(300, Callback::make(&quit_func), G_PRIORITY_LOW);

  g_main_loop_run(loop);

  g_assert_cmpint(Test::get_count(), ==, 3);
}

static void test_timeout_lambda() {

  test = new Test;
  loop = g_main_loop_new(0, false);

  start_timeout(1, [] (bool& keep) {test->timeout_func(keep);},
		test->releaser);
  start_timeout(300, [] (bool& keep) {quit_func(keep);}, G_PRIORITY_LOW);

  g_main_loop_run(loop);

  g_assert_cmpint(Test::get_count(), ==, 3);
}
} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/timeout/timeout", test_timeout); 
  g_test_add_func("/timeout/timeout_lambda", test_timeout_lambda); 

  return g_test_run();
}
