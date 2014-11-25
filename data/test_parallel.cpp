/* Copyright (C) 2013 Chris Vine

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
#include <list>
#include <numeric>
#include <iterator>
#include <utility>
#include <tuple>

#include <c++-gtk-utils/task_manager.h>
#include <c++-gtk-utils/parallel.h>

using namespace Cgu;

extern "C" {
static void test_for_each() {
  // use a list in case the compiler does not yet correctly implement
  // the C++11 memory model for arrays/vectors and other contiguous
  // storage, as we mutate below
  std::list<int> l{1, 2, 3, 4, 5};
  Thread::TaskManager tm;

  Thread::parallel_for_each(tm,
			    l.begin(),
			    l.end(),
			    [] (int& elt) {elt *= 2;});
  int res = std::accumulate(l.begin(), l.end(), 0);

  g_assert_cmpint(res, ==, 30);

  int thrown = 0;

  try {
    Thread::parallel_for_each(tm,
			      l.begin(),
			      l.end(),
			      [] (int elt) {throw bool();});
  }
  catch (Thread::ParallelError&) {
    thrown = 1;
  }
  
  g_assert_cmpint(thrown, ==, 1);
}

static void test_for_each_partial() {
  // use a list in case the compiler does not yet correctly implement
  // the C++11 memory model for arrays/vectors and other contiguous
  // storage, as we mutate below
  std::list<int> l{1, 2, 3, 4, 5};
  std::list<int>::iterator source_chunk = l.begin();
  std::advance(source_chunk, 3);
  Thread::TaskManager tm;

  auto dest_chunk1 =
    Thread::parallel_for_each_partial(tm,
				      l.begin(),
				      source_chunk,
				      -1,
				      [] (int& elt) {elt *= 2;});
  g_assert(dest_chunk1 == source_chunk);
  Thread::parallel_for_each_partial(tm,
				    dest_chunk1,
				    l.end(),
				    -1,
				    [] (int& elt) {elt *= 2;});
  int res = std::accumulate(l.begin(), l.end(), 0);

  g_assert_cmpint(res, ==, 30);

  auto dest_chunk2 =
    Thread::parallel_for_each_partial(tm,
				      l.begin(),
				      l.end(),
				      3,
				      [] (int& elt) {elt *= 2;});
  g_assert(dest_chunk2 != l.end());
  Thread::parallel_for_each_partial(tm,
				    dest_chunk2,
				    l.end(),
				    3,
				    [] (int& elt) {elt *= 2;});
  res = std::accumulate(l.begin(), l.end(), 0);

  g_assert_cmpint(res, ==, 60);

  int thrown = 0;

  try {
    Thread::parallel_for_each_partial(tm,
				      l.begin(),
				      source_chunk,
				      -1,
				      [] (int elt) {throw bool();});
  }
  catch (Thread::ParallelError&) {
    thrown = 1;
  }
  
  g_assert_cmpint(thrown, ==, 1);
}

static void test_transform() {
  std::vector<int> v1{1, 2, 3, 4, 5};
  Thread::TaskManager tm;

  Thread::parallel_transform(tm,
			     v1.begin(),
			     v1.end(),
			     v1.begin(),
			     [] (int elt) {return elt * 2;});
  int res = std::accumulate(v1.begin(), v1.end(), 0);

  g_assert_cmpint(res, ==, 30);

  std::vector<int> v2{10, 20, 30, 40, 50};
  std::vector<int> v3;

  Thread::parallel_transform(tm,
			     v1.begin(),
			     v1.end(),
			     v2.begin(),
			     std::back_inserter(v3),
			     [] (const int& elt1, const int& elt2) {return elt1 + elt2;});
  res = std::accumulate(v3.begin(), v3.end(), 0);

  g_assert_cmpint(res, ==, 180);

  int thrown = 0;

  try {
    Thread::parallel_transform(tm,
			       v1.begin(),
			       v1.end(),
			       v1.begin(),
			       [] (int elt) -> int {
				 throw bool();
				 return elt * 2;
			       });
  }
  catch (Thread::ParallelError&) {
    thrown = 1;
  }
  
  g_assert_cmpint(thrown, ==, 1);

  thrown = 0;
  v3.clear();

  try {
    Thread::parallel_transform(tm,
			       v1.begin(),
			       v1.end(),
			       v2.begin(),
			       std::back_inserter(v3),
			       [] (const int& elt1, const int& elt2) -> int {
				 throw bool();
				 return elt1 + elt2;
			       });
  }
  catch (Thread::ParallelError&) {
    thrown = 1;
  }
  
  g_assert_cmpint(thrown, ==, 1);
  g_assert_cmpuint(v3.size(), ==, 0);
}

static void test_transform_partial() {
  std::vector<int> v1{1, 2, 3, 4, 5};
  std::vector<int>::iterator source_chunk = v1.begin() + 3;
  Thread::TaskManager tm;

  auto dest_chunk1 =
    Thread::parallel_transform_partial(tm,
				       v1.begin(),
				       source_chunk,
				       v1.begin(),
				       -1,
				       [] (int elt) {return elt * 2;});
  g_assert(dest_chunk1.first == source_chunk);
  Thread::parallel_transform_partial(tm,
				     dest_chunk1.first,
				     v1.end(),
				     dest_chunk1.second,
				     -1,
				     [] (int elt) {return elt * 2;});
  int res = std::accumulate(v1.begin(), v1.end(), 0);

  g_assert_cmpint(res, ==, 30);

  auto dest_chunk2 =
    Thread::parallel_transform_partial(tm,
				       v1.begin(),
				       v1.end(),
				       v1.begin(),
				       3,
				       [] (int elt) {return elt * 2;});
  g_assert(dest_chunk2.first != v1.end());
  Thread::parallel_transform_partial(tm,
				     dest_chunk2.first,
				     v1.end(),
				     dest_chunk2.second,
				     3,
				     [] (int elt) {return elt * 2;});
  res = std::accumulate(v1.begin(), v1.end(), 0);

  g_assert_cmpint(res, ==, 60);

  std::vector<int> v2{10, 20, 30, 40, 50};
  std::vector<int> v3;
  source_chunk = v1.begin() + 3;

  auto chunk_tuple1 =
    Thread::parallel_transform_partial(tm,
				       v1.begin(),
				       source_chunk,
				       v2.begin(),
				       std::back_inserter(v3),
				       -1,
				       [] (const int& elt1, const int& elt2) {return elt1 + elt2;});
  g_assert(std::get<0>(chunk_tuple1) == source_chunk);
  Thread::parallel_transform_partial(tm,
				     std::get<0>(chunk_tuple1),
				     v1.end(),
				     std::get<1>(chunk_tuple1), // end of previous second source
				     std::get<2>(chunk_tuple1), // end of previous destination
				     -1,
				     [] (const int& elt1, const int& elt2) {return elt1 + elt2;});
  res = std::accumulate(v3.begin(), v3.end(), 0);

  g_assert_cmpint(res, ==, 210);

  v3.clear();
  auto chunk_tuple2 =
    Thread::parallel_transform_partial(tm,
				       v1.begin(),
				       v1.end(),
				       v2.begin(),
				       std::back_inserter(v3),
				       3,
				       [] (int& elt1, int& elt2) {return elt1 + elt2;});
  g_assert(std::get<0>(chunk_tuple2) != v1.end());
  Thread::parallel_transform_partial(tm,
				     std::get<0>(chunk_tuple2),
				     v1.end(),
				     std::get<1>(chunk_tuple2), // end of previous second source
				     std::get<2>(chunk_tuple2), // end of previous destination
				     3,
				     [] (int& elt1, int& elt2) {return elt1 + elt2;});
  res = std::accumulate(v3.begin(), v3.end(), 0);

  g_assert_cmpint(res, ==, 210);

  int thrown = 0;
  source_chunk = v1.begin() + 3;

  try {
    Thread::parallel_transform_partial(tm,
				       v1.begin(),
				       source_chunk,
				       v1.begin(),
				       -1,
				       [] (int elt) -> int {
					 throw bool();
					 return elt * 2;
				       });
  }
  catch (Thread::ParallelError&) {
    thrown = 1;
  }
  
  g_assert_cmpint(thrown, ==, 1);

  thrown = 0;
  source_chunk = v1.begin() + 3;
  v3.clear();

  try {
    Thread::parallel_transform_partial(tm,
				       v1.begin(),
				       source_chunk,
				       v2.begin(),
				       std::back_inserter(v3),
				       3,
				       [] (const int& elt1, const int& elt2) -> int {
					 throw bool();
					 return elt1 + elt2;
				       });
  }
  catch (Thread::ParallelError&) {
    thrown = 1;
  }
  
  g_assert_cmpint(thrown, ==, 1);
  g_assert_cmpuint(v3.size(), ==, 0);
}
} // extern "C"


int main (int argc, char* argv[]) {
#if !(GLIB_CHECK_VERSION(2,32,0))
  g_thread_init(0);
#endif
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/parallel/for_each", test_for_each);
  g_test_add_func("/parallel/for_each_partial", test_for_each_partial);
  g_test_add_func("/parallel/transform", test_transform);
  g_test_add_func("/parallel/transform_partial", test_transform_partial);

  return g_test_run();
}
