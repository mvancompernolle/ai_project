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
//  FILE     : t_for.cpp
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

typedef unsigned int uint;

// -------------------------------------------
// static uint foo (uint val_i) { return (val_i / 2); }
static void foo (uint val_i) { val_i / 2; }


// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################




// =============================
void for_action (void)
{
SET_SIZES;

const string htext__for_loop_vector     ("for-loop - vector");
const string htext__for_loop_string     ("for-loop - string");
const string htext__for_loop_list       ("for-loop - list  ");
const string htext__for_loop_set        ("for-loop - set   ");

const string htext__for_each_vector     ("for_each - vector");
const string htext__for_each_string     ("for_each - string");
const string htext__for_each_list       ("for_each - list  ");
const string htext__for_each_set        ("for_each - set   ");


  // -------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t cur_size (v_sizes[i]);

    vector<uint>       uint_vector;
    string             char_string;
    list<uint>         uint_list;
    set<uint>          uint_set;

    vector<uint>::iterator        iter_vector;
    string::iterator              iter_string;
    list<uint>::iterator          iter_list;
    set<uint>::iterator           iter_set;


    for (uint j = 0; j < cur_size; j++)
    {
      uint_vector.push_back(j);
      char_string += j;
      uint_list.push_back(j);
      uint_set.insert(j);
    }					

    assert (uint_vector.size() == cur_size);
    assert (char_string.size() == cur_size);
    assert (uint_list.size()   == cur_size);
    assert (uint_set.size()    == cur_size);


    // ---------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__for_loop_vector, cur_size) 
      {
        for (iter_vector = uint_vector.begin(); 
             iter_vector != uint_vector.end();
             iter_vector++)
        {
          foo (*iter_vector);
        }
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__for_loop_string, cur_size) 
      {
        for (iter_string = char_string.begin(); 
             iter_string != char_string.end();
             iter_string++)
        {
          foo (*iter_string);
        }
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__for_loop_list, cur_size) 
      {
        for (iter_list = uint_list.begin(); 
             iter_list != uint_list.end();
             iter_list++)
        {
          foo (*iter_list);
        }
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__for_loop_set, cur_size) 
      {
        for (iter_set = uint_set.begin(); 
             iter_set != uint_set.end();
             iter_set++)
        {
          foo (*iter_set);
        }
      }
    }



    {
      TURN_ON_DEFAULT_TIMER (htext__for_each_vector, cur_size) 
      {
        for_each (uint_vector.begin(), uint_vector.end(), foo);
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__for_each_string, cur_size) 
      {
        for_each (char_string.begin(), char_string.end(), foo);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__for_each_list, cur_size) 
      {
        for_each (uint_list.begin(), uint_list.end(), foo);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__for_each_set, cur_size) 
      {
        for_each (uint_set.begin(), uint_set.end(), foo);
      }
    }


  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // for_action


///////////////
// End-Of-File
///////////////

