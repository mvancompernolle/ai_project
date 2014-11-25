#include <more/gen/nary_tree.h>
#include <iostream>
#include <iomanip>
#include <algorithm>


int
main()
{
    typedef more::gen::nary_tree<int> tree_type;
    typedef tree_type::iterator iterator;

    tree_type x;

    iterator it1 = x.insert(x.root().begin(), 1);
    iterator it2 = x.insert(x.root().end(), 2);
    x.insert(x.root().begin(), 3);

    x.insert(it1.begin(), 11);
    x.insert(it1.begin(), 12);
    x.insert(it1.begin(), 13);
    x.insert(it1.begin(), 14);

    x.insert(it2.end(), 21);
    x.insert(it2.end(), 22);
    x.insert(it2.end(), 23);

    tree_type x0(x);
    tree_type y;
    y = x;

    y.erase(y.erase(y.erase(it2.begin())));
    y.erase(--it1.end());
    y.erase(----it1.end());

    subtree_dump(y.root(), std::cout);
    subtree_dump(y.root(), std::cout);

    std::cout << std::endl;
}
