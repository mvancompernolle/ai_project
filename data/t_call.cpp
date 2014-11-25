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
//  FILE     : t_call.cpp
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
#define CSTR0	"0123456789"
#define CSTR1	CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0
#define CSTR2	CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1

#define SET_STRS  \
	const char*          a_cstrs[] = {CSTR0, CSTR1, CSTR2}; \
	const vector<string> v_strs (a_cstrs, a_cstrs + sizeof(a_cstrs)/sizeof(*a_cstrs))
// -------------------------------------------


// ##########################################
template <typename T>
static T templated_foo (const T&) { T t; return t; }

static int ordinary_foo (int) { int t; return t; }

static string ordinary_foo (const string&) { string t; return t; }


// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void calling_functions (void)
{
SET_STRS;

const string htext__templated_int     ("templated int   ");
const string htext__templated_string  ("templated string");
const string htext__ordinary_int      ("ordinary  int   ");
const string htext__ordinary_string   ("ordinary  string");

  // -------------------------------

int int_value = 12345;

  {
    TURN_ON_DEFAULT_TIMER (htext__templated_int, "No") 
    {
      templated_foo (int_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__ordinary_int, "No") 
    {
      ordinary_foo (int_value);
    }
  }


  for (size_t i = 0; i < v_strs.size(); i++)
  {
    string str			= v_strs[i].c_str();
    const size_t str_size	= str.size();

    {
      TURN_ON_DEFAULT_TIMER (htext__templated_string, str_size) 
      {
        templated_foo (str);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__ordinary_string, str_size) 
      {
        ordinary_foo (str);
      }
    }


  } // for (size_t i = 0; i < v_strs.size(); i++)


} // calling_functions


///////////////
// End-Of-File
///////////////

