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
//  FILE     : t_virt.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============



// #########################################

// --------------
struct Foo00
{
  void of();
  virtual void vf();

};
void Foo00::of () {}
void Foo00::vf () {}

// --------------
struct Foo01 : public Foo00
{
  void of();
  void vf();
};
void Foo01::of () {}
void Foo01::vf () {}

// --------------
struct Foo02 : public Foo01
{
  void of();
  void vf();
};
void Foo02::of () {}
void Foo02::vf () {}


// --------------
struct Foo03 : public Foo02
{
  void of();
  void vf();
};
void Foo03::of () {}
void Foo03::vf () {}


// --------------
struct Foo04 : public Foo03
{
  void of();
  void vf();
};
void Foo04::of () {}
void Foo04::vf () {}


// --------------
struct Foo05 : public Foo04
{
  void of();
  void vf();
};
void Foo05::of () {}
void Foo05::vf () {}




// --------------
struct Bar00
{
  void f0();
};
void Bar00::f0 () {}


// --------------
struct Bar01 : public Bar00
{
  void f1();
};
void Bar01::f1 () {}

// --------------
struct Bar02 : public Bar01
{
  void f2();
};
void Bar02::f2 () {}


// --------------
struct Bar03 : public Bar02
{
  void f3();
};
void Bar03::f3 () {}


// --------------
struct Bar04 : public Bar03
{
  void f4();
};
void Bar04::f4 () {}


// --------------
struct Bar05 : public Bar04
{
  void f5();
};
void Bar05::f5 () {}






// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void virtual_methods (void)
{

const string htext__Foo00_obj_ord     ("Level-V0, object,  ordinary");
const string htext__Foo00_obj_virt    ("Level-V0, object,  virtual ");
const string htext__Foo00_ptr_ord     ("Level-V0, pointer, ordinary");
const string htext__Foo00_ptr_virt    ("Level-V0, pointer, virtual ");

const string htext__Foo01_obj_ord     ("Level-V1, object,  ordinary");
const string htext__Foo01_obj_virt    ("Level-V1, object,  virtual ");
const string htext__Foo01_ptr_ord     ("Level-V1, pointer, ordinary");
const string htext__Foo01_ptr_virt    ("Level-V1, pointer, virtual ");

const string htext__Foo02_obj_ord     ("Level-V2, object,  ordinary");
const string htext__Foo02_obj_virt    ("Level-V2, object,  virtual ");
const string htext__Foo02_ptr_ord     ("Level-V2, pointer, ordinary");
const string htext__Foo02_ptr_virt    ("Level-V2, pointer, virtual ");


const string htext__Foo03_obj_ord     ("Level-V3, object,  ordinary");
const string htext__Foo03_obj_virt    ("Level-V3, object,  virtual ");
const string htext__Foo03_ptr_ord     ("Level-V3, pointer, ordinary");
const string htext__Foo03_ptr_virt    ("Level-V3, pointer, virtual ");


const string htext__Foo04_obj_ord     ("Level-V4, object,  ordinary");
const string htext__Foo04_obj_virt    ("Level-V4, object,  virtual ");
const string htext__Foo04_ptr_ord     ("Level-V4, pointer, ordinary");
const string htext__Foo04_ptr_virt    ("Level-V4, pointer, virtual ");


const string htext__Foo05_obj_ord     ("Level-V5, object,  ordinary");
const string htext__Foo05_obj_virt    ("Level-V5, object,  virtual ");
const string htext__Foo05_ptr_ord     ("Level-V5, pointer, ordinary");
const string htext__Foo05_ptr_virt    ("Level-V5, pointer, virtual ");



const string htext__Bar00_obj_ord     ("Level-N0, object,  ordinary");
const string htext__Bar00_ptr_ord     ("Level-N0, pointer, ordinary");

const string htext__Bar01_obj_ord     ("Level-N1, object,  ordinary");
const string htext__Bar01_ptr_ord     ("Level-N1, pointer, ordinary");

const string htext__Bar02_obj_ord     ("Level-N2, object,  ordinary");
const string htext__Bar02_ptr_ord     ("Level-N2, pointer, ordinary");

const string htext__Bar03_obj_ord     ("Level-N3, object,  ordinary");
const string htext__Bar03_ptr_ord     ("Level-N3, pointer, ordinary");

const string htext__Bar04_obj_ord     ("Level-N4, object,  ordinary");
const string htext__Bar04_ptr_ord     ("Level-N4, pointer, ordinary");

const string htext__Bar05_obj_ord     ("Level-N5, object,  ordinary");
const string htext__Bar05_ptr_ord     ("Level-N5, pointer, ordinary");


const string suftext_no_size ("No");


Foo00 foo0;
Foo01 foo1;
Foo02 foo2;
Foo03 foo3;
Foo04 foo4;
Foo05 foo5;

Foo00* pfoo0 = new Foo00;
Foo00* pfoo1 = new Foo01;
Foo00* pfoo2 = new Foo02;
Foo00* pfoo3 = new Foo03;
Foo00* pfoo4 = new Foo04;
Foo00* pfoo5 = new Foo05;


Bar00 bar0;
Bar01 bar1;
Bar02 bar2;
Bar03 bar3;
Bar04 bar4;
Bar05 bar5;

Bar00* pbar0 = new Bar00;
Bar00* pbar1 = new Bar01;
Bar00* pbar2 = new Bar02;
Bar00* pbar3 = new Bar03;
Bar00* pbar4 = new Bar04;
Bar00* pbar5 = new Bar05;


  // -------------------------------

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo00_obj_ord, suftext_no_size) 
    {
      foo0.of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo00_obj_virt, suftext_no_size) 
    {
      foo0.vf();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Foo00_ptr_ord, suftext_no_size) 
    {
      pfoo0->of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo00_ptr_virt, suftext_no_size) 
    {
      pfoo0->vf();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Foo01_obj_ord, suftext_no_size) 
    {
      foo1.of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo01_obj_virt, suftext_no_size) 
    {
      foo1.vf();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Foo01_ptr_ord, suftext_no_size) 
    {
      pfoo1->of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo01_ptr_virt, suftext_no_size) 
    {
      pfoo1->vf();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Foo02_obj_ord, suftext_no_size) 
    {
      foo2.of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo02_obj_virt, suftext_no_size) 
    {
      foo2.vf();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Foo02_ptr_ord, suftext_no_size) 
    {
      pfoo2->of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo02_ptr_virt, suftext_no_size) 
    {
      pfoo2->vf();
    }
  }





  {
    TURN_ON_DEFAULT_TIMER (htext__Foo03_obj_ord, suftext_no_size) 
    {
      foo3.of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo03_obj_virt, suftext_no_size) 
    {
      foo3.vf();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Foo03_ptr_ord, suftext_no_size) 
    {
      pfoo3->of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo03_ptr_virt, suftext_no_size) 
    {
      pfoo3->vf();
    }
  }





  {
    TURN_ON_DEFAULT_TIMER (htext__Foo04_obj_ord, suftext_no_size) 
    {
      foo4.of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo04_obj_virt, suftext_no_size) 
    {
      foo4.vf();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Foo04_ptr_ord, suftext_no_size) 
    {
      pfoo4->of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo04_ptr_virt, suftext_no_size) 
    {
      pfoo4->vf();
    }
  }




  {
    TURN_ON_DEFAULT_TIMER (htext__Foo05_obj_ord, suftext_no_size) 
    {
      foo5.of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo05_obj_virt, suftext_no_size) 
    {
      foo5.vf();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Foo05_ptr_ord, suftext_no_size) 
    {
      pfoo5->of();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Foo05_ptr_virt, suftext_no_size) 
    {
      pfoo5->vf();
    }
  }




  // -------------------------------

  {
    TURN_ON_DEFAULT_TIMER (htext__Bar00_obj_ord, suftext_no_size) 
    {
      bar0.f0();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Bar00_ptr_ord, suftext_no_size) 
    {
      pbar0->f0();
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext__Bar01_obj_ord, suftext_no_size) 
    {
      bar1.f1();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Bar01_ptr_ord, suftext_no_size) 
    {
      pbar1->f0();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Bar02_obj_ord, suftext_no_size) 
    {
      bar2.f2();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Bar02_ptr_ord, suftext_no_size) 
    {
      pbar2->f0();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Bar03_obj_ord, suftext_no_size) 
    {
      bar3.f3();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Bar03_ptr_ord, suftext_no_size) 
    {
      pbar3->f0();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Bar04_obj_ord, suftext_no_size) 
    {
      bar4.f4();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Bar04_ptr_ord, suftext_no_size) 
    {
      pbar4->f0();
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext__Bar05_obj_ord, suftext_no_size) 
    {
      bar5.f5();
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__Bar05_ptr_ord, suftext_no_size) 
    {
      pbar5->f0();
    }
  }


} // virtual_methods



///////////////
// End-Of-File
///////////////

