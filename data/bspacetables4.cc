#include "sys.h"
#include "debug.h"
#include <iostream>
#include <iomanip>
#include <libecc/point.h>
#include "utils.h"

using std::cout;
using std::endl;
using std::flush;

#define HTML 0
#define OUTPUT 1

#if HTML
#define WRITE_OUTPUT(str, data) do { cout << "<tr><td>" << str << "</td><td>" << (data) << "</td></tr>\n"; } while(0)
#else
#define WRITE_OUTPUT(str, data) do { cout << str << ' ' << (data) << '\n'; } while(0)
#endif

int main()
{
  Debug(debug::init());

  initialize_utils();

  poly_t const mg(generator());
  poly_t b(mg);
  do
  {
    if (is_generator(b) && is_normal_basis(b))
    {
      cout << "b = " << b;

      poly_t sqrt_b(b);
      sqrt_b.sqrt();
      
      unsigned long solutions = 0;
      poly_t x(mg);
      do
      {
        if (x.trace() == (sqrt_b / x).trace())
	  ++solutions;
      }
      while((x *= mg) != mg);
      cout << "; #E = " << (2 + 2 * solutions) << endl;
    }
  }
  while((b *= mg) != mg);
}

