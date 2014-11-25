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
#include <exception>

#include <c++-gtk-utils/future.h>
#include <c++-gtk-utils/callback.h>
#include <c++-gtk-utils/thread.h>

using namespace Cgu;

GMainLoop* loop;
int count = 0;

class Test {
  int a;
public:

  struct Movable {
    int a;
    Movable& operator=(Movable&& m) {a = 10; return *this;}
    Movable& operator=(const Movable& m) {a = 5; return *this;}
    Movable(Movable&&): a(10) {}
    Movable(const Movable&): a(5) {}
    Movable(): a(0) {}
  };

  Releaser releaser;

  Test(): a(0) {}

  int increment() {
    a++;
    return a;
  }
  int add(int i) {
    return a + i;
  }
  int add_const(int i) const {
    return a + i;
  }
  static int add_static(int i) {
    return 100 + i;
  }
  int add_ref(const int& i) {
    return a + i;
  }
  static Movable get_movable() {
    static Movable m;
    return m;
  }
  void async_result(int i, const int& j) {
    g_assert_cmpint(i, ==, j);
    ++count;
    if (count == 6) g_main_loop_quit(loop);
  }
  void async_result_yield(int i, const int& j) {
    g_usleep(10000);
    g_assert_cmpint(i, ==, j);
    ++count;
    if (count == 6) g_main_loop_quit(loop);
  }
  void async_fail(const IntrusivePtr<Thread::Future<int> >& f) {
    g_assert(f->is_error());
    ++count;
    if (count == 6) g_main_loop_quit(loop);
  }
  void async_fail_yield(const IntrusivePtr<Thread::Future<int> >& f) {
    g_usleep(10000);
    g_assert(f->is_error());
    ++count;
    if (count == 6) g_main_loop_quit(loop);
  }
};

int do_nothing_exit() {
  throw Thread::Exit();
  // we will never enter this
  return 0;
}

int do_nothing_exception() {
  throw std::exception();
  // we will never enter this
  return 0;
}

void test_async_impl() {
  
  static Test test;
  int i = 10;
  {
    auto f = Thread::make_future(test, &Test::add, i);
    f->when(Callback::make(test, &Test::async_result, i));
    f->run();
  }
  {
    auto f = Thread::make_future(test, &Test::add_const, i);
    f->when(Callback::make(test, &Test::async_result, i));
    f->run();
    g_usleep(10000);
  }
  {
    auto f = Thread::make_future([i] () -> int {return test.add_ref(i);});
    f->when([i] (const int& j) {test.async_result_yield(i, j);}, test.releaser);
    f->run();
  }
  {
    auto f = Thread::make_future(&Test::add_static, i);
    f->when(Callback::make(test, &Test::async_result_yield, i + 100));
    f->run();
    g_usleep(10000);
  }
  {
    auto f = Thread::make_future([] () -> int {return do_nothing_exit();});
    f->fail([f] () {test.async_fail(f);}, test.releaser);
    f->run();
    g_usleep(10000);
  }
  {
    auto f = Thread::make_future(&do_nothing_exception);
    f->fail(Callback::make_ref(test, &Test::async_fail_yield, f));
    f->run();
  }
}

extern "C" {
static void test_sync() {

  Test test;
  int i = 10;
  {
    auto f = Thread::make_future(test, &Test::add, i);
    f->run();
    int j = f->get();
    g_assert_cmpint(j, ==, i);
  }
  {
    auto f = Thread::make_future(test, &Test::add_const, i);
    f->run();
    int j = f->get();
    g_assert_cmpint(j, ==, i);
  }
  {
    auto f = Thread::make_future(test, &Test::add_ref, i);
    f->run();
    int j = f->get();
    g_assert_cmpint(j, ==, i);
  }
  {
    auto f = Thread::make_future(&Test::add_static, i);
    f->run();
    int j = f->get();
    g_assert_cmpint(j, ==, i + 100);
  }
  {
    auto f = Thread::make_future([&test] () -> int {return test.increment();});
    f->run();
    int j = f->get();
    g_assert_cmpint(j, ==, 1);
  }
  {
    auto f = Thread::make_future(&Test::get_movable);
    f->run();
    Test::Movable m{f->move_get()};
    g_assert_cmpint(m.a, ==, 10);
  }
  {
    auto f = Thread::make_future(&do_nothing_exit);
    f->run();
    f->get();
    g_assert_cmpuint((unsigned int)f->is_error(), !=, 0);
  }
  {
    auto f = Thread::make_future(&do_nothing_exception);
    f->run();
    f->get();
    g_assert_cmpuint((unsigned int)f->is_error(), !=, 0);
  }
}

static void test_async() {
#if !(GLIB_CHECK_VERSION(2,32,0))
  g_thread_init(0);
#endif
  loop = g_main_loop_new(0, true);
  Callback::post(Callback::make(&test_async_impl));
  g_main_loop_run(loop);
  g_assert_cmpint(count, ==, 6);
}
} // extern "C"


int main (int argc, char* argv[]) {

#if !(GLIB_CHECK_VERSION(2,32,0))
  g_thread_init(0);
#endif
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/future/sync", test_sync);
  g_test_add_func("/future/async", test_async);

  return g_test_run();
}
