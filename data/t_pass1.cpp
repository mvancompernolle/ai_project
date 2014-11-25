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
//  FILE     : t_pass1.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============



// #########################################
int merely_rand = rand();

// -------------------------------------------
static int sink = 0;
#define FOO1_BODY	sink += merely_rand
#define FOO2v_BODY	sink += var
#define FOO2p_BODY	sink += *ptr


// #########################################
void foo1_no_vars        ()                         { FOO1_BODY; }

// ---------------
void foo1_bool_val       (bool )                    { FOO1_BODY; }
void foo1_bool_ref       (bool& )                   { FOO1_BODY; }
void foo1_bool_cref      (const bool& )             { FOO1_BODY; }
void foo1_bool_ptr       (bool* )                   { FOO1_BODY; }
void foo1_bool_cptr      (const bool* )             { FOO1_BODY; }

void foo1_char_val       (char )                    { FOO1_BODY; }
void foo1_char_ref       (char& )                   { FOO1_BODY; }
void foo1_char_cref      (const char& )             { FOO1_BODY; }
void foo1_char_ptr       (char* )                   { FOO1_BODY; }
void foo1_char_cptr      (const char* )             { FOO1_BODY; }

void foo1_short_val      (short )                   { FOO1_BODY; }
void foo1_short_ref      (short& )                  { FOO1_BODY; }
void foo1_short_cref     (const short& )            { FOO1_BODY; }
void foo1_short_ptr      (short* )                  { FOO1_BODY; }
void foo1_short_cptr     (const short* )            { FOO1_BODY; }

void foo1_int_val        (int )                     { FOO1_BODY; }
void foo1_int_ref        (int& )                    { FOO1_BODY; }
void foo1_int_cref       (const int& )              { FOO1_BODY; }
void foo1_int_ptr        (int* )                    { FOO1_BODY; }
void foo1_int_cptr       (const int* )              { FOO1_BODY; }

void foo1_long_val       (long )                    { FOO1_BODY; }
void foo1_long_ref       (long& )                   { FOO1_BODY; }
void foo1_long_cref      (const long& )             { FOO1_BODY; }
void foo1_long_ptr       (long* )                   { FOO1_BODY; }
void foo1_long_cptr      (const long* )             { FOO1_BODY; }

void foo1_long_long_val  (ulonglong )               { FOO1_BODY; }
void foo1_long_long_ref  (ulonglong& )              { FOO1_BODY; }
void foo1_long_long_cref (const ulonglong& )        { FOO1_BODY; }
void foo1_long_long_ptr  (ulonglong* )              { FOO1_BODY; }
void foo1_long_long_cptr (const ulonglong* )        { FOO1_BODY; }


// ---------------
void foo2_bool_val       (bool var)                 { FOO2v_BODY; }
void foo2_bool_ref       (bool& var)                { FOO2v_BODY; }
void foo2_bool_cref      (const bool& var)          { FOO2v_BODY; }
void foo2_bool_ptr       (bool* ptr)                { FOO2p_BODY; }
void foo2_bool_cptr      (const bool* ptr)          { FOO2p_BODY; }

void foo2_char_val       (char var)                 { FOO2v_BODY; }
void foo2_char_ref       (char& var)                { FOO2v_BODY; }
void foo2_char_cref      (const char& var)          { FOO2v_BODY; }
void foo2_char_ptr       (char* ptr)                { FOO2p_BODY; }
void foo2_char_cptr      (const char* ptr)          { FOO2p_BODY; }

void foo2_short_val      (short var)                { FOO2v_BODY; }
void foo2_short_ref      (short& var)               { FOO2v_BODY; }
void foo2_short_cref     (const short& var)         { FOO2v_BODY; }
void foo2_short_ptr      (short* ptr)               { FOO2p_BODY; }
void foo2_short_cptr     (const short* ptr)         { FOO2p_BODY; }

void foo2_int_val        (int var)                  { FOO2v_BODY; }
void foo2_int_ref        (int& var)                 { FOO2v_BODY; }
void foo2_int_cref       (const int& var)           { FOO2v_BODY; }
void foo2_int_ptr        (int* ptr)                 { FOO2p_BODY; }
void foo2_int_cptr       (const int* ptr)           { FOO2p_BODY; }

void foo2_long_val       (long var)                 { FOO2v_BODY; }
void foo2_long_ref       (long& var)                { FOO2v_BODY; }
void foo2_long_cref      (const long& var)          { FOO2v_BODY; }
void foo2_long_ptr       (long* ptr)                { FOO2p_BODY; }
void foo2_long_cptr      (const long* ptr)          { FOO2p_BODY; }

