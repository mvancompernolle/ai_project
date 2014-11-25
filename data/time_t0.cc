#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <more/sys/time.h>
#include <more/sys/date.h>
#include <more/diag/debug.h>


namespace sys = more::sys;

void test_parse(std::string s_time) {
    std::istringstream iss_time(s_time);
    sys::date d_time;
    iss_time >> d_time;
	std::cout << '"' << s_time << '"' << std::setw(40 - s_time.size())
		  << "--> ";
    if (iss_time.fail())
	std::cout << "FAIL" << std::endl;
    else
	std::cout << d_time << std::endl;
}

int
main(int argc, char const** argv)
{
    if (argc == 2) {
	sys::date d;
	std::istringstream iss(argv[1]);
	iss >> d;
	std::cout << "I think you entrered " << d << ".\n";
	return 0;
    }
    double t0_proc, t1_proc, t0_real, t1_real, t0_clock, t1_clock;

    t0_real = sys::current_time();
    t0_proc = sys::process_time();
    t0_clock = std::clock()/(double)CLOCKS_PER_SEC;
    volatile int j = 0;
    for (int i = 0; i < 10000000; ++i)
	j += i;
    t1_real = sys::current_time();
    t1_proc = sys::process_time();
    t1_clock = std::clock()/(double)CLOCKS_PER_SEC;

    MORE_SHOW(t1_real - t0_real);
    MORE_SHOW(t1_proc - t0_proc);
    MORE_SHOW(t1_clock - t0_clock);

    t0_real = sys::current_time();
    t0_proc = sys::process_time();
    for (int i = 0; i < 1000; ++i)
	j += i;
    t1_real = sys::current_time();
    t1_proc = sys::process_time();
    MORE_SHOW(t1_real - t0_real);
    MORE_SHOW(t1_proc - t0_proc);

    std::cout << "Current Epoch time is "
	      << std::setprecision(std::numeric_limits<double>::digits10)
	      << sys::current_time() << '\n'
	      << "Current local time is " << sys::date(t1_real) << '\n'
	      << "Current UTC time is   "
	      << sys::date(t1_real, sys::date::within_1_s,
			   (sys::date::timezone_type)0) << '\n';

    test_parse("2001-01-20T12:30:53+0200");
    test_parse("2001-01-20 12:30:53");
    test_parse("2001-01-20t12:30:53");
    test_parse("2001-01-20 12:30:53 trash");
    test_parse("2001-01-20 12:30");
    test_parse("2001-01-20 12:30 trash");
    test_parse("2001-01-20");
    test_parse("2001-01-20 trash");
    test_parse("2001-01");
    test_parse("2001-01 trash");
    test_parse("2001");
    test_parse("2001 trash");
}
