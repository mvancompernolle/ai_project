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

#include <c++-gtk-utils/do_if.h>

using namespace Cgu;

class A {
  int a;
public:
  A(): a(1) {}
  int inc_a() {++a; return a;}
  int get_a() const { return a;}
  void set_a(int i) { a = i;}
  void nil_a() {g_assert_not_reached();}
};

class B: public A {
  int b;
public:
  B(): b(1) {}
  int inc_b() {++b; return b;}
  int get_b() const { return b;}
  void set_b(int& i) { b = i;}
  void nil_b() {g_assert_not_reached();}
};

class C {
  int c;
public:
  C(): c(1) {}
  int inc_c() {++c; return c;}
  int get_c() const { return c;}
  void set_c(int i) { c = i;}
  void nil_c() {g_assert_not_reached();}
};

void call_a(A* a, int i) {
  a->set_a(i);
}

void call_b(B* b, int& i) {
  b->set_b(i);
}

int call_c(C* c, int i) {
  c->set_c(i);
  return c->get_c();
}

void nilcall_a(A* a, int i) {
  g_assert_not_reached();
}

void nilcall_b(B* b, int& i) {
  g_assert_not_reached();
}

int nilcall_c(C* c, int i) {
  g_assert_not_reached();
  return -1;
}

extern "C" {
static void test_do_something() {
  A a;
  B b;
  C c;
  int res;
  int hundred = 100;
  int thousand = 1000;

  res = DoIf::mem_fun(a, &A::inc_a);
  g_assert_cmpint(res, >, 1);

  res = DoIf::mem_fun(b, &A::inc_a);
  g_assert_cmpint(res, >, 1);

  res = DoIf::mem_fun(b, &B::inc_b);
  g_assert_cmpint(res, >, 1);


  DoIf::mem_fun(a, &A::set_a, 50);
  res = DoIf::mem_fun(a, &A::get_a);
  g_assert_cmpint(res, ==, 50);

  DoIf::mem_fun(b, &A::set_a, 60);
  res = DoIf::mem_fun(b, &A::get_a);
  g_assert_cmpint(res, ==, 60);

  DoIf::mem_fun(b, &B::set_b, hundred);
  res = DoIf::mem_fun(b, &B::get_b);
  g_assert_cmpint(res, ==, 100);


  DoIf::fun(a, &call_a, 200);
  res = DoIf::mem_fun(a, &A::get_a);
  g_assert_cmpint(res, ==, 200);

  DoIf::fun(b, &call_a, 300);
  res = DoIf::mem_fun(b, &A::get_a);
  g_assert_cmpint(res, ==, 300);

  DoIf::fun(b, &call_b, thousand);
  res = DoIf::mem_fun(b, &B::get_b);
  g_assert_cmpint(res, ==, 1000);

  res = DoIf::fun(c, &call_c, 500);
  g_assert_cmpint(res, ==, 500);
}

static void test_do_nothing() {

  A a;
  C c;
  int res;
  int hundred = 100;

  DoIf::mem_fun(a, &B::nil_b);
  DoIf::mem_fun(a, &C::nil_c);
  DoIf::mem_fun(c, &A::nil_a);
  DoIf::mem_fun(c, &B::nil_b);

  DoIf::fun(a, &nilcall_b, hundred);
  res = DoIf::fun(a, &nilcall_c, 50);
  g_assert_cmpint(res, ==, 0);
  DoIf::fun(c, &nilcall_a, 60);
  DoIf::fun(c, &nilcall_b, hundred);

  res = DoIf::mem_fun(a, &B::inc_b);
  g_assert_cmpint(res, ==, 0);

  res = DoIf::mem_fun(a, &C::inc_c);
  g_assert_cmpint(res, ==, 0);

  res = DoIf::mem_fun(c, &A::inc_a);
  g_assert_cmpint(res, ==, 0);

  res = DoIf::mem_fun(c, &B::inc_b);
  g_assert_cmpint(res, ==, 0);


  DoIf::mem_fun(a, &B::set_b, hundred);
  res = DoIf::mem_fun(a, &B::get_b);
  g_assert_cmpint(res, ==, 0);

  DoIf::mem_fun(a, &C::set_c, 60);
  res = DoIf::mem_fun(a, &C::get_c);
  g_assert_cmpint(res, ==, 0);

  DoIf::mem_fun(c, &A::set_a, 50);
  res = DoIf::mem_fun(c, &A::get_a);
  g_assert_cmpint(res, ==, 0);

  DoIf::mem_fun(c, &B::set_b, hundred);
  res = DoIf::mem_fun(c, &B::get_b);
  g_assert_cmpint(res, ==, 0);


  DoIf::fun(a, &call_b, hundred);
  res = DoIf::mem_fun(a, &B::get_b);
  g_assert_cmpint(res, ==, 0);

  DoIf::fun(c, &call_a, 200);
  res = DoIf::mem_fun(c, &A::get_a);
  g_assert_cmpint(res, ==, 0);

  DoIf::fun(c, &call_b, hundred);
  res = DoIf::mem_fun(c, &B::get_b);
  g_assert_cmpint(res, ==, 0);

  res = DoIf::fun(a, &call_c, 200);
  g_assert_cmpint(res, ==, 0);
}

} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/do_if/do_something", test_do_something); 
  g_test_add_func("/do_if/do_nothing", test_do_nothing); 

  return g_test_run();
}
