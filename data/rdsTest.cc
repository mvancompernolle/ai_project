#include "sys.h"
#include "debug.h"
#include <libecc/rds.h>

int main()
{
  libecc::rds::pool_type seed("1");
  libecc::rds r(seed, false);

  libecc::bitset<64> b;
  for (int j = 0; j < 10; ++j)
  {
    r.read(b);
    std::cout << b << std::endl;
  }
}
