/* Copyright (C) 2014 Chris Vine

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

NOTE: The binary produced by compilation of this file links with
libguile and is therefore governed by the LGPL version 3 or greater.

*/

#include <glib.h>

#include <c++-gtk-utils/cgu_config.h>

#ifdef CGU_USE_GUILE

#include <libguile.h>

#include <string>
#include <cstring>

#include <c++-gtk-utils/extension.h>
#include <c++-gtk-utils/task_manager.h>
#include <c++-gtk-utils/mutex.h>

using namespace Cgu;

extern "C" {
static void test_extension_sync() {
  {
    long res = Extension::exec_shared("(define arg1 4)(define arg2 6)(define arg3 \"unwrapped\")",
				      "./guile-test.scm",
				      Extension::integer_to_long);
    g_assert(res == 10);
  }
  {
    double res = Extension::exec("(define arg1 2.5)(define arg2 3.5)(define arg3 \"unwrapped\")",
				 "./guile-test.scm",
				 Extension::real_to_double);
    g_assert(res > 5.99 && res <  6.01);
  }
  {
    std::string res = Extension::exec_shared("(define arg1 \"one-\")(define arg2 \"two\")(define arg3 \"unwrapped\")",
					     "./guile-test.scm",
					     Extension::string_to_string);
    g_assert(res == "one-two");
  }
  {
    std::vector<long> res =
      Extension::exec("(define arg1 4)(define arg2 6)(define arg3 \"wrapped\")",
		      "./guile-test.scm",
		      Extension::list_to_vector_long);
    g_assert(res.size() == 1 && res[0] == 10);
  }
  {
    std::vector<double> res =
      Extension::exec_shared("(define arg1 2.5)(define arg2 3.5)(define arg3 \"wrapped\")",
			     "./guile-test.scm",
			     Extension::list_to_vector_double);
    g_assert(res.size() == 1 && res[0] > 5.99 && res[0] <  6.01);
  }
  {
    std::vector<std::string> res =
      Extension::exec("(define arg1 \"one-\")(define arg2 \"two\")(define arg3 \"wrapped\")",
		      "./guile-test.scm",
		      Extension::list_to_vector_string);
    g_assert(res.size() == 1 && res[0] == "one-two");
  }
  {
    bool except = false;
    try{ 
      Extension::exec_shared("(define arg \"My exception\")",
			     "./guile-test-exception.scm",
			     Extension::any_to_void);
      g_assert_not_reached();
    }
    catch (Extension::GuileException& e) {
      g_assert(std::strstr(e.what(), "My exception"));
      except = true;
    }
    g_assert_cmpuint((unsigned int)except, !=, 0);
  }
}

static void test_extension_async() {

  GMainLoop* loop = g_main_loop_new(0, false);

  Thread::TaskManager tm{1};
  Thread::Mutex mutex;
  int count = 0;

  tm.make_task_when(
    [&count, &mutex] (const long& res) {
      g_assert(res == 10);
      mutex.lock();
      ++count;
      mutex.unlock();
    },
    0,
    [] () {   
      return Extension::exec("(define arg1 4)(define arg2 6)(define arg3 \"unwrapped\")",
			     "./guile-test.scm",
			     Extension::integer_to_long);
    });
  tm.make_task_when_full(
    [] (const std::string&) {
      g_assert_not_reached();
    },
    0,
    [&count, &mutex, loop] () {
      mutex.lock();
      ++count;
      mutex.unlock();
      g_main_loop_quit(loop);
    },
    0,
    G_PRIORITY_DEFAULT,
    0,
    [] () {
      return Extension::exec_shared("(define arg \"My exception\")",
				    "./guile-test-exception.scm",
				    Extension::string_to_string);
    });

  g_main_loop_run(loop);
  mutex.lock();
  g_assert_cmpint(count, ==, 2);
  mutex.unlock();
}
} // extern "C"
#endif


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

#ifdef CGU_USE_GUILE
  g_test_add_func("/test_extension/test_extension_sync", test_extension_sync); 
  g_test_add_func("/test_extension/test_extension_async", test_extension_async); 
#endif

  return g_test_run();  
}
