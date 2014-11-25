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
//  FILE     : t_prim.cpp
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
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void primitive_action (void)
{
#define	VALUE1 1
int val = rand();
int sink;

const string htext__do_nothing             ("do nothing ");
const string htext__prefix_plus_plus       ("++x        ");
const string htext__postfix_plus_plus      ("x++        ");
const string htext__prefix_minus_minus     ("--x        ");
const string htext__postfix_minus_minus    ("x--        ");
const string htext__plus_assign            ("x += 1     ");
const string htext__minus_assign           ("x -= 1     ");
const string htext__assign_plus            ("x = x + 1  ");
const string htext__assign_minus           ("x = x - 1  ");
const string htext__left_shift_assign      ("x <<= 1    ");
const string htext__right_shift_assign     ("x >>= 1    ");
const string htext__assign_left_shift      ("x = x << 1 ");
const string htext__assign_right_shift     ("x = x >> 1 ");
const string htext__bitwise_AND_assign     ("x &= 1     ");
const string htext__bitwise_OR_assign      ("x |= 1     ");
const string htext__bitwise_XOR_assign     ("x ^= 1     ");
const string htext__assign_bitwise_AND     ("x = x & 1  ");
const string htext__assign_bitwise_OR      ("x = x | 1  ");
const string htext__assign_bitwise_XOR     ("x = x ^ 1  ");
const string htext__sizeof_type            ("sizeof(int)");
const string htext__sizeof_value           ("sizeof(x)  ");

  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext__do_nothing, "No") 
    {
      // Do nothing
    }
  }

  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext__do_nothing, "No") 
    {
      // Do nothing
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__prefix_plus_plus, "No") 
    {
      ++val;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__postfix_plus_plus, "No") 
    {
      val++;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__prefix_minus_minus, "No") 
    {
      --val;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__postfix_minus_minus, "No") 
    {
      val--;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__plus_assign, "No") 
    {
      val += VALUE1;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__minus_assign, "No") 
    {
      val -= VALUE1;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__assign_plus, "No") 
    {
      val = val + VALUE1;
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__assign_minus, "No") 
    {
      val = val - VALUE1;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__left_shift_assign, "No") 
    {
      val <<= VALUE1;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__right_shift_assign, "No") 
    {
      val >>= VALUE1;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__assign_left_shift, "No") 
    {
      val = val << VALUE1;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__assign_right_shift, "No") 
    {
      val = val >> VALUE1;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__bitwise_AND_assign, "No") 
    {
      val &= VALUE1;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__bitwise_OR_assign, "No") 
    {
      val |= VALUE1;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__bitwise_XOR_assign, "No") 
    {
      val ^= VALUE1;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__assign_bitwise_AND, "No") 
    {
      val = val & VALUE1;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__assign_bitwise_OR, "No") 
    {
      val = val | VALUE1;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__assign_bitwise_XOR, "No") 
    {
      val = val & VALUE1;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__sizeof_value, "No") 
    {
      sink = sizeof val;
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__sizeof_type, "No") 
    {
      sink = sizeof (int);
    }
  }

} // primitive_action




///////////////
// End-Of-File
///////////////

