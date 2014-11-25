#include <more/diag/errno.h>
#include <iostream>
#include <cstdio>

namespace diag = more::diag;

int main()
try {
    std::fopen("there_is_no_file_like_this_i_hope", "r");
    throw diag::errno_exception();
    return 1;
}
catch (diag::errno_exception const& xc) {
    std::cerr << "xc.what() = \"" << xc.what() << "\"\n"
	      << "xc.errno() = " << xc.the_errno() << '\n';
    return 0;
}
