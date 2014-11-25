#include "sys.h"
#include "debug.h"
#include <libecc/point.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "utils.h"

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::fstream;
using libecc::bitset;

unsigned int const hd = (m - 1) / 4 + 1;	// Hex digits.

poly_t a;
poly_t b;

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    cerr << "Provide exactly one argument.\n";
    return 1;
  }

  poly_t x(argv[1]);
  poly_t y(x);
  
  cout << y;
  do
  {
    y *= y;
    cout << " -> " << y;
  }
  while (x != y);
  cout << endl;
}
