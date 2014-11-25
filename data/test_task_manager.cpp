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

#include <exception>
#include <functional>
#include <memory>
#include <future>
#include <string>

#include <glib.h>

#include <c++-gtk-utils/task_manager.h>
#include <c++-gtk-utils/mutex.h>
#include <c++-gtk-utils/callback.h>
#include <c++-gtk-utils/emitter.h>
#include <c++-gtk-utils/cgu_config.h>

using namespace Cgu;

/**************** for the basic TaskManager tests ****************/

static unsigned int count;
static Thread::Mutex mutex;
static Thread::Cond cond;

void task1(int microsec) {
  g_usleep(microsec);
  mutex.lock();
  ++count;
  mutex.unlock();
  cond.signal();
}

void task2(int microsec) {
  mutex.lock();
  ++count;
  mutex.unlock();
  cond.signal();
  g_usleep(microsec);
}

void task3() {
  throw Thread::Exit();
}

void task4() {
  throw std::exception();
}

void fail_cb() {
  mutex.lock();
  ++count;
  mutex.unlock();
  cond.signal();
}

/************ for the TaskManager::make_task_* tests ************/

class Test {
  int i;
public:
  Releaser releaser;

  int add0() {return i;}
  int add1(int j) {return i + j;}
  int add2(int j, int k) {return i + j + k;}
  int add3(int j, int k, int l) {return i + j + k + l;}
  int add_fail1() {throw Thread::Exit(); return i;}
  int add_fail2() {throw std::exception(); return i;}
  int uwait(GMainLoop* loop, unsigned int max_count,
	    int microsec) {
    g_usleep(microsec); 
    mutex.lock();
    ++count;
    if (count == max_count) g_main_loop_quit(loop);
    mutex.unlock();
    return i;
  }

  Test() : i(1) {}
};

class ConstTest {
  int i;
public:
  mutable Releaser releaser;

  int add0() const {return i;}
  int add1(int j) const {return i + j;}
  int add2(int j, int k) const {return i + j + k;}
  int add3(int j, int k, int l) const {return i + j + k + l;}
  int add_fail1() const {throw Thread::Exit(); return i;}
  int add_fail2() const {throw std::exception(); return i;}
  int uwait(GMainLoop* loop, unsigned int max_count,
	    int microsec) const {
    g_usleep(microsec); 
    mutex.lock();
    ++count;
    if (count == max_count) g_main_loop_quit(loop);
    mutex.unlock();
    return i;
  }

  ConstTest() : i(1) {}
};

int add0() {return 0;}
int add1(int j) {return j;}
int add2(int j, int k) {return j + k;}
int add3(int j, int k, int l) {return j + k + l;}
int add4(int j, int k, int l, int m) {return j + k + l + m;}
int add_fail1() {throw Thread::Exit(); return 0;}
int add_fail2() {throw std::exception(); return 0;}
int uwait(GMainLoop* loop, unsigned int max_count,
	  int microsec) {
  g_usleep(microsec); 
  mutex.lock();
  ++count;
  if (count == max_count) g_main_loop_quit(loop);
  mutex.unlock();
  return 0;
}

void when(GMainLoop* loop, unsigned int max_count,
	  int expected, const int& obtained) {
  g_assert_cmpint(expected, ==, obtained);
  mutex.lock();
  ++count;
  if (count == max_count) g_main_loop_quit(loop);
  mutex.unlock();
}

void fail(GMainLoop* loop, unsigned int max_count) {
  mutex.lock();
  ++count;
  if (count == max_count) g_main_loop_quit(loop);
  mutex.unlock();
}

void no_when(const int&) {
  g_assert_not_reached();
}

void no_fail() {
  g_assert_not_reached();
}

/***************** end of test support stuff *****************/

