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
//  FILE     : t_trd4.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============

// #########################################
// Technical Report on C++ Performance
// (ISO/IEC PDTR 18015; Date: 2003-08-11; WG21 N1487=03-0070) :
// http://std.dkuug.dk/JTC1/SC22/WG21/docs/PDTR18015.pdf
// http://anubis.dkuug.dk/jtc1/sc22/wg21/docs/papers/2003/n1487.pdf
// 
// Appendix D: Timing Code
// 
// <QUOTE>
// D.4 Comparing Function Objects to Function Pointers
// =============================================================================
// This is a program to measure the relative efficiency of qsort vs std::sort
// and of function objects vs function pointers.
//
// Optional Arguments: number of iterations to repeat
// size of array of doubles to sort
// name of output file
//
// In all cases, an array of doubles is filled with random numbers.
// This array is sorted in ascending order, then the same random numbers are
// reloaded into the array and sorted again. Repeat ad libitum.
//
//
// What is measured:
// These measurements operate on an array of doubles
// 1. Using qsort + user-defined comparison function to sort array
// 2. Using std::sort + a function pointer (not a function object)
// 3. Using std::sort + user-defined function object, out-of-line code
// 4. Using std::sort + user-defined function object, inline code
// 5. Using std::sort + std::less
// 6. Using std::sort + native operator <
//
// These measurements operate on an std::vector of doubles
// instead of a primitive array
//
// 7. Using std::sort + std::less
// 8. Using std::sort + native operator <
// 9. Using std::sort + function pointer from test 2
//
//
// Since qsort's comparison function must return int (less than 0, 0, greater than 0)
// and std::sort's must return a bool, it is not possible to test them with each
// other's comparator.
// =============================================================================
// </QUOTE>
// 
// #########################################


// ###############################################
//
// Note. Also testing stable_sort has been added
//
//    Alex Vinokur
//
// ###############################################


// #########################################
// --------- helper functions --------------

// qsort passes void * arguments to its comparison function,
// which must return negative, 0, or positive value
int less_than_function1( const void * lhs, const void * rhs )
{
int retcode = 0;
  if( *(const double *) lhs < *(const double *) rhs ) retcode = -1;
  if( *(const double *) lhs > *(const double *) rhs ) retcode = 1;
  return retcode;
}

// std::sort, on the other hand, needs a comparator that returns true or false
bool less_than_function2 (const double lhs, const double rhs)
{
  if (lhs < rhs) return true;
  else           return false;
}

// the comparison operator in the following functor is defined out of line
struct less_than_functor
{
  bool operator() (const double& lhs, const double& rhs) const;
};

bool less_than_functor::operator() (const double& lhs, const double& rhs) const
{
  return (lhs < rhs? true : false);
}

// the comparison operator in the following functor is defined inline
struct inline_less_than_functor
{
  bool operator() (const double& lhs, const double& rhs) const
  {
    return (lhs < rhs? true : false);
  }
};



// #########################################

#define SIZE0	100
#define SIZE1	1000
#define SIZE2	10000


