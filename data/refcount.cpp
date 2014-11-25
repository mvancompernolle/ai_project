#include <misc/refcount.h>

#include <iostream>
#include <cstdlib>
using namespace std;

class A
{
public:
    A()
    {
        cout << "A()" << endl;
    }

    ~A()
    {
        cout << "~A()" << endl;
    }
};

int main(int argc,char *argv[])
{
    // Create an object dynamically and
    // use a reference-counting pointer.

    ReferenceCountPtr<A> ptr1 = new A();

    // A second reference-counting pointer
    // also points to the same thing.

    {
        ReferenceCountPtr<A> ptr2(ptr1);
        cout << "Destroying ptr2..." << endl;
    }

    // The last remaining pointer will be
    // destroyed, along with the dynamically
    // created A

    cout << "Destroying ptr1..." << endl;

    return EXIT_SUCCESS;
}