extern "C" {
static void test_used_threads() {
  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm;

    // use deprecated add_task() functions
    tm.add_task(Callback::make(&task1, 1000));
    tm.add_task(Callback::make(&task1, 1000));
    tm.add_task(Callback::make(&task1, 1000));
    g_assert_cmpuint(tm.get_used_threads(), ==, 3U);
    g_assert_cmpuint(tm.get_idle_time(), ==, 10000U);
    g_assert_cmpuint(tm.get_min_threads(), ==, 0U);
    g_assert_cmpuint(tm.get_max_threads(), ==, 8U);
    mutex.lock();
    timespec ts;
    Thread::Cond::get_abs_time(ts, 5000);
    while (count < 3) {
      if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
    }
    mutex.unlock();
    // once count == 3, 'tasks' should be 0 very soon thereafter
    while (tm.get_tasks());
    g_assert_cmpuint(tm.get_used_threads(), ==, 3U);
    tm.stop_all();
    g_assert_cmpuint(tm.get_used_threads(), ==, 0U);
  }

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm(3, 2, 10000);

    tm.add_task([] () {task2(1000);});
    tm.add_task([] () {task2(1000);});
    tm.add_task([] () {task2(1000);});
    tm.add_task([] () {task2(1000);});
    mutex.lock();
    timespec ts;
    Thread::Cond::get_abs_time(ts, 5000);
    while (count < 3) {
      if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
    }
    mutex.unlock();
    g_assert_cmpuint(tm.get_used_threads(), ==, 3U);
    tm.stop_all();
    mutex.lock();
    g_assert_cmpuint(count, ==, 4U);
    mutex.unlock();
  }

  // the next test considerably extends the time taken to run these
  // tests: uncomment it to run the test
  /*
  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm(3, 2, 10000, true, Thread::TaskManager::wait_for_running);

    // this test will give a false fail if the system is so heavily
    // loaded that it takes more than 500 milliseconds to begin
    // executing tm.stop_all(): however, this is almost inconceivable.
    tm.add_task(Callback::make(&task2, 500000));
    tm.add_task(Callback::make(&task2, 500000));
    tm.add_task(Callback::make(&task2, 500000));
    tm.add_task(Callback::make(&task2, 500000));
    g_assert_cmpuint(tm.get_tasks(), ==, 4U);
    g_assert_cmpuint(tm.get_used_threads(), ==, 3U);
    mutex.lock();
    timespec ts;
    Thread::Cond::get_abs_time(ts, 5000);
    while (count < 3) {
      if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
    }
    mutex.unlock();
    tm.stop_all();
    mutex.lock();
    g_assert_cmpuint(count, ==, 3U);
    mutex.unlock();
  }
  */
}

static void test_non_block() {
  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm;
    tm.set_blocking(false);

    tm.add_task(Callback::make(&task1, 100000));
    tm.add_task(Callback::make(&task1, 100000));
    tm.add_task(Callback::make(&task1, 100000));
    tm.add_task(Callback::make(&task1, 100000));
  }
  mutex.lock();
  timespec ts;
  Thread::Cond::get_abs_time(ts, 5000);
  while (count < 4) {
    if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
  }
  mutex.unlock();
  g_usleep(1000); // add a bit more randomness
}

