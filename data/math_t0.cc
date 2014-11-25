#include <more/math/math.h>
#include <more/diag/debug.h>
#include <cstdlib>


int
main()
{
    using more::math::lower_bit;
    using more::math::upper_bit;
    for (int i = 0; i < 10000; ++i) {
	unsigned int n = std::rand();
	if (n == 0)
	    continue;

	int l = lower_bit(n);
	int u = upper_bit(n);
	MORE_CHECK(n & (1 << l));
	MORE_CHECK(n & (1 << u));
	if ((1 << u + 1) != 0)
	    MORE_CHECK((n & ((1 << u + 1) - (1 << l))) == n);
    }

    MORE_CHECK_EQ(lower_bit(5), 0);
    MORE_CHECK_EQ(lower_bit(0x260), 5);
    return more::diag::check_exit_status();
}
