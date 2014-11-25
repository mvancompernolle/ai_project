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
//  FILE     : t_find.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
#include <list>
// ===============


// #########################################
// -------------------------------------------
#define SIZE0	10
#define SIZE1	100
#define SIZE2	1000

#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))

// -------------------------------------------


// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################




// =============================
void find_action (void)
{
SET_SIZES;

const string dummy_str ("ABCDEF");

const string htext__find_algorithm_vector      ("find algorithm - vector");
const string htext__find_algorithm_list        ("find algorithm - list  ");
const string htext__find_algorithm_set         ("find algorithm - set   ");
const string htext__find_method_set            ("find method    - set   ");
const string htext__find_method_map            ("find method    - map   ");

  // -------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t cur_size (v_sizes[i]);
    const int median_value = static_cast<int>(cur_size/2);

    vector<int>      int_vector;
    list<int>        int_list;
    set<int>         int_set;
    map<int, string> int_map;


    for (size_t j = 0; j < cur_size; j++)
    {
      int_vector.push_back(j);
      int_list.push_back(j);
      int_set.insert(j);
      int_map[j] = dummy_str;
    }

    assert (int_vector.size() == cur_size);
    assert (int_list.size()   == cur_size);
    assert (int_set.size()    == cur_size);
    assert (int_map.size()    == cur_size);


    // ---------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__find_algorithm_vector, cur_size) 
      {
        find (int_vector.begin(), int_vector.end(), median_value);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__find_algorithm_list, cur_size) 
      {
        find (int_list.begin(), int_list.end(), median_value);
      }
    }
  
    {
      TURN_ON_DEFAULT_TIMER (htext__find_algorithm_set, cur_size) 
      {
        find (int_set.begin(), int_set.end(), median_value);
      }
    }
  
    {
      TURN_ON_DEFAULT_TIMER (htext__find_method_set, cur_size) 
      {
        int_set.find(median_value);
      }
    }
  
    {
      TURN_ON_DEFAULT_TIMER (htext__find_method_map, cur_size) 
      {
        int_map.find(median_value);
      }
    }


  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // find_action


///////////////
// End-Of-File
///////////////