static void test_max_threads() {
  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm(3, 2, 10000);

    // this test will give a false fail if the system is so heavily
    // loaded that it takes more than 300 milliseconds to begin
    // executing tm.set_max_threads(): however, this is almost
    // inconceivable
    tm.add_task(Callback::make(&task2, 300000));
    tm.add_task(Callback::make(&task2, 300000));
    tm.add_task(Callback::make(&task2, 300000));
    tm.add_task(Callback::make(&task2, 300000));
    tm.add_task(Callback::make(&task2, 300000));
    g_assert_cmpuint(tm.get_used_threads(), ==, 3U);
    tm.set_max_threads(6);
    g_assert_cmpuint(tm.get_used_threads(), ==, 5U);
    g_assert_cmpuint(tm.get_max_threads(), ==, 6U);
    mutex.lock();
    timespec ts;
    Thread::Cond::get_abs_time(ts, 5000);
    while (count < 5) {
      if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
    }
    mutex.unlock();
  }

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm(3);

    // this test will give a false fail if the system is so heavily
    // loaded that it takes more than 300 milliseconds to begin
    // executing tm.set_max_threads(): however, this is almost
    // inconceivable
    tm.add_task(Callback::make(&task1, 300000));
    tm.add_task(Callback::make(&task1, 300000));
    tm.add_task(Callback::make(&task1, 300000));
    tm.add_task(Callback::make(&task1, 1000));
    tm.change_max_threads(-2);
    mutex.lock();
    timespec ts;
    Thread::Cond::get_abs_time(ts, 5000);
    while (count < 4) {
      if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
    }
    mutex.unlock();
    // once count == 4, 'used_threads' should be 1 very soon thereafter
    while (tm.get_used_threads() != 1);
    g_assert_cmpuint(tm.get_max_threads(), ==, 1U);
    g_assert_cmpuint(tm.get_used_threads(), ==, 1U);
  }
}

static void test_fail() {
  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm;

    tm.add_task(to_unique(Callback::make(&task3)),
		to_unique(Callback::make(&fail_cb)));
    mutex.lock();
    timespec ts;
    Thread::Cond::get_abs_time(ts, 5000);
    while (!count) {
      if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
    }
    mutex.unlock();
  }

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm;

    tm.add_task(to_unique(Callback::make(&task4)),
		to_unique(Callback::make(&fail_cb)));
    mutex.lock();
    timespec ts;
    Thread::Cond::get_abs_time(ts, 5000);
    while (!count) {
      if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
    }
    mutex.unlock();
  }  

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm;

    tm.add_task([] () {task3();},
		[] () {fail_cb();});
    mutex.lock();
    timespec ts;
    Thread::Cond::get_abs_time(ts, 5000);
    while (!count) {
      if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
    }
    mutex.unlock();
  }  

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Thread::TaskManager tm;

    auto l1 = [] () {task4();};
    auto l2 = [] () {fail_cb();};
    tm.add_task(l1, l2);
    mutex.lock();
    timespec ts;
    Thread::Cond::get_abs_time(ts, 5000);
    while (!count) {
      if (cond.timed_wait(mutex, ts)) g_assert_not_reached();
    }
    mutex.unlock();
  }  
}

static void test_idle() {
  mutex.lock();
  count = 0;
  mutex.unlock();
  Thread::TaskManager tm(8, 1, 1);

  // this test will give a false fail if the system is so heavily
  // loaded that it takes more than 200 milliseconds for TaskManager's
  // condition variable to unblock (instead of the 1 millisecond given
  // as the constructor argument): however, this is almost
  // inconceivable
  tm.add_task(Callback::make(&task1, 1000));
  tm.add_task(Callback::make(&task1, 1000));
  tm.add_task(Callback::make(&task1, 1000));
  g_usleep(200000);
  g_assert_cmpuint(tm.get_tasks(), ==, 0U);
  g_assert_cmpuint(tm.get_used_threads(), ==, 1U);
}

