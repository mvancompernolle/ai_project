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
//  FILE     : t_contnr.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============


// ######### Container Performance Test Cases : BEGIN #########
// Alex Stepanov & Bjarne Stroustrup's slightly changed program


#include <stddef.h>	// some older implementations lack <cstddef>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>
#include <list>
#include <deque>
#include <set>

#include <iostream>
#include <iomanip>

typedef double element_t;

using namespace std;

void array_test(element_t* first, element_t* last)
{
  element_t* array = new element_t[last - first];
  copy(first, last, array);
  sort(array, array + (last - first));
  unique(array, array + (last - first));
  delete [] array;	  
}

void vector_pointer_test(element_t* first, element_t* last)
{ 
  vector<element_t> container(first, last);
    // &*container.begin() gets us a pointer to the first element
  sort(&*container.begin(), &*container.end());
  unique(&*container.begin(), &*container.end());
}

void vector_iterator_test(element_t* first, element_t* last)
{
  vector<element_t> container(first, last);
  sort(container.begin(), container.end());
  unique(container.begin(), container.end());
}

void deque_test(element_t* first, element_t* last)
{  
  // deque<element_t> container(first, last); CANNOT BE USED BECAUSE OF MVC++ 6
  deque<element_t> container(size_t(last - first), 0.0);
  copy(first, last, container.begin()); 
  sort(container.begin(), container.end());
  unique(container.begin(), container.end());
}
    
void list_test(element_t* first, element_t* last)
{ 
  list<element_t> container(first, last);
  container.sort();
  container.unique();
}
 
void set_test(element_t* first, element_t* last)
{
  set<element_t> container(first, last);
}

void multiset_test(element_t* first, element_t* last)
{
  multiset<element_t> container(first, last);
  typedef multiset<element_t>::iterator iterator;
  {
    iterator first = container.begin();
    iterator last = container.end();

    while (first != last) 
    {
      iterator next = first;
      if (++next == last) break;

      if (*first == *next) container.erase(next);
      else ++first;
     }
   }
}

void initialize(element_t* first, element_t* last)
{
  element_t value = 0.0;
  while (first != last) 
  {
    *first++ = value;
    value += 1.;
  }
}


// ######### Container Performance Test Cases : END ###########




// #########################################
// -------------------------------------------
#define SIZE0	10
#define SIZE1	100
#define SIZE2	1000

#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))

// -------------------------------------------

typedef unsigned int uint;


// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################




// =============================
void container_action (void)
{
SET_SIZES;

const string htext__array        ("array      ");
const string htext__vector_ptr   ("vector-ptr ");
const string htext__vector_iter  ("vector-iter");
const string htext__deque        ("deque      ");
const string htext__list         ("list       ");
const string htext__set          ("set        ");
const string htext__multiset     ("multiset   ");


  // -------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const size_t cur_size (v_sizes[i]);

    const size_t length = 2 * cur_size;

    vector<element_t> buf(length);
    element_t* buffer = &buf[0];
    element_t* buffer_end = &buf[length];
    initialize(buffer, buffer + cur_size);	// elements
    initialize(buffer + cur_size, buffer_end);	// duplicate elements
    random_shuffle(buffer, buffer_end);

    // ---------------------------
    {
      TURN_ON_DEFAULT_TIMER (htext__array, cur_size) 
      {
        array_test (buffer, buffer_end);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__vector_ptr, cur_size) 
      {
        vector_pointer_test (buffer, buffer_end);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__vector_iter, cur_size) 
      {
        vector_iterator_test (buffer, buffer_end);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__deque, cur_size) 
      {
        deque_test (buffer, buffer_end);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__list, cur_size) 
      {
        list_test (buffer, buffer_end);
      }
    }


    {
      TURN_ON_DEFAULT_TIMER (htext__set, cur_size) 
      {
        set_test (buffer, buffer_end);
      }
    }

    {
      TURN_ON_DEFAULT_TIMER (htext__multiset, cur_size) 
      {
        multiset_test (buffer, buffer_end);
      }
    }


  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // container_action


///////////////
// End-Of-File
///////////////

