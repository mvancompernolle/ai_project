#include <more/gen/partial_ordering.h>
#include <more/diag/debug.h>
#include <more/gen/functional.h>

#include <iostream>
#include <list>
#include <set>
#include <cstdlib>
#include <cstring>


struct test_string {
    test_string() : s(""), ok(true) {
	std::cerr << "test_string::test_string() [this = " << this << "]"
		  << std::endl;
    }
    test_string(char const* s_) : s(s_), ok(true) {
	std::cerr << "test_string::test_string(\"" << s << "\") "
		  << "[this = " << this << "]" << std::endl;
    }
    test_string(test_string const& x) : s(x.s), ok(true) {
	std::cerr << "test_string::test_string(test_string const& that) "
		  << "[this = " << this << "; &that = " << &x
		  << "; s= \"" << s << "\"]" << std::endl;
    }
    ~test_string() {
	if (!ok) {
	    std::cerr << "test_string::~test_string() [this = " << this
		      << "; s = \"" << s << "\"]  !!ALREADY DELETED!!"
		      << std::endl;
	    abort();
	}
	assert(s);
	std::cerr << "test_string::~test_string() [this = " << this
		  << "; s = \"" << s << "\"]" << std::endl;
	ok = false;
    }
    test_string& operator=(test_string const& x) { s = x.s; return *this; }

    bool operator==(test_string const& rhs) const {
	return std::strcmp(s, rhs.s) == 0;
    }
    bool operator!=(test_string const& rhs) const {
	return std::strcmp(s, rhs.s) != 0;
    }
    bool operator<(test_string const& rhs) const {
	return std::strcmp(s, rhs.s) < 0;
    }

    friend std::ostream& operator<<(std::ostream& os, test_string const& s) {
	assert(s.s);
	os << s.s;
	return os;
    }

  private:
    char const* s;
    bool ok;
};


template<typename Iterator>
  void show_elements(Iterator first, Iterator last) {
      if (first != last) {
	  std::cout << **first;
	  while (++first != last)
	      std::cout << ' ' << **first;
      }
  }

template<typename PartialOrdering>
  void show_structure(PartialOrdering const& po) {
      typedef typename PartialOrdering::const_element element;
      typedef std::list<element> container;
      container l;
      po.copy_range(po.min(), po.max(), back_inserter(l));
      l.push_back(po.max());
      std::cout << "Partial ordering structure:" << std::endl;
      for (typename container::iterator it = l.begin(); it != l.end(); ++it) {
	  std::cout << "  " << **it;
	  std::list<element> l0;
	  PartialOrdering::copy_supremums(*it, back_inserter(l0));
	  std::cout << "\n    LUBs: ";
	  show_elements(l0.begin(), l0.end());
	  l0.clear();
	  PartialOrdering::copy_infimums(*it, back_inserter(l0));
	  std::cout << "\n    GLBs: ";
	  show_elements(l0.begin(), l0.end());
	  std::cout << std::endl;
      }
      std::cout << std::endl;
  }

template<typename ForwardIterator, typename BinaryFunction>
  void check_transitive(ForwardIterator first, ForwardIterator last,
			BinaryFunction rel) {
      for (ForwardIterator it0 = first; it0 != last; ++it0)
	  for (ForwardIterator it1 = first; it1 != last; ++it1)
	      for (ForwardIterator it2 = first; it2 != last; ++it2)
		  if (rel(*it0, *it1) && rel(*it1, *it2))
		      MORE_CHECK(rel(*it0, *it2));
  }

template<typename ForwardIterator, typename BinaryFunction, typename Equal>
  void check_antisymmetric(
      ForwardIterator first, ForwardIterator last,
      BinaryFunction rel,
      Equal eq =
      std::equal<typename std::iterator_traits<ForwardIterator>::value_type>())
  {
      for (ForwardIterator it0 = first; it0 != last; ++it0)
	  for (ForwardIterator it1 = first; it1 != last; ++it1)
	      if (rel(*it0, *it1) && rel(*it1, *it0))
		  MORE_CHECK(eq(*it0, *it1));
  }
      

void killer_check() {
    std::cout << '[';
    for (int cnt = 0; cnt < 74; ++cnt) {
	std::cout << '.' << std::flush;
	int const n_elm = 1 + std::rand()%39;
	int const n_rel = std::rand()%40;
	typedef more::gen::partial_ordering<int> ordering;
	typedef ordering::element element;

	ordering po;
	*po.min() = -1;
	*po.max() = -2;
	element e[n_elm];

	for (int i = 0; i < n_elm; ++i)
	    e[i] = po.insert(i);

	for (int i = 0; i < n_rel; ++i) {
	    int j = std::rand() % n_elm;
	    int k = std::rand() % n_elm;
	    try {
		po.constrain_prec(e[j], e[k]);
	    }
	    catch (int) {
		std::cout << "\n" << *e[j] << " <= " << *e[k] << " failed\n";
		show_structure(po);
	    }
	}
	std::list<element> es;
	ordering::copy_range(po.min(), po.max(), back_inserter(es));
	check_antisymmetric(es.begin(), es.end(),
			    more::gen::adapt(&ordering::preceq),
			    more::gen::adapt(&ordering::eq));
	check_transitive(es.begin(), es.end(),
			 more::gen::adapt(&ordering::preceq));
    }
    std::cout << ']' << std::endl;
}


int main() /*try*/ {
    typedef test_string string_type;
//    typedef std::string string_type;
    typedef more::gen::partial_ordering<string_type> ordering;
    typedef ordering::element element;
    ordering po;
    *po.min() = "minimum";
    *po.max() = "maximum";
    element e_0 = po.insert("0");
    element e_1_0 = po.insert("1.0");
    element e_1_1 = po.insert("1.1");
    element e_2 = po.insert("2");
    element e_3 = po.insert("3");

    show_structure(po);

    po.constrain_prec(e_0, e_1_0);
    po.constrain_prec(e_0, e_1_1);
    po.constrain_prec(e_1_0, e_2);
    po.constrain_prec(e_1_1, e_2);
    po.constrain_prec(e_2, e_3);

    MORE_CHECK(preceq(e_0, e_1_0));
    MORE_CHECK(preceq(e_1_0, e_2));
    MORE_CHECK(preceq(e_0, e_3));
    MORE_CHECK(preceq(po.min(), e_3));
    MORE_CHECK(!preceq(e_2, e_0));

    std::list<element> l;
    ordering::copy_range(e_0, e_3, std::back_inserter(l));
    std::cout << "range(e_0, e_3) = ";
    show_elements(l.begin(), l.end());
    std::cout << std::endl;

    show_structure(po);
    ordering po0(po);
    std::cout << "A copy of the above:\n";
    show_structure(po0);

    std::cout << "Supremums of " << *e_0 << " are ";
    std::pair<ordering::supremum_iterator, ordering::supremum_iterator>
	sup_rng = supremums(e_0); //ordering::supremum_range(e_0);
    while (sup_rng.first != sup_rng.second) {
	if (0) { sup_rng.first->operator*(); } // just checking
	std::cout << **sup_rng.first << ' ';
	++sup_rng.first;
    }
    std::cout << std::endl;

//    test_unify();
//    killer_check();

    std::cout << "Done." << std::endl;
    return more::diag::check_exit_status();
}
// catch (std::exception const& xc) {
//     std::cerr << "exception: " << xc.what() << std::endl;
//     return 1;
// }