static void test_task_result() {

  Thread::TaskManager tm;

  {
    Test t;
    // make some arguments temporaries and some named arguments
    int one = 1;
    auto res0 = tm.make_task_result(t, &Test::add0);
    auto res1 = tm.make_task_result(t, &Test::add1, one);
    auto res2 = tm.make_task_result(t, &Test::add2, one, 2);
    auto res3 = tm.make_task_result(t, &Test::add3, one, 2, 3);
    auto res4 = tm.make_task_result(t, &Test::add_fail1);
    auto res5 = tm.make_task_result([&t] () mutable {return t.add_fail2();});
    g_assert_cmpint(res0->get(), ==, 1);
    g_assert_cmpint(res1->get(), ==, 2);
    g_assert_cmpint(res2->get(), ==, 4);
    g_assert_cmpint(res3->get(), ==, 7);
    res4->get();
    res5->get();
    g_assert_cmpint(res1->get_error(), ==, 0);
    g_assert_cmpint(res4->get_error(), ==, -1);
    g_assert_cmpint(res5->get_error(), ==, -1);
  }

  {
    const ConstTest t;
    // make some arguments temporaries and some named arguments
    int one = 1;
    auto res0 = tm.make_task_result(t, &ConstTest::add0);
    auto res1 = tm.make_task_result(t, &ConstTest::add1, one);
    auto res2 = tm.make_task_result(t, &ConstTest::add2, one, 2);
    auto res3 = tm.make_task_result(t, &ConstTest::add3, one, 2, 3);
    auto res4 = tm.make_task_result([&t] () {return t.add_fail1();});
    auto res5 = tm.make_task_result(t, &ConstTest::add_fail2);
    g_assert_cmpint(res0->get(), ==, 1);
    g_assert_cmpint(res1->get(), ==, 2);
    g_assert_cmpint(res2->get(), ==, 4);
    g_assert_cmpint(res3->get(), ==, 7);
    res4->get();
    res5->get();
    g_assert_cmpint(res1->get_error(), ==, 0);
    g_assert_cmpint(res4->get_error(), ==, -1);
    g_assert_cmpint(res5->get_error(), ==, -1);
  }

  {
    // make some arguments temporaries and some named arguments
    int one = 1, four = 4;
    auto res0 = tm.make_task_result(&add0);
    auto res1 = tm.make_task_result(&add1, one);
    auto res2 = tm.make_task_result(&add2, one, 2);
    auto res3 = tm.make_task_result(&add3, one, 2, 3);
    auto res4 = tm.make_task_result(&add4, one, 2, 3, four);
    auto res5 = tm.make_task_result(&add_fail1);
    auto res6 = tm.make_task_result(&add_fail2);
    g_assert_cmpint(res0->get(), ==, 0);
    g_assert_cmpint(res1->get(), ==, 1);
    g_assert_cmpint(res2->get(), ==, 3);
    g_assert_cmpint(res3->get(), ==, 6);
    g_assert_cmpint(res4->get(), ==, 10);
    res5->get();
    res6->get();
    g_assert_cmpint(res1->get_error(), ==, 0);
    g_assert_cmpint(res5->get_error(), ==, -1);
    g_assert_cmpint(res6->get_error(), ==, -1);
  }

  {
    // make some arguments temporaries and some named arguments
    int four = 4;
    auto res1 = tm.make_task_result(std::bind(&add2, 3, four));
    auto f1 = std::bind(&add3, 3, four, 5);
    auto res2 = tm.make_task_result<int>(f1);
    auto res3 = tm.make_task_result(std::function<int()>(&add_fail1));
    std::function<int()> f2(&add_fail2);
    auto res4 = tm.make_task_result(f2);
    g_assert_cmpint(res1->get(), ==, 7);
    g_assert_cmpint(res2->get(), ==, 12);
    res3->get();
    res4->get();
    g_assert_cmpint(res1->get_error(), ==, 0);
    g_assert_cmpint(res3->get_error(), ==, -1);
    g_assert_cmpint(res4->get_error(), ==, -1);
  }
}

static void test_task_packaged() {

  Thread::TaskManager tm;

  // make some arguments temporaries and some named arguments
  int four = 4;
  int thrown = 0;
  std::future<int> res1 = tm.make_task_packaged(std::bind(&add2, 3, four));
  auto f1 = std::bind(&add3, 3, four, 5);
  auto res2 = tm.make_task_packaged(f1);
  auto res3 = tm.make_task_packaged(std::function<int()>(&add_fail1));
  auto f2 = [] () {g_usleep(1000); add_fail2();};
  auto res4 = tm.make_task_packaged(f2);
  g_assert_cmpint(res1.get(), ==, 7);
  g_assert_cmpint(res2.get(), ==, 12);
  try {
    res3.get();
  }
  catch (Thread::Exit&) {
    ++thrown;
  }
  try {
    res4.get();
  }
  catch (std::exception&) {
    ++thrown;
  }
  g_assert_cmpint(thrown, ==, 2);
}

