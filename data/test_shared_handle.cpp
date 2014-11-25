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

#include <utility>  // for std::move

#include <c++-gtk-utils/shared_handle.h>

using namespace Cgu;

class B {
  int b;
public:
  static int val;
  static void reset() {val = 0;}
  B(): b(0) {reset();}
  virtual ~B() {val = 1;}
};

int B::val;

typedef SharedHandle<B*, GSliceDestroy<B*>> BHandle;
typedef SharedLockHandle<B*, GSliceDestroy<B*>> BLockHandle;
typedef ScopedHandle<B*, GSliceDestroy<B*>> BScopedHandle;

BHandle make_B() {
  B* p = g_slice_new(B);
  new(p) B;
  return BHandle(p);
}

BLockHandle make_BLock() {
  B* p = g_slice_new(B);
  new(p) B;
  return BLockHandle(p);
}

extern "C" {
static void test_shared_handle() {
  {
    SharedHandle<unsigned char*> handle{new unsigned char[10]};
    handle.get()[0] = 'a';
    handle[1] = 'b';
    g_assert_cmpuint(handle[0], ==, (unsigned int)'a');
    g_assert_cmpuint(handle.get()[1], ==, (unsigned int)'b');
  }

  {
    GcharSharedHandle s1{(gchar*) g_malloc(20)};
    {
      GcharSharedHandle s2{s1};
      GcharSharedHandle s3 = s1;
      g_assert_cmpuint(s1.get_refcount(), ==, s3.get_refcount());
      g_assert_cmpuint(s1.get_refcount(), ==, 3);
    }
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
  }

  {
    SharedHandle<char*, CFree> s1{(char*) malloc(20)};
    {
      SharedHandle<char*, CFree> s2{s1};
      SharedHandle<char*, CFree> s3 = s1;
      g_assert_cmpuint(s1.get_refcount(), ==, s3.get_refcount());
      g_assert_cmpuint(s1.get_refcount(), ==, 3);
    }
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
  }

  {
    BHandle s1{make_B()};
    BHandle s2{s1};

    BHandle s3 = make_B();

    g_assert_cmpint(B::val, ==, 0);
    s3 = s1;
    g_assert_cmpint(B::val, ==, 1);
    g_assert_cmpuint(s1.get_refcount(), ==, 3);

    B* p = g_slice_new(B);
    new(p) B;
    s2.reset(p, SharedHandleAllocFail::leave);
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

static void test_shared_lock_handle() {
  {
    SharedLockHandle<unsigned char*> handle{new unsigned char[10]};
    handle.get()[0] = 'a';
    handle[1] = 'b';
    g_assert_cmpuint(handle[0], ==, (unsigned int)'a');
    g_assert_cmpuint(handle.get()[1], ==, (unsigned int)'b');
  }

  {
    SharedLockHandle<gchar*, GFree> s1{(gchar*) g_malloc(20)};
    {
      SharedLockHandle<gchar*, GFree> s2{s1};
      SharedLockHandle<gchar*, GFree> s3 = s1;
      g_assert_cmpuint(s1.get_refcount(), ==, s3.get_refcount());
      g_assert_cmpuint(s1.get_refcount(), ==, 3);
    }
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
  }

  {
    SharedLockHandle<char*, CFree> s1{(char*) malloc(20)};
    {
      SharedLockHandle<char*, CFree> s2{s1};
      SharedLockHandle<char*, CFree> s3 = s1;
      g_assert_cmpuint(s1.get_refcount(), ==, s3.get_refcount());
      g_assert_cmpuint(s1.get_refcount(), ==, 3);
    }
    g_assert_cmpuint(s1.get_refcount(), ==, 1);
  }

  {
    BLockHandle s1{make_BLock()};
    BLockHandle s2{s1};

    BLockHandle s3 = make_BLock();

    g_assert_cmpint(B::val, ==, 0);
    s3 = s1;
    g_assert_cmpint(B::val, ==, 1);
    g_assert_cmpuint(s1.get_refcount(), ==, 3);

    B* p = g_slice_new(B);
    new(p) B;
    s2.reset(p, SharedHandleAllocFail::leave);
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

static void test_scoped_handle() {
  {
    ScopedHandle<unsigned char*> tmp{new unsigned char[10]};
    ScopedHandle<unsigned char*> handle{std::move(tmp)};
    handle.get()[0] = 'a';
    handle[1] = 'b';
    g_assert_cmpuint(handle[0], ==, (unsigned int)'a');
    g_assert_cmpuint(handle.get()[1], ==, (unsigned int)'b');
  }

  {
    GcharScopedHandle tmp{(gchar*) g_malloc(20)};
    GcharScopedHandle handle;
    handle = std::move(tmp);
    handle.get()[0] = 'a';
    handle[1] = 'b';
    g_assert_cmpuint(handle[0], ==, (unsigned int)'a');
    g_assert_cmpuint(handle.get()[1], ==, (unsigned int)'b');
  }

  {
    ScopedHandle<char*, CFree> handle{(char*) malloc(20)};
    handle.get()[0] = 'a';
    handle[1] = 'b';
    g_assert_cmpuint(handle[0], ==, (unsigned int)'a');
    g_assert_cmpuint(handle.get()[1], ==, (unsigned int)'b');
  }

  {
    B* p1 = g_slice_new(B);
    new(p1) B;
    BScopedHandle handle{p1};
    g_assert_cmpint(B::val, ==, 0);

    B* p2 = g_slice_new(B);
    new(p2) B;
    handle.reset(p2);
    g_assert_cmpint(B::val, ==, 1); // first B object deleted
    handle.get()->reset();
    g_assert_cmpint(B::val, ==, 0);
    
    B* p3 = handle.release();
    handle.reset(p3);
    g_assert_cmpint(B::val, ==, 0);
  }
  g_assert_cmpint(B::val, ==, 1);   // second B object deleted
}

} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/shared_handle/plain", test_shared_handle); 
  g_test_add_func("/shared_handle/lock", test_shared_lock_handle); 
  g_test_add_func("/shared_handle/scoped", test_scoped_handle);

  return g_test_run();
}
