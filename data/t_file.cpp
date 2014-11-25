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
//  FILE     : t_file.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"

#if ((defined UNIX_ENV) && !(defined DJGPP_ENV))
// -------------------
#include <fcntl.h>
#include <sys/mman.h>
// -------------------
#endif



// #########################################
// ----------------------------------------
// Functions ReadFile1 - ReadFile6 are 
//    lightly changed functions from the arctile :
// --------
// From: "Tom Hines" <tom_hines@yahoo.com>
// Subject: Re: How to efficiently read contents from file into one string?
// Newsgroup: comp.lang.c++.moderated
// Date: Thursday, January 30, 2003 3:55 PM
// ----------------------------------------


// ---------
static void ReadFile1 (const string& filename_i, string& str_o)
{
  str_o.erase();

  // --- Using getline() ---

string line;
ifstream infile (filename_i.c_str());
  assert (infile.is_open());

  while (getline (infile, line))
  {
    str_o.append(line);
    line.erase();
    if (!infile.eof()) str_o.append("\n");
  }
}


// ---------
static void ReadFile2(const string& filename_i, string& str_o)
{
  str_o.erase();

  // --- Using vector char at a time ---

ifstream infile (filename_i.c_str());
  assert (infile.is_open());

vector<char> v;
char ch;

  while (infile.get(ch)) v.push_back(ch);

  str_o = v.empty() ? string() : string (v.begin(), v.end());

}

// ---------
static void ReadFile3(const string& filename_i, string& str_o)
{
  str_o.erase();

  // --- Using string, char at a time ---

ifstream infile (filename_i.c_str(), ios::in | ios::ate);
  assert (infile.is_open());

streampos sz = infile.tellg();

 infile.seekg(0, ios::beg);

 str_o.resize (sz, '0');

char ch;
int  i = 0;
  while (infile.get(ch)) str_o[i++] = ch;

  str_o.erase (i);

}

// ---------
static void ReadFile4(const string& filename_i, string& str_o)
{
  str_o.erase();

  // --- Using vector, reading whole file at once ---

ifstream infile (filename_i.c_str(), ios::in | ios::ate);
  assert (infile.is_open());

streampos sz = infile.tellg();

  infile.seekg(0, ios::beg);

vector<char> v(sz);
  infile.read(&v[0], sz);
  // v.erase(remove(v.begin(), v.end(), '\n'), v.end());

  str_o = v.empty() ? string() : string (v.begin(), v.end());
}


////////////////////////////////////////////////
#if ((defined UNIX_ENV) && !(defined DJGPP_ENV))
////////////////////////////////////////////////
// ---------
static void ReadFile5(const string& filename_i, string& str_o)
{
  str_o.erase();

  // --- Using mmap ---
int fd = open(filename_i.c_str(), O_RDONLY);
  assert (fd > 2);

off_t sz = lseek(fd, 0, SEEK_END);
char* ptr = (char*)mmap(0, sz, PROT_READ, MAP_SHARED, fd, 0);

  if (ptr == MAP_FAILED)
  {
    close(fd);
    return;
  }

  assert (ptr != MAP_FAILED);

  str_o = string(ptr, ptr + sz);
  munmap(ptr, sz);

  // str.erase(remove(str.begin(), str.end(), '\n'), str.end());
  // str.erase(remove(str.begin(), str.end(), '\r'), str.end());

  close(fd);
}
//////
#endif
//////

// ---------
static void ReadFile6 (const string& filename_i, string& str_o)
{
  str_o.erase();

  // --- Using iterator ---
ifstream infile (filename_i.c_str());
  assert (infile.is_open());

  infile >> noskipws;

istream_iterator<char> iter(infile), eos;
 str_o = string(iter, eos);

  // str.erase(remove(str.begin(), str.end(), '\n'), str.end());

}


// ---------
static void ReadFile7 (const string& filename_i, string& str_o)
{
  str_o.erase();

  // --- Using rdbuf() ---

string line;
ifstream infile (filename_i.c_str());
  assert (infile.is_open());

ostringstream oss;
  oss << infile.rdbuf();
  str_o = oss.str();
}



// #########################################
// -------------------------------------------
#define SIZE0	0
#define SIZE1	10
#define SIZE2	100
#define SIZE3	1000
#define SIZE4	10000
#define SIZE5	100000