static void test_task_when() {

  Thread::TaskManager tm;
  GMainLoop* loop = g_main_loop_new(0, true);

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Test t;

    // make some arguments temporaries and some named arguments
    int one = 1;
    tm.make_task_when(to_unique(Callback::make(&when, loop, 4U, 1)),
		      0, t, &Test::add0);
    tm.make_task_when(to_unique(Callback::make(&when, loop, 4U, 2)),
		      0, t, &Test::add1, one);
    tm.make_task_when(to_unique(Callback::make(&when, loop, 4U, 4)),
		      0, [&t, one] () -> int {return t.add2(one, 2);});
    tm.make_task_when([loop] (const int& i) {when(loop, 4U, 7, i);},
		      0, [&t, one] () -> int {return t.add3(one, 2, 3);});
    g_main_loop_run(loop);
    mutex.lock();
    g_assert_cmpuint(count, ==, 4U);
    mutex.unlock();
  }

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    const ConstTest t;

    // make some arguments temporaries and some named arguments
    int one = 1;
    tm.make_task_when(to_unique(Callback::make(&when, loop, 4U, 1)),
		      0, t, &ConstTest::add0);
    tm.make_task_when(to_unique(Callback::make(&when, loop, 4U, 2)),
		      0, t, &ConstTest::add1, one);
    tm.make_task_when(to_unique(Callback::make(&when, loop, 4U, 4)),
		      0, [&t, one] () -> int {return t.add2(one, 2);});
    tm.make_task_when([loop] (const int& i) {when(loop, 4U, 7, i);},
		      0, [&t, one] () -> int {return t.add3(one, 2, 3);});
    g_main_loop_run(loop);
    mutex.lock();
    g_assert_cmpuint(count, ==, 4U);
    mutex.unlock();
  }

  {
    mutex.lock();
    count = 0;
    mutex.unlock();

    // make some arguments temporaries and some named arguments
    int one = 1, four = 4;
    tm.make_task_when(to_unique(Callback::make(&when, loop, 5U, 0)),
		      0, &add0);
    tm.make_task_when(to_unique(Callback::make(&when, loop, 5U, 1)),
		      0, [one] () -> int {return add1(one);});
    tm.make_task_when([loop] (const int& i) {when(loop, 5U, 3, i);},
		      0, [one] () -> int {return add2(one, 2);});
    tm.make_task_when([loop] (const int& i) {return when(loop, 5U, 6, i);},
		      0, [one] () -> int {return add3(one, 2, 3);});
    tm.make_task_when(to_unique(Callback::make(&when, loop, 5U, 10)),
		      0, &add4, one, 2, 3, four);
    g_main_loop_run(loop);
    mutex.lock();
    g_assert_cmpuint(count, ==, 5U);
    mutex.unlock();
  }

  {
    mutex.lock();
    count = 0;
    mutex.unlock();

    // make some arguments temporaries and some named arguments
    int four = 4;
    tm.make_task_when<int>(to_unique(Callback::make(&when, loop, 2U, 7)),
			   0, std::bind(&add2, 3, four));
    std::function<int()> f(std::bind(&add3, 3, four, 5));
    tm.make_task_when(to_unique(Callback::make(&when, loop, 2U, 12)),
		      0, f);
    g_main_loop_run(loop);
    mutex.lock();
    g_assert_cmpuint(count, ==, 2U);
    mutex.unlock();
  }
}

