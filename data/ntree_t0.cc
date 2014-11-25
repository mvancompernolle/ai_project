#include <more/gen/ntree.h>
#include <iostream>
#include <assert.h>

namespace gen = more::gen;

template<typename Tree>
void
dump(Tree t, std::ostream& os,
     int level = 0, unsigned int cont = 0)
{
    if (level) {
	for (int i = 0; i < level; ++i)
	    os << (cont & (1<<i)? "|  " : "   ");
	os << "|\n";
    }
    for (int i = 0; i < level; ++i)
	os << (cont & (1<<i)? "|  " : "   ");
    os << (level? '+' : '-')
       << (t.branches().empty()? "----" : "--+-") << t.value() << '\n';
    cont |= 2<<level;
    for (typename Tree::branch_const_iterator
	     it = t.branches().begin(); it != t.branches().end(); ++it) {
	if (it == --t.branches().end())
	    cont &= ~(2<<level);
	dump(*it, os, level+1, cont);
    }
}

template<typename Tree>
void
check_stem_links(Tree const& t)
{
    for (typename Tree::branch_const_iterator it = t.branches().begin();
	 it != t.branches().end(); ++it) {
	assert(!it->is_root());
	assert(&it->stem() == &t);
	check_stem_links(*it);
    }
}



int
main()
{
    typedef gen::ntree<std::string> tree;
    typedef tree::branch_iterator iterator;
    tree t;
    t.value() = "root";
    t.branches().push_back("branch0");
    tree& t0 = t.branches().back();
    t0.branches().push_back("leaf0");
    t0.branches().push_back("leaf1");
    t.branches().push_back("leaf2");
    dump(t, std::cout);

    tree t_copy(t);
    tree t_assigned = t;
    assert(t == t_copy);
    assert(t == t_assigned);

    tree::swapper t_swapper(t);
    tree t_swapped(t_swapper);
    assert(t_swapped == t_copy);

    // Check stem links
    assert(t.is_root());
    assert(t_copy.is_root());
    assert(t_swapped.is_root());
    check_stem_links(t);
    check_stem_links(t_copy);
    check_stem_links(t_swapped);

    return 0;
}
