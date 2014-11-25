#include <iostream>
#include <iomanip>
#include <more/gen/lambda.h>
#include <more/cf/thread.h>
#include <unistd.h>
#include <cassert>

using namespace std;

void f() { throw std::runtime_error("Testing exceptions."); }

int main(int, char**) {
    more::cf::thread th1;
    try {
	more::cf::thread th(more::gen::adapt(f));
	th1 = th;
	th.start();
	sleep(1);
	cerr << "joining thread.\n";
	th.join();
	cerr << "err, exception was not rethrown" << endl;
    } catch(runtime_error const& xc) {
	cerr << "catched runtime error: " << xc.what() << endl;
    } catch(logic_error const& xc) {
	cerr << "catched logic error: " << xc.what() << endl;
    } catch(...) {
	cerr << "catched unkown exception\n";
    }
    cerr << "done." << endl;
    assert(th1);
    return 0;
}

