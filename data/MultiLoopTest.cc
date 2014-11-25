#include "MultiLoop.h"
#include <iostream>

int main()
{
  for (MultiLoop ml(5); !ml.finished(); ml.next_loop())
  {
    for(; ml() < 3; ++ml)
    {
      if (*ml == 4)		// Most inner loop.
      {
        for (int i = 0; i < 5; ++i)
	  std::cout << ml[i] << " ";
        std::cout << std::endl;
	if (ml[2] == 1)
	{
	  ml.breaks(3);
	  break;
        }
      }
    }
  }
}
