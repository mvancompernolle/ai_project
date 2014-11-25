#include <iostream>
#include <vector>
#include <more/gen/iterator.h>
#include <more/gen/iterator_composition.h>
#include <functional>

namespace gen = more::gen;

int main()
{
    std::vector<int> v1;
    std::vector<int> v2;
    for(int i = 0; i < 10; ++i) {
	v1.push_back(i);
	v2.push_back(i*i);
    }
    std::copy(gen::compose_iterators(v1.begin(), v2.begin(), std::plus<int>()),
	      gen::compose_iterators(v1.end(), v2.end(), std::plus<int>()),
	      gen::ostream_iterator<int>(std::cout, " "));
}
