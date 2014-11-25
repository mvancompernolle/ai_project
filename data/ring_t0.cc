#include <more/gen/ring.h>
#include <iostream>


template<typename T, typename Alloc>
  void dump(more::gen::ring<T, Alloc> const& r) {
      typedef more::gen::ring<T, Alloc> container;
      typedef typename container::const_iterator iterator;
      std::cout << '(';
      iterator it = r.principal();
      do
	  std::cout << *it << ", ";
      while (++it != r.principal());
      std::cout << "self)";
  }


int main(int, char**) {
    typedef more::gen::ring<int> container;
    typedef container::iterator iterator;
    container r;
    for (int i = 0; i < 10; ++i)
	r.push_front(i);

    std::cout << "r = ";
    dump(r);
    std::cout << '\n';

    container r0 = r;
    container r1(r);

    std::cout << "Copy: "; dump(r0); std::cout << '\n';
    std::cout << "Copy: "; dump(r1); std::cout << '\n';

    r.erase(r.principal());
    iterator it = r.principal();
    ++it;
    it = r.erase(it);
    r.erase(it);
    std::cout << "Removed 9, 7, 6: "; dump(r); std::cout << '\n';

    return 0;
}
