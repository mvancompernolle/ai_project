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
//  FILE     : t_excep.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============



// #########################################
// -----------
static bool foo_if_statement (bool b) 
{ 
  if (b) return false;
  return true;
}

// -----------
static bool foo_exception_handling (bool b) 
{ 
  try            { if (b) throw false; }
  catch (bool t) { return t; }
  return true;
}



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void exception_action (void)
{
const string htext__if_statement_true         ("if-statement       true ");
const string htext__if_statement_false        ("if-statement       false");
const string htext__exception_handling_true   ("exception handling true ");
const string htext__exception_handling_false  ("exception handling false");

  // -------------------------------

  {
    TURN_ON_DEFAULT_TIMER (htext__if_statement_false, "No") 
    {
      foo_if_statement (false);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__if_statement_true, "No") 
    {
      foo_if_statement (true);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__exception_handling_false, "No") 
    {
      foo_exception_handling (false);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__exception_handling_true, "No") 
    {
      foo_exception_handling (true);
    }
  }

} // exception_action




///////////////
// End-Of-File
///////////////

