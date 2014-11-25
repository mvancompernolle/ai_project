#include <iostream>
#include <iterator>
#include <algorithm>
#include <utility>
#include <string>
#include <more/gen/degenerate_set.h>
#include <assert.h>


int main() {
    typedef more::gen::degenerate_set<char> container;
    typedef container::iterator iterator;
    container s;

    // insert some stuff
    iterator it_a = s.insert('a').first;
    iterator it_b = s.insert('b').first;
    iterator it_c0 = s.insert('c').first;
    iterator it_c1 = s.insert('c').first;
    iterator it_d = s.insert('d').first;

    // iterators
    iterator it_0 = s.begin();
    iterator it_1 = s.begin();
    assert(it_0 == it_1 && !(it_0 != it_1));
    ++it_1;
    assert(!(it_0 == it_1) && it_0 != it_1);
    ++it_0;
    assert(it_0 == it_1 && !(it_0 != it_1));
    iterator it_2; it_2 = it_1;
    iterator it_3(it_1);
    assert(it_2 == it_1 && it_3 == it_1);

    std::copy(s.begin(), s.end(),
	      std::ostream_iterator<char>(std::cout, " "));
    std::cout << std::endl;

    // erase and degeneracy
    std::cout << s.degeneracy('c') << std::endl;
    assert(s.degeneracy('c') == 2);
    assert(s.degeneracy(it_c0) == 2);
    assert(s.degeneracy(it_c1) == 2);
    s.erase(it_c0);
    assert(s.degeneracy('c') == 1);
    assert(s.degeneracy(it_c0) == 1);
    assert(s.degeneracy(it_c1) == 1);
    s.erase(it_c0);
    assert(s.degeneracy('c') == 0);

    std::copy(s.rbegin(), s.rend(),
	      std::ostream_iterator<char>(std::cout, " "));
    std::cout << std::endl;

    it_c0 = s.insert('c').first;
    it_c1 = s.insert('c').first;
    s.erase('c');
    assert(s.degeneracy(it_c1) == 1);

    // copy constructor and copy assignment
    container s0(s);
    container s1; s1 = s;
    assert(s0 == s);
    assert(s1 == s);
    s.insert('x');
    assert(!(s0 == s));
    s.erase('x');
    assert(s0 == s);

    // find
    assert(s.find('a') == it_a);
    assert(s.find('y') == s.end());

    return 0;
}
