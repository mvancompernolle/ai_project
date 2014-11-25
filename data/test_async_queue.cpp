/* Copyright (C) 2012 and 2014 Chris Vine

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
#include <deque>
#include <memory>
#include <utility>    // for std::move

#include <c++-gtk-utils/async_queue.h>
#include <c++-gtk-utils/thread.h>
#include <c++-gtk-utils/cgu_config.h>

using namespace Cgu;

extern "C" {
static void test_async_queue_simple() {
  {
    AsyncQueue<std::string> q1;
    q1.push(std::string("One"));
    std::string two{"Two"};
    q1.push(two);
    q1.emplace("Three");

    AsyncQueue<std::string> q2{q1};
    AsyncQueue<std::string> q3;
    q3 = q1;
    AsyncQueue<std::string> q4{std::move(q3)};
    AsyncQueue<std::string> q5 = std::move(q4);
    AsyncQueue<std::string> q6;
    swap(q5, q6);

    g_assert_cmpuint((unsigned int)q1.empty(), ==, 0);
    g_assert_cmpuint((unsigned int)q1.size(), ==, 3);
    g_assert_cmpuint((unsigned int)q2.empty(), ==, 0);
    g_assert_cmpuint((unsigned int)q2.size(), ==, 3);
    g_assert_cmpuint((unsigned int)q6.empty(), ==, 0);
    g_assert_cmpuint((unsigned int)q6.size(), ==, 3);
    /*
    // only include these assertions if the implementation in fact moves
    // by nulling the movant
    g_assert_cmpuint((unsigned int)q3.empty(), !=, 0);
    g_assert_cmpuint((unsigned int)q3.size(), ==, 0);
    g_assert_cmpuint((unsigned int)q4.empty(), !=, 0);
    g_assert_cmpuint((unsigned int)q4.size(), ==, 0);
    g_assert_cmpuint((unsigned int)q5.empty(), !=, 0);
    g_assert_cmpuint((unsigned int)q5.size(), ==, 0);
    */

    std::string s;
    q1.pop(s);
    g_assert_cmpstr(s.c_str(), ==, "One");
    q1.move_pop_basic(s);
    g_assert_cmpstr(s.c_str(), ==, "Two");
    q1.move_pop(s);
    g_assert_cmpstr(s.c_str(), ==, "Three");
    g_assert_cmpuint((unsigned int)q1.empty(), !=, 0);
    g_assert_cmpuint((unsigned int)q1.size(), ==, 0);
  }

  {
    AsyncQueue<std::string, std::deque<std::string>> q1;
    q1.push(std::string("One"));
    std::string two{"Two"};
    q1.push(two);
    q1.emplace("Three");

    std::string s;
    q1.pop(s);
    g_assert_cmpstr(s.c_str(), ==, "One");
    q1.move_pop_basic(s);
    g_assert_cmpstr(s.c_str(), ==, "Two");
    q1.move_pop(s);
    g_assert_cmpstr(s.c_str(), ==, "Three");
    g_assert_cmpuint((unsigned int)q1.empty(), !=, 0);
  }
}