#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void tech_report_d4_action (void)
{
SET_SIZES;

const string htext__no_sort_only_copy                                ("no sort, only copy                                 ");
const string htext__qsort_array_comparison_function1                 ("qsort array with comparison function1              ");

const string htext__sort_array_function_pointer                      ("sort array with function pointer                   ");
const string htext__sort_array_user_supplied_functor                 ("sort array with user-supplied functor              ");
const string htext__sort_array_user_supplied_inline_functor          ("sort array with user-supplied inline functor       ");
const string htext__sort_array_standard_functor                      ("sort array with standard functor                   ");
const string htext__sort_array_native_less_operator                  ("sort array with native < operator                  ");
const string htext__sort_vector_iterator_standard_functor            ("sort vector iterator with standard functor         ");
const string htext__sort_vector_iterator_native_less_operator        ("sort vector iterator with native < operator        ");
const string htext__sort_vector_iterator_function_pointer            ("sort vector iterator with function pointer         ");
const string htext__sort_vector_pointer_standard_functor             ("sort vector pointer  with standard functor         ");
const string htext__sort_vector_pointer_native_less_operator         ("sort vector pointer  with native < operator        ");
const string htext__sort_vector_pointer_function_pointer             ("sort vector pointer  with function pointer         ");

const string htext__stable_sort_array_function_pointer               ("stable_sort array with function pointer            ");
const string htext__stable_sort_array_user_supplied_functor          ("stable_sort array with user-supplied functor       ");
const string htext__stable_sort_array_user_supplied_inline_functor   ("stable_sort array with user-supplied inline functor");
const string htext__stable_sort_array_standard_functor               ("stable_sort array with standard functor            ");
const string htext__stable_sort_array_native_less_operator           ("stable_sort array with native < operator           ");
const string htext__stable_sort_vector_iterator_standard_functor     ("stable_sort vector iterator with standard functor  ");
const string htext__stable_sort_vector_iterator_native_less_operator ("stable_sort vector iterator with native < operator ");
const string htext__stable_sort_vector_iterator_function_pointer     ("stable_sort vector iterator with function pointer  ");
const string htext__stable_sort_vector_pointer_standard_functor      ("stable_sort vector pointer  with standard functor  ");
const string htext__stable_sort_vector_pointer_native_less_operator  ("stable_sort vector pointer  with native < operator ");
const string htext__stable_sort_vector_pointer_function_pointer      ("stable_sort vector pointer  with function pointer  ");

  // -------------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    const int tablesize (v_sizes[i]);

    // seed the random number generator
    srand (clock());

    // --------------------
    // initialize the table to sort. we use the same table for all tests,
    // in case one randomly-generated table might require more work than
    // another to sort
    double * master_table = new double[tablesize];
    for( int n = 0; n < tablesize; ++n )
    {
      master_table[n] = double (rand());
    }

    double * table = new double[tablesize]; // working copy


    // -------------------------------
    {
      // TEST 0: no sort, only copy
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__no_sort_only_copy, tablesize) 
      {
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // TEST 1: qsort array with a C-style comparison function
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__qsort_array_comparison_function1, tablesize) 
      {
        qsort (table, tablesize, sizeof(double), less_than_function1);
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // TEST 2: std::sort array with function pointer
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__sort_array_function_pointer, tablesize) 
      {
        sort (table, table + tablesize, less_than_function2);
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // TEST 3: std::sort array with out-of-line functor
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__sort_array_user_supplied_functor, tablesize) 
      {
        sort (table, table + tablesize, less_than_functor());
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // TEST 4: std::sort array with inline functor
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__sort_array_user_supplied_inline_functor, tablesize) 
      {
        sort (table, table + tablesize, inline_less_than_functor());
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // TEST 5: std::sort array with std::<less> functor
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__sort_array_standard_functor, tablesize) 
      {
        sort (table, table + tablesize, less<double>());
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // TEST 6: std::sort array using native operator <
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__sort_array_native_less_operator, tablesize) 
      {
        sort (table, table + tablesize);
        copy (master_table, master_table + tablesize, table);
      }
    }

vector<double> v_table(master_table, master_table + tablesize);
    {
      // TEST 7: std::sort vector using iterator with std::less functor,
      // on a vector rather than primitive array
      TURN_ON_DEFAULT_TIMER (htext__sort_vector_iterator_standard_functor, tablesize) 
      {
        sort (v_table.begin(), v_table.end(), less<double>());
        copy (master_table, master_table + tablesize, v_table.begin());
      }
    }

    {
      // TEST 8: std::sort vector using iterator using native operator <
      v_table.assign( master_table, master_table + tablesize );
      TURN_ON_DEFAULT_TIMER (htext__sort_vector_iterator_native_less_operator, tablesize) 
      {
        sort (v_table.begin(), v_table.end());
        copy (master_table, master_table + tablesize, v_table.begin());
      }
    }

    {
      // TEST 9: std::sort vector using iterator using function pointer from test 2
      v_table.assign( master_table, master_table + tablesize );
      TURN_ON_DEFAULT_TIMER (htext__sort_vector_iterator_function_pointer, tablesize) 
      {
        sort (v_table.begin(), v_table.end(), less_than_function2);
        copy (master_table, master_table + tablesize, v_table.begin());
      }
    }



    {
      // TEST 10: std::sort vector using pointer with std::less functor,
      // on a vector rather than primitive array
      TURN_ON_DEFAULT_TIMER (htext__sort_vector_pointer_standard_functor, tablesize) 
      {
        sort (&v_table[0], &v_table[0] + tablesize, less<double>());
        copy (master_table, master_table + tablesize, v_table.begin());
      }
    }

    {
      // TEST 11: std::sort vector using pointer using native operator <
      v_table.assign( master_table, master_table + tablesize );
      TURN_ON_DEFAULT_TIMER (htext__sort_vector_pointer_native_less_operator, tablesize) 
      {
        sort (&v_table[0], &v_table[0] + tablesize);
        copy (master_table, master_table + tablesize, v_table.begin());
      }
    }

    {
      // TEST 12: std::sort vector using pointer using function pointer from test 2
      v_table.assign( master_table, master_table + tablesize );
      TURN_ON_DEFAULT_TIMER (htext__sort_vector_pointer_function_pointer, tablesize) 
      {
        sort (&v_table[0], &v_table[0] + tablesize, less_than_function2);
        copy (master_table, master_table + tablesize, v_table.begin());
      }
    }

