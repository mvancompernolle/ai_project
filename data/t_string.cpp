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
//  FILE     : t_string.cpp
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
#define CSTR0	"12345"
#define CSTR1	CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0 CSTR0
#define CSTR2	CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1 CSTR1
#define CSTR3	CSTR2 CSTR2 CSTR2 CSTR2 CSTR2 CSTR2 CSTR2 CSTR2 CSTR2 CSTR2


#define SET_STRS  \
	const char*          a_cstrs[] = {CSTR0, CSTR1, CSTR2}; \
	const vector<string> v_strs (a_cstrs, a_cstrs + sizeof(a_cstrs)/sizeof(*a_cstrs))

#define SET_STRS_AND_LEN     SET_STRS; \
	const size_t         max_str_len (v_strs.back().size())
// -------------------------------------------


// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void string_init1 (void)
{
SET_STRS;

const string htext__ptr_char               ("init char*            ");
const string htext__char_array             ("init char[]           ");


  // -------------------------------
  for (size_t i = 0; i < v_strs.size(); i++)
  {
    char* cstr			= const_cast<char*> (v_strs[i].c_str());
    const size_t cstr_size	= string (cstr).size();

    assert (v_strs[i].size() == strlen (cstr));

    // -----------------------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__ptr_char, cstr_size) 
      {
        char* dummy = cstr;
      }
    }


    {
      switch (i)
      {
        case 0 :
          {
            TURN_ON_DEFAULT_TIMER (htext__char_array, string(CSTR0).size()) 
            {
              char dummy[] = CSTR0;
            }
	  }
          break;

        case 1 :
          {
            TURN_ON_DEFAULT_TIMER (htext__char_array, string(CSTR1).size()) 
            {
              char dummy[] = CSTR1;
            }
          }
          break;

        case 2 :
          {
            TURN_ON_DEFAULT_TIMER (htext__char_array, string(CSTR2).size()) 
            {
              char dummy[] = CSTR2;
            }
          }
          break;

	default :
          assert (0);
          break;
      }
    }

  } // for (size_t i = 0; i < v_strs.size(); i++)


} // string_init1



// =============================
void string_init2 (void)
{
SET_STRS;

const string htext__malloc_and_free        ("malloc & free         ");
const string htext__malloc_strcpy_and_free ("(malloc+strcpy) & free");
const string htext__malloc_memset_and_free ("(malloc+memset) & free");
const string htext__ctor_and_dtor          ("ctor & dtor           ");
const string htext__new_and_delete         ("new & delete          ");

  // -------------------------------
  for (size_t i = 0; i < v_strs.size(); i++)
  {
    char* cstr			= const_cast<char*> (v_strs[i].c_str());
    const size_t cstr_size	= string (cstr).size();
    const size_t cstr_size1	= cstr_size + 1;

    assert (v_strs[i].size() == strlen (cstr));


    {
      TURN_ON_DEFAULT_TIMER (htext__malloc_and_free, cstr_size) 
      {
        free (malloc (cstr_size));
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__malloc_strcpy_and_free, cstr_size) 
      {
        char* dummy = (char*) malloc (cstr_size1);
        strcpy (dummy, cstr);
        free (dummy);
      }
    }

  
    {
      TURN_ON_DEFAULT_TIMER (htext__malloc_memset_and_free, cstr_size) 
      {
        char* dummy = (char*) malloc (cstr_size);
        memset (dummy, 0, cstr_size);
        free (dummy);
      }
    }
  

    {
      TURN_ON_DEFAULT_TIMER (htext__ctor_and_dtor, cstr_size) 
      {
        string (cstr);
      }
    }    

    {
      TURN_ON_DEFAULT_TIMER (htext__new_and_delete, cstr_size) 
      {
        delete (new string (cstr));
      }
    }    
  
  } // for (size_t i = 0; i < v_strs.size(); i++)


} // string_init2



// =============================
void string_treat1 (void)
{
SET_STRS_AND_LEN;

string sstore;

#define INCREASE_TIMES 100
const size_t malloc_size = INCREASE_TIMES * max_str_len + 1;
const size_t erase_index = malloc_size - max_str_len - 1;
  assert (erase_index + max_str_len < malloc_size);

char* cstore = (char*) malloc (malloc_size);
  assert (!(cstore == NULL));

const string htext__strcpy                                ("strcpy                ");
const string htext__assignation_operator                  ("operator=             ");
const string htext__strcat                                ("strcat                ");
const string htext__plus_assignation_operator             ("operator+=            ");
const string htext__plus_operator                         ("operator+             ");


  // -------------------------------
  for (size_t i = 0; i < v_strs.size(); i++)
  {
    string str (v_strs[i]);
    char* cstr			= const_cast<char*> (str.c_str());
    const size_t cstr_size	= string (cstr).size();
    assert (str.size() == cstr_size);

    assert (str.size() == strlen (cstr));

    {
      TURN_ON_DEFAULT_TIMER (htext__strcpy, cstr_size) 
      {
        strcpy (cstore, cstr);
      }
      assert (strlen(cstr) <= malloc_size);
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__assignation_operator, cstr_size) 
      {
        sstore = str;
      }
    }


    {
      cstore[0] = 0; 
      TURN_ON_DEFAULT_TIMER (htext__strcat, cstr_size) 
      {
        strcat (cstore, cstr);
        cstore [0] = 0;
      }
      assert (strlen(cstr) <= malloc_size);
    }


    {
      sstore.erase();
      TURN_ON_DEFAULT_TIMER (htext__plus_assignation_operator, cstr_size) 
      {
        sstore += str;
        sstore.erase();
      }
    }


    {
      sstore.erase();
      TURN_ON_DEFAULT_TIMER (htext__plus_operator, cstr_size) 
      {
        sstore = sstore + str;
        sstore.erase();
      }
    }

  } // for (size_t i = 0; i < v_strs.size(); i++)


} // string_treat1



