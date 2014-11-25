#include <more/gen/po_map.h>
#include <more/gen/po_algo.h>
#include <cstdlib>

const int highbit = 5;

struct bit_subseteq : std::binary_function<int, int, bool> {
    bool operator()(int x, int y) const {
	return (x & y) == x;
    }
};

void put_binary(int i) {
    int xp = highbit;
    while (xp > 0) {
	--xp;
	std::cout << ((i >> xp) & 1);
    }
}

static int failcnt = 0;
#define ASSERT(x) \
    ((x)? 0 : (std::cout << "** failed: "#x << std::endl, ++failcnt))


int main() {
    bit_subseteq subseteq;
    typedef more::gen::po_map<int, int, bit_subseteq> container;
    typedef container::value_type value_type;

    int repeat = 40;
    bool verbose = false;

    while (repeat--) {
	container C;
	std::set<int> S;

	C.min()->first = 0;
	C.max()->first = (2 << highbit) - 1;

	for (int i = 0; i < 2*(1<<highbit); ++i) {
	    int n = std::rand() % (1<<highbit);

	    std::list<container::iterator> L;
	    C.find_supremums(n, std::back_inserter(L));
	    for (std::list<container::iterator>::iterator it = L.begin();
		 it != L.end(); ++it) {
		if (verbose) {
		    std::cout << ' ';
		    put_binary((*it)->second);
		}
	    }
	    if (verbose) std::cout << " >= ";

	    std::pair<container::iterator, bool> rr = C.insert(value_type(n, n));
	    std::pair<std::set<int>::iterator, bool> rs = S.insert(n);
	    if (verbose) {
		put_binary(n);
		if (rr.second)
		    std::cout << " inserted";
		std::cout << std::endl;
	    }

	    ASSERT(rr.second == rs.second);
	}

	std::set<container::iterator> L;
	more::gen::copy_open_range(C.min(), C.max(), inserter(L, L.begin()));

	for (std::set<container::iterator>::iterator it = L.begin();
	     it != L.end();) {
	    if (rand() % 10 == 0) {
		C.erase(*it);
		std::set<container::iterator>::iterator tmp = it;
		++it;
		L.erase(tmp);
	    }
	    else
		++it;
	}

	std::set<container::iterator> L2;
	more::gen::copy_open_range(C.min(), C.max(), inserter(L2, L2.begin()));
	ASSERT(L == L2);

	for (std::set<container::iterator>::iterator it0 = L.begin();
	     it0 != L.end(); ++it0) {
	    ASSERT(*it0 != C.min());
	    ASSERT(*it0 != C.max());
	    for (std::set<container::iterator>::iterator it1 = L.begin();
		 it1 != L.end(); ++it1) {
		ASSERT(subseteq((*it0)->first, (*it1)->first)
		       == preceq(*it0, *it1));
	    }
	}
    }
    return failcnt? 1 : 0;
}
