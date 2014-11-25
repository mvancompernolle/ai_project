#include <iostream>
#include <iterator>
#include <algorithm>
#include <utility>
#include <string>
#include <more/gen/degenerate_map.h>
#include <assert.h>


int main() {
    typedef more::gen::degenerate_map<char, int> container;
    typedef container::iterator iterator;
    typedef container::value_type value_type;
    container s;

    // insert some stuff
    iterator it_a = s.insert(value_type('a', 0)).first;
    iterator it_b = s.insert(value_type('b', 0)).first;
    iterator it_c0 = s.insert(value_type('c', 0)).first;
    iterator it_c1 = s.insert(value_type('c', 0)).first;
    iterator it_d = s.insert(value_type('d', 0)).first;

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

    it_c0 = s.insert(value_type('c', 0)).first;
    it_c1 = s.insert(value_type('c', 0)).first;
    s.erase('c');
    assert(s.degeneracy(it_c1) == 1);

    // copy constructor and copy assignment
    container s0(s);
    container s1; s1 = s;
    assert(s0 == s);
    assert(s1 == s);
    s.insert(value_type('x', 0));
    assert(!(s0 == s));
    s.erase('x');
    assert(s0 == s);

    // find
    assert(s.find('a') == it_a);
    assert(s.find('y') == s.end());

    return 0;
}
