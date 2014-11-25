#include <more/gen/binary_heap.h>
#include <algorithm>
#include <iostream>

#define ASSERT(x) ((x)? 0 : ++fail)

int main() {
    int fail = 0;
    int const N = 2000;
    int stat[N];
    std::fill(stat+0, stat+N, 0);
    more::gen::binary_heap<int> h;
    for (int i = 0; i < N; ++i) {
	int v = rand() % N;
// 	std::cout << v << ' ';
	h.insert(v);
	++stat[v];
    }
//     std::cout << std::endl;
    int prev = -1;
    while (!h.empty()) {
// 	std::cout << h.front() << ' ';
	ASSERT(h.front() >= prev);
	--stat[h.front()];
	h.pop_front();
    }
    for (int i = 0; i < N; ++i)
	ASSERT(stat[i] == 0);
//     std::cout << std::endl;
    if (fail > 0) {
	std::cerr << fail << " failures.\n";
	return 1;
    }
    else
	return 0;
}
