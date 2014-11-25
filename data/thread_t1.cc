#include <iostream>
#include <iomanip>
#include <more/gen/lambda.h>
#include <more/cf/thread.h>
#include <unistd.h>

int traces[10];

void run(int value) {
    std::cout << "enter code for thread " << value << std::endl;
    traces[value] = value;
    sleep(value+2);
    std::cout << "exit code for thread " << value << std::endl;
}

int
main(int, char**)
try {
    for(int i = 0; i < 10; i++) traces[i] = 0;
    more::cf::thread th[4];
    for (int i = 0; i < 4; i++)
	th[i].set_call(more::gen::apply(more::gen::adapt(run), i));
    for (int j = 0; j < 4; j++)
	th[j].start();
    bool still;
    int countout = 10;
    do {
	sleep(1);
	std::cout << "alive:";
	still = false;
	for(int j = 0; j < 4; j++) {
	    if (th[j].is_running())
		std::cout << ' ' << j, still = true;
	    else
		std::cout << "  ";
	}
	std::cout << std::endl;
	if (!--countout) {
	    std::cerr << "*** It does not seem line the threads will exit.\n";
	    return 1;
	}
    }
    while (still);
    std::cout << "traces: ";
    for(int i = 0; i < 10; i++)  std::cout << traces[i] << ' ';
    std::cout << std::endl;
    return 0;
} catch (std::exception const& xc) {
    std::cerr << "** exception: " << xc.what() << std::endl;
    return 1;
}
