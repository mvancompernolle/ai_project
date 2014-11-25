#include <limits>
#include <fstream>
#include <iomanip>
#include <more/io/binary_io.h>
#include <cassert>
#include <cstdlib>


template<typename T, typename U>
  void test(int count) {
      T special_1 = 0;
      T special_2 = (T)std::numeric_limits<U>::max();
      T special_3 = (T)std::numeric_limits<U>::min();
      long seed = lrand48();
      srand48(seed);
      {
	  std::ofstream os("tmp.binary_io_t3.bin");
	  assert(os.good());
	  more::io::binary_write(os, special_1);
	  more::io::binary_write(os, special_2);
	  more::io::binary_write(os, special_3);
	  int i = count;
	  while(--i) {
	      U x = lrand48();
	      more::io::binary_write(os, (T)x);
	  }
	  assert(os.good());
      }
      srand48(seed);
      {
	  std::ifstream is("tmp.binary_io_t3.bin");
	  assert(is.good());
	  U x;
	  more::io::binary_read(is, x); assert(x == special_1);
	  more::io::binary_read(is, x); assert(x == special_2);
	  more::io::binary_read(is, x); assert(x == special_3);
	  int i = count;
	  while(--i) {
	      U x, y = (T)lrand48();
	      more::io::binary_read(is, x);
	      if(x != y) std::cout << x << ' ' << y << std::endl;
	      assert(x == y);
	  }
	  assert(is.good());
      }
  }


int main() {
    test<char, char>(400);
    test<signed char, signed char>(400);
    test<unsigned char, unsigned char>(400);
    test<short, short>(1000);
//     test<unsigned short, unsigned short>(1000);
//     test<int, int>(1000);
    test<unsigned int, unsigned int>(1000);
    test<long, long>(10000);
    test<unsigned long, unsigned long>(10000);

    test<unsigned char, unsigned short>(400);
//     test<unsigned char, unsigned int>(400);
    test<unsigned char, unsigned long>(400);
//     test<unsigned short, unsigned int>(400);
//     test<unsigned short, unsigned long>(400);
    test<unsigned int, unsigned long>(400);

//     test<unsigned short, unsigned char>(400);
    test<unsigned int, unsigned char>(400);
//     test<unsigned long, unsigned char>(400);
//     test<unsigned int, unsigned short>(400);
//     test<unsigned long, unsigned short>(400);
    test<unsigned long, unsigned int>(400);

    test<signed char, short>(400);
//     test<signed char, int>(400);
//     test<signed char, long>(400);
    test<short, int>(400);
//     test<short, long>(400);
//     test<int, long>(400);

//     test<short, signed char>(400);
//     test<int, signed char>(400);
    test<long, signed char>(400);
//     test<int, short>(400);
//     test<long, short>(400);
    test<long, int>(400);

    return 0;
}
