#include <more/gen/string.h>
#include <assert.h>
#include <iostream>
#include <iomanip>

namespace gen = more::gen;

int
main()
{
    typedef unsigned int uint;
    assert(gen::struniq_static("one") == gen::struniq_static("one"));
    assert(gen::struniq_copy("one") != gen::struniq_copy("two"));
    assert(gen::struniq_static("two") == gen::struniq_copy("two"));
    char const* x1 = gen::struniq_dense(-1);
    char const* x0 = gen::struniq_dense(0);
    char const* x1af = gen::struniq_dense(0x1af);
    assert(gen::struniq_dense(0) == x0);
    assert(gen::struniq_dense(0x1af) == x1af);
    assert(gen::struniq_dense(-1) == x1);
    std::cout << '('
	      << gen::struniq_dense(0) << ','
	      << gen::struniq_dense(-0x13) << ','
	      << gen::struniq_sparse(-0x13) << ','
	      << gen::struniq_dense(3) << ','
	      << gen::struniq_sparse(3) << ','
	      << gen::struniq_dense(0xa8f) << ')'
	      << std::endl;
    return 0;
}