// =============================
void string_treat2 (void)
{
SET_STRS_AND_LEN;

string sstore;

#define INCREASE_TIMES 100
const size_t malloc_size = INCREASE_TIMES * max_str_len + 1;
const size_t erase_index = malloc_size - max_str_len - 1;
  assert (erase_index + max_str_len < malloc_size);

char* cstore = (char*) malloc (malloc_size);
  assert (!(cstore == NULL));

const string htext__accumulated_strcat                    ("accumulated strcat    ");
const string htext__accumulated_plus_assignation_operator ("accumulated operator+=");
const string htext__accumulated_plus_operator             ("accumulated operator+ ");



  // -------------------------------
  for (size_t i = 0; i < v_strs.size(); i++)
  {
    string str (v_strs[i]);
    char* cstr			= const_cast<char*> (str.c_str());
    const size_t cstr_size	= string (cstr).size();
    assert (str.size() == cstr_size);

    assert (str.size() == strlen (cstr));


    {
      cstore[0] = 0; 
      TURN_ON_DEFAULT_TIMER (htext__accumulated_strcat, cstr_size) 
      {
        strcat (cstore, cstr);
        cstore [erase_index] = 0;
      }
    }


    {
      sstore.erase();
      TURN_ON_DEFAULT_TIMER (htext__accumulated_plus_assignation_operator, cstr_size) 
      {
        sstore += str;
        sstore.erase((sstore.size() > erase_index) ? erase_index : sstore.size());
      }
    }

    {
      sstore.erase();
      TURN_ON_DEFAULT_TIMER (htext__accumulated_plus_operator, cstr_size) 
      {
        sstore = sstore + str;
        sstore.erase((sstore.size() > erase_index) ? erase_index : sstore.size());
      }
    }
  } // for (size_t i = 0; i < v_strs.size(); i++)


} // string_treat2




// =============================
void str_mem_treat (void)
{
SET_STRS_AND_LEN;

#define INCREASE_TIMES 100
const size_t malloc_size = INCREASE_TIMES * max_str_len + 1;
const size_t erase_index = malloc_size - max_str_len - 1;
  assert (erase_index + max_str_len < malloc_size);

char* cstore = (char*) malloc (malloc_size);
  assert (!(cstore == NULL));

const string htext__strcpy    ("strcpy ");
const string htext__strcat    ("strcat ");
const string htext__memcpy    ("memcpy ");
const string htext__memmove   ("memmove");
const string htext__memset    ("memset ");


  // -------------------------------
  for (size_t i = 0; i < v_strs.size(); i++)
  {
    string str (v_strs[i]);
    char* cstr			= const_cast<char*> (str.c_str());
    const size_t cstr_size	= string (cstr).size();
    assert (str.size() == cstr_size);

    assert (str.size() == strlen (cstr));


    {
      TURN_ON_DEFAULT_TIMER (htext__strcpy, cstr_size) 
      {
        strcpy (cstore, cstr);
      }
      assert (strlen(cstr) <= strlen(cstore));
    }


    {
      cstore[0] = 0; 
      TURN_ON_DEFAULT_TIMER (htext__strcat, cstr_size) 
      {
        strcat (cstore, cstr);
        cstore [0] = 0;
      }
      assert (strlen(cstr) <= malloc_size);
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__memcpy, cstr_size) 
      {
        memcpy (cstore, cstr, cstr_size);
      }
      assert (strlen(cstr) <= malloc_size);
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__memmove, cstr_size) 
      {
        memmove (cstore, cstr, cstr_size);
      }
      assert (strlen(cstr) <= malloc_size);
    }


    {
      char ch = 'a';
      TURN_ON_DEFAULT_TIMER (htext__memset, cstr_size) 
      {
        memset (cstore, ch, cstr_size);
      }
      assert (strlen(cstr) <= malloc_size);
    }



  } // for (size_t i = 0; i < v_strs.size(); i++)


} // str_mem_treat




// =============================
void string_len (void)
{
SET_STRS;

const string htext__strlen            ("strlen       ");
const string htext__string_size       ("string.size()");


  // -------------------------------
  for (size_t i = 0; i < v_strs.size(); i++)
  {
    string str (v_strs[i]);
    char* cstr			= const_cast<char*> (str.c_str());
    const size_t cstr_size	= string (cstr).size();

    assert (str.size() == strlen (cstr));

    {
      TURN_ON_DEFAULT_TIMER (htext__strlen, cstr_size) 
      {
        strlen (cstr);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__string_size, cstr_size) 
      {
        str.size();
      }
    }
  
  } // for (size_t i = 0; i < v_strs.size(); i++)


} // string_len



///////////////
// End-Of-File
///////////////

