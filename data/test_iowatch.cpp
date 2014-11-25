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
#include <vector>
#include <utility>

#include <c++-gtk-utils/io_watch.h>
#include <c++-gtk-utils/timeout.h>
#include <c++-gtk-utils/callback.h>
#include <c++-gtk-utils/thread.h>
#include <c++-gtk-utils/emitter.h>
#include <c++-gtk-utils/pipes.h>

using namespace Cgu;

class Test {
  int count;
public:
  Releaser releaser;
  void iowatch_untracked_func(int, bool&);
  void iowatch_tracked_func(int, bool&);
  void iowatch_untracked_cond_func(int, GIOCondition, bool&);
  void iowatch_tracked_cond_func(int, GIOCondition, bool&);
  Test(): count(0) {}
};

Test* test;
GMainLoop* loop;
PipeFifo io_pipe;
std::vector<unsigned char> result;

// this executes in the main thread
void Test::iowatch_untracked_func(int max, bool& keep) {
  ++count;
  g_assert_cmpint(count, <=, max);
  result.push_back(io_pipe.read());

  if (count == max) keep = false;
}

// this executes in the main thread
void Test::iowatch_tracked_func(int max, bool& keep) {
  ++count;
  g_assert_cmpint(count, <=, max);
  result.push_back(io_pipe.read());
  
  if (count == max) delete test; // Releaser will remove the watch callback so we
                                 // won't get anything after 'max' characters
}

// this executes in the main thread
void Test::iowatch_untracked_cond_func(int max, GIOCondition c, bool& keep) {
  g_assert(c & G_IO_IN);
  ++count;
  g_assert_cmpint(count, <=, max);
  result.push_back(io_pipe.read());

  if (count == max) keep = false;
}

// this executes in the main thread
void Test::iowatch_tracked_cond_func(int max, GIOCondition c, bool& keep) {
  g_assert(c & G_IO_IN);
  ++count;
  g_assert_cmpint(count, <=, 3);
  result.push_back(io_pipe.read());
  
  if (count == max) delete test; // Releaser will remove the watch callback so we
                                 // won't get anything after 'max' characters
}

// this executes in the main thread.
void timeout_func(int max, bool& keep) {
  // this function is executed below with a timeout of 200ms, so the
  // write and read of the pipe should be well done by the time this
  // executes, but test for an incomplete read anyway
  if (result.size() < (std::vector<unsigned char>::size_type)max) return;
  g_main_loop_quit(loop);
  keep = false;
}

void thread_func() {
  int count = 0;
  char letter = 'a';
  for (; count < 5; ++count, ++letter) {
    io_pipe.write(letter);
  }
}

extern "C" {
static void test_iowatch_untracked() {

  loop = g_main_loop_new(0, false);

  test = new Test;
  result.clear();
  {
    PipeFifo tmp{PipeFifo::block};
    io_pipe = std::move(tmp);
  }
  start_iowatch(io_pipe.get_read_fd(),
                Callback::make(*test, &Test::iowatch_untracked_func, 4),
                GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR));
  Thread::JoinableHandle t1{Thread::Thread::start(Callback::make(thread_func), true),
                            Thread::JoinableHandle::join_on_exit};
  start_timeout(200, Callback::make(&timeout_func, 4), G_PRIORITY_LOW);
  g_main_loop_run(loop);
  t1.join();
  delete test;
  g_assert_cmpuint(result.size(), ==, 4);
  g_assert_cmpuint(result[0], ==, (unsigned int)'a');
  g_assert_cmpuint(result[1], ==, (unsigned int)'b');
  g_assert_cmpuint(result[2], ==, (unsigned int)'c');
  g_assert_cmpuint(result[3], ==, (unsigned int)'d');

  test = new Test;
  result.clear();
  {
    PipeFifo tmp{PipeFifo::block};
    io_pipe = std::move(tmp);  // closes old pipe
  }
  start_iowatch(io_pipe.get_read_fd(),
                Callback::make(*test, &Test::iowatch_untracked_cond_func, 4),
                GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR));
  Thread::JoinableHandle t2{Thread::Thread::start(Callback::make(thread_func), true),
                            Thread::JoinableHandle::join_on_exit};
  start_timeout(200, Callback::make(&timeout_func, 4), G_PRIORITY_LOW);
  g_main_loop_run(loop);
  t2.join();
  delete test;
  g_assert_cmpuint(result.size(), ==, 4);
  g_assert_cmpuint(result[0], ==, (unsigned int)'a');
  g_assert_cmpuint(result[1], ==, (unsigned int)'b');
  g_assert_cmpuint(result[2], ==, (unsigned int)'c');
  g_assert_cmpuint(result[3], ==, (unsigned int)'d');

  g_main_loop_unref(loop);
}

