#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <more/math/math.h>
#include <stdlib.h>

using namespace more;

template<typename T>
  void mkplot(int cnt) {
      typedef std::map< T, std::pair<T, T> > map_type;
      map_type mp;
#if 0
      while (cnt--) {
	  T x = drand48();
	  T y = drand48();
	  mp.insert(std::make_pair(math::bitpow(x, 2)+2*math::bitpow(y, 2),
				   std::make_pair(x, y)));
      }
#else
      for (int i = 0; i < (int)std::sqrt((double)cnt); ++i)
	  for (int j = 0; j < (int)std::sqrt((double)cnt); ++j) {
	      T x = 1.04*i, y = j;
	      mp.insert(std::make_pair(math::bitpow(x, 2)+2*math::bitpow(y, 2),
				       std::make_pair(x, y)));
	  }
#endif

      std::cout << "$ data=curve2d\n"
		<< "% mt=13\n";
      for (typename map_type::iterator it = mp.begin();
	   it != mp.end(); ++it)
	  std::cout << it->second.first << ' '
		    << it->second.second << '\n';
      std::cout << "$ end\n";
  }

int main(int argc, char** argv) {
    int n;
    if (argc != 2) {
	std::cerr << "usage: " << argv[0] << " n\n";
	return 1;
    }
    std::istringstream iss(argv[1]);
    iss >> n;
    if (iss.fail()) {
	std::cerr << "Bad value of arg #1.\n";
	return 1;
    }
    mkplot<double>(n);
    return 0;
}
