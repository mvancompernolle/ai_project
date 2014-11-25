#include <more/math/hash.h>
#include <more/io/cmdline.h>
#include <iostream>
#include <iomanip>
#include <set>
#include <cmath>

template <int NBits>
void
check(unsigned int const n_rpt, unsigned int const n_ins)
{
    int const n_bits = NBits;

    typedef more::math::hash<n_bits> hash_type;
    typedef std::set<hash_type> set_type;
    int n_dup = 0;
    int i_print = 40;

    long double p0 = std::ldexp(1.0, -n_bits);
    long double p_no_dup = 1.0;
    for (unsigned int i = 0; i < n_ins; ++i)
	p_no_dup *= 1.0 - p0*i;
    std::cout << "p_dup_avg = " << n_dup/(long double)n_rpt << std::endl;

    for (unsigned int i_rpt = 0; i_rpt < n_rpt; ++i_rpt) {
	set_type S;
	int i_print_ins = 0;
	if (i_print) {
	    --i_print;
	    i_print_ins = 76*16/(5*n_bits);
	    std::cout << '\n';
	}
	for (unsigned int i_ins = 0; i_ins < n_ins; ++i_ins) {
	    hash_type h(0);
	    h.insert(i_ins*16);
	    for (int j = 0; j < 50; ++j)
		h.insert(j + i_rpt*16);
	    if (i_print_ins) {
		--i_print_ins;
		std::cout << h << ' ';
	    }
	    if (!S.insert(h).second) {
		++n_dup;
		std::cout << "[" << S.size() << "]";
		break;
	    }
	}
    }
    std::cout << "\n\n";

    std::cout << "p_dup_avg = " << n_dup/(double)n_rpt << '\n'
	      << "p_dup_th  = " << (1.0 - p_no_dup)
	      << std::endl;
}

int
main(int argc, char** argv)
try {
    int n_ins = 1 << 8;
    int n_rpt = 10000;
    int n_bits = 32;
    more::io::cmdline cl;
    cl.insert_reference("", "n_rpt#Number of repetitions.", n_rpt);
    cl.insert_reference("", "n_ins#Number of insertions.", n_ins);
//      cl.insert_setter("--16",  "Use 16 bits hash.", n_bits, 16);
    cl.insert_setter("--32",  "Use 32 bits hash.", n_bits, 32);
    cl.insert_setter("--64",  "Use 64 bits hash.", n_bits, 64);
    cl.insert_setter("--128", "Use 128 bits hash.", n_bits, 128);
    cl.insert_setter("--96", "Use 96 bits hash.", n_bits, 96);
    cl.insert_setter("--192", "Use 192 bits hash.", n_bits, 192);
    cl.parse(argc, argv);
//      if (n_bits == 16)
//  	check<16>(n_rpt, n_ins);
    switch (n_bits) {
      case 32:
  	check<32>(n_rpt, n_ins);
	break;
      case 64:
 	check<64>(n_rpt, n_ins);
	break;
      case 96:
	check<96>(n_rpt, n_ins);
	break;
      case 128:
	check<128>(n_rpt, n_ins);
	break;
      case 192:
	check<192>(n_rpt, n_ins);
	break;
    }
    return 0;
} catch (more::io::cmdline::relax) {
    return 0;
} catch (std::exception const& xc) {
    std::cerr << "** " << xc.what() << std::endl;
    return 1;
}

