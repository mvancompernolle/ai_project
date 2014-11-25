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

#include <c++-gtk-utils/intrusive_ptr.h>

using namespace Cgu;

class B: public IntrusiveCounter {
  int b;
public:
  static int val;
  static void reset() {val = 0;}
  B(): b(0) {reset();}
  virtual ~B() {val = 1;}
};

class D: public B {
  int d;
public:
  D(): d(0) {}
};

class Bl: public IntrusiveLockCounter {
  int b;
public:
  static int val;
  static void reset() {val = 0;}
  Bl(): b(0) {reset();}
  virtual ~Bl() {val = 1;}
};

class Dl: public Bl {
  int d;
public:
  Dl(): d(0) {}
};

int B::val;
int Bl::val;

extern "C" {
static void test_intrusive_ptr_counter() {

  {
    D* d = new D;
    IntrusivePtr<D> i1{d};
    IntrusivePtr<B> i2 = i1;
    IntrusivePtr<B> i3{d};
  }
  g_assert_cmpint(B::val, ==, 1);

  {
    IntrusivePtr<D> i1{new D};
    IntrusivePtr<B> i2{i1};

    IntrusivePtr<B> i3{new D};

    g_assert_cmpint(B::val, ==, 0);
    i3 = i1;
    g_assert_cmpint(B::val, ==, 1);

    i2.reset(new D);
    i3.reset();

    i1 = i1;
    g_assert_cmpint(B::val, ==, 0);
  }
  g_assert_cmpint(B::val, ==, 1);

  {
    IntrusivePtr<B> i;
    {
      i.reset(new B);
      i->ref();
    }
    i->unref();
    g_assert_cmpint(B::val, ==, 0);
  }
  g_assert_cmpint(B::val, ==, 1);
}

static void test_intrusive_ptr_lock_counter() {

  {
    Dl* dl = new Dl;
    IntrusivePtr<Dl> i1{dl};
    IntrusivePtr<Bl> i2 = i1;
    IntrusivePtr<Bl> i3{dl};
  }
  g_assert_cmpint(Bl::val, ==, 1);

  {
    IntrusivePtr<Dl> i1{new Dl};
    IntrusivePtr<Bl> i2{i1};

    IntrusivePtr<Bl> i3{new Dl};

    g_assert_cmpint(Bl::val, ==, 0);
    i3 = i1;
    g_assert_cmpint(Bl::val, ==, 1);

    i2.reset(new Dl);
    i3.reset();

    i1 = i1;
    g_assert_cmpint(Bl::val, ==, 0);
  }
  g_assert_cmpint(Bl::val, ==, 1);

  {
    IntrusivePtr<Bl> i;
    {
      i.reset(new Bl);
      i->ref();
    }
    i->unref();
    g_assert_cmpint(Bl::val, ==, 0);
  }
  g_assert_cmpint(Bl::val, ==, 1);
}

} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/intrusive_ptr/counter", test_intrusive_ptr_counter); 
  g_test_add_func("/intrusive_ptr/lock_counter", test_intrusive_ptr_lock_counter); 

  return g_test_run();
}
