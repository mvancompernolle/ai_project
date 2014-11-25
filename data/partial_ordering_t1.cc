#include <more/gen/partial_ordering.h>
#include <more/gen/po_algo.h>
#include <more/diag/debug.h>
#include <more/gen/functional.h>
#include <algorithm>
#include <list>
//#include <string>
#include <cstring>



struct c_str_less : std::binary_function<char const*, char const*, bool>
{
    bool
    operator()(char const* s0, char const* s1) const
    {
	return std::strcmp(s0, s1) < 0;
    }
};

struct bit_subset : std::binary_function<unsigned int, unsigned int, bool>
{
    bool
    operator()(unsigned int x, unsigned int y) const
    {
//	return (x | y) == y;
	return ((x | y) == y && x != y);
    }
};

namespace gen = more::gen;

void chkmany()
{
    // XXX
    int const n_elt = 40;
    int const mask = 0xff;
    typedef gen::partial_ordering<unsigned int> container;
    typedef container::element element;

    bit_subset preceq;
    container po;
    *po.min() = 0;
    *po.max() = mask;
    element e[n_elt];

    std::cout.setf(std::ios_base::hex);
    for (int i = 0; i < n_elt; ++i) {
	int value = rand() & mask;
	std::cout << "\nInsering " << value << ": ";
	std::set<element> S_inf;
	gen::find_infimums(po.min(), value, preceq,
			   std::inserter(S_inf, S_inf.begin()));
	e[i] = po.insert(value);
	for (int j = 0; j < i; ++j) {
	    if (preceq(*e[i], *e[j])) {
		std::cout << *e[i] << " <= " << *e[j] << "; ";
		po.constrain_prec(e[i], e[j]);
	    }
	    if (preceq(*e[j], *e[i])) {
		std::cout << *e[j] << " <= " << *e[i] << "; ";
		po.constrain_prec(e[j], e[i]);
	    }
	}
	std::set<element> S_inf2;
	std::copy(e[i].infimums_begin(), e[i].infimums_end(),
		  std::inserter(S_inf2, S_inf2.begin()));
	S_inf.erase(po.min());
	S_inf2.erase(po.min());
	if (!S_inf.empty() || !S_inf2.empty()) {
	    std::cout << "\n   S_inf = ";
	    std::transform(S_inf.begin(), S_inf.end(),
			   gen::ostream_iterator<unsigned int>(std::cout, ", "),
			   gen::dereferencer<element>());
	    std::cout << "\n  S_inf2 = ";
	    std::transform(S_inf2.begin(), S_inf2.end(),
			   gen::ostream_iterator<unsigned int>(std::cout, ", "),
			   gen::dereferencer<element>());
	    std::cout << std::endl;
	}
	MORE_CHECK(S_inf == S_inf2);
    }
}


int main() {
#if 1
    typedef char const* string_type;
    c_str_less string_less;
#else
//    typedef std::string string_type;
    typedef test_string string_type;
    std::less<test_string> string_less;
#endif

    typedef gen::partial_ordering<string_type> container;
    typedef container::element element;

    container po;
    *po.min() = "aa_min";
    *po.max() = "zz_max";
    element e_alpha0 = po.insert("alpha0");
    element e_alpha1 = po.insert("alpha1");
    element e_beta0 = po.insert("beta0");
    element e_beta1 = po.insert("beta1");
    element e_gamma0 = po.insert("gamma0");
    element e_gamma1 = po.insert("bgamma1");
    element e_gamma2 = po.insert("gamma2");
    po.constrain_prec(e_alpha0, e_beta0);
    po.constrain_prec(e_alpha1, e_beta0);
    po.constrain_prec(e_alpha0, e_beta1);
    po.constrain_prec(e_alpha1, e_beta1);
    po.constrain_prec(e_beta0, e_gamma0);
    po.constrain_prec(e_beta0, e_gamma1);
    po.constrain_prec(e_beta0, e_gamma2);
    po.constrain_prec(e_beta1, e_gamma0);
    po.constrain_prec(e_beta1, e_gamma1);
    po.constrain_prec(e_beta1, e_gamma2);

    std::list<element> l_infs;
    gen::find_infimums(po.min(), string_type("bzeta"),
		      string_less, back_inserter(l_infs));
    std::cout << "Found " << l_infs.size() << " infimums: ";
    std::transform(l_infs.begin(), l_infs.end(),
		   gen::ostream_iterator<string_type>(std::cout, ", "),
		   gen::dereferencer<element>());
    std::cout << std::endl;

    container po0(po), po1;
    po1 = po;
//     MORE_CHECK(po0 == po);
//     MORE_CHECK(po1 == po);
    po1.insert("make a difference");
//     MORE_CHECK(po != po1);

    chkmany();

    return more::diag::check_exit_status();
}
