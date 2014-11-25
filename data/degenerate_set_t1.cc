#include <iostream>
#include <iterator>
#include <algorithm>
#include <utility>
#include <string>
#include <more/gen/degenerate_set.h>
#include <assert.h>


int main() {
    using std::cout;
    using std::endl;

    typedef more::gen::degenerate_set<std::string> cont;
    typedef cont::iterator iter;

    cont s;
    s.insert("has");
    s.insert("been");
    s.insert("aik");
    s.insert("has");
    s.insert("has");
    s.insert("aik");
    std::copy(s.begin(), s.end(),
	      more::gen::ostream_iterator<std::string>(cout, " "));
    cout << endl;
    std::copy(s.rbegin(), s.rend(),
	      more::gen::ostream_iterator<std::string>(cout, " "));
    cout << endl;
    s.erase("aik");
    s.erase("aik");
    s.erase("has");
    std::copy(s.begin(), s.end(),
	      more::gen::ostream_iterator<std::string>(cout, " "));
    cout << endl;
    assert(s.degeneracy("has") == 2);
    assert(s.degeneracy("been") == 1);
    iter u = s.find("has"), v = s.find("none");
    assert(u != s.end());
    assert(v == s.end());
    assert(s.degeneracy(u) == 2);
    assert(s.degeneracy("none") == 0);
}