static void test_async_queue_dispatch() {
  {
    AsyncQueueDispatch<std::string> q1;
    q1.push(std::string("One"));
    std::string two{"Two"};
    q1.push(two);
    q1.emplace("Three");
    q1.emplace("Four");
    q1.emplace("Five");
    q1.emplace("Six");

    AsyncQueueDispatch<std::string> q2{q1};
    AsyncQueueDispatch<std::string> q3;
    q3 = q1;
    AsyncQueueDispatch<std::string> q4{std::move(q3)};
    AsyncQueueDispatch<std::string> q5 = std::move(q4);
    AsyncQueueDispatch<std::string> q6;
    swap(q5, q6);

    g_assert_cmpuint((unsigned int)q1.empty(), ==, 0);
    g_assert_cmpuint((unsigned int)q1.size(), ==, 6);
    g_assert_cmpuint((unsigned int)q2.empty(), ==, 0);
    g_assert_cmpuint((unsigned int)q2.size(), ==, 6);
    g_assert_cmpuint((unsigned int)q6.empty(), ==, 0);
    g_assert_cmpuint((unsigned int)q6.size(), ==, 6);
    /*
    // only include these assertions if the implementation in fact moves
    // by nulling the movant
    g_assert_cmpuint((unsigned int)q3.empty(), !=, 0);
    g_assert_cmpuint((unsigned int)q3.size(), ==, 0);
    g_assert_cmpuint((unsigned int)q4.empty(), !=, 0);
    g_assert_cmpuint((unsigned int)q4.size(), ==, 0);
    g_assert_cmpuint((unsigned int)q5.empty(), !=, 0);
    g_assert_cmpuint((unsigned int)q5.size(), ==, 0);
    */

    std::string s;
    q1.pop(s);
    g_assert_cmpstr(s.c_str(), ==, "One");
    q1.move_pop_basic(s);
    g_assert_cmpstr(s.c_str(), ==, "Two");
    q1.move_pop(s);
    g_assert_cmpstr(s.c_str(), ==, "Three");
    q1.pop_dispatch(s);
    g_assert_cmpstr(s.c_str(), ==, "Four");
    q1.move_pop_dispatch_basic(s);
    g_assert_cmpstr(s.c_str(), ==, "Five");
    q1.move_pop_dispatch(s);
    g_assert_cmpstr(s.c_str(), ==, "Six");
    g_assert_cmpuint((unsigned int)q1.empty(), !=, 0);
    g_assert_cmpuint((unsigned int)q1.size(), ==, 0);

    std::unique_ptr<Thread::Thread> t(
      Thread::Thread::start([&q1] () {
	  q1.push(std::string("push1"));
	  q1.push(std::string("push2"));
	  q1.push(std::string("push3"));
	},
	false)
    );
    g_assert(t.get() != static_cast<Thread::Thread*>(0));

    bool res;
    res = q1.pop_timed_dispatch(s, 1000000);
    g_assert_cmpstr(s.c_str(), ==, "push1");
    g_assert_cmpuint((unsigned int)res, ==, 0);
    res = q1.move_pop_timed_dispatch_basic(s, 1000000);
    g_assert_cmpstr(s.c_str(), ==, "push2");
    g_assert_cmpuint((unsigned int)res, ==, 0);
    res = q1.move_pop_timed_dispatch(s, 1000000);
    g_assert_cmpstr(s.c_str(), ==, "push3");
    g_assert_cmpuint((unsigned int)res, ==, 0);

    res = q1.pop_timed_dispatch(s, 100);
    g_assert_cmpstr(s.c_str(), ==, "push3"); // unchanged
    g_assert_cmpuint((unsigned int)res, !=, 0);
    res = q1.move_pop_timed_dispatch_basic(s, 100);
    g_assert_cmpstr(s.c_str(), ==, "push3"); // unchanged
    g_assert_cmpuint((unsigned int)res, !=, 0);
    res = q1.move_pop_timed_dispatch(s, 100);
    g_assert_cmpstr(s.c_str(), ==, "push3"); // unchanged
    g_assert_cmpuint((unsigned int)res, !=, 0);
  }

  {
    AsyncQueueDispatch<std::string, std::deque<std::string>> q1;
    q1.push(std::string("One"));
    std::string two{"Two"};
    q1.push(two);
    q1.emplace("Three");

    std::string s;
    q1.move_pop_basic(s);
    g_assert_cmpstr(s.c_str(), ==, "One");
    q1.move_pop_dispatch_basic(s);
    g_assert_cmpstr(s.c_str(), ==, "Two");
    q1.move_pop_timed_dispatch_basic(s, 1000);
    g_assert_cmpstr(s.c_str(), ==, "Three");
    g_assert_cmpuint((unsigned int)q1.empty(), !=, 0);
  }
}
} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/async_queue/simple", test_async_queue_simple); 
  g_test_add_func("/async_queue/dispatch", test_async_queue_dispatch); 

  return g_test_run();
}
