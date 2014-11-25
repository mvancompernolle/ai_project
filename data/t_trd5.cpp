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
//  FILE     : t_trd5.cpp
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
// D.5 Measuring the Cost of Synchronized I/O
// 
// Test program to
// (1) compare the performance of classic iostreams,
//     standard iostreams, and C-style stdio for output, and
// (2) test any overhead of sync_with_stdio(true). 
// Standard iostreams by default are synchronized with stdio streams;
// the opposite was true of classic iostreams.
// optional command line argument:
// - how many numbers to output (default 1,000,000)
// - name of output file (default cout)
// When compiling, define CLASSIC or STDIO to enable
// those options; otherwise the default is to use
// standard iostreams.
// 
// </QUOTE>
// 
// #########################################


// ###############################################
//
// Note. The original tests were slightly changed
//
//    Alex Vinokur
//
// ###############################################


// #########################################

#define SIZE0	100
#define SIZE1	1000
#define SIZE2	10000

#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))



#define TMP_OUT_FILE	"tmp.out"


// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void tech_report_d5_action (void)
{
SET_SIZES;

const string htext__dec_int_to_stdio        ("integers to stdio                         ");
const string htext__hex_int_to_stdio        ("hex integers to stdio                     ");

const string htext__dec_int_to_stand_false  ("integers to standard IO     (sync = false)");
const string htext__dec_int_to_stand_true   ("integers to standard IO     (sync = true) ");
const string htext__hex_int_to_stand_false  ("hex integers to standard IO (sync = false)");
const string htext__hex_int_to_stand_true   ("hex integers to standard IO (sync = true) ");

int i;
int rc;

  // -------------------------------------
  for (size_t k = 0; k < v_sizes.size(); k++)
  {
    const int cur_size (v_sizes[k]);

    // seed the random number generator
    srand (clock());

    // -------------------------------
    {
      FILE * target;
      target = fopen (TMP_OUT_FILE, "w");
      assert (!(target == NULL));

      TURN_ON_DEFAULT_TIMER (htext__dec_int_to_stdio, cur_size) 
      {
        for (i = 0; i != cur_size; ++i) fprintf ( target, "%d ", i);
      }

      rc = fclose (target);
      assert (rc == 0);
      remove (TMP_OUT_FILE);
    }

    {
      FILE * target;
      target = fopen (TMP_OUT_FILE, "w");
      assert (!(target == NULL));

      TURN_ON_DEFAULT_TIMER (htext__hex_int_to_stdio, cur_size) 
      {
        for (i = 0; i != cur_size; ++i) fprintf ( target, "%x ", i);
      }

      rc = fclose (target);
      assert (rc == 0);

      remove (TMP_OUT_FILE);
    }


    {
      ofstream target (TMP_OUT_FILE);
      assert (target);
      assert (target.is_open());

      target.sync_with_stdio (false);
      target << dec;

      TURN_ON_DEFAULT_TIMER (htext__dec_int_to_stand_false, cur_size) 
      {
        for (i = 0; i != cur_size; ++i) target << i << ' ';
      }

      target.close();
      assert (!target.is_open());
      remove (TMP_OUT_FILE);
    }


    {
      ofstream target (TMP_OUT_FILE);
      assert (target);
      assert (target.is_open());

      target.sync_with_stdio (false);
      target << hex;

      TURN_ON_DEFAULT_TIMER (htext__hex_int_to_stand_false, cur_size) 
      {
        for (i = 0; i != cur_size; ++i) target << i << ' ';
      }

      target.close();
      assert (!target.is_open());
      remove (TMP_OUT_FILE);
    }


    {
      ofstream target (TMP_OUT_FILE);
      assert (target);
      assert (target.is_open());

      target.sync_with_stdio (true);
      target << dec;

      TURN_ON_DEFAULT_TIMER (htext__dec_int_to_stand_true, cur_size) 
      {
        for (i = 0; i != cur_size; ++i) target << i << ' ';
      }

      target.close();
      assert (!target.is_open());
      remove (TMP_OUT_FILE);
    }


    {
      ofstream target (TMP_OUT_FILE);
      assert (target);
      assert (target.is_open());

      target.sync_with_stdio (true);
      target << hex;

      TURN_ON_DEFAULT_TIMER (htext__hex_int_to_stand_true, cur_size) 
      {
        for (i = 0; i != cur_size; ++i) target << i << ' ';
      }

      target.close();
      assert (!target.is_open());
      remove (TMP_OUT_FILE);
    }


  } // for (size_t k = 0; k < v_sizes.size(); k++)


} // tech_report_d5_action


///////////////
// End-Of-File
///////////////

