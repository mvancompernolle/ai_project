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
//  FILE     : t_iter.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============


// #########################################

#define SIZE0	100
#define SIZE1	1000
#define SIZE2	10000


#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))




// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void iterator_action (void)
{
SET_SIZES;

const string htext__vect_double_iter        ("vector<double>      iterator");
const string htext__vect_double_index       ("vector<double>      index   ");

const string htext__vect_string_iter        ("vector<string>      iterator");
const string htext__vect_string_index       ("vector<string>      index   ");

const string htext__map_long_double_iter    ("map<long,   double> iterator");
const string htext__map_double_double_iter  ("map<double, double> iterator");
const string htext__map_string_double_iter  ("map<string, double> iterator");

const string htext__map_long_string_iter    ("map<long,   string> iterator");
const string htext__map_double_string_iter  ("map<double, string> iterator");
const string htext__map_string_string_iter  ("map<string, string> iterator");


#define MAX_DATA_SIZE	1000
  // -------------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const ulong cur_size (v_sizes[i]);


    // --------------------------------
    vector<double> vect_double;
    vector<string> vect_string;

    map<long,   double>  map_long_double;
    map<double, double>  map_double_double;
    map<string, double>  map_string_double;

    map<long,   string>  map_long_string;
    map<double, string>  map_double_string;
    map<string, string>  map_string_string;


    // --------------------------------
    string tmp_str;
    for (ulong k = 0; k < cur_size; k++)
    {
      const int rand_val = rand();

      double tmp_double = double(rand_val);  
      tmp_str += char (rand_val);

      vect_double.push_back(tmp_double);
      vect_string.push_back(tmp_str);

      map_long_double[k]            = tmp_double;
      map_double_double[tmp_double] = tmp_double;
      map_string_double[tmp_str]    = tmp_double;

      map_long_string[k]            = tmp_str;
      map_double_string[tmp_double] = tmp_str;
      map_string_string[tmp_str]    = tmp_str;

    }

    assert (vect_double.size() == cur_size); 
    assert (vect_string.size() == cur_size); 

    assert (map_long_double.size()   == cur_size); 
    assert (map_double_double.size() == cur_size); 
    assert (map_string_double.size() == cur_size); 

    assert (map_long_string.size()   == cur_size); 
    assert (map_double_string.size() == cur_size); 
    assert (map_string_string.size() == cur_size); 


    // --------------------------------
    // --------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__vect_double_iter, cur_size) 
      {
        for (vector<double>::const_iterator pos_iter = vect_double.begin();
             pos_iter != vect_double.end();
	     pos_iter++
             ) 
        {
          *pos_iter;
        }
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__vect_double_index, cur_size) 
      {
        for (size_t id = 0; id < cur_size; id++)
        {
          vect_double[id];
        }
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__vect_string_iter, cur_size) 
      {
        for (vector<string>::const_iterator pos_iter = vect_string.begin();
             pos_iter != vect_string.end();
	     pos_iter++
             ) 
        {
          *pos_iter;
        }
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__vect_string_index, cur_size) 
      {
        for (size_t id = 0; id < cur_size; id++)
        {
          vect_string[id];
        }
      }
    }


    // --------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__map_long_double_iter, cur_size) 
      {
        for (map<long, double>::const_iterator pos_iter = map_long_double.begin();
             pos_iter != map_long_double.end();
	     pos_iter++
             ) 
        {
          pos_iter->second;
        }
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__map_double_double_iter, cur_size) 
      {
        for (map<double, double>::const_iterator pos_iter = map_double_double.begin();
             pos_iter != map_double_double.end();
	     pos_iter++
             ) 
        {
          pos_iter->second;
        }
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__map_string_double_iter, cur_size) 
      {
        for (map<string, double>::const_iterator pos_iter = map_string_double.begin();
             pos_iter != map_string_double.end();
	     pos_iter++
             ) 
        {
          pos_iter->second;
        }
      }
    }



    // --------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__map_long_string_iter, cur_size) 
      {
        for (map<long, string>::const_iterator pos_iter = map_long_string.begin();
             pos_iter != map_long_string.end();
	     pos_iter++
             ) 
        {
          pos_iter->second;
        }
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__map_double_string_iter, cur_size) 
      {
        for (map<double, string>::const_iterator pos_iter = map_double_string.begin();
             pos_iter != map_double_string.end();
	     pos_iter++
             ) 
        {
          pos_iter->second;
        }
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__map_string_string_iter, cur_size) 
      {
        for (map<string, string>::const_iterator pos_iter = map_string_string.begin();
             pos_iter != map_string_string.end();
	     pos_iter++
             ) 
        {
          pos_iter->second;
        }
      }
    }


  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // iterator_action


///////////////
// End-Of-File
///////////////

