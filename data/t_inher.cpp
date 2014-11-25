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
//  FILE     : t_inher.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============


// #########################################
/*
Here is a classes hierarchy.

                          Foo0
                           |
                           |
                          Foo1
                           |
                           |
              _____________|_____________
    __________|__________               |
    |                   |               |
    |                   |               |
   Foo2_v1             Foo2_v2         Foo2
    |                   |               |
    |___________________|               |
              |                         |
              |                         |
             Foo3_m                    Foo3
              |                         |
              |                         |
             Foo4_m                    Foo4

*/

// --------------
class Foo0
{
  public:
    Foo0 () {}
};

// --------------
class Foo1 : public Foo0
{
  public:
    Foo1 () : Foo0 () {}
};

// --------------
class Foo2_v1 : virtual public Foo1
{
  public:
    Foo2_v1 () : Foo1 () {}

};

// --------------
class Foo2_v2 : virtual public Foo1
{
  public:
    Foo2_v2 () : Foo1 () {}
};


// --------------
class Foo2 : public Foo1
{
  public:
    Foo2 () : Foo1 () {}

};

// --------------
class Foo3_m : public Foo2_v1, public Foo2_v2
{
  public:
    Foo3_m () : Foo1 (), Foo2_v1 (), Foo2_v2 () {}

};

// --------------
class Foo3 : public Foo2
{
  public:
    Foo3 () : Foo2 () {}

};

// --------------
class Foo4_m : public Foo3_m
{
  public:
    Foo4_m () : Foo1 (), Foo3_m () {}
};

// --------------
class Foo4 : public Foo3
{
  public:
    Foo4 () : Foo3 () {}
};



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void creating_inherited_objects (void)
{

const string htext__foo0_base     ("Level-0 base class          ");
const string htext__foo1_base     ("Level-1 single   inheritance");

const string htext__foo2_base     ("Level-2 single   inheritance");
const string htext__foo3_base     ("Level-3 single   inheritance");
const string htext__foo4_base     ("Level-4 single   inheritance");

const string htext__foo2v_base    ("Level-2 virtual  inheritance");
const string htext__foo3m_base    ("Level-3 multiple inheritance");
const string htext__foo4m_base    ("Level-4 multiple inheritance");

const string suftext_no_size ("No");

  // -------------------------------

  {
    TURN_ON_DEFAULT_TIMER (htext__foo0_base, suftext_no_size) 
    {
      Foo0();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__foo1_base, suftext_no_size) 
    {
      Foo1();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__foo2_base, suftext_no_size) 
    {
      Foo2();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__foo2v_base, suftext_no_size) 
    {
      Foo2_v1();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__foo2v_base, suftext_no_size) 
    {
      Foo2_v2();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__foo3_base, suftext_no_size) 
    {
      Foo3();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__foo3m_base, suftext_no_size) 
    {
      Foo3_m();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__foo4_base, suftext_no_size) 
    {
      Foo4();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__foo4m_base, suftext_no_size) 
    {
      Foo4_m();
    }
  }



} // creating_inherited_objects


///////////////
// End-Of-File
///////////////

