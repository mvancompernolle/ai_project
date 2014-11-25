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
//  FILE     : t_demo1.cpp
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
// =============================
void demo_01 (void)
{
  TURN_ON_CLOCK ("do nothing") 
  {
  }
} // demo_01


// =============================
void demo_02 (void)
{

  {
    TURN_ON_THE_TIME ("delete new string", 40) 
    {
      delete new std::string("1234567890123456789012345678901234567890");
    }
  }


  {
    TURN_ON_THE_CLOCK ("delete new string", 40) 
    {
      delete new std::string("1234567890123456789012345678901234567890");
    }
  }

  {
    TURN_ON_DEFAULT_TIMER ("delete new string", 20) 
    {
      delete new std::string("12345678901234567890");
    }
  }

} // demo_02


// =============================
void demo_03 (void)
{
  {
    TURN_ON_THE_CLOCK ("delete new string", 4) 
    {
      delete new std::string("ABCD");
    }
  }

  {
    TURN_ON_THE_CLOCK ("delete new string", 9) 
    {
      delete new std::string("123456789");
    }
  }


  {
    TURN_ON_THE_TIME ("delete new string", 9) 
    {
      delete new std::string("123456789");
    }
  }


} // demo_03 


// =============================
void demo_04 (void)
{
  TURN_ON_THE_CLOCK ("delete new string /10 times/", 4) 
  {
    for (int i = 0; i < 10; i++)
    {
      delete new std::string("ABCD");
    }
  }

} // demo_04


// =============================
void demo_05 (void)
{
  TURN_ON_THE_CLOCK ("delete new string /20 times/", 4) 
  {
    for (int i = 0; i < 20; i++)
    {
      delete new std::string("ABCD");
    }
  }
} // demo_05


// =============================
void demo_06 (void)
{

  {
    TURN_ON_THE_TIME ("delete new string", 6) 
    {
      delete new std::string("ABCDEF");
    }
  }


  {
    TURN_ON_THE_CLOCK ("delete new string", 6) 
    {
      delete new std::string("ABCDEF");
    }
  }

//////////////////////
#if (defined UNIX_ENV)
//////////////////////
  {
    TURN_ON_THE_RUSAGE_USER_TIME (double, "delete new string", 6) 
    {
      delete new std::string("ABCDEF");
    }
  }

  {
    TURN_ON_THE_RUSAGE_USER_TIME (ulonglong, "delete new string", 6) 
    {
      delete new std::string("ABCDEF");
    }
  }


  {
    TURN_ON_THE_RUSAGE_SYSTEM_TIME (double, "delete new string", 6) 
    {
      delete new std::string("ABCDEF");
    }
  }

  {
    TURN_ON_THE_RUSAGE_SYSTEM_TIME (ulonglong, "delete new string", 6) 
    {
      delete new std::string("ABCDEF");
    }
  }



  {
    TURN_ON_THE_GETTIMEOFDAY_REAL_TIME (double, "delete new string", 6) 
    {
      delete new std::string("ABCDEF");
    }
  }

  {
    TURN_ON_THE_GETTIMEOFDAY_REAL_TIME (ulonglong, "delete new string", 6) 
    {
      delete new std::string("ABCDEF");
    }
  }


//////
#endif
//////

  {
    TURN_ON (Dummy1, Dummy2, RESOURCE_dummy, "delete new string (size is 6)") 
    {
      delete new std::string("ABCDEF");
    }
  }

} // demo_06


// =============================
void demo_07 (void)
{
  {
    TURN_ON_THE_CLOCK ("delete new string", 9) 
    {
      delete new std::string("ABCDEF123");
    }
  }

} // demo_07


// =============================
void demo_08 (void)
{
//////////////////////
#if (defined UNIX_ENV)
//////////////////////
  {
    TURN_ON_THE_RUSAGE_USER_TIME (double, "delete new string", 9) 
    {
      delete new std::string("ABCDEF123");
    }
  }

  {
    TURN_ON_THE_RUSAGE_USER_TIME (ulonglong, "delete new string", 9) 
    {
      delete new std::string("ABCDEF123");
    }
  }


  {
    TURN_ON_THE_RUSAGE_SYSTEM_TIME (double, "delete new string", 9) 
    {
      delete new std::string("ABCDEF123");
    }
  }

  {
    TURN_ON_THE_RUSAGE_SYSTEM_TIME (ulonglong, "delete new string", 9) 
    {
      delete new std::string("ABCDEF123");
    }
  }


  {
    TURN_ON_THE_GETTIMEOFDAY_REAL_TIME (double, "delete new string", 9) 
    {
      delete new std::string("ABCDEF123");
    }
  }

  {
    TURN_ON_THE_GETTIMEOFDAY_REAL_TIME (ulonglong, "delete new string", 9) 
    {
      delete new std::string("ABCDEF123");
    }
  }



//////
#endif
//////
} // demo_08

// =============================
void demo_09 (void)
{
  {
    TURN_ON (Dummy1, Dummy2, RESOURCE_dummy, "delete new string (size is 9)") 
    {
      delete new std::string("ABCDEF123");
    }
  }

} // demo_09



// =============================
void demo_10 (void)
{
  TURN_ON_THE_CLOCK ("delete new string", 5) 
  {
    delete new std::string("ABCDE");
  }
} // demo_10 


// =============================
void demo_11 (void)
{
const char str1[] = "ABCD";
const char str2[] = "ABCDEFG";
  {
    TURN_ON_THE_CLOCK ("strlen", string(str1).size()) 
    {
      strlen(str1);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER ("strlen", string(str2).size()) 
    {
      strlen(str2);
    }
  }

} // demo_11 


// =============================
void demo_12 (void)
{
  {
    TURN_ON_DEFAULT_TIMER ("delete new string", 9) 
    {
      delete new std::string("ABCDEF123");
    }
  }

} // demo_12


// ###########################################



///////////////
// End-Of-File
///////////////