static void test_task_compose() {

  Thread::TaskManager tm;
  GMainLoop* loop = g_main_loop_new(0, true);

  mutex.lock();
  count = 0;
  mutex.unlock();

  // make some arguments temporaries and some named arguments
  int four = 4;
  tm.make_task_compose<int>(std::bind(&add2, 3, four), 0,
			    to_unique(Callback::make(&when, loop, 3U, 7)));
  std::function<int()> f(std::bind(&add3, 3, four, 5));
  tm.make_task_compose(f, 0,
		       [loop] (const int& i) {when(loop, 3U, 12, i);});
  tm.make_task_compose([four] () -> int {return add3(four, 2, 3);},
		       0, [loop] (const int& i) {when(loop, 3U, 9, i);});
  g_main_loop_run(loop);
  mutex.lock();
  g_assert_cmpuint(count, ==, 3U);
  mutex.unlock();
}

static void test_task_when_full() {

  // TaskManager::make_test_when_full() is of necessity tested by
  // test_task_when().  All we need to do here is test the fail
  // callback and releasers

  // test fail callback
  Thread::TaskManager tm;
  GMainLoop* loop = g_main_loop_new(0, true);
  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Test t;

    tm.make_task_when_full(to_unique(Callback::make(&no_when)), 0,
			   to_unique(Callback::make(&fail, loop, 2U)),
			   &t.releaser,
			   G_PRIORITY_DEFAULT, 0, t, &Test::add_fail1);
    tm.make_task_when_full(to_unique(Callback::make(&no_when)), 0,
			   to_unique(Callback::make(&fail, loop, 2U)), 0,
			   G_PRIORITY_DEFAULT, 0, t, &Test::add_fail2);
    g_main_loop_run(loop);
    mutex.lock();
    g_assert_cmpuint(count, ==, 2U);
    mutex.unlock();
  }

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    const ConstTest t;

    tm.make_task_when_full(to_unique(Callback::make(&no_when)), 0,
			   to_unique(Callback::make(&fail, loop, 2U)), 0,
			   G_PRIORITY_DEFAULT, 0, t, &ConstTest::add_fail1);
    tm.make_task_when_full(to_unique(Callback::make(&no_when)), 0,
			   to_unique(Callback::make(&fail, loop, 2U)),
			   &t.releaser,
			   G_PRIORITY_DEFAULT, 0, t, &ConstTest::add_fail2);
    g_main_loop_run(loop);
    mutex.lock();
    g_assert_cmpuint(count, ==, 2U);
    mutex.unlock();
  }

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Test t;

    tm.make_task_when_full(to_unique(Callback::make(&no_when)), 0,
			   to_unique(Callback::make(&fail, loop, 2U)),
			   &t.releaser,
			   G_PRIORITY_DEFAULT, 0, &add_fail1);
    tm.make_task_when_full(to_unique(Callback::make(&no_when)), 0,
			   to_unique(Callback::make(&fail, loop, 2U)), 0,
			   G_PRIORITY_DEFAULT, 0, &add_fail2);
    g_main_loop_run(loop);
    mutex.lock();
    g_assert_cmpuint(count, ==, 2U);
    mutex.unlock();
  }

  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Test t;

    tm.make_task_when_full([] (const int& i) {no_when(i);}, 0,
			   [loop] () {fail(loop, 2U);},
			   &t.releaser,
			   G_PRIORITY_DEFAULT, 0, [&t]() {return t.add_fail1();});
    tm.make_task_when_full([] (const int& i) {no_when(i);}, 0,
			   [loop] () {fail(loop, 2U);}, 0,
			   G_PRIORITY_DEFAULT, 0, [&t]() {return t.add_fail2();});
    g_main_loop_run(loop);
    mutex.lock();
    g_assert_cmpuint(count, ==, 2U);
    mutex.unlock();
  }

  // more tests on releasers
  {
    mutex.lock();
    count = 0;
    mutex.unlock();
    Test t;
    const ConstTest ct;
    tm.make_task_when_full(to_unique(Callback::make(&when, loop, 4U, 2)),
			   &t.releaser,
			   to_unique(Callback::make(&no_fail)),
			   &t.releaser,
			   G_PRIORITY_DEFAULT, 0, t, &Test::add1, 1);
    tm.make_task_when_full(to_unique(Callback::make(&when, loop, 4U, 2)),
			   &t.releaser,
			   to_unique(Callback::make(&no_fail)),
			   &t.releaser,
			   G_PRIORITY_DEFAULT, 0, ct, &ConstTest::add1, 1);
    tm.make_task_when_full(to_unique(Callback::make(&when, loop, 4U, 1)),
			   &t.releaser,
			   to_unique(Callback::make(&no_fail)),
			   &t.releaser,
			   G_PRIORITY_DEFAULT, 0, &add1, 1);
    tm.make_task_when_full(to_unique(Callback::make(&when, loop, 4U, 1)),
			   &t.releaser,
			   to_unique(Callback::make(&no_fail)),
			   &t.releaser,
			   G_PRIORITY_DEFAULT, 0,
			   [] () {return add1(1);});
    g_main_loop_run(loop);
    mutex.lock();
    g_assert_cmpuint(count, ==, 4U);
    mutex.unlock();
  }

  {
    Test t;
    const ConstTest ct;
    {
      Releaser releaser;
      tm.make_task_when_full(to_unique(Callback::make(&no_when)),
			     &releaser,
			     to_unique(Callback::make(&no_fail)),
			     &releaser,
			     G_PRIORITY_DEFAULT, 0,
			     t, &Test::add0);
      tm.make_task_when_full(to_unique(Callback::make(&no_when)),
			     &releaser,
			     to_unique(Callback::make(&no_fail)),
			     &releaser,
			     G_PRIORITY_DEFAULT, 0,
			     t, &Test::add_fail1);

      tm.make_task_when_full(to_unique(Callback::make(&no_when)),
			     &releaser,
			     to_unique(Callback::make(&no_fail)),
			     &releaser,
			     G_PRIORITY_DEFAULT, 0,
			     ct, &ConstTest::add0);
      tm.make_task_when_full(to_unique(Callback::make(&no_when)),
			     &releaser,
			     to_unique(Callback::make(&no_fail)),
			     &releaser,
			     G_PRIORITY_DEFAULT, 0,
			     ct, &ConstTest::add_fail1);

      tm.make_task_when_full(to_unique(Callback::make(&no_when)),
			     &releaser,
			     to_unique(Callback::make(&no_fail)),
			     &releaser,
			     G_PRIORITY_DEFAULT, 0,
			     &add0);
      tm.make_task_when_full(to_unique(Callback::make(&no_when)),
			     &releaser,
			     to_unique(Callback::make(&no_fail)),
			     &releaser,
			     G_PRIORITY_DEFAULT, 0,
			     &add_fail1);

      tm.make_task_when_full(to_unique(Callback::make(&no_when)),
			     &releaser,
			     to_unique(Callback::make(&no_fail)),
			     &releaser,
			     G_PRIORITY_DEFAULT, 0,
			     [&t]() {return t.add0();});
      tm.make_task_when_full(to_unique(Callback::make(&no_when)),
			     &releaser,
			     to_unique(Callback::make(&no_fail)),
			     &releaser,
			     G_PRIORITY_DEFAULT, 0,
			     [&t]() {return t.add_fail1();});

      tm.add_task([loop]() -> void {
	  g_usleep(100000);
	  Callback::post([loop] () -> void {
	      g_main_loop_quit(loop);
	    });
	});
    }
    g_main_loop_run(loop);
  }
}

