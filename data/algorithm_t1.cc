#include <more/gen/algorithm.h>
#include <iostream>
#include <iomanip>


int main() {
    int lst[20], *first = lst, *last = lst;
    for (int i = 0; i < 40; ++i) {
	if (rand() % 4) {
	    more::gen::insert_sorted(first, last, rand() % 10);
	    ++last;
	}
	else
	    last = std::remove(first, last, rand() % 10);
	for(int *it = first; it != last; ++it)
	    std::cout << *it << ' ';
	std::cout << std::endl;
    }
    std::cout << "done." << std::endl;
    return 0;
}
