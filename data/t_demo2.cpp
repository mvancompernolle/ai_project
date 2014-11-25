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
//  FILE     : t_demo2.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============

// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file demo.h
// #
// #########################################

#define	STRING_SIZE_1	5
#define	STRING_SIZE_2	25
#define	STRING_SIZE_3	125

// ----------------------------
void demo_13 (void)
{
string sstr1 (STRING_SIZE_1, 'a');
string sstr2 (STRING_SIZE_2, 'b');
string sstr3 (STRING_SIZE_3, 'c');

char cstr1 [STRING_SIZE_1 + 1];
char cstr2 [STRING_SIZE_2 + 1];
char cstr3 [STRING_SIZE_3 + 1];

  strcpy (cstr1, sstr1.c_str());
  strcpy (cstr2, sstr2.c_str());
  strcpy (cstr3, sstr3.c_str());


const string text_string_size ("string.size()");
const string text_strlen ("strlen");

  // ---------------------------------------------------------
  {
    TURN_ON_THE_CLOCK (text_string_size, sstr1.size()) 
    {
      sstr1.size();
    }
  }


  // ---------------------------------------------------------
  {
    TURN_ON_THE_CLOCK (text_string_size, sstr2.size()) 
    {
      sstr2.size();
    }
  }


  // ---------------------------------------------------------
  {
    TURN_ON_THE_CLOCK (text_string_size, sstr3.size()) 
    {
      sstr3.size();
    }
  }

  // ---------------------------------------------------------
  {
    TURN_ON_THE_CLOCK (text_strlen, strlen (cstr1)) 
    {
      strlen (cstr1);
    }
  }

  // ---------------------------------------------------------
  {
    TURN_ON_THE_CLOCK (text_strlen, strlen (cstr2)) 
    {
      strlen (cstr2);
    }
  }

//////////////////////
#if (defined UNIX_ENV)
//////////////////////
  // ---------------------------------------------------------
  {
    TURN_ON_THE_RUSAGE_USER_TIME (ulonglong, text_strlen, strlen (cstr2)) 
    {
      strlen (cstr2);
    }
  }

  {
    TURN_ON_THE_RUSAGE_SYSTEM_TIME (ulonglong, text_strlen, strlen (cstr2)) 
    {
      strlen (cstr2);
    }
  }

//////
#endif
//////

  // ---------------------------------------------------------
  {
    TURN_ON_THE_CLOCK (text_strlen, strlen (cstr3)) 
    {
      strlen (cstr3);
    }
  }


//////////////////////
#if (defined UNIX_ENV)
//////////////////////
  // ---------------------------------------------------------
  {
    TURN_ON_THE_RUSAGE_USER_TIME (ulonglong, text_strlen, strlen (cstr3)) 
    {
      strlen (cstr3);
    }
  }

  {
    TURN_ON_THE_RUSAGE_SYSTEM_TIME (ulonglong, text_strlen, strlen (cstr3)) 
    {
      strlen (cstr3);
    }
  }

//////
#endif
//////


} // demo_13


///////////////
// End-Of-File
///////////////

