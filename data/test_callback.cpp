/* Copyright (C) 2012 to 2014 Chris Vine

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
#include <string>
#include <memory>

#include <c++-gtk-utils/callback.h>
#include <c++-gtk-utils/emitter.h>

using namespace Cgu;

class Test {
  int val;
public:
  void set1(int i) {val = i;}
  void set2(const int& i) {val = i;}
  void set3(int& i) {val = i;}
  int get() const {return val;}
  Test(): val(0) {}
} obj;

// provide a function with C++ linkage specification
void loop_quit(GMainLoop* loop) {
  g_main_loop_quit(loop);
}

// provide a function to wrap the glib macro so we can take its
// address
void assert_not_reached() {g_assert_not_reached();}

extern "C" {
static void test_make() {
  int i = 5, j = 0;
  {
    std::unique_ptr<const Callback::Callback> cb(to_unique(Callback::make(obj, &Test::set1, i)));
    cb->dispatch();
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 6;
    std::unique_ptr<const Callback::CallbackArg<int>> cb(to_unique(Callback::make(obj, &Test::set1)));
    cb->dispatch(i);
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 7;
    std::unique_ptr<const Callback::Callback> cb(to_unique(Callback::make_ref(obj, &Test::set2, i)));
    cb->dispatch();
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 0;
    std::unique_ptr<const Callback::Callback> cb(to_unique(Callback::make<Test, int&>(obj, &Test::set3, i)));
    i = 8;
    cb->dispatch();
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 9;
    Callback::Functor f = Callback::make_ref(obj, &Test::set2, i);
    f();
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 10;
    Callback::SafeFunctor f = Callback::make_ref(obj, &Test::set2, i);
    f();
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }
}

static void test_lambda() {
  auto string_assert = [] (const std::string& s) {g_assert_cmpstr(s.c_str(), ==, "Test");};
  auto multiply = [] (int j, int k, int& r) {r = j * k;};

  {
    std::string s("Test");
    auto cb = to_unique(Callback::lambda<const std::string&>(string_assert));
    cb->dispatch(s);
  }

  {
    int res = 0;
    auto cb = to_unique(Callback::lambda<int, int, int&>(multiply));
    cb->dispatch(2, 3, res);
    g_assert_cmpint(res, ==, 6);
  }

  {
    int res = 0;
    int i = 2;
    auto cb = to_unique(Callback::lambda<int, int, int&>([=](int j, int k, int& r) {r = i * j * k;}));
    cb->dispatch(2, 3, res);
    g_assert_cmpint(res, ==, 12);
  }

  {
    int res = 0;
    int i = 2;
    auto f = to_functor(Callback::lambda<int, int, int&>([=](int j, int k, int& r) {r = i * j * k;}));
    f(3, 4, res);
    g_assert_cmpint(res, ==, 24);
  }

  {
    int res = 0;
    int i = 2;
    auto f = to_safe_functor(Callback::lambda<int, int, int&>([=](int j, int k, int& r) {r = i * j * k;}));
    f(5, 6, res);
    g_assert_cmpint(res, ==, 60);
  }
}

static void test_post() {
  GMainLoop* loop = g_main_loop_new(0, true);

  int i = 10, j = 0;
  Releaser* r = new Releaser;

  auto cb = Callback::make(obj, &Test::set1, i);
  Callback::post(cb);
  Callback::post(Callback::make(&assert_not_reached), *r);
  Callback::post(Callback::make(&loop_quit, loop));
  
  delete r;
  g_main_loop_run(loop);

  j = obj.get();
  g_assert_cmpint(j, ==, i);

  i = 20;
  j = 0;
  r = new Releaser;
  auto l = [i] () {obj.set1(i);};
  Callback::post(l);
  Callback::post([] () -> void {g_assert_not_reached();}, *r);
  Callback::post([loop] () {loop_quit(loop);});
  
  delete r;
  g_main_loop_run(loop);

  j = obj.get();
  g_assert_cmpint(j, ==, i);

  g_main_loop_unref(loop);
}

} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/callbacks/make", test_make); 
  g_test_add_func("/callbacks/lambda", test_lambda); 
  g_test_add_func("/callbacks/post", test_post); 

  return g_test_run();
}
