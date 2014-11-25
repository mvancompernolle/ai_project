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

#include <math.h>
#include <vector>
#include <numeric>

#include <c++-gtk-utils/async_result.h>
#include <c++-gtk-utils/async_queue.h>
#include <c++-gtk-utils/shared_ptr.h>
#include <c++-gtk-utils/thread.h>
#include <c++-gtk-utils/callback.h>
#include <c++-gtk-utils/cgu_config.h>

using namespace Cgu;

class Calcs {
  AsyncQueueDispatch<Callback::SafeFunctor> jobs;
  Thread::JoinableHandle t;

  void do_jobs() {
    for (;;) {
      Callback::SafeFunctor job;
      jobs.move_pop_dispatch(job);
      job();
    }
  }

  static void mean_impl(const std::vector<double>& nums,
			const SharedLockPtr<AsyncResult<double>>& res) {
    if (nums.empty()) res->set(0.0);
    else res->set(std::accumulate(nums.begin(), nums.end(), 0.0)/nums.size());
  }

  static void all_done() {throw Thread::Exit();}

public:

  SharedLockPtr<AsyncResult<double>> mean(const std::vector<double>& nums) {
    SharedLockPtr<AsyncResult<double>> res(new AsyncResult<double>);
    jobs.emplace(Callback::make_ref(&mean_impl, nums, res));
    return res;
  }

  Calcs() {
#ifdef CGU_USE_AUTO_PTR
    Thread::JoinableHandle temp(Thread::Thread::start(Callback::make(*this, &Calcs::do_jobs), true), 
                                Thread::JoinableHandle::join_on_exit);
    if (!temp.is_managing()) throw "Thread start error";
    t = temp;
#else
    t = Thread::JoinableHandle(Thread::Thread::start(Callback::make(*this, &Calcs::do_jobs), true), 
			       Thread::JoinableHandle::join_on_exit);
    if (!t.is_managing()) throw "Thread start error";
#endif
  }
  ~Calcs() {
    jobs.emplace(Callback::make(&all_done));
    t.join();
  }
};

struct Movable {
  int a;
  Movable& operator=(Movable&& m) {a = 10; return *this;}
  Movable& operator=(const Movable& m) {a = 5; return *this;}
  Movable(Movable&&): a(10) {}
  Movable(const Movable&): a(5) {}
  Movable(): a(0) {}
};

Movable get_movable() {
  static Movable m;
  return m;
}

extern "C" {
static void test_async_result() {
  Calcs calcs;

  std::vector<double> vec1{1, 2, 8, 0};
  std::vector<double> vec2{101, 53.7, 87, 1.2};

  auto res1 = calcs.mean(vec1);
  auto res2 = calcs.mean(vec2);

  double sync1 = std::accumulate(vec1.begin(), vec1.end(), 0.0)/vec1.size();
  double sync2 = std::accumulate(vec2.begin(), vec2.end(), 0.0)/vec2.size();

  g_assert(fabs(res1->get() - sync1) < 0.001);
  g_assert(fabs(res2->get() - sync2) < 0.001);

  AsyncResult<Movable> res3;
  res3.set(get_movable());
  Movable m{res3.move_get()};
  g_assert_cmpint(m.a, ==, 10);
}
} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/async_result/result", test_async_result); 

  return g_test_run();
}
