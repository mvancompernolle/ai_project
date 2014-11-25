#include <iostream>
#include <algorithm>
#include <list>
#include <functional>
#include <more/gen/view.h>

struct odd_filter
  : std::unary_function<int, bool>
  {
      bool operator()(int i) { return i%2; }
  };

int main() {
    int init[] = { 3, 6, 8, 13, 1, 13, 8, 6, 2, 7, 0 };
    std::list<int> lst(init+0, init+sizeof(init)/sizeof(int));
    more::gen::if_view< std::list<int>::iterator, odd_filter >
      view(lst.begin(), lst.end(), odd_filter());
    std::copy(view.begin(), view.end(),
	      std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
    return 0;
}
