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
//  FILE     : t_access.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
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
void access_action (void)
{
SET_SIZES;

#define DUMMY_CHAR1 123
#define DUMMY_CHAR2 125

const string htext__access_delta               ("delta              ");

const string htext__access_array_operator      ("array  - operator[]");
const string htext__access_array_pointer       ("array  - pointer   ");

const string htext__access_vector_operator     ("vector - operator[]");
const string htext__access_vector_iterator     ("vector - iterator  ");
const string htext__access_vector_pointer      ("vector - pointer   ");
const string htext__access_vector_at_method    ("vector - at method ");

const string htext__access_string_operator     ("string - operator[]");
const string htext__access_string_iterator     ("string - iterator  ");
const string htext__access_string_pointer      ("string - pointer   ");
const string htext__access_string_at_method    ("string - at method ");


  // -------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t cur_size (v_sizes[i]);
    const size_t median_value = cur_size/2;

    char* char_array = (char*) malloc (cur_size * sizeof (char));
    for (size_t j = 0; j < cur_size; j++) char_array[j] = DUMMY_CHAR1;
    char_array[median_value] = DUMMY_CHAR2;

    vector<char> char_vector (char_array, char_array + cur_size);
    string char_string (cur_size, DUMMY_CHAR1);
    char_string[median_value] = DUMMY_CHAR2;

    assert (median_value < cur_size);
    assert (median_value < char_vector.size());
    assert (median_value < char_string.size());


char ch = 0;
    // ---------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__access_delta, cur_size) 
      {
        ch += repetition_no;
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;


    {
      TURN_ON_DEFAULT_TIMER (htext__access_array_operator, cur_size) 
      {
        ch += (char_array [median_value] + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;


const char* array_ptr = &char_array[0];
    {
      TURN_ON_DEFAULT_TIMER (htext__access_array_pointer, cur_size) 
      {
        ch += (*(array_ptr + median_value) + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;



    {
      TURN_ON_DEFAULT_TIMER (htext__access_vector_operator, cur_size) 
      {
        ch += (char_vector [median_value] + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;


const vector<char>::const_iterator vector_iter (char_vector.begin());
    {
      TURN_ON_DEFAULT_TIMER (htext__access_vector_iterator, cur_size) 
      {
        ch += (*(vector_iter + median_value) + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;


const char* vector_ptr = &char_vector[0];
    {
      TURN_ON_DEFAULT_TIMER (htext__access_vector_pointer, cur_size) 
      {
        ch += (*(vector_ptr + median_value) + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;


    {
      TURN_ON_DEFAULT_TIMER (htext__access_vector_at_method, cur_size) 
      {
        ch += (char_vector.at (median_value) + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;



    {
      TURN_ON_DEFAULT_TIMER (htext__access_string_operator, cur_size) 
      {
        ch += (char_string [median_value] + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;


const string::const_iterator string_iter (char_string.begin());
    {
      TURN_ON_DEFAULT_TIMER (htext__access_string_iterator, cur_size) 
      {
        ch += (*(string_iter + median_value) + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;


const char* string_ptr = &char_string[0];
    {
      TURN_ON_DEFAULT_TIMER (htext__access_string_pointer, cur_size) 
      {
        ch += (*(string_ptr + median_value) + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;


    {
      TURN_ON_DEFAULT_TIMER (htext__access_string_at_method, cur_size) 
      {
        ch += (char_string.at (median_value) + repetition_no);
      }
    }
    // assert (ch == DUMMY_CHAR2);
    ch = 0;

  } // for (size_t i = 0; i < v_sizes.size(); i++)

} // access_action


///////////////
// End-Of-File
///////////////

