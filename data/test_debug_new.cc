#define MORE_CHECK_ALLOCATIONS 1
#include <more/diag/debug.h>

struct A {
    A() { std::cerr << "constructing A" << std::endl; }
    ~A() { std::cerr << "destructing A" << std::endl; }
};

int main() {
    A* a = MORE_NEW(A);
    MORE_DELETE(A, a);
    return 0;
}

