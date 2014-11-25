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
//  FILE     : t_i2o.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============


// #################################################
// Stream I/O performance tests below are based
// on the article "Stream I/O" 
// presented at http://www.glenmccl.com/strm_cmp.htm
// by Glen McCluskey & Associates LLC
// #################################################



// #########################################
// -------------------------------------------
#define SIZE0	100
#define SIZE1	1000
#define SIZE2	10000

#define SET_SIZES  \
	const size_t         a_sizes[] = {SIZE0, SIZE1, SIZE2}; \
	const vector<size_t> v_sizes (a_sizes, a_sizes + sizeof(a_sizes)/sizeof(*a_sizes))

// -------------------------------------------



// #########################################



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################


// =============================
void input_to_output_action (void)
{
SET_SIZES;

#define BUFFER_SIZE  4096

const string htext__functions_getc_putc                ("[C]   functions getc, putc          ");
const string htext__functions_fgetc_fputc              ("[C]   functions fgetc, fputc        ");
const string htext__functions_fread_fwrite             ("[C]   functions fread, fwrite       ");

const string htext__operators_in_out                   ("[C++] operators >>, <<              ");
const string htext__methods_get_put                    ("[C++] methods get, put              ");
const string htext__methods_sbumpc_sputc               ("[C++] methods sbumpc, sputc         ");
const string htext__method_sbumpc__op_out              ("[C++] method  sbumpc, operator <<   ");
const string htext__method_rdbuf__op_out               ("[C++] method  rdbuf,  operator <<   ");
const string htext__methods_cpp_read_write__const_buf  ("[C++] methods read, write; const buf");
const string htext__methods_cpp_read_write__max_buf    ("[C++] methods read, write; max buf  ");

const string htext__c_overhead                         ("[C]   overhead  clearerr/rewind     ");
const string htext__cpp_overhead                       ("[C++] overhead  clear/seekg/seekp   ");


// ------------------------------
#define INPUT_FILE_NAME		"test_i2o.in"
#define OUTPUT_FILE_NAME	"test_i2o.out"

ifstream in_fs;
ofstream out_fs;

FILE*	 in_fp;
FILE*	 out_fp;

char ch;
int  ich;


  // -------------------------------------
  for (size_t i = 0; i < v_sizes.size(); i++)
  {
    string file_content__functions_getc_putc;
    string file_content__functions_fgetc_fputc;
    string file_content__functions_fread_fwrite;
    string file_content__operators_in_out;
    string file_content__methods_get_put;
    string file_content__methods_sbumpc_sputc;
    string file_content__method_sbumpc__op_out;
    string file_content__method_rdbuf__op_out;
    string file_content__methods_cpp_read_write__const_buf;
    string file_content__methods_cpp_read_write__max_buf;

    // --------------------------------
    const size_t cur_size (v_sizes[i]);

    // ------ Filling input file ------
    ofstream test_infile (INPUT_FILE_NAME);
    assert (test_infile);
    assert (test_infile.is_open());

    ch = 0;
    for (size_t i = 0; i < cur_size; i++)
    {
      while (!isprint(ch)) ch++;
      test_infile << ch++;
      if (ch == SCHAR_MAX) ch = 0;
    }

    test_infile.close();
    assert (!test_infile.is_open());

    const string file_content (file_to_string (INPUT_FILE_NAME));

    // --------------------------------



    // --------------------------------
    // --- C-1. functions_getc_putc ---
    {

      in_fp  = fopen(INPUT_FILE_NAME, "r");
      assert(in_fp);

      out_fp = fopen(OUTPUT_FILE_NAME, "w");
      assert(out_fp);


      TURN_ON_DEFAULT_TIMER (htext__functions_getc_putc, cur_size) 
      {
        // ---- Overhead ----
        clearerr(in_fp);
        rewind (in_fp);
  
        clearerr(out_fp);
        rewind (out_fp);
        // ------------------
  
        // ------ Body ------
        while ((ich = getc(in_fp)) != EOF) putc(ich, out_fp);
        // ------------------
      }

      clearerr(in_fp);
      fclose(in_fp);

      clearerr(out_fp);
      fclose(out_fp);

      // ---------------------
      assert (file_content__functions_getc_putc.empty());
      file_content__functions_getc_putc = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }


    // ----------------------------------
    // --- C-2. functions_fgetc_fputc ---
    {

      in_fp  = fopen(INPUT_FILE_NAME, "r");
      assert(in_fp);

      out_fp = fopen(OUTPUT_FILE_NAME, "w");
      assert(out_fp);


      TURN_ON_DEFAULT_TIMER (htext__functions_fgetc_fputc, cur_size) 
      {
        // ---- Overhead ----
        clearerr(in_fp);
        rewind (in_fp);
  
        clearerr(out_fp);
        rewind (out_fp);
        // ------------------
  
        // ------ Body ------
        while ((ich = fgetc(in_fp)) != EOF) fputc(ich, out_fp);
        // ------------------
      }

      clearerr(in_fp);
      fclose(in_fp);

      clearerr(out_fp);
      fclose(out_fp);

      // ---------------------
      assert (file_content__functions_fgetc_fputc.empty());
      file_content__functions_fgetc_fputc = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }


    // ----------------------------------
    // --- C-3. functions_fread_fwrite ---
    {
      char   buffer[BUFFER_SIZE];
      size_t nread;

      in_fp  = fopen(INPUT_FILE_NAME, "r");
      assert(in_fp);

      out_fp = fopen(OUTPUT_FILE_NAME, "w");
      assert(out_fp);


      TURN_ON_DEFAULT_TIMER (htext__functions_fread_fwrite, cur_size) 
      {
        // ---- Overhead ----
        clearerr(in_fp);
        rewind (in_fp);
  
        clearerr(out_fp);
        rewind (out_fp);
        // ------------------

        // ------ Body ------
        while ((nread = fread(buffer, sizeof(char), sizeof(buffer), in_fp)) > 0)
        {
           fwrite(buffer, sizeof(char), nread, out_fp);
        }

        // ------------------
      }

      clearerr(in_fp);
      fclose(in_fp);

      clearerr(out_fp);
      fclose(out_fp);

      // ---------------------
      assert (file_content__functions_fread_fwrite.empty());
      file_content__functions_fread_fwrite = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }




    // -------------------------------
    // --- CPP-1. operators_in_out ---
    {
      in_fs.open (INPUT_FILE_NAME);
      assert (in_fs);
      assert (in_fs.is_open());

      out_fs.open (OUTPUT_FILE_NAME);
      assert (out_fs);
      assert (out_fs.is_open());

      TURN_ON_DEFAULT_TIMER (htext__operators_in_out, cur_size) 
      {
        // ---- Overhead ----
        in_fs.clear();
        in_fs.seekg (0, ios::beg);
  
        out_fs.clear();
        out_fs.seekp (0, ios::beg);
        // ------------------
  
        // ------ Body ------
        in_fs.unsetf(ios::skipws);
        while (in_fs >> ch) out_fs << ch;
        // ------------------
      }

      in_fs.clear();
      in_fs.close();
      assert (!in_fs.is_open());

      out_fs.close();
      assert (!out_fs.is_open());

      // ---------------------
      assert (file_content__operators_in_out.empty());
      file_content__operators_in_out = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }


    // --- CPP-2. methods_get_put ---
    {
      in_fs.open (INPUT_FILE_NAME);
      assert (in_fs);
      assert (in_fs.is_open());

      out_fs.open (OUTPUT_FILE_NAME);
      assert (out_fs);
      assert (out_fs.is_open());

      TURN_ON_DEFAULT_TIMER (htext__methods_get_put, cur_size) 
      {
        // ---- Overhead ----
        in_fs.clear();
        in_fs.seekg (0, ios::beg);
  
        out_fs.clear();
        out_fs.seekp (0, ios::beg);
        // ------------------
  
        // ------ Body ------
        while (in_fs.get(ch)) out_fs.put(ch);
        // ------------------
      }

      in_fs.clear();
      in_fs.close();
      assert (!in_fs.is_open());

      out_fs.close();
      assert (!out_fs.is_open());

      // ---------------------
      assert (file_content__methods_get_put.empty());
      file_content__methods_get_put = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }


    // --- CPP-3. methods_sbumpc_sputc ---
    {
      in_fs.open (INPUT_FILE_NAME);
      assert (in_fs);
      assert (in_fs.is_open());

      out_fs.open (OUTPUT_FILE_NAME);
      assert (out_fs);
      assert (out_fs.is_open());

      TURN_ON_DEFAULT_TIMER (htext__methods_sbumpc_sputc, cur_size) 
      {
        // ---- Overhead ----
        in_fs.clear();
        in_fs.seekg (0, ios::beg);
  
        out_fs.clear();
        out_fs.seekp (0, ios::beg);
        // ------------------
  
        // ------ Body ------
        while ((ch = in_fs.rdbuf()->sbumpc()) != EOF) out_fs.rdbuf()->sputc(ch);
        // ------------------
      }

      in_fs.clear();
      in_fs.close();
      assert (!in_fs.is_open());

      out_fs.close();
      assert (!out_fs.is_open());

      // ---------------------
      assert (file_content__methods_sbumpc_sputc.empty());
      file_content__methods_sbumpc_sputc = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }


    // --- CPP-4. method_sbumpc__op_out ---
    {

      in_fs.open (INPUT_FILE_NAME);
      assert (in_fs);
      assert (in_fs.is_open());

      out_fs.open (OUTPUT_FILE_NAME);
      assert (out_fs);
      assert (out_fs.is_open());

      TURN_ON_DEFAULT_TIMER (htext__method_sbumpc__op_out, cur_size) 
      {
        // ---- Overhead ----
        in_fs.clear();
        in_fs.seekg (0, ios::beg);
  
        out_fs.clear();
        out_fs.seekp (0, ios::beg);
        // ------------------
  
        // ------ Body ------
        ch = in_fs.rdbuf()->sbumpc();
        out_fs << ch;

        while (ch != EOF)
        {
          out_fs << in_fs.rdbuf();
          ch = in_fs.rdbuf()->sbumpc();
        } 

        // ------------------
      }

      in_fs.clear();
      in_fs.close();
      assert (!in_fs.is_open());

      out_fs.close();
      assert (!out_fs.is_open());

      // ---------------------
      assert (file_content__method_sbumpc__op_out.empty());
      file_content__method_sbumpc__op_out = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }


    // --- CPP-5. method_rdbuf__op_out ---
    {
      in_fs.open (INPUT_FILE_NAME);
      assert (in_fs);
      assert (in_fs.is_open());

      out_fs.open (OUTPUT_FILE_NAME);
      assert (out_fs);
      assert (out_fs.is_open());

      TURN_ON_DEFAULT_TIMER (htext__method_rdbuf__op_out, cur_size) 
      {
        // ---- Overhead ----
        in_fs.clear();
        in_fs.seekg (0, ios::beg);
  
        out_fs.clear();
        out_fs.seekp (0, ios::beg);
        // ------------------
  
        // ------ Body ------
        out_fs << in_fs.rdbuf();
        // ------------------
      }

      in_fs.clear();
      in_fs.close();
      assert (!in_fs.is_open());

      out_fs.close();
      assert (!out_fs.is_open());

      // ---------------------
      assert (file_content__method_rdbuf__op_out.empty());
      file_content__method_rdbuf__op_out = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }



    // --- CPP-6. methods_cpp_read_write__const_buf ---
    {
      char buffer[BUFFER_SIZE];

      in_fs.open (INPUT_FILE_NAME);
      assert (in_fs);
      assert (in_fs.is_open());

      out_fs.open (OUTPUT_FILE_NAME);
      assert (out_fs);
      assert (out_fs.is_open());

      TURN_ON_DEFAULT_TIMER (htext__methods_cpp_read_write__const_buf, cur_size) 
      {
        // ---- Overhead ----
        in_fs.clear();
        in_fs.seekg (0, ios::beg);
  
        out_fs.clear();
        out_fs.seekp (0, ios::beg);
        // ------------------
  
        // ------ Body ------
        while (!in_fs.eof())
        {
          in_fs.read (buffer, sizeof(buffer));
          out_fs.write (buffer, in_fs.gcount());
        }
        // ------------------
      }

      in_fs.clear();
      in_fs.close();
      assert (!in_fs.is_open());

      out_fs.close();
      assert (!out_fs.is_open());

      // ---------------------
      assert (file_content__methods_cpp_read_write__const_buf.empty());
      file_content__methods_cpp_read_write__const_buf = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }

    // --- CPP-7. methods_cpp_read_write__max_buf ---
    {
      in_fs.open (INPUT_FILE_NAME);
      assert (in_fs);
      assert (in_fs.is_open());

      out_fs.open (OUTPUT_FILE_NAME);
      assert (out_fs);
      assert (out_fs.is_open());

      // --- get size of input file ---
      in_fs.seekg(0,ifstream::end);
      long file_size = in_fs.tellg();
      in_fs.seekg(0);

      char* buffer = new char [file_size];


      TURN_ON_DEFAULT_TIMER (htext__methods_cpp_read_write__max_buf, cur_size) 
      {
        // ---- Overhead ----
        in_fs.clear();
        in_fs.seekg (0, ios::beg);
  
        out_fs.clear();
        out_fs.seekp (0, ios::beg);
        // ------------------
  
        // ------ Body ------
        in_fs.read   (buffer, file_size);
        out_fs.write (buffer, file_size);
        // ------------------
      }

      delete[] buffer;

      in_fs.clear();
      in_fs.close();
      assert (!in_fs.is_open());

      out_fs.close();
      assert (!out_fs.is_open());

      // ---------------------
      assert (file_content__methods_cpp_read_write__max_buf.empty());
      file_content__methods_cpp_read_write__max_buf = file_to_string(OUTPUT_FILE_NAME);
      // ---------------------

    }


    // -----------------------------
    assert (file_content.size() == file_content__functions_getc_putc.size());
    assert (file_content.size() == file_content__functions_fgetc_fputc.size());
    assert (file_content.size() == file_content__functions_fread_fwrite.size());
    assert (file_content.size() == file_content__operators_in_out.size());
    assert (file_content.size() == file_content__methods_get_put.size());
    assert (file_content.size() == file_content__methods_sbumpc_sputc.size());
    assert (file_content.size() == file_content__method_sbumpc__op_out.size());
    assert (file_content.size() == file_content__method_rdbuf__op_out.size());
    assert (file_content.size() == file_content__methods_cpp_read_write__const_buf.size());
    assert (file_content.size() == file_content__methods_cpp_read_write__max_buf.size());


    assert (file_content == file_content__functions_getc_putc);
    assert (file_content == file_content__functions_fgetc_fputc);
    assert (file_content == file_content__functions_fread_fwrite);
    assert (file_content == file_content__operators_in_out);
    assert (file_content == file_content__methods_get_put);
    assert (file_content == file_content__methods_sbumpc_sputc);
    assert (file_content == file_content__method_sbumpc__op_out);
    assert (file_content == file_content__method_rdbuf__op_out);
    assert (file_content == file_content__methods_cpp_read_write__const_buf);
    assert (file_content == file_content__methods_cpp_read_write__max_buf);

    // -----------------------------


    // --- c overhead ---
    {
      in_fp  = fopen(INPUT_FILE_NAME, "r");
      assert(in_fp);

      out_fp = fopen(OUTPUT_FILE_NAME, "w");
      assert(out_fp);

      TURN_ON_DEFAULT_TIMER (htext__c_overhead, cur_size) 
      {
        // ---- Overhead ----
        clearerr(in_fp);
        rewind (in_fp);
  
        clearerr(out_fp);
        rewind (out_fp);
        // ------------------
  
      }

      clearerr(in_fp);
      fclose(in_fp);

      clearerr(out_fp);
      fclose(out_fp);

    }


    // --- cpp overhead ---
    {
      in_fs.open (INPUT_FILE_NAME);
      assert (in_fs);
      assert (in_fs.is_open());

      out_fs.open (OUTPUT_FILE_NAME);
      assert (out_fs);
      assert (out_fs.is_open());

      TURN_ON_DEFAULT_TIMER (htext__cpp_overhead, cur_size) 
      {
        // ---- Overhead ----
        in_fs.clear();
        in_fs.seekg (0, ios::beg);
  
        out_fs.clear();
        out_fs.seekp (0, ios::beg);
        // ------------------
  
      }

      in_fs.clear();
      in_fs.close();
      assert (!in_fs.is_open());

      out_fs.close();
      assert (!out_fs.is_open());

    }


  } // for (size_t i = 0; i < v_sizes.size(); i++)

  // -----------------------
  remove (INPUT_FILE_NAME);
  remove (OUTPUT_FILE_NAME);

} // input_to_output_action



///////////////
// End-Of-File
///////////////

