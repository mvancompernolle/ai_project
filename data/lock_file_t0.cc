// The real test: Run this simultaneously on two or more machines in
// the name NFS mounted directory.  Errors should be detected by the
// integrity checks in lock_file.cc.

#include <more/io/lock_file.h>
#include <more/diag/debug.h>
#include <cstdlib>

namespace io = more::io;
namespace diag = more::diag;

int
main()
{
    io::lock_file lk0("tmp.lock_file_t0.lock");
//     io::lock_file lk1("tmp.lock_file_t0_1.lock");

    int n_ok = 0;
    int n_tot = 10000;
    for (int i = 0; i < n_tot; ++i) {
	io::access_t acc;
	switch (std::rand() % 3) {
	case 0:
	    acc = io::access_none;
	    break;
	case 1:
	    acc = io::access_r;
	    break;
	case 2:
	    acc = io::access_rw;
	    break;
	default:
	    assert(!"Not reached.");
	    return 1;
	}
	if (lk0.adjust(acc)) {
	    ++n_ok;
	    std::cout << acc;
	    MORE_CHECK_EQ(lk0.access(), acc);
	}
    }
    std::cout << '\n'
	      << n_ok << " of " << n_tot << " adjustments succeeded."
	      << std::endl;

//     MORE_CHECK_EQ(lk0.access(), io::access_none);
//     MORE_CHECK(lk0.aquire(io::access_rw));
//     MORE_CHECK_EQ(lk0.access(), io::access_rw);
//     MORE_CHECK(!lk1.aquire(io::access_rw));
//     MORE_CHECK_EQ(lk1.access(), io::access_none);
//     MORE_CHECK(!lk1.aquire(io::access_r));
//     MORE_CHECK_EQ(lk1.access(), io::access_none);

//     lk0.release(io::access_w);
//     MORE_CHECK_EQ(lk0.access(), io::access_r);
//     MORE_CHECK(!lk1.aquire(io::access_rw));
//     MORE_CHECK_EQ(lk1.access(), io::access_none);
//     MORE_CHECK(lk1.aquire(io::access_r));
//     MORE_CHECK_EQ(lk1.access(), io::access_r);

    return diag::check_exit_status();
}
