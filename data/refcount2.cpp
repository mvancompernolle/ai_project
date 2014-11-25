#include <misc/refcount.h>

#include <iostream>
#include <cstdlib>
using namespace std;

int main(int argc,char *argv[])
{
    ReferenceCountPtr<int> ptr = new int;

    // Access the dynamically allocated
    // integer

    *ptr = 12345;

    //

    cout << *ptr << endl;
    cout << *(ptr.get()) << endl;

    // Create a second pointer to
    // the same object

    ReferenceCountPtr<int> ptr2(ptr);

    cout << *ptr2 << endl;

    // Resetting ptr2 should leave ptr
    // intact

    ptr2.clear();
    cout << ptr2.get() << endl;
    cout << *ptr << endl;

    // Resetting ptr...

    ptr.clear();
    cout << ptr.get() << endl;

    return EXIT_SUCCESS;
}
