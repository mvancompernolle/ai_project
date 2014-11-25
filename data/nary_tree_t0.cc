#include <more/gen/nary_tree.h>
#include <iostream>

namespace gen = more::gen;

int
main()
{
    typedef gen::nary_tree<std::string> tree_type;

    tree_type t_orig;
    t_orig.set_root("root");
    tree_type::iterator it_branch
	= t_orig.insert(t_orig.root().end(), "branch");
    t_orig.insert(it_branch.end(), "leaf0");
    t_orig.insert(it_branch.end(), "leaf1");
    t_orig.insert(t_orig.root().end(), "leaf2");
    subtree_dump(t_orig.root(), std::cout);

    tree_type t_copy(t_orig);

    tree_type::swapper t_swapper_0(t_orig);
    tree_type::swapper t_swapper_1(t_swapper_0);
    tree_type t_swapped(t_swapper_1);
    subtree_dump(t_swapped.root(), std::cout);

    assert(t_copy == t_swapped);
    return 0;
}