static void test_task_packaged_when() {

  Thread::TaskManager tm;
  GMainLoop* loop = g_main_loop_new(0, true);
  count = 0;

  tm.make_task_packaged_when(
    [loop] (std::future<int>& fut) -> void {
      g_assert_cmpint(fut.get(), ==, 7);
      mutex.lock();
      ++count;
      if (count == 2) g_main_loop_quit(loop);
      mutex.unlock();
    },
    0,
    G_PRIORITY_DEFAULT,
    0,
    [] () -> int {return 3 + 4;}
  );

  tm.make_task_packaged_when(
    [loop] (std::future<int>& fut) -> void {
      try {
	int i = fut.get();
	g_assert_not_reached();
	++i;
      }
      catch (std::exception&) {
	mutex.lock();
	++count;
	if (count == 2) g_main_loop_quit(loop);
	mutex.unlock();
      }
    },
    0,
    [] () -> int {throw std::exception(); return 0;}
  );
  g_main_loop_run(loop);
  mutex.lock();
  g_assert_cmpuint(count, ==, 2);
  mutex.unlock();

  {
    Releaser releaser;
    tm.make_task_packaged_when(
      [](std::future<int>&) -> void {g_assert_not_reached();},
      &releaser,
      G_PRIORITY_DEFAULT, 0,
      []() -> int {return 0;}
    );
    tm.add_task([loop]() -> void {
	g_usleep(100000);
	Callback::post([loop] () -> void {
	    g_main_loop_quit(loop);
	  });
      });
  }
  g_main_loop_run(loop);
}

