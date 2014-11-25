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

#include <c++-gtk-utils/shared_ptr.h>

using namespace Cgu;

class B {
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

int B::val;

extern "C" {
static void test_shared_ptr() {

  {
    SharedPtr<D> s1{new D};
    {
      SharedPtr<B> s2{s1};
      SharedPtr<B> s3 = s1;
      SharedPtr<D> s4{s1};
      SharedPtr<D> s5 = s1;
      g_assert_cmpuint(s1.get_refcount(), ==, s5.get_refcount());
      g_assert_cmpuint(s1.get_refcount(), ==, 5);
    }
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
    g_assert_cmpint(B::val, ==, 0);
  }
  g_assert_cmpint(B::val, ==, 1);

  {
    SharedPtr<D> s1{new D};
    SharedPtr<B> s2{s1};

    SharedPtr<B> s3{new D};

    g_assert_cmpint(B::val, ==, 0);
    s3 = s1;
    g_assert_cmpint(B::val, ==, 1);
    g_assert_cmpuint(s1.get_refcount(), ==, 3);
    s1->reset();

    s2.reset(new D, SharedPtrAllocFail::leave);
    s3.reset();
    
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
    g_assert_cmpuint(s2.get_refcount(), ==, 1);
    g_assert_cmpuint(s3.get_refcount(), ==, 0);
    g_assert_cmpint(B::val, ==, 0);
    s1 = s1;
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
    g_assert_cmpint(B::val, ==, 0);
  }
  g_assert_cmpint(B::val, ==, 1);
}

static void test_shared_lock_ptr() {

  {
    SharedLockPtr<D> s1{new D};
    {
      SharedLockPtr<B> s2{s1};
      SharedLockPtr<B> s3 = s1;
      SharedLockPtr<D> s4{s1};
      SharedLockPtr<D> s5 = s1;
      g_assert_cmpuint(s1.get_refcount(), ==, s5.get_refcount());
      g_assert_cmpuint(s1.get_refcount(), ==, 5);
    }
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
    g_assert_cmpint(B::val, ==, 0);
  }
  g_assert_cmpint(B::val, ==, 1);

  {
    SharedLockPtr<D> s1{new D};
    SharedLockPtr<B> s2{s1};

    SharedLockPtr<B> s3{new D};

    g_assert_cmpint(B::val, ==, 0);
    s3 = s1;
    g_assert_cmpint(B::val, ==, 1);
    g_assert_cmpuint(s1.get_refcount(), ==, 3);
    s1->reset();

    s2.reset(new D, SharedPtrAllocFail::leave);
    s3.reset();
    
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
    g_assert_cmpuint(s2.get_refcount(), ==, 1);
    g_assert_cmpuint(s3.get_refcount(), ==, 0);
    g_assert_cmpint(B::val, ==, 0);
    s1 = s1;
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
    g_assert_cmpint(B::val, ==, 0);
  }
  g_assert_cmpint(B::val, ==, 1);
}

} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/shared_ptr/plain", test_shared_ptr); 
  g_test_add_func("/shared_ptr/lock", test_shared_lock_ptr); 

  return g_test_run();
}