static void test_iowatch_tracked() {

  loop = g_main_loop_new(0, false);

  test = new Test;
  result.clear();
  {
    PipeFifo tmp{PipeFifo::block};
    io_pipe = std::move(tmp);
  }
  start_iowatch(io_pipe.get_read_fd(),
                Callback::make(*test, &Test::iowatch_tracked_func, 3),
		test->releaser,
                GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR));
  Thread::JoinableHandle t1{Thread::Thread::start(Callback::make(thread_func), true),
                            Thread::JoinableHandle::join_on_exit};
  start_timeout(200, Callback::make(&timeout_func, 3), G_PRIORITY_LOW);
  g_main_loop_run(loop);
  t1.join();
  g_assert_cmpuint(result.size(), ==, 3);
  g_assert_cmpuint(result[0], ==, (unsigned int)'a');
  g_assert_cmpuint(result[1], ==, (unsigned int)'b');
  g_assert_cmpuint(result[2], ==, (unsigned int)'c');

  test = new Test;
  result.clear();
  {
    PipeFifo tmp{PipeFifo::block};
    io_pipe = std::move(tmp);
  }
  start_iowatch(io_pipe.get_read_fd(),
                Callback::make(*test, &Test::iowatch_tracked_cond_func, 3),
		test->releaser,
                GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR));
  Thread::JoinableHandle t2{Thread::Thread::start(Callback::make(thread_func), true),
                            Thread::JoinableHandle::join_on_exit};
  start_timeout(200, Callback::make(&timeout_func, 3), G_PRIORITY_LOW);
  g_main_loop_run(loop);
  t2.join();
  g_assert_cmpuint(result.size(), ==, 3);
  g_assert_cmpuint(result[0], ==, (unsigned int)'a');
  g_assert_cmpuint(result[1], ==, (unsigned int)'b');
  g_assert_cmpuint(result[2], ==, (unsigned int)'c');

  g_main_loop_unref(loop);
}

static void test_iowatch_lambda() {

  loop = g_main_loop_new(0, false);

  test = new Test;
  result.clear();
  {
    PipeFifo tmp{PipeFifo::block};
    io_pipe = std::move(tmp);
  }
  start_iowatch(io_pipe.get_read_fd(),
		[] (GIOCondition c, bool& keep) {int max = 2; test->iowatch_untracked_cond_func(max, c, keep);},
                GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR));
  Thread::JoinableHandle t1{Thread::Thread::start(Callback::make(thread_func), true),
                            Thread::JoinableHandle::join_on_exit};
  start_timeout(200, Callback::make(&timeout_func, 2), G_PRIORITY_LOW);
  g_main_loop_run(loop);
  t1.join();
  delete test;
  g_assert_cmpuint(result.size(), ==, 2);
  g_assert_cmpuint(result[0], ==, (unsigned int)'a');
  g_assert_cmpuint(result[1], ==, (unsigned int)'b');

  test = new Test;
  result.clear();
  {
    PipeFifo tmp{PipeFifo::block};
    io_pipe = std::move(tmp);
  }

  test = new Test;
  result.clear();
  {
    PipeFifo tmp{PipeFifo::block};
    io_pipe = std::move(tmp);
  }
  start_iowatch(io_pipe.get_read_fd(),
		[] (GIOCondition c, bool& keep) {int max = 3; test->iowatch_tracked_cond_func(max, c, keep);},
		test->releaser,
                GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR));
  Thread::JoinableHandle t2{Thread::Thread::start(Callback::make(thread_func), true),
                            Thread::JoinableHandle::join_on_exit};
  start_timeout(200, Callback::make(&timeout_func, 3), G_PRIORITY_LOW);
  g_main_loop_run(loop);
  t2.join();
  g_assert_cmpuint(result.size(), ==, 3);
  g_assert_cmpuint(result[0], ==, (unsigned int)'a');
  g_assert_cmpuint(result[1], ==, (unsigned int)'b');
  g_assert_cmpuint(result[2], ==, (unsigned int)'c');

  g_main_loop_unref(loop);
}
} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/iowatch/iowatch_untracked", test_iowatch_untracked);
  g_test_add_func("/iowatch/iowatch_tracked", test_iowatch_tracked);
  g_test_add_func("/iowatch/iowatch_lambda", test_iowatch_lambda);

  return g_test_run();
}
