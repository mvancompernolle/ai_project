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
//  FILE     : t_recurs.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
#include <numeric>
// ===============


// #########################################
// -------------------------------------------
#define SIZE0	10
#define SIZE1	100
#define SIZE2	1000
#define MAX_SIZE	SIZE2

#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))
// -------------------------------------------


// -------------------------------------------
static int get_sum_via_iteration (int a[], size_t n) 
{
int sum = 0;
  for (size_t i = 0; i < n; i++) sum += a[i];  

  return sum;
}

// -------------------------------------------
static int get_sum_via_recursion (int a[], size_t n) 
{
  if (n == 0) return 0;
  return (a[n - 1] + get_sum_via_recursion(a, n - 1));
}



// -------------------------------------------
static int get_sum_via_accumulate (int a[], size_t n) 
{
  return (accumulate (a, a + n, 0));
}


// -------------------------------------------
static int get_sum_via_iteration_index (const vector<int>& v) 
{
int sum = 0;
  for (size_t i = 0; i < v.size(); i++) sum += v[i];  

  return sum;
}


// -------------------------------------------
static int get_sum_via_iteration_iterator (const vector<int>& v) 
{
int sum = 0;
  for (vector<int>::const_iterator i = v.begin(); i != v.end(); i++) sum += *i;  

  return sum;
}



// -------------------------------------------
static int get_sum_via_accumulate (const vector<int>& v) 
{
  return (accumulate (v.begin(), v.end(), 0));
}


// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################




// =============================
void recursion_action (void)
{
SET_SIZES;

const string htext__array_iteration           ("array           iteration ");
const string htext__array_recursion           ("array           recursion ");
const string htext__array_accumulate          ("array           accumulate");
const string htext__vector_index_iteration    ("vector-index    iteration ");
const string htext__vector_iterator_iteration ("vector-iterator iteration ");
const string htext__vector_accumulate         ("vector          accumulate");


int arr[MAX_SIZE];
int sum;

  for (size_t i = 0; i < MAX_SIZE; i++) arr[i] = 2 * i;


  // -------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t cur_size (v_sizes[i]);
    const vector<int> vect (arr, arr + cur_size);


    // ---------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__array_iteration, cur_size) 
      {
        sum = get_sum_via_iteration (arr, cur_size);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__array_recursion, cur_size) 
      {
        sum = get_sum_via_recursion (arr, cur_size);
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__array_accumulate, cur_size) 
      {
        sum = get_sum_via_accumulate (arr, cur_size);
      }
    }


    // ---------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__vector_index_iteration, cur_size) 
      {
        sum = get_sum_via_iteration_index (vect);
      }
    }


    // ---------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__vector_iterator_iteration, cur_size) 
      {
        sum = get_sum_via_iteration_iterator (vect);
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__vector_accumulate, cur_size) 
      {
        sum = get_sum_via_accumulate (vect);
      }
    }


  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // recursion_action


///////////////
// End-Of-File
///////////////

