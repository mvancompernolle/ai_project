#include <misc/observer.h>

#include <iostream>
using namespace std;

/*
    Subject-Observer Pattern Example

    In this example, we have a class A observing a class B.
    When B is updated, A is notified.

*/

class B : public GltSubject
{
public:
    B() : _v(0)
    {
    }

    void update(int v)
    {
        _v = v;
        notify(this);
    }

    int _v;
};

class A : public GltObserver<B>
{
public:

    void OnNotify(const B *ptr)
    {
        cout << "B updated to " << ptr->_v << endl;
    }
};

int main(int argc,char *argv[])
{
    A a;
    B b;

    // a observes b

    a.observe(b);

    // These two updates should be noticed by a

    b.update(1);
    b.update(2);

    // A copy of the observer does not know about b

    A a2(a);
    b.update(3);

    // A copy of the subject is not automatically observed

    B b2(b);
    b2.update(4);

    // Now forget b, so that further updates are not received by a

    a.forget(b);

    // This update should not be visible to a

    b.update(5);

    return EXIT_SUCCESS;
}
