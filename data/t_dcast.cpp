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
//  FILE     : t_dcast.cpp
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
#define	TAG_VALUE_Doo	50

// -----------
class Boo
{
  protected :
    int tag_;
  public :
    int get_tag () const {return tag_; }
    Boo () : tag_ (0) {}
    virtual ~Boo () {}
};

// -----------
class Doo : public Boo
{
  public :
    Doo() : Boo() {tag_ = TAG_VALUE_Doo;}
    ~Doo() {}
};



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void dcast_and_switch_action (void)
{
const string htext__switch_10_cases      ("switch -  10 cases    ");
const string htext__switch_20_cases      ("switch -  20 cases    ");
const string htext__switch_50_cases      ("switch -  50 cases    ");
const string htext__switch_100_cases     ("switch - 100 cases    ");
#ifdef __GNUC__
const string htext__switch_50_gcc_cases  ("switch -  50 gcc-cases");
const string htext__switch_100_gcc_cases ("switch - 100 gcc-cases");
#endif
const string htext__dynamic_cast         ("dynamic_cast          ");


Boo* const ptr_Boo = new Doo;

  // -------------------------------

  {
    TURN_ON_DEFAULT_TIMER (htext__dynamic_cast, "No") 
    {
      dynamic_cast<Doo*> (ptr_Boo);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__switch_10_cases, "No") 
    {
      switch (ptr_Boo->get_tag())
      {
        case  5 : break;
        case 10 : break;
        case 15 : break;
        case 20 : break;
        case 25 : break;
        case 30 : break;
        case 35 : break;
        case 40 : break;
        case 45 : break;
        case 50 : break;
  
        default : break;
  
      } // switch
    }
  }

  // -------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext__switch_20_cases, "No") 
    {
      switch (ptr_Boo->get_tag())
      {
        case  5 : break;
        case 10 : break;
        case 15 : break;
        case 20 : break;
        case 25 : break;
        case 30 : break;
        case 35 : break;
        case 40 : break;
        case 45 : break;
        case 50 : break;
        case 55 : break;
        case 60 : break;
        case 65 : break;
        case 70 : break;
        case 75 : break;
        case 80 : break;
        case 85 : break;
        case 90 : break;
        case 95 : break;
        case 100 : break;
  
        default : break;
  
      } // switch
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__switch_50_cases, "No") 
    {
      switch (ptr_Boo->get_tag())
      {
        case  1 : break;
        case  2 : break;
        case  3 : break;
        case  4 : break;
        case  5 : break;
        case  6 : break;
        case  7 : break;
        case  8 : break;
        case  9 : break;
        case 10 : break;
 
        case 11 : break;
        case 12 : break;
        case 13 : break;
        case 14 : break;
        case 15 : break;
        case 16 : break;
        case 17 : break;
        case 18 : break;
        case 19 : break;
        case 20 : break;
 
        case 21 : break;
        case 22 : break;
        case 23 : break;
        case 24 : break;
        case 25 : break;
        case 26 : break;
        case 27 : break;
        case 28 : break;
        case 29 : break;
        case 30 : break;
 
        case 31 : break;
        case 32 : break;
        case 33 : break;
        case 34 : break;
        case 35 : break;
        case 36 : break;
        case 37 : break;
        case 38 : break;
        case 39 : break;
        case 40 : break;
 
        case 41 : break;
        case 42 : break;
        case 43 : break;
        case 44 : break;
        case 45 : break;
        case 46 : break;
        case 47 : break;
        case 48 : break;
        case 49 : break;
        case 50 : break;
 
        default : break;

      } // switch
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__switch_100_cases, "No") 
    {
      switch (ptr_Boo->get_tag())
      {
        case  1 : break;
        case  2 : break;
        case  3 : break;
        case  4 : break;
        case  5 : break;
        case  6 : break;
        case  7 : break;
        case  8 : break;
        case  9 : break;
        case 10 : break;
 
        case 11 : break;
        case 12 : break;
        case 13 : break;
        case 14 : break;
        case 15 : break;
        case 16 : break;
        case 17 : break;
        case 18 : break;
        case 19 : break;
        case 20 : break;
 
        case 21 : break;
        case 22 : break;
        case 23 : break;
        case 24 : break;
        case 25 : break;
        case 26 : break;
        case 27 : break;
        case 28 : break;
        case 29 : break;
        case 30 : break;
 
        case 31 : break;
        case 32 : break;
        case 33 : break;
        case 34 : break;
        case 35 : break;
        case 36 : break;
        case 37 : break;
        case 38 : break;
        case 39 : break;
        case 40 : break;
 
        case 41 : break;
        case 42 : break;
        case 43 : break;
        case 44 : break;
        case 45 : break;
        case 46 : break;
        case 47 : break;
        case 48 : break;
        case 49 : break;
        case 50 : break;

        case 51 : break;
        case 52 : break;
        case 53 : break;
        case 54 : break;
        case 55 : break;
        case 56 : break;
        case 57 : break;
        case 58 : break;
        case 59 : break;
        case 60 : break;
 
        case 61 : break;
        case 62 : break;
        case 63 : break;
        case 64 : break;
        case 65 : break;
        case 66 : break;
        case 67 : break;
        case 68 : break;
        case 69 : break;
        case 70 : break;
 
        case 71 : break;
        case 72 : break;
        case 73 : break;
        case 74 : break;
        case 75 : break;
        case 76 : break;
        case 77 : break;
        case 78 : break;
        case 79 : break;
        case 80 : break;
 
        case 81 : break;
        case 82 : break;
        case 83 : break;
        case 84 : break;
        case 85 : break;
        case 86 : break;
        case 87 : break;
        case 88 : break;
        case 89 : break;
        case 90 : break;
 
        case 91 : break;
        case 92 : break;
        case 93 : break;
        case 94 : break;
        case 95 : break;
        case 96 : break;
        case 97 : break;
        case 98 : break;
        case 99 : break;
        case 100 : break;

        default : break;

      } // switch
    }
  }

#ifdef __GNUC__
  {
    TURN_ON_DEFAULT_TIMER (htext__switch_50_gcc_cases, "No") 
    {
      switch (ptr_Boo->get_tag())
      {
        case  1 ... 50 : break;
 
        default : break;

      } // switch
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__switch_100_gcc_cases, "No") 
    {
      switch (ptr_Boo->get_tag())
      {
        case  1 ... 100 : break;
 
        default : break;

      } // switch
    }
  }
#endif

} // dcast_and_switch_action




///////////////
// End-Of-File
///////////////

