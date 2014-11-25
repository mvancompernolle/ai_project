#include <more/gen/memory.h>
#include <more/diag/debug.h>
#include <more/cf/thread.h>
#include <algorithm>

using more::gen::weak_ptr;

void
test0()
{
    int* p0 = new(UseGC) int;
    weak_ptr<int> wp0 = p0;
    weak_ptr<int> wp1 = new(UseGC) int;
    weak_ptr<int> wp0a = wp0;
    weak_ptr<int> wp0b(wp0);
    weak_ptr<int> wp0c(p0);
    MORE_CHECK(wp0a == wp0);
    MORE_CHECK(wp0b == wp0);
    MORE_CHECK(wp0c == wp0);
    MORE_CHECK(!(wp0 == wp1));
    MORE_CHECK_EQ(wp0.get(), p0);
    MORE_CHECK_EQ(wp0a.get(), p0);
    MORE_CHECK((wp0 < wp1) != (wp1 < wp0));
}

void
test1()
{
    int const n = 1000;
    weak_ptr<double> warr[n];
    {
	double* arr[n];
	for (int i = 0; i < n; ++i)
	    warr[i] = arr[i] = new(UseGC) double;
	std::sort(warr + 0, warr + n);
    }
    GC_gcollect();
    for (int i = 1; i < n; ++i)
	MORE_CHECK(warr[i - 1] < warr[i]);
}

int
main()
{
    more::cf::notify_foreign_thread();
    for (int i = 0; i < 10000; ++i)
	test0();
    for (int i = 0; i < 100; ++i) {
#if 0 // GC configured with threads
	if (i % 3 == 0) {
	    more::cf::thread th(test1);
	    th.start();
	}
	else
#endif
	    test1();
    }
    return more::diag::check_exit_status();
}
