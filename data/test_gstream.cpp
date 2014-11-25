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
#include <locale.h>
#include <glib.h>
#include <gio/gio.h>
#include <fstream>
#include <string>
#include <cstring>
#include <cstddef>

#include <c++-gtk-utils/gstream.h>
#include <c++-gtk-utils/gobj_handle.h>

using namespace Cgu;

#define FILENAME "test_gstream.cpp"
#define COPY_FILENAME "test_gstream.cpp.copy"
#define W_COPY_FILENAME "test_gstream.cpp.w_copy"
#define U16_COPY_FILENAME "test_gstream.cpp.u16_copy"
#define U32_COPY_FILENAME "test_gstream.cpp.u32_copy"
#define CONVERT_FILENAME "test_gstream.cpp.conv.gz"
#define RECONVERT_FILENAME "test_gstream.cpp.conv"
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

#if GLIB_CHECK_VERSION(2,16,0)
extern "C" {
static void test_gstream_copy() {

  unlink(COPY_FILENAME);

  GFileInputStream* istream;
  GobjHandle<GFile> in{g_file_new_for_path(FILENAME)};
  istream = g_file_read(in,
			0,
			0);
  GFileOutputStream* ostream;
  GobjHandle<GFile> out{g_file_new_for_path(COPY_FILENAME)};
  ostream = g_file_replace(out,
			   0,
			   false,
			   G_FILE_CREATE_NONE,
			   0,
			   0);
  if (!istream || !ostream) {
    g_assert_not_reached();
  }
        
  gistream instream{GobjHandle<GInputStream>{G_INPUT_STREAM(istream)}, true};
  gostream outstream{GobjHandle<GOutputStream>{G_OUTPUT_STREAM(ostream)}, true};

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

static void test_gstream_wide_copy() {

  unlink(W_COPY_FILENAME "1");
  unlink(W_COPY_FILENAME "2");

  GFileInputStream* istream;
  GFileOutputStream* ostream;

  // copy twice so we can compare on a whole number of wchar_t
  // characters
  for (int iteration = 0; iteration < 2; ++iteration) {
    GobjHandle<GFile> in;
    GobjHandle<GFile> out;

    if (!iteration) {
      in.reset(g_file_new_for_path(FILENAME));
      out.reset(g_file_new_for_path(W_COPY_FILENAME "1"));
    }
    else {
      in.reset(g_file_new_for_path(W_COPY_FILENAME "1"));
      out.reset(g_file_new_for_path(W_COPY_FILENAME "2"));
    }

    istream = g_file_read(in,
			  0,
			  0);
    ostream = g_file_replace(out,
			     0,
			     false,
			     G_FILE_CREATE_NONE,
			     0,
			     0);
    if (!istream || !ostream) {
      g_assert_not_reached();
    }
        
    wgistream instream{GobjHandle<GInputStream>{G_INPUT_STREAM(istream)}, true};
    wgostream outstream{GobjHandle<GOutputStream>{G_OUTPUT_STREAM(ostream)}, true};

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

static void test_gstream_u16_copy() {

  unlink(U16_COPY_FILENAME "1");
  unlink(U16_COPY_FILENAME "2");

  GFileInputStream* istream;
  GFileOutputStream* ostream;

  // copy twice so we can compare on a whole number of char16_t
  // characters
  for (int iteration = 0; iteration < 2; ++iteration) {
    GobjHandle<GFile> in;
    GobjHandle<GFile> out;

    if (!iteration) {
      in.reset(g_file_new_for_path(FILENAME));
      out.reset(g_file_new_for_path(U16_COPY_FILENAME "1"));
    }
    else {
      in.reset(g_file_new_for_path(U16_COPY_FILENAME "1"));
      out.reset(g_file_new_for_path(U16_COPY_FILENAME "2"));
    }

    istream = g_file_read(in,
			  0,
			  0);
    ostream = g_file_replace(out,
			     0,
			     false,
			     G_FILE_CREATE_NONE,
			     0,
			     0);
    if (!istream || !ostream) {
      g_assert_not_reached();
    }
        
    u16gistream instream{GobjHandle<GInputStream>{G_INPUT_STREAM(istream)}, true};
    u16gostream outstream{GobjHandle<GOutputStream>{G_OUTPUT_STREAM(ostream)}, true};

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

static void test_gstream_u32_copy() {

  unlink(U32_COPY_FILENAME "1");
  unlink(U32_COPY_FILENAME "2");

  GFileInputStream* istream;
  GFileOutputStream* ostream;

  // copy twice so we can compare on a whole number of char32_t
  // characters
  for (int iteration = 0; iteration < 2; ++iteration) {
    GobjHandle<GFile> in;
    GobjHandle<GFile> out;

    if (!iteration) {
      in.reset(g_file_new_for_path(FILENAME));
      out.reset(g_file_new_for_path(U32_COPY_FILENAME "1"));
    }
    else {
      in.reset(g_file_new_for_path(U32_COPY_FILENAME "1"));
      out.reset(g_file_new_for_path(U32_COPY_FILENAME "2"));
    }

    istream = g_file_read(in,
			  0,
			  0);
    ostream = g_file_replace(out,
			     0,
			     false,
			     G_FILE_CREATE_NONE,
			     0,
			     0);
    if (!istream || !ostream) {
      g_assert_not_reached();
    }
        
    u32gistream instream{GobjHandle<GInputStream>{G_INPUT_STREAM(istream)}, true};
    u32gostream outstream{GobjHandle<GOutputStream>{G_OUTPUT_STREAM(ostream)}, true};

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

static void test_gstream_convert() {

  unlink(CONVERT_FILENAME);
  unlink(RECONVERT_FILENAME);
  GFileInputStream* istream;
  GFileOutputStream* ostream;

  for (int iteration = 0; iteration < 2; ++iteration) {
    GobjHandle<GFile> in;
    GobjHandle<GFile> out;

    if (!iteration) {
      in.reset(g_file_new_for_path(FILENAME));
      out.reset(g_file_new_for_path(CONVERT_FILENAME));
    }
    else {
      in.reset(g_file_new_for_path(CONVERT_FILENAME));
      out.reset(g_file_new_for_path(RECONVERT_FILENAME));
    }

    istream = g_file_read(in,
			  0,
			  0);
    ostream = g_file_replace(out,
			     0,
			     false,
			     G_FILE_CREATE_NONE,
			     0,
			     0);
    if (!istream || !ostream) {
      g_assert_not_reached();
    }

    gistream instream;
    gostream outstream;
    if (!iteration) {
      instream.attach(GobjHandle<GInputStream>{G_INPUT_STREAM(istream)}, true);
      outstream.attach(GobjHandle<GOutputStream>{G_OUTPUT_STREAM(ostream)},
		       true,
		       GobjHandle<GConverter>{
			 G_CONVERTER(g_zlib_compressor_new(G_ZLIB_COMPRESSOR_FORMAT_GZIP, -1))
		       });
      outstream << instream.rdbuf();
      g_assert(instream.is_error() == 0);
      g_assert(outstream.is_error() == 0);
      g_assert(!outstream.fail());
    }
    else {
      instream.attach(GobjHandle<GInputStream>{G_INPUT_STREAM(istream)},
		      true,
		      GobjHandle<GConverter>{
			G_CONVERTER(g_zlib_decompressor_new(G_ZLIB_COMPRESSOR_FORMAT_GZIP))
		      });
      outstream.attach(GobjHandle<GOutputStream>{G_OUTPUT_STREAM(ostream)}, true);
      outstream << instream.rdbuf();
      g_assert(instream.is_error() == 0);
      g_assert(outstream.is_error() == 0);
      g_assert(!outstream.fail());
    }
  }
  int res = compare_files(FILENAME, RECONVERT_FILENAME);
  g_assert_cmpint(res, ==, 0);

  unlink(CONVERT_FILENAME);
  unlink(RECONVERT_FILENAME);
}

static void test_gstream_random() {
  
  unlink(RAND_FILENAME1);
  unlink(RAND_FILENAME2);
  GFileIOStream* iostream;

  for (int iteration = 0; iteration < 2; ++iteration) {
    
    GobjHandle<GFile> gfile;
    if (!iteration)
      gfile.reset(g_file_new_for_path(RAND_FILENAME1));
    else 
      gfile.reset(g_file_new_for_path(RAND_FILENAME2));

    iostream = g_file_replace_readwrite(gfile,
					0,
					false,
					G_FILE_CREATE_NONE,
					0,
					0);
    if (!iostream) {
      g_assert_not_reached();
    }
        
    giostream file{GobjHandle<GIOStream>{G_IO_STREAM(iostream)}, true};
    if (iteration)
      file.set_output_buffered(false);

    file << "......... Text line 1\n";
    file << "......... Text line 2\n";
    file << "......... Text line 3\n";
    file << "......... Text line 4\n";

    file.seekp(0, std::ios_base::beg);
    std::string line;
    int count;
    for (count = 0; count < 2; ++count) {
      std::getline(file, line);
    }

    if (!iteration)
      file.seekp(file.tellp());
    file.write("rewritten", std::strlen("rewritten"));

    file.seekp(0, std::ios::beg);
    for (std::size_t index = 0; index < sizeof(random_out) - 1; ++index) {
      g_assert_cmpint(static_cast<int>((unsigned char)random_out[index]), ==, file.get());
    }
  }
  unlink(RAND_FILENAME1);
  unlink(RAND_FILENAME2);
}

} // extern "C"
#endif // GLIB_CHECK_VERSION(2,16,0)

int main (int argc, char* argv[]) {
  setlocale(LC_ALL, "");
#if !(GLIB_CHECK_VERSION(2,32,0))
  g_thread_init(0);
#endif
#if !(GLIB_CHECK_VERSION(2,36,0))
  g_type_init();
#endif
  g_test_init(&argc, &argv, static_cast<void*>(0));

#if GLIB_CHECK_VERSION(2,16,0)
  g_test_add_func("/gstream/copy", test_gstream_copy); 
  g_test_add_func("/gstream/wide_copy", test_gstream_wide_copy); 
  g_test_add_func("/gstream/u16_copy", test_gstream_u16_copy); 
  g_test_add_func("/gstream/u32_copy", test_gstream_u32_copy); 
  g_test_add_func("/gstream/convert", test_gstream_convert); 
  g_test_add_func("/gstream/random", test_gstream_random); 

  return g_test_run();
#endif
}
