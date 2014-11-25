#include "sys.h"
#include "debug.h"
#include <libecc/sha.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <queue>

int main(int argc, char* argv[])
{
  libecc::bitset<102408> m;
  char buf[200];
  libecc::sha1 sha;
  static char const* hashes_filename[] = { "/sha1-vectors/bit-hashes.sha1", "/sha1-vectors/byte-hashes.sha1" };
  static char const* messages_filename[] = { "/sha1-vectors/bit-messages.sha1", "/sha1-vectors/byte-messages.sha1" };

  std::string srcdir;

  if (argc > 1)
    srcdir = argv[1];
  else
    srcdir = ".";

  for (int file = 0; file <= 1; ++file)
  {
    std::queue<libecc::bitset<160> > hashes;
    std::ifstream infile;
    infile.open((srcdir + hashes_filename[file]).c_str());
    if (!infile)
    {
      std::cerr << "Cannot open " << (srcdir + hashes_filename[file]) << std::endl;
      return 1;
    }
    while(infile.getline(buf, 200))
    {
      if (*buf == '#')
	continue;
      if (*buf == 'H' && buf[1] == '>')
      {
	size_t len = strlen(buf);
	std::string s(buf + 2, len - 5);
	std::cout << "Reading " << s << '\n';
	continue;
      }
      if (*buf == '<' && buf[1] == 'D')
	continue;
      if (*buf == 'D' && buf[1] == '>')
      {
	for(;;)
	{
	  std::string s;
	  infile >> s;
	  if (s[0] == '<')
	    break;
	  hashes.push(libecc::bitset<160>(s.c_str()));
	  char c;
	  infile >> c;
	  assert( c == '^' );
	}
      }
    }
    infile.close();
    infile.clear();

    infile.open((srcdir + messages_filename[file]).c_str());
    int data_type = 0;
    while(infile.getline(buf, 200))
    {
      if (*buf == '#')
	continue;
      if (*buf == 'H' && buf[1] == '>')
      {
	size_t len = strlen(buf);
	std::string s(buf + 2, len - 5);
	std::cout << "Testing " << s << '\n';
	continue;
      }
      if (*buf == '<' && buf[1] == 'D')
	continue;
      if (*buf == 'D' && buf[1] == '>')
      {
	++data_type;
	int n;
	while (infile >> n)
	{
	  int val;
	  infile >> val;
	  std::vector<int> runs;
	  int k = n;
	  int sum = 0;
	  while (k--)
	  {
	    int d;
	    infile >> d;
	    runs.push_back(d);
	    sum += d;
	  }
	  char c;
	  infile >> c;
	  assert( c == '^' );
	  assert( sum <= m.number_of_bits );
	  int b = sum;
	  for (std::vector<int>::const_iterator iter = runs.begin(); iter != runs.end(); ++iter)
	  {
	    for (int c = 0 ; c < *iter; ++c)
	    {
	      if (val)
		m.set(--b);
	      else
		m.clear(--b);
	    }
	    val = 1 - val;
	  }
	  if (data_type == 3)
	  {
	    assert( sum <= 512 );
	    libecc::bitset<512> buf = m;
	    int n = sum;
	    for (int j = 0; j < 100; ++j)
	    {
	      std::cout << hashes.front() << "  " << std::flush;
	      for(int i = 1; i <= 50000; ++i)
	      {
		assert( n + j / 4 * 8 + 24 + 32 <= 512 );
		for (int a = 1; a <= j / 4 * 8 + 24; ++a)
		  buf.shift_op<1, libecc::left, libecc::assign>(buf);
		n += j / 4 * 8 + 24;
		buf.shift_op<32, libecc::left, libecc::assign>(buf);
		n += 32;
		buf.rawdigit(0) |= i;
		sha.process_msg(buf, n);
		buf = sha.digest();
		n = 160;
	      }
	      assert( hashes.front() == sha.digest() );
	      std::cout << "OK\n";
	      hashes.pop();
	    }
	  }
	  else
	  {
	    std::cout << hashes.front() << "  " << std::flush;
	    sha.process_msg(m, sum);
	    assert( hashes.front() == sha.digest() );
	    std::cout << "OK\n";
	    hashes.pop();
	  }
	}
	infile.clear();
      }
    }
    infile.close();
    assert( hashes.empty() );
  }

  return 0;
}

