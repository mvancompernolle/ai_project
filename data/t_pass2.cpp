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
//  FILE     : t_pass2.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
#include <list>
// ===============


// #########################################
// -------------------------------------------
#define SIZE0	10
#define SIZE1	100
#define SIZE2	1000

#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))

// -------------------------------------------

// -------------------------------------------
static int sink = 0;
#define FOO_BODY  ++sink


// #########################################
// -----------------
void foo_char         (char )                     { FOO_BODY; }
void foo_char_ref     (char )                     { FOO_BODY; }
void foo_char_ptr     (char* )                    { FOO_BODY; }
void foo_char_array   (char[] )                   { FOO_BODY; }

// -----------------
void foo_string_value (string )                   { FOO_BODY; }
void foo_string_ref   (const string& )            { FOO_BODY; }

void foo_vector_value (vector<char> )             { FOO_BODY; }
void foo_vector_ref   (const vector<char>& )      { FOO_BODY; }

void foo_list_value   (list<char> )               { FOO_BODY; }
void foo_list_ref     (const list<char>& )        { FOO_BODY; }

void foo_set_value    (set<size_t> )              { FOO_BODY; }
void foo_set_ref      (const set<size_t>& )       { FOO_BODY; }

void foo_map_value    (map<size_t, int> )         { FOO_BODY; }
void foo_map_ref      (const map<size_t, int>& )  { FOO_BODY; }
		      

// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################
// -----------------
static const string htext__char_ptr_via_char_ptr_two       ("ptr-to-char  via  ptr-to-char     ");
static const string htext__char_ptr_via_char_array         ("ptr-to-char  via  char-array      ");
static const string htext__char_array_via_char_ptr         ("char-array   via  ptr-to-char     ");
static const string htext__char_array_via_char_array       ("char-array   via  char-array      ");

static const string htext__string_via_string_value         ("string       via  string-value    ");
static const string htext__string_via_string_ref           ("string       via  ref-to-string   ");
static const string htext__char_ptr_via_string_value       ("ptr-to-char  via  string-value    ");
static const string htext__char_ptr_via_string_ref         ("ptr-to-char  via  ref-to-string   ");
static const string htext__char_array_via_string_value     ("char-array   via  string-value    ");
static const string htext__char_array_via_string_ref       ("char-array   via  ref-to-string   ");

static const string htext__vector_via_vector_value         ("vector       via  vector-value    ");
static const string htext__vector_via_vector_ref           ("vector       via  ref-to-vector   ");
static const string htext__list_via_list_value             ("list         via  list-value      ");
static const string htext__list_via_list_ref               ("list         via  ref-to-list     ");
static const string htext__set_via_set_value               ("set          via  set-value       ");
static const string htext__set_via_set_ref                 ("set          via  ref-to-set      ");
static const string htext__map_via_map_value               ("map          via  map-value       ");
static const string htext__map_via_map_ref                 ("map          via  ref-to-map      ");



// =============================
void pass_sized_action1 (void)
{
SET_SIZES;

  // -------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t cur_size	= v_sizes[i];
    string           str;
    vector<char>     vct;
    list<char>       lst;
    set<size_t>      st;
    map<size_t,int>  mp;

    int rand_value;
    char rand_ch_value;
    for (size_t j = 0; j < cur_size; j++) 
    {
      do 
      {
	rand_value = rand();
        rand_ch_value = static_cast<char>(rand_value);
      }	while (!isprint(rand_ch_value) || (mp.count(rand_value) == 2));

      str += static_cast<char> (rand_ch_value);
      vct.push_back (static_cast<char> (rand_ch_value));
      lst.push_back (static_cast<char> (rand_ch_value));

      assert (st.count(rand_value) == 0);
      st.insert(rand_value);
      assert (mp.count(rand_value) == 0);
      mp[rand_value] = j;
    }
    char*  pcstr = const_cast<char*> (str.c_str());

///////////////
#ifdef __GNUC__
///////////////
    char acstr [cur_size + 2];
    strcpy (acstr, pcstr);
//////
#endif
//////

    assert (str.size() == cur_size);
    assert (strlen (pcstr) == cur_size);
    assert (string (pcstr) == str);
///////////////
#ifdef __GNUC__
///////////////
    assert (strlen(acstr) == cur_size);
    assert (strcmp (acstr, pcstr) == 0);
    assert (string (acstr) == str);
//////
#endif
//////

    assert (vct.size() == cur_size);
    assert (lst.size() == cur_size);
    assert (st.size() == cur_size);
    assert (mp.size() == cur_size);


    // ----------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__char_ptr_via_char_ptr_two, cur_size) 
      {
        foo_char_ptr (pcstr);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__char_ptr_via_char_array, cur_size) 
      {
        foo_char_array (pcstr);
      }
    }

///////////////
#ifdef __GNUC__
///////////////
    {
      TURN_ON_DEFAULT_TIMER (htext__char_array_via_char_ptr, cur_size) 
      {
        foo_char_ptr (acstr);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__char_array_via_char_array, cur_size) 
      {
        foo_char_array (acstr);
      }
    }
