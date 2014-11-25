// ==============================================================
//
//  Copyright (C) 2002-2004 Alex Vinokur.
//
//  ------------------------------------------------------------
//  This file is part of C/C++ Program Perfometer.
//
//  C/C++ Program Perfometer is free software; 
//  you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License,
//  or (at your option) any later version.
//
//  C/C++ Program Perfometer is distributed in the hope 
//  that it will be useful, but WITHOUT ANY WARRANTY; 
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with C/C++ Program Perfometer; 
//  if not, write to the Free Software Foundation, Inc., 
//  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ------------------------------------------------------------
// 
//  http://up.to/alexv
//
// ==============================================================


// ##############################################################
//
//  SOFTWARE : C/C++ Program Perfometer
//  FILE     : t_thread.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"

////////////////////////////////
#if (__CYGWIN32__ || __CYGWIN__)
////////////////////////////////

#include <pthread.h>

typedef double type_t;


type_t compute (type_t d_i)
{
type_t ret_value = 1.0;
#define TIMES 10

  for (int i = 0; i < TIMES; i++) ret_value *= d_i;

  return ret_value;
}

// ===============
struct data_t
{
  vector<type_t>& dv_;
  size_t          start_id_; 
  size_t          n_;
  
  data_t (
	vector<type_t>& dv_i, 
	size_t start_id_i, 
	size_t n_i
	)
	:
	dv_ (dv_i), 
	start_id_ (start_id_i),
	n_ (n_i)
  {}

	
};


// #########################################
// ------------------
static void run (data_t& data_i)
{
  for (size_t i = 0; i < data_i.n_; i++)
  {
    data_i.dv_[data_i.start_id_ + i] = compute (data_i.dv_[data_i.start_id_ + i]);
  }
}


// ------------------
static void do_pthreads (vector<type_t>& dv_i)
{
pthread_t thread1, thread2;
const size_t id1 = 0;
const size_t id2 = dv_i.size()/2;
const size_t n1  = id2;
const size_t n2  = dv_i.size() - id2;

  assert ((n1 + n2) == dv_i.size());

data_t data1 (dv_i, id1, n1);
data_t data2 (dv_i, id2, n2);

  if (pthread_create(
		&thread1, 
		NULL,
		reinterpret_cast<void * (*) (void *)> (run),
		reinterpret_cast<void*>(&data1)
		) != 0
		)
  {
    assert(0); 
  }


  if (pthread_create(
		&thread2, 
		NULL, 
		reinterpret_cast<void * (*) (void *)> (run),
		reinterpret_cast<void*>(&data2)
		) != 0
		)
  {
    assert(0); 
  }

  if (pthread_join(thread1, NULL) != 0) assert (0);
  if (pthread_join(thread2, NULL) != 0) assert (0);

}


// ------------------
static void do_ordinary (vector<type_t>& dv_i)
{
const size_t id = 0;
const size_t n  = dv_i.size();

data_t data (dv_i, id, n);

  run (data);

}


// #########################################

#define SIZE0	1000
#define SIZE1	10000
#define SIZE2	100000


#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))




// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void thread_action (void)
{
SET_SIZES;

const string htext__pthreads  ("pthreads");
const string htext__ordinary  ("ordinary");

  // -------------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const ulong cur_size (v_sizes[i]);
    vector<type_t> dv (cur_size);

    for (ulong i = 0; i < cur_size; i++) dv[i] = type_t (rand());

    vector<type_t> dv_pthreads (dv);

    {
      TURN_ON_DEFAULT_TIMER (htext__pthreads, cur_size) 
      {
        do_pthreads(dv_pthreads);
      }
    }

    vector<type_t> dv_ordinary (dv);

    {
      TURN_ON_DEFAULT_TIMER (htext__ordinary, cur_size) 
      {
        do_ordinary(dv_ordinary);
      }
    }

  }

} // thread_action


//////////////////////////////////////////
#endif // #if (__CYGWIN32__ || __CYGWIN__)
//////////////////////////////////////////


///////////////
// End-Of-File
///////////////

