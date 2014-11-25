/* Copyright (C) 2012 Chris Vine

The library comprised in this file or of which this file is part is
distributed by Chris Vine under the GNU Lesser General Public
License as follows:

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License, version 2.1, for more details.

   You should have received a copy of the GNU Lesser General Public
   License, version 2.1, along with this library (see the file LGPL.TXT
   which came with this source code package in the c++-gtk-utils
   sub-directory); if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <glib.h>
#include <fstream>
#include <string>
#include <cstring>
#include <cstddef>

#include <c++-gtk-utils/fdstream.h>
#include <c++-gtk-utils/gobj_handle.h>

using namespace Cgu;

#define FILENAME "test_fdstream.cpp"
#define COPY_FILENAME "test_fdstream.cpp.copy"
#define W_COPY_FILENAME "test_fdstream.cpp.w_copy"
#define U16_COPY_FILENAME "test_fdstream.cpp.u16_copy"
#define U32_COPY_FILENAME "test_fdstream.cpp.u32_copy"
#define RAND_FILENAME1 "random1.txt"
#define RAND_FILENAME2 "random2.txt"

const char random_out[] = "......... Text line 1\n"
                          "......... Text line 2\n"
                          "rewritten Text line 3\n"
                          "......... Text line 4\n";

// returns 0 if the files are the same, -1 if not
int compare_files(const char* file1, const char* file2) {
  std::ifstream strm1{file1, std::ios::in | std::ios::binary | std::ios::ate};
  std::ifstream strm2{file2, std::ios::in | std::ios::binary | std::ios::ate};

  if (!strm1 || !strm2
      || strm1.tellg() != strm2.tellg())
    return -1;

  strm1.seekg(0, std::ios::beg);
  strm2.seekg(0, std::ios::beg);

  char byte1 = 0;
  char byte2 = 0;
  while (strm1.get(byte1)) {
    strm2.get(byte2);
    if (byte1 != byte2) return -1;
  }
  return 0;
}

extern "C" {
static void test_fdstream_copy() {

  unlink(COPY_FILENAME);

  int fdin = open(FILENAME, O_RDONLY);
  int fdout = open(COPY_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fdin < 0 || fdout < 0) {
    g_assert_not_reached();
  }
        
  fdistream instream{fdin};
  fdostream outstream{fdout};

  // do various types of put and get
  do {
    // single letter puts and gets
    char letter;
    for (int count = 0; count < 500 && instream.get(letter); ++count) outstream.put(letter);

    // small blocks
    const int SMALL_BLOCKSIZE = 20;
    char block1[SMALL_BLOCKSIZE];
    for (int count = 0; count < 20 && instream; ++count) {
      instream.read(block1, SMALL_BLOCKSIZE);
      outstream.write(block1, instream.gcount());
    }
    // big blocks
    const int BIG_BLOCKSIZE = 1200;
    char block2[BIG_BLOCKSIZE];
    for (int count = 0; count < 2 && instream; ++count) {
      instream.read(block2, BIG_BLOCKSIZE);
      outstream.write(block2, instream.gcount());
    }
  } while (instream);

  outstream.flush();
  int res = compare_files(FILENAME, COPY_FILENAME);
  g_assert_cmpint(res, ==, 0);

  unlink(COPY_FILENAME);
}

static void test_fdstream_wide_copy() {

  unlink(W_COPY_FILENAME "1");
  unlink(W_COPY_FILENAME "2");

  // copy twice so we can compare on a whole number of wchar_t
  // characters
  for (int iteration = 0; iteration < 2; ++iteration) {

    int fdin;
    int fdout;

    if (!iteration) {
      fdin = open(FILENAME, O_RDONLY);
      fdout = open(W_COPY_FILENAME "1", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    else {
      fdin = open(W_COPY_FILENAME "1", O_RDONLY);
      fdout = open(W_COPY_FILENAME "2", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    if (fdin < 0 || fdout < 0) {
      g_assert_not_reached();
    }

    wfdistream instream{fdin};
    wfdostream outstream{fdout};

    // do various types of put and get
    do {
      // single letter puts and gets
      wchar_t letter;
      for (int count = 0; count < 500 && instream.get(letter); ++count) outstream.put(letter);

      // small blocks
      const int SMALL_BLOCKSIZE = 20;
      wchar_t block1[SMALL_BLOCKSIZE];
      for (int count = 0; count < 20 && instream; ++count) {
	instream.read(block1, SMALL_BLOCKSIZE);
	outstream.write(block1, instream.gcount());
      }
      // big blocks
      const int BIG_BLOCKSIZE = 1200;
      wchar_t block2[BIG_BLOCKSIZE];
      for (int count = 0; count < 2 && instream; ++count) {
	instream.read(block2, BIG_BLOCKSIZE);
	outstream.write(block2, instream.gcount());
      }
    } while (instream);
  }
  int res = compare_files(W_COPY_FILENAME "1", W_COPY_FILENAME "2");
  g_assert_cmpint(res, ==, 0);

  unlink(W_COPY_FILENAME "1");
  unlink(W_COPY_FILENAME "2");
}

static void test_fdstream_u16_copy() {

  unlink(U16_COPY_FILENAME "1");
  unlink(U16_COPY_FILENAME "2");

  // copy twice so we can compare on a whole number of wchar_t
  // characters
  for (int iteration = 0; iteration < 2; ++iteration) {

    int fdin;
    int fdout;

    if (!iteration) {
      fdin = open(FILENAME, O_RDONLY);
      fdout = open(U16_COPY_FILENAME "1", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    else {
      fdin = open(U16_COPY_FILENAME "1", O_RDONLY);
      fdout = open(U16_COPY_FILENAME "2", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    if (fdin < 0 || fdout < 0) {
      g_assert_not_reached();
    }

    u16fdistream instream{fdin};
    u16fdostream outstream{fdout};

    // do various types of put and get
    do {
      // single letter puts and gets
      char16_t letter;
      for (int count = 0; count < 500 && instream.get(letter); ++count) outstream.put(letter);

      // small blocks
      const int SMALL_BLOCKSIZE = 20;
      char16_t block1[SMALL_BLOCKSIZE];
      for (int count = 0; count < 20 && instream; ++count) {
	instream.read(block1, SMALL_BLOCKSIZE);
	outstream.write(block1, instream.gcount());
      }
      // big blocks
      const int BIG_BLOCKSIZE = 1200;
      char16_t block2[BIG_BLOCKSIZE];
      for (int count = 0; count < 2 && instream; ++count) {
	instream.read(block2, BIG_BLOCKSIZE);
	outstream.write(block2, instream.gcount());
      }
    } while (instream);
  }
  int res = compare_files(U16_COPY_FILENAME "1", U16_COPY_FILENAME "2");
  g_assert_cmpint(res, ==, 0);

  unlink(U16_COPY_FILENAME "1");
  unlink(U16_COPY_FILENAME "2");
}

static void test_fdstream_u32_copy() {

  unlink(U32_COPY_FILENAME "1");
  unlink(U32_COPY_FILENAME "2");

  // copy twice so we can compare on a whole number of wchar_t
  // characters
  for (int iteration = 0; iteration < 2; ++iteration) {

    int fdin;
    int fdout;

    if (!iteration) {
      fdin = open(FILENAME, O_RDONLY);
      fdout = open(U32_COPY_FILENAME "1", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    else {
      fdin = open(U32_COPY_FILENAME "1", O_RDONLY);
      fdout = open(U32_COPY_FILENAME "2", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    if (fdin < 0 || fdout < 0) {
      g_assert_not_reached();
    }

    u32fdistream instream{fdin};
    u32fdostream outstream{fdout};

    // do various types of put and get
    do {
      // single letter puts and gets
      char32_t letter;
      for (int count = 0; count < 500 && instream.get(letter); ++count) outstream.put(letter);

      // small blocks
      const int SMALL_BLOCKSIZE = 20;
      char32_t block1[SMALL_BLOCKSIZE];
      for (int count = 0; count < 20 && instream; ++count) {
	instream.read(block1, SMALL_BLOCKSIZE);
	outstream.write(block1, instream.gcount());
      }
      // big blocks
      const int BIG_BLOCKSIZE = 1200;
      char32_t block2[BIG_BLOCKSIZE];
      for (int count = 0; count < 2 && instream; ++count) {
	instream.read(block2, BIG_BLOCKSIZE);
	outstream.write(block2, instream.gcount());
      }
    } while (instream);
  }
  int res = compare_files(U32_COPY_FILENAME "1", U32_COPY_FILENAME "2");
  g_assert_cmpint(res, ==, 0);

  unlink(U32_COPY_FILENAME "1");
  unlink(U32_COPY_FILENAME "2");
}

static void test_fdstream_random() {
  
  unlink(RAND_FILENAME1);
  unlink(RAND_FILENAME2);

  for (int iteration = 0; iteration < 2; ++iteration) {
    
    int fd;
    if (!iteration)
      fd = open(RAND_FILENAME1, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    else 
      fd = open(RAND_FILENAME2, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
      g_assert_not_reached();
    }
        
    fdistream filein{fd, false};
    fdostream fileout{fd};

    if (iteration)
      fileout.set_buffered(false);

    fileout << "......... Text line 1\n";
    fileout << "......... Text line 2\n";
    fileout << "......... Text line 3\n";
    fileout << "......... Text line 4\n";

    if (!iteration)
      fileout.flush();
      
    filein.seekg(0, std::ios_base::beg);
    std::string line;
    int count;
    for (count = 0; count < 2; ++count) {
      std::getline(filein, line);
    }

    filein.seekg(filein.tellg());
    fileout.write("rewritten", std::strlen("rewritten"));
    if (!iteration)
      fileout.flush();

    filein.seekg(0, std::ios::beg);
    for (std::size_t index = 0; index < sizeof(random_out) - 1; ++index) {
      g_assert_cmpint(static_cast<int>((unsigned char)random_out[index]), ==, filein.get());
    }
  }
  unlink(RAND_FILENAME1);
  unlink(RAND_FILENAME2);
}

} // extern "C"


int main (int argc, char* argv[]) {
  g_test_init(&argc, &argv, static_cast<void*>(0));

  g_test_add_func("/fdstream/copy", test_fdstream_copy); 
  g_test_add_func("/fdstream/wide_copy", test_fdstream_wide_copy); 
  g_test_add_func("/fdstream/u16_copy", test_fdstream_u16_copy); 
  g_test_add_func("/fdstream/u32_copy", test_fdstream_u32_copy); 
  g_test_add_func("/fdstream/random", test_fdstream_random); 

  return g_test_run();
}