//////
#endif
//////

    // ----------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__string_via_string_ref, cur_size) 
      {
        foo_string_ref (str);
      }
    }


    // ----------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__vector_via_vector_ref, cur_size) 
      {
        foo_vector_ref (vct);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__list_via_list_ref, cur_size) 
      {
        foo_list_ref (lst);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__set_via_set_ref, cur_size) 
      {
        foo_set_ref (st);
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__map_via_map_ref, cur_size) 
      {
        foo_map_ref (mp);
      }
    }


  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // pass_sized_action1


// =============================
void pass_sized_action2 (void)
{
SET_SIZES;

  // -------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t cur_size	= v_sizes[i];
    string           str;
    vector<char>     vct;
    list<char>       lst;
    set<size_t>      st;
    map<size_t,int>  mp;

    int rand_value;
    char rand_ch_value;
    for (size_t j = 0; j < cur_size; j++) 
    {
      do 
      {
	rand_value = rand();
        rand_ch_value = static_cast<char>(rand_value);
      }	while (!isprint(rand_ch_value) || (mp.count(rand_value) == 2));

      str += static_cast<char> (rand_ch_value);
      vct.push_back (static_cast<char> (rand_ch_value));
      lst.push_back (static_cast<char> (rand_ch_value));

      assert (st.count(rand_value) == 0);
      st.insert(rand_value);
      assert (mp.count(rand_value) == 0);
      mp[rand_value] = j;
    }
    char*  pcstr = const_cast<char*> (str.c_str());

///////////////
#ifdef __GNUC__
///////////////
    char acstr [cur_size + 2];
    strcpy (acstr, pcstr);
//////
#endif
//////

    assert (str.size() == cur_size);
    assert (strlen (pcstr) == cur_size);
    assert (string (pcstr) == str);
///////////////
#ifdef __GNUC__
///////////////
    assert (strlen(acstr) == cur_size);
    assert (strcmp (acstr, pcstr) == 0);
    assert (string (acstr) == str);
//////
#endif
//////

    assert (vct.size() == cur_size);
    assert (lst.size() == cur_size);
    assert (st.size() == cur_size);
    assert (mp.size() == cur_size);


    // ----------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__string_via_string_value, cur_size) 
      {
        foo_string_value (str);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__char_ptr_via_string_ref, cur_size) 
      {
        foo_string_ref (pcstr);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__char_ptr_via_string_value, cur_size) 
      {
        foo_string_value (pcstr);
      }
    }

///////////////
#ifdef __GNUC__
///////////////
    {
      TURN_ON_DEFAULT_TIMER (htext__char_array_via_string_ref, cur_size) 
      {
        foo_string_ref (acstr);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__char_array_via_string_value, cur_size) 
      {
        foo_string_value (acstr);
      }
    }

//////
#endif
//////

    // ----------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__vector_via_vector_value, cur_size) 
      {
        foo_vector_value (vct);
      }
    }

  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // pass_sized_action2



// =============================
void pass_sized_action3 (void)
{
SET_SIZES;

  // -------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t cur_size	= v_sizes[i];
    string           str;
    vector<char>     vct;
    list<char>       lst;
    set<size_t>      st;
    map<size_t,int>  mp;

    int rand_value;
    char rand_ch_value;
    for (size_t j = 0; j < cur_size; j++) 
    {
      do 
      {
	rand_value = rand();
        rand_ch_value = static_cast<char>(rand_value);
      }	while (!isprint(rand_ch_value) || (mp.count(rand_value) == 2));

      str += static_cast<char> (rand_ch_value);
      vct.push_back (static_cast<char> (rand_ch_value));
      lst.push_back (static_cast<char> (rand_ch_value));

      assert (st.count(rand_value) == 0);
      st.insert(rand_value);
      assert (mp.count(rand_value) == 0);
      mp[rand_value] = j;
    }
    char*  pcstr = const_cast<char*> (str.c_str());

///////////////
#ifdef __GNUC__
///////////////
    char acstr [cur_size + 2];
    strcpy (acstr, pcstr);
//////
#endif
//////

    assert (str.size() == cur_size);
    assert (strlen (pcstr) == cur_size);
    assert (string (pcstr) == str);
///////////////
#ifdef __GNUC__
///////////////
    assert (strlen(acstr) == cur_size);
    assert (strcmp (acstr, pcstr) == 0);
    assert (string (acstr) == str);
//////
#endif
//////

    assert (vct.size() == cur_size);
    assert (lst.size() == cur_size);
    assert (st.size() == cur_size);
    assert (mp.size() == cur_size);


    // ----------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__list_via_list_value, cur_size) 
      {
        foo_list_value (lst);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__set_via_set_value, cur_size) 
      {
        foo_set_value (st);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__map_via_map_value, cur_size) 
      {
        foo_map_value (mp);
      }
    }

  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // pass_sized_action3



///////////////
// End-Of-File
///////////////

