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

#include <c++-gtk-utils/callback.h>
#include <c++-gtk-utils/emitter.h>

using namespace Cgu;

class Test {
  int val;
public:
  Releaser releaser;
  void set1(int i) {val = i;}
  void set2(const int& i) {val = i;}
  void set3(int& i) {val = i;}
  void nil() {g_assert_not_reached();}
  int get() const {return val;}
  Test(): val(0) {}
};

extern "C" {
static void test_emit() {
  Test obj;
  int i = 5, j = 0;
  {
    Emitter e;
    e.connect(Callback::make(obj, &Test::set1, i));
    e();
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 6;
    SafeEmitterArg<const int&> e;
    e.connect(Callback::make(obj, &Test::set2));
    e.emit(i);
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 7;
    SafeEmitter e;
    Callback::SafeFunctor f1{Callback::make<Test, int&>(obj, &Test::set3, i)}; 
    auto f2 = e.connect(f1);
    g_assert(f1 == f2);
    e.emit();
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 8;
    Emitter e;
    auto l = [i, &obj] () {obj.set1(i);};
    e.connect(l);
    e();
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 9;
    SafeEmitterArg<int&> e;
    e.connect([&obj] (int& i) {obj.set2(i);});
    e.emit(i);
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 10;
    EmitterArg<int&> e;
    auto f = e.connect([&obj] (int& a) mutable {obj.set3(a);});
    e.emit(i);
    j = obj.get();
    g_assert_cmpint(j, ==, i);

    i = 11;
    f(i);
    j = obj.get();
    g_assert_cmpint(j, ==, i);

    e.disconnect(f);
    int k = 12;
    e.emit(k);
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    i = 12;
    SafeEmitterArg<int&> e;
    auto l = [&obj] (int& a) mutable {obj.set3(a);};
    auto f = e.connect(l);
    e.emit(i);
    j = obj.get();
    g_assert_cmpint(j, ==, i);

    i = 13;
    f(i);
    j = obj.get();
    g_assert_cmpint(j, ==, i);

    e.disconnect(f);
    int k = 14;
    e.emit(k);
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

  {
    // from the previous block, 'i' has value 13
    SafeEmitterArg<int> e;
    {
      // take a copy of the Test object, leaving 'obj' unaltered.
      // This will also test that a copy of the lambda is taken, as
      // the lambda is a lvalue (otherwise this would provoke a
      // segfault)
      auto l = [obj] (int a) mutable {
	obj.set1(a);
	int b = obj.get();
	g_assert_cmpint(a, ==, b);
      };
      e.connect(l);
    }
    e.emit(15);
    j = obj.get();
    g_assert_cmpint(j, ==, i);
  }

}

static void test_release() {

  {
    int i = 5, j = 6, k = 0;
    Test* obj = new Test;
    Emitter* e1 = new Emitter;
    EmitterArg<const int&>* e2 = new EmitterArg<const int&>;
      
    e1->connect(Callback::make(*obj, &Test::set1, i), obj->releaser);
    e2->connect([obj] (const int& a) {obj->set2(a);}, obj->releaser);

    e1->emit();
    k = obj->get();
    g_assert_cmpint(k, ==, i);

    e2->emit(j);
    k = obj->get();
    g_assert_cmpint(k, ==, j);

    // destroy e2 and emit again on e1
    delete e2;
    e1->emit();
    k = obj->get();
    g_assert_cmpint(k, ==, i);

    // connect additional callback to e1 and destroy the Test obj
    e1->connect(Callback::make(*obj, &Test::nil), obj->releaser);
    delete obj;
    e1->emit();
    delete e1;
  }

  // repeat for SafeEmitter
  {
    int i = 5, j = 6, k = 0;
    Test* obj = new Test;
    SafeEmitter* e1 = new SafeEmitter;
    SafeEmitterArg<const int&>* e2 = new SafeEmitterArg<const int&>;
      
    e1->connect([obj, i] () {obj->set1(i);}, obj->releaser);
    e2->connect(Callback::make(*obj, &Test::set2), obj->releaser);

    e1->emit();
    k = obj->get();
    g_assert_cmpint(k, ==, i);

    e2->emit(j);
    k = obj->get();
    g_assert_cmpint(k, ==, j);

    // destroy e2 and emit again on e1
    delete e2;
    e1->emit();
    k = obj->get();
    g_assert_cmpint(k, ==, i);

    // connect additional callback to e1 and destroy the Test obj
    e1->connect(Callback::make(*obj, &Test::nil), obj->releaser);
    delete obj;
    e1->emit();
    delete e1;
  }
}

} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/emitter/emit", test_emit); 
  g_test_add_func("/emitter/release", test_release); 

  return g_test_run();
}
