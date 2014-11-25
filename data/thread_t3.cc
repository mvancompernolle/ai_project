#include <iostream>
#include <iomanip>
#include <more/gen/functional.h>
#include <more/gen/closure.h>
#include <more/cf/signal.h>
#include <more/cf/thread.h>
#include <unistd.h>

void f()
{
    for(int i = 0; i < 2; ++i) {
	sleep(1);
	more::cf::checkpoint();
    }
    throw std::logic_error("This should not happen.");
}

int main() try {
    more::cf::thread th(more::gen::adapt(f));
    th.start();
    th.cancel();
    th.join();
} catch (std::exception const& xc) {
    std::cerr << "exception: " << xc.what() << std::endl;
    return 1;
} catch (more::cf::thread_canceled const&) {
    std::cerr << "Thread canceled." << std::endl;
    return 0;
}