#ifndef _MSC_VER
    // --------- stable sort ---------

    {
      // STABLE_TEST 2: std::stable_sort with function pointer
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_array_function_pointer, tablesize) 
      {
        stable_sort (table, table + tablesize, less_than_function2);
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // STABLE_TEST 3: std::stable_sort with out-of-line functor
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_array_user_supplied_functor, tablesize) 
      {
        stable_sort (table, table + tablesize, less_than_functor());
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // STABLE_TEST 4: std::stable_sort with inline functor
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_array_user_supplied_inline_functor, tablesize) 
      {
        stable_sort (table, table + tablesize, inline_less_than_functor());
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // STABLE_TEST 5: std::stable_sort with std::<less> functor
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_array_standard_functor, tablesize) 
      {
        stable_sort (table, table + tablesize, less<double>());
        copy (master_table, master_table + tablesize, table);
      }
    }

    {
      // STABLE_TEST 6: std::stable_sort using native operator <
      copy(master_table, master_table + tablesize, table);
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_array_native_less_operator, tablesize) 
      {
        stable_sort (table, table + tablesize);
        copy (master_table, master_table + tablesize, table);
      }
    }

vector<double> v_stable_table(master_table, master_table + tablesize);
    {
      // STABLE_TEST 7: std::stable_sort vector using iterator with std::less functor,
      // on a vector rather than primitive array
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_vector_iterator_standard_functor, tablesize) 
      {
        stable_sort (v_stable_table.begin(), v_stable_table.end(), less<double>());
        copy (master_table, master_table + tablesize, v_stable_table.begin());
      }
    }

    {
      // STABLE_TEST 8: std::stable_sort vector using iterator using native operator <
      v_stable_table.assign( master_table, master_table + tablesize );
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_vector_iterator_native_less_operator, tablesize) 
      {
        stable_sort (v_stable_table.begin(), v_stable_table.end());
        copy (master_table, master_table + tablesize, v_stable_table.begin());
      }
    }

    {
      // STABLE_TEST 9: std::stable_sort vector using iterator using function pointer from test 2
      v_stable_table.assign( master_table, master_table + tablesize );
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_vector_iterator_function_pointer, tablesize) 
      {
        stable_sort (v_stable_table.begin(), v_stable_table.end(), less_than_function2);
        copy (master_table, master_table + tablesize, v_stable_table.begin());
      }
    }


    {
      // STABLE_TEST 10: std::stable_sort vector using pointer with std::less functor,
      // on a vector rather than primitive array
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_vector_pointer_standard_functor, tablesize) 
      {
        stable_sort (&v_table[0], &v_table[0] + tablesize, less<double>());
        copy (master_table, master_table + tablesize, v_stable_table.begin());
      }
    }

    {
      // STABLE_TEST 11: std::stable_sort vector using pointer using native operator <
      v_stable_table.assign( master_table, master_table + tablesize );
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_vector_pointer_native_less_operator, tablesize) 
      {
        stable_sort (&v_table[0], &v_table[0] + tablesize);
        copy (master_table, master_table + tablesize, v_stable_table.begin());
      }
    }

    {
      // STABLE_TEST 12: std::stable_sort vector using pointer using function pointer from test 2
      v_stable_table.assign( master_table, master_table + tablesize );
      TURN_ON_DEFAULT_TIMER (htext__stable_sort_vector_pointer_function_pointer, tablesize) 
      {
        stable_sort (&v_table[0], &v_table[0] + tablesize, less_than_function2);
        copy (master_table, master_table + tablesize, v_stable_table.begin());
      }
    }


#endif // _MSC_VER

  } // for (size_t i = 0; i < v_sizes.size(); i++)


} // tech_report_d4_action


///////////////
// End-Of-File
///////////////

