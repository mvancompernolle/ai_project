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
//  FILE     : t_stub.cpp
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
#define SIZE0	20
#define SIZE1	200
#define SIZE2	2000

#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))
// -------------------------------------------


// -------------------------------------------
#define CSTR0	"0123456789"
#define CSTR1	CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0
#define CSTR2	CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1

#define SET_STRS  \
	const char*          a_cstrs[] = {CSTR0, CSTR1, CSTR2}; \
	const vector<string> v_strs (a_cstrs, a_cstrs + sizeof(a_cstrs)/sizeof(*a_cstrs))

#define SET_STRS_AND_LEN     SET_STRS; \
	const size_t         max_str_len (v_strs.back().size())

// -------------------------------------------


// #########################################
// -----------
#define return_t	void	// sample
#define arg_t1		int	// sample
#define arg_t2		string	// sample


// -----------
static return_t stub_foo1__size_independent (arg_t1 /* arg_i; */) 
{ 
  // Stuff
  // Do something
}

// -----------
static return_t stub_foo2__size_independent (arg_t1 /* arg_i; */) 
{ 
  // Stuff
  // Do something
}

// -----------
static return_t stub_foo3__size_dependent (arg_t1 /* arg_i; */) 
{ 
  // Stuff
  // Do something
}

// -----------
static return_t stub_foo4__size_dependent (arg_t1 /* arg_i; */) 
{ 
  // Stuff
  // Do something
}


// -----------
static return_t stub_foo5__size_dependent (arg_t2 /* arg_i; */) 
{ 
  // Stuff
  // Do something
}


// -----------
static return_t stub_foo6__size_dependent (arg_t2 /* arg_i; */) 
{ 
  // Stuff
  // Do something
}

// -----------
static return_t stub_foo7__size_independent (arg_t1 /* arg_i; */) 
{ 
  // Stuff
  // Do something
}

// -----------
static return_t stub_foo8__size_dependent (arg_t1 /* arg_i; */) 
{ 
  // Stuff
  // Do something
}


// -----------
static return_t stub_foo9__size_dependent (arg_t2 /* arg_i; */) 
{ 
  // Stuff
  // Do something
}



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void stub_action1 (void)
{
SET_SIZES;
SET_STRS;

const string htext__tested_foo1  ("foo1 description");
const string htext__tested_foo2  ("foo2 description");
const string htext__tested_foo3  ("foo3 description");
const string htext__tested_foo4  ("foo4 description");
const string htext__tested_foo5  ("foo5 description");
const string htext__tested_foo6  ("foo6 description");

  // -------------------------------
const arg_t1	 val_a = 12345; 
  {
    TURN_ON_DEFAULT_TIMER (htext__tested_foo1, "No") 
    {
      stub_foo1__size_independent (val_a);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__tested_foo2, "No") 
    {
      stub_foo2__size_independent (val_a);
    }
  }


  // -------------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t	cur_size (v_sizes[i]);
    const arg_t1	val_b = cur_size/2; 	// sample

    {
      TURN_ON_DEFAULT_TIMER (htext__tested_foo3, cur_size) 
      {
        stub_foo3__size_dependent (val_b);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__tested_foo4, cur_size) 
      {
        stub_foo4__size_dependent (val_b);
      }
    }

  } // for (size_t i = 0; i < v_sizes.size(); i++)


  // -------------------------------------
  for (size_t i = 0; i < v_strs.size(); i++) 
  {
    string str			(v_strs[i]);
    const size_t str_size	= str.size();

    {
      TURN_ON_DEFAULT_TIMER (htext__tested_foo5, str_size) 
      {
        stub_foo5__size_dependent (str);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__tested_foo6, str_size) 
      {
        stub_foo6__size_dependent (str);
      }
    }


  } // for (size_t i = 0; i < v_strs.size(); i++)


} // stub_action1


// =============================
void stub_action2 (void)
{
SET_SIZES;
SET_STRS;

const string htext__tested_foo7  ("foo7 description");
const string htext__tested_foo8  ("foo8 description");
const string htext__tested_foo9  ("foo9 description");

  // -------------------------------
const arg_t1	 val_a = 12345; 
  {
    TURN_ON_DEFAULT_TIMER (htext__tested_foo7, "No") 
    {
      stub_foo7__size_independent (val_a);
    }
  }

  // -------------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t	cur_size (v_sizes[i]);
    const arg_t1	val_b = cur_size/2; 	// sample

    {
      TURN_ON_DEFAULT_TIMER (htext__tested_foo8, cur_size) 
      {
        stub_foo8__size_dependent (val_b);
      }
    }

  } // for (size_t i = 0; i < v_sizes.size(); i++)


  // -------------------------------------
  for (size_t i = 0; i < v_strs.size(); i++) 
  {
    string str			(v_strs[i]);
    const size_t str_size	= str.size();

    {
      TURN_ON_DEFAULT_TIMER (htext__tested_foo9, str_size) 
      {
        stub_foo9__size_dependent (str);
      }
    }

  } // for (size_t i = 0; i < v_strs.size(); i++)


} // stub_action2



///////////////
// End-Of-File
///////////////