static void test_task_packaged_compose() {

  Thread::TaskManager tm;
  GMainLoop* loop = g_main_loop_new(0, true);
  count = 0;

  tm.make_task_packaged_compose(
    [] () -> std::string {return "Test";},
    0,
    [loop] (std::future<std::string>& fut) -> void {
      g_assert(fut.get() == "Test");
      mutex.lock();
      ++count;
      if (count == 2) g_main_loop_quit(loop);
      mutex.unlock();
    }
  );

  tm.make_task_packaged_compose(
    [] () -> std::string {throw std::exception(); return "Test";},
    0,
    [loop] (std::future<std::string>& fut) -> void {
      try {
	std::string s = fut.get();
	g_assert_not_reached();
	s = "";
      }
      catch (std::exception&) {
	mutex.lock();
	++count;
	if (count == 2) g_main_loop_quit(loop);
	mutex.unlock();
      }
    }
  );
  g_main_loop_run(loop);
  mutex.lock();
  g_assert_cmpuint(count, ==, 2);
  mutex.unlock();
}

static void test_inc_handle() {

  Thread::TaskManager tm{1};
  g_assert_cmpuint(tm.get_used_threads(), ==, 0U);
  g_assert_cmpuint(tm.get_max_threads(), ==, 1U);
  
  {
    Thread::TaskManager::IncHandle h{tm};
    auto res1 = tm.make_task_result([] () -> int {g_usleep(100000); return 0;});
    auto res2 = tm.make_task_result([] () -> int {g_usleep(100000); return 0;});
    res1->get();
    res2->get();
    g_assert_cmpuint(tm.get_used_threads(), ==, 2U);
    g_assert_cmpuint(tm.get_max_threads(), ==, 2U);
  }
  g_assert_cmpuint(tm.get_max_threads(), ==, 1U);
}

} // extern "C"


int main (int argc, char* argv[]) {
#if !(GLIB_CHECK_VERSION(2,32,0))
  g_thread_init(0);
#endif
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/task_manager/used_threads", test_used_threads); 
  g_test_add_func("/task_manager/non_block", test_non_block); 
  g_test_add_func("/task_manager/max_threads", test_max_threads); 
  g_test_add_func("/task_manager/fail", test_fail); 
  g_test_add_func("/task_manager/idle", test_idle); 
  g_test_add_func("/task_manager/task_result", test_task_result); 
  g_test_add_func("/task_manager/task_packaged", test_task_packaged); 
  g_test_add_func("/task_manager/task_when", test_task_when); 
  g_test_add_func("/task_manager/task_compose", test_task_compose); 
  g_test_add_func("/task_manager/task_when_full", test_task_when_full); 
  g_test_add_func("/task_manager/task_packaged_when", test_task_packaged_when); 
  g_test_add_func("/task_manager/task_packaged_compose", test_task_packaged_compose); 
  g_test_add_func("/task_manager/inc_handle", test_inc_handle); 

  return g_test_run();
}