#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2, SIZE3, SIZE4, SIZE5}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))

// -------------------------------------------




// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################




// =============================
void file_action (void)
{
SET_SIZES;

const string htext__using_getline ("getline                   ");
const string htext__vector_char   ("vector, reading char      ");
const string htext__string_char   ("string, reading char      ");
const string htext__vector_whole  ("vector, reading whole file");
const string htext__mmap          ("mmap                      ");
const string htext__iterator      ("iterator                  ");
const string htext__using_rdbuf   ("ostringstream, rdbuf      ");


#define INPUT_FILE_SUFFIIX		".in"
#define CHARS_PER_LINE_IN_INFILE	100

char ch;
  // -------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    string		file_content;
    const size_t	cur_infile_size = v_sizes[i];

    // ------ Filling input file ------
    const string cur_infile_name(to_string (cur_infile_size) + INPUT_FILE_SUFFIIX);
    ofstream cur_infile (cur_infile_name.c_str());
    assert (cur_infile);
    assert (cur_infile.is_open());

    ch = 0;
    for (size_t i = 0; i < cur_infile_size; i++)
    {
      while (!isprint(ch)) ch++;

      if ((i + 1)%CHARS_PER_LINE_IN_INFILE)	cur_infile << ch++;
      else					cur_infile << '\n';

      if (ch == SCHAR_MAX) ch = 0;
    }

    cur_infile.close();
    assert (!cur_infile.is_open());

    const string cur_infile_content (file_to_string (cur_infile_name));
    assert (cur_infile_size == cur_infile_content.size());
    // --------------------------------



    // ---------------------------------------
    {
      file_content.erase();
      TURN_ON_DEFAULT_TIMER (htext__using_getline, cur_infile_size) 
      {
        ReadFile1 (cur_infile_name, file_content);
      }
      assert (file_content.size() == cur_infile_content.size());
      assert (file_content == cur_infile_content);
    }


    {
      file_content.erase();
      TURN_ON_DEFAULT_TIMER (htext__vector_char, cur_infile_size) 
      {
        ReadFile2 (cur_infile_name, file_content);
      }
      assert (file_content.size() == cur_infile_content.size());
      assert (file_content == cur_infile_content);
    }


    {
      file_content.erase();
      TURN_ON_DEFAULT_TIMER (htext__string_char, cur_infile_size) 
      {
        ReadFile3 (cur_infile_name, file_content);
      }
      assert (file_content.size() == cur_infile_content.size());
      assert (file_content == cur_infile_content);
    }


    {
      file_content.erase();
      TURN_ON_DEFAULT_TIMER (htext__vector_whole, cur_infile_size) 
      {
        ReadFile4 (cur_infile_name, file_content);
      }
      assert (file_content.size() == cur_infile_content.size());
      assert (file_content == cur_infile_content);
    }


////////////////////////////////////////////////
#if ((defined UNIX_ENV) && !(defined DJGPP_ENV))
////////////////////////////////////////////////

    {
      file_content.erase();
      TURN_ON_DEFAULT_TIMER (htext__mmap, cur_infile_size) 
      {
        ReadFile5 (cur_infile_name, file_content);
      }
      file_content.erase (remove (file_content.begin(), file_content.end(), '\r'), file_content.end());
      assert (file_content.size() == cur_infile_content.size());
      assert (file_content == cur_infile_content);
    }

//////
#endif
//////


    {
      file_content.erase();
      TURN_ON_DEFAULT_TIMER (htext__iterator, cur_infile_size) 
      {
        ReadFile6 (cur_infile_name, file_content);
      }
      assert (file_content.size() == cur_infile_content.size());
      assert (file_content == cur_infile_content);
    }


    {
      file_content.erase();
      TURN_ON_DEFAULT_TIMER (htext__using_rdbuf, cur_infile_size) 
      {
        ReadFile7 (cur_infile_name, file_content);
      }
      assert (file_content.size() == cur_infile_content.size());
      assert (file_content == cur_infile_content);
    }

    // -----------------------
    remove (cur_infile_name.c_str());
    // -----------------------

  } // for (size_t i = 0; i < t_vfiles.size(); i++)


} // file_action


///////////////
// End-Of-File
///////////////

