#include <more/gen/outer_product_container.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <iterator>

int main() {
    std::list<int> L;
    std::list<int> Lp;
    L.push_back(0);
    L.push_back(1);
    L.push_back(2);
    Lp.push_back(10);
    Lp.push_back(20);
    Lp.push_back(30);
    typedef more::gen::outer_product_container
		<std::list<int>::iterator, std::vector>
	    oprod_container;
    oprod_container C;
    C.insert_component(L.begin(), L.end());
    C.insert_component(L.begin(), L.end());
    C.insert_component(Lp.begin(), Lp.end());
    for (oprod_container::iterator it = C.begin(); it != C.end(); ++it) {
	std::copy(it->begin(), it->end(),
		  std::ostream_iterator<int>(std::cout, " "));
	std::cout << std::endl;
    }
    return 0;
}