void foo2_long_long_val  (ulonglong var)            { FOO2v_BODY; }
void foo2_long_long_ref  (ulonglong& var)           { FOO2v_BODY; }
void foo2_long_long_cref (const ulonglong& var)     { FOO2v_BODY; }
void foo2_long_long_ptr  (ulonglong* ptr)           { FOO2p_BODY; }
void foo2_long_long_cptr (const ulonglong* ptr)     { FOO2p_BODY; }



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################

// =============================
void pass_action_no_size (void)
{

const string htext__do_nothing                        ("do nothing                                  ");

const string htext__no_varuments                      ("no varuments                                ");

const string htext1__bool_via_bool_value              ("(n) bool              via  bool-value       ");
const string htext1__bool_via_bool_ref                ("(n) bool              via  ref-to-bool      ");
const string htext1__bool_via_bool_cref               ("(n) bool              via  cref-to-bool     ");
const string htext1__bool_via_bool_ptr                ("(n) bool              via  ptr-to-bool      ");
const string htext1__bool_ptr_via_bool_ptr            ("(n) ptr-to-bool       via  ptr-to-bool      ");
const string htext1__bool_via_bool_cptr               ("(n) bool              via  cptr-to-bool     ");
const string htext1__bool_ptr_via_bool_cptr           ("(n) ptr-to-bool       via  cptr-to-bool     ");

const string htext1__char_via_char_value              ("(n) char              via  char-value       ");
const string htext1__char_via_char_ref                ("(n) char              via  ref-to-char      ");
const string htext1__char_via_char_cref               ("(n) char              via  cref-to-char     ");
const string htext1__char_via_char_ptr                ("(n) char              via  ptr-to-char      ");
const string htext1__char_ptr_via_char_ptr            ("(n) ptr-to-char       via  ptr-to-char      ");
const string htext1__char_via_char_cptr               ("(n) char              via  cptr-to-char     ");
const string htext1__char_ptr_via_char_cptr           ("(n) ptr-to-char       via  cptr-to-char     ");

const string htext1__short_via_short_value            ("(n) short             via  short-value      ");
const string htext1__short_via_short_ref              ("(n) short             via  ref-to-short     ");
const string htext1__short_via_short_cref             ("(n) short             via  cref-to-short    ");
const string htext1__short_via_short_ptr              ("(n) short             via  ptr-to-short     ");
const string htext1__short_ptr_via_short_ptr          ("(n) ptr-to-short      via  ptr-to-short     ");
const string htext1__short_via_short_cptr             ("(n) short             via  cptr-to-short    ");
const string htext1__short_ptr_via_short_cptr         ("(n) ptr-to-short      via  cptr-to-short    ");

const string htext1__int_via_int_value                ("(n) int               via  int-value        ");
const string htext1__int_via_int_ref                  ("(n) int               via  ref-to-int       ");
const string htext1__int_via_int_cref                 ("(n) int               via  cref-to-int      ");
const string htext1__int_via_int_ptr                  ("(n) int               via  ptr-to-int       ");
const string htext1__int_ptr_via_int_ptr              ("(n) ptr-to-int        via  ptr-to-int       ");
const string htext1__int_via_int_cptr                 ("(n) int               via  cptr-to-int      ");
const string htext1__int_ptr_via_int_cptr             ("(n) ptr-to-int        via  cptr-to-int      ");

const string htext1__long_via_long_value              ("(n) long              via  long-value       ");
const string htext1__long_via_long_ref                ("(n) long              via  ref-to-long      ");
const string htext1__long_via_long_cref               ("(n) long              via  cref-to-long     ");
const string htext1__long_via_long_ptr                ("(n) long              via  ptr-to-long      ");
const string htext1__long_ptr_via_long_ptr            ("(n) ptr-to-long       via  ptr-to-long      ");
const string htext1__long_via_long_cptr               ("(n) long              via  cptr-to-long     ");
const string htext1__long_ptr_via_long_cptr           ("(n) ptr-to-long       via  cptr-to-long     ");

const string htext1__long_long_via_long_long_value    ("(n) long-long         via  long-long-value  ");
const string htext1__long_long_via_long_long_ref      ("(n) long-long         via  ref-to-long-long ");
const string htext1__long_long_via_long_long_cref     ("(n) long-long         via  cref-to-long-long");
const string htext1__long_long_via_long_long_ptr      ("(n) long-long         via  ptr-to-long-long ");
const string htext1__long_long_ptr_via_long_long_ptr  ("(n) ptr-to-long-long  via  ptr-to-long-long ");
const string htext1__long_long_via_long_long_cptr     ("(n) long-long         via  cptr-to-long-long");
const string htext1__long_long_ptr_via_long_long_cptr ("(n) ptr-to-long-long  via  cptr-to-long-long");


// -----

const string htext2__bool_via_bool_value              ("(a) bool              via  bool-value       ");
const string htext2__bool_via_bool_ref                ("(a) bool              via  ref-to-bool      ");
const string htext2__bool_via_bool_cref               ("(a) bool              via  cref-to-bool     ");
const string htext2__bool_via_bool_ptr                ("(a) bool              via  ptr-to-bool      ");
const string htext2__bool_ptr_via_bool_ptr            ("(a) ptr-to-bool       via  ptr-to-bool      ");
const string htext2__bool_via_bool_cptr               ("(a) bool              via  cptr-to-bool     ");
const string htext2__bool_ptr_via_bool_cptr           ("(a) ptr-to-bool       via  cptr-to-bool     ");

const string htext2__char_via_char_value              ("(a) char              via  char-value       ");
const string htext2__char_via_char_ref                ("(a) char              via  ref-to-char      ");
const string htext2__char_via_char_cref               ("(a) char              via  cref-to-char     ");
const string htext2__char_via_char_ptr                ("(a) char              via  ptr-to-char      ");
const string htext2__char_ptr_via_char_ptr            ("(a) ptr-to-char       via  ptr-to-char      ");
const string htext2__char_via_char_cptr               ("(a) char              via  cptr-to-char     ");
const string htext2__char_ptr_via_char_cptr           ("(a) ptr-to-char       via  cptr-to-char     ");

const string htext2__short_via_short_value            ("(a) short             via  short-value      ");
const string htext2__short_via_short_ref              ("(a) short             via  ref-to-short     ");
const string htext2__short_via_short_cref             ("(a) short             via  cref-to-short    ");
const string htext2__short_via_short_ptr              ("(a) short             via  ptr-to-short     ");
const string htext2__short_ptr_via_short_ptr          ("(a) ptr-to-short      via  ptr-to-short     ");
const string htext2__short_via_short_cptr             ("(a) short             via  cptr-to-short    ");
const string htext2__short_ptr_via_short_cptr         ("(a) ptr-to-short      via  cptr-to-short    ");

const string htext2__int_via_int_value                ("(a) int               via  int-value        ");
const string htext2__int_via_int_ref                  ("(a) int               via  ref-to-int       ");
const string htext2__int_via_int_cref                 ("(a) int               via  cref-to-int      ");
const string htext2__int_via_int_ptr                  ("(a) int               via  ptr-to-int       ");
const string htext2__int_ptr_via_int_ptr              ("(a) ptr-to-int        via  ptr-to-int       ");
const string htext2__int_via_int_cptr                 ("(a) int               via  cptr-to-int      ");
const string htext2__int_ptr_via_int_cptr             ("(a) ptr-to-int        via  cptr-to-int      ");

const string htext2__long_via_long_value              ("(a) long              via  long-value       ");
const string htext2__long_via_long_ref                ("(a) long              via  ref-to-long      ");
const string htext2__long_via_long_cref               ("(a) long              via  cref-to-long     ");
const string htext2__long_via_long_ptr                ("(a) long              via  ptr-to-long      ");
const string htext2__long_ptr_via_long_ptr            ("(a) ptr-to-long       via  ptr-to-long      ");
const string htext2__long_via_long_cptr               ("(a) long              via  cptr-to-long     ");
const string htext2__long_ptr_via_long_cptr           ("(a) ptr-to-long       via  cptr-to-long     ");

const string htext2__long_long_via_long_long_value    ("(a) long-long         via  long-long-value  ");
const string htext2__long_long_via_long_long_ref      ("(a) long-long         via  ref-to-long-long ");
const string htext2__long_long_via_long_long_cref     ("(a) long-long         via  cref-to-long-long");
const string htext2__long_long_via_long_long_ptr      ("(a) long-long         via  ptr-to-long-long ");
const string htext2__long_long_ptr_via_long_long_ptr  ("(a) ptr-to-long-long  via  ptr-to-long-long ");
const string htext2__long_long_via_long_long_cptr     ("(a) long-long         via  cptr-to-long-long");
const string htext2__long_long_ptr_via_long_long_cptr ("(a) ptr-to-long-long  via  cptr-to-long-long");

// ======================================
bool       bool_value      = true;
char       char_value      = 'a';
short      short_value     = 'a';
int        int_value       = 'a';
long       long_value      = 'a';
ulonglong  long_long_value = 'a';

bool*      ptr_bool        = &bool_value;
char*      ptr_char        = &char_value;
short*     ptr_short       = &short_value;
int*       ptr_int         = &int_value;
long*      ptr_long        = &long_value;
ulonglong* ptr_long_long   = &long_long_value;


  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext__do_nothing, "No") 
    {
      // Do nothing
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext__no_varuments, "No") 
    {
      foo1_no_vars ();
    }
  }

  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext1__bool_via_bool_value, "No") 
    {
      foo1_bool_val (bool_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__bool_via_bool_ref, "No") 
    {
      foo1_bool_ref (bool_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__bool_via_bool_cref, "No") 
    {
      foo1_bool_cref (bool_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__bool_via_bool_ptr, "No") 
    {
      foo1_bool_ptr (&bool_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__bool_ptr_via_bool_ptr, "No") 
    {
      foo1_bool_ptr (ptr_bool);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__bool_via_bool_cptr, "No") 
    {
      foo1_bool_cptr (&bool_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__bool_ptr_via_bool_cptr, "No") 
    {
      foo1_bool_cptr (ptr_bool);
    }
  }



  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext1__char_via_char_value, "No") 
    {
      foo1_char_val (char_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__char_via_char_ref, "No") 
    {
      foo1_char_ref (char_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__char_via_char_cref, "No") 
    {
      foo1_char_cref (char_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__char_via_char_ptr, "No") 
    {
      foo1_char_ptr (&char_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__char_ptr_via_char_ptr, "No") 
    {
      foo1_char_ptr (ptr_char);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext1__char_via_char_cptr, "No") 
    {
      foo1_char_cptr (&char_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__char_ptr_via_char_cptr, "No") 
    {
      foo1_char_cptr (ptr_char);
    }
  }



  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext1__short_via_short_value, "No") 
    {
      foo1_short_val (short_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__short_via_short_ref, "No") 
    {
      foo1_short_ref (short_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__short_via_short_cref, "No") 
    {
      foo1_short_cref (short_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__short_via_short_ptr, "No") 
    {
      foo1_short_ptr (&short_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__short_ptr_via_short_ptr, "No") 
    {
      foo1_short_ptr (ptr_short);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__short_via_short_cptr, "No") 
    {
      foo1_short_cptr (&short_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__short_ptr_via_short_cptr, "No") 
    {
      foo1_short_cptr (ptr_short);
    }
  }





  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext1__int_via_int_value, "No") 
    {
      foo1_int_val (int_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__int_via_int_ref, "No") 
    {
      foo1_int_ref (int_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__int_via_int_cref, "No") 
    {
      foo1_int_cref (int_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__int_via_int_ptr, "No") 
    {
      foo1_int_ptr (&int_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__int_ptr_via_int_ptr, "No") 
    {
      foo1_int_ptr (ptr_int);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__int_via_int_cptr, "No") 
    {
      foo1_int_cptr (&int_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__int_ptr_via_int_cptr, "No") 
    {
      foo1_int_cptr (ptr_int);
    }
  }



  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext1__long_via_long_value, "No") 
    {
      foo1_long_val (long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__long_via_long_ref, "No") 
    {
      foo1_long_ref (long_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__long_via_long_cref, "No") 
    {
      foo1_long_cref (long_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__long_via_long_ptr, "No") 
    {
      foo1_long_ptr (&long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__long_ptr_via_long_ptr, "No") 
    {
      foo1_long_ptr (ptr_long);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext1__long_via_long_cptr, "No") 
    {
      foo1_long_cptr (&long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__long_ptr_via_long_cptr, "No") 
    {
      foo1_long_cptr (ptr_long);
    }
  }


  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext1__long_long_via_long_long_value, "No") 
    {
      foo1_long_long_val (long_long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__long_long_via_long_long_ref, "No") 
    {
      foo1_long_long_ref (long_long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__long_long_via_long_long_cref, "No") 
    {
      foo1_long_long_cref (long_long_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__long_long_via_long_long_ptr, "No") 
    {
      foo1_long_long_ptr (&long_long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__long_long_ptr_via_long_long_ptr, "No") 
    {
      foo1_long_long_ptr (ptr_long_long);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext1__long_long_via_long_long_cptr, "No") 
    {
      foo1_long_long_cptr (&long_long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext1__long_long_ptr_via_long_long_cptr, "No") 
    {
      foo1_long_long_cptr (ptr_long_long);
    }
  }


  // ----------------------------------
  // ----------------------------------

  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext2__bool_via_bool_value, "No") 
    {
      foo2_bool_val (bool_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__bool_via_bool_ref, "No") 
    {
      foo2_bool_ref (bool_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__bool_via_bool_cref, "No") 
    {
      foo2_bool_cref (bool_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__bool_via_bool_ptr, "No") 
    {
      foo2_bool_ptr (&bool_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__bool_ptr_via_bool_ptr, "No") 
    {
      foo2_bool_ptr (ptr_bool);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__bool_via_bool_cptr, "No") 
    {
      foo2_bool_cptr (&bool_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__bool_ptr_via_bool_cptr, "No") 
    {
      foo2_bool_cptr (ptr_bool);
    }
  }



  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext2__char_via_char_value, "No") 
    {
      foo2_char_val (char_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__char_via_char_ref, "No") 
    {
      foo2_char_ref (char_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__char_via_char_cref, "No") 
    {
      foo2_char_cref (char_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__char_via_char_ptr, "No") 
    {
      foo2_char_ptr (&char_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__char_ptr_via_char_ptr, "No") 
    {
      foo2_char_ptr (ptr_char);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext2__char_via_char_cptr, "No") 
    {
      foo2_char_cptr (&char_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__char_ptr_via_char_cptr, "No") 
    {
      foo2_char_cptr (ptr_char);
    }
  }



  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext2__short_via_short_value, "No") 
    {
      foo2_short_val (short_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__short_via_short_ref, "No") 
    {
      foo2_short_ref (short_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__short_via_short_cref, "No") 
    {
      foo2_short_cref (short_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__short_via_short_ptr, "No") 
    {
      foo2_short_ptr (&short_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__short_ptr_via_short_ptr, "No") 
    {
      foo2_short_ptr (ptr_short);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__short_via_short_cptr, "No") 
    {
      foo2_short_cptr (&short_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__short_ptr_via_short_cptr, "No") 
    {
      foo2_short_cptr (ptr_short);
    }
  }





  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext2__int_via_int_value, "No") 
    {
      foo2_int_val (int_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__int_via_int_ref, "No") 
    {
      foo2_int_ref (int_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__int_via_int_cref, "No") 
    {
      foo2_int_cref (int_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__int_via_int_ptr, "No") 
    {
      foo2_int_ptr (&int_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__int_ptr_via_int_ptr, "No") 
    {
      foo2_int_ptr (ptr_int);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__int_via_int_cptr, "No") 
    {
      foo2_int_cptr (&int_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__int_ptr_via_int_cptr, "No") 
    {
      foo2_int_cptr (ptr_int);
    }
  }



  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext2__long_via_long_value, "No") 
    {
      foo2_long_val (long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__long_via_long_ref, "No") 
    {
      foo2_long_ref (long_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__long_via_long_cref, "No") 
    {
      foo2_long_cref (long_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__long_via_long_ptr, "No") 
    {
      foo2_long_ptr (&long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__long_ptr_via_long_ptr, "No") 
    {
      foo2_long_ptr (ptr_long);
    }
  }



  {
    TURN_ON_DEFAULT_TIMER (htext2__long_via_long_cptr, "No") 
    {
      foo2_long_cptr (&long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__long_ptr_via_long_cptr, "No") 
    {
      foo2_long_cptr (ptr_long);
    }
  }


  // ----------------------------------
  {
    TURN_ON_DEFAULT_TIMER (htext2__long_long_via_long_long_value, "No") 
    {
      foo2_long_long_val (long_long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__long_long_via_long_long_ref, "No") 
    {
      foo2_long_long_ref (long_long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__long_long_via_long_long_cref, "No") 
    {
      foo2_long_long_cref (long_long_value);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__long_long_via_long_long_ptr, "No") 
    {
      foo2_long_long_ptr (&long_long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__long_long_ptr_via_long_long_ptr, "No") 
    {
      foo2_long_long_ptr (ptr_long_long);
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (htext2__long_long_via_long_long_cptr, "No") 
    {
      foo2_long_long_cptr (&long_long_value);
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (htext2__long_long_ptr_via_long_long_cptr, "No") 
    {
      foo2_long_long_cptr (ptr_long_long);
    }
  }


  // -----------------------
  bogus += sink;

} // pass_action_no_size



///////////////
// End-Of-File
///////////////

