#include <misc/stlutil.h>

#include <iostream>
using namespace std;

template<class T,class A>
void report(const vector<T,A> &v)
{
    cout << "size: " << v.size() << ", capacity: " << v.capacity() << endl;
}

int main(int argc,char *argv[])
{
    // Initialise the vector

    vector<int> v;
    cout << "create... ";
    report(v);

    // Push three items

    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    cout << "push 3... ";
    report(v);

    // Clear the vector

    v.clear();
    cout << "clear...  ";
    report(v);

    // Shrink it

    shrink(v);
    cout << "shrink... ";
    report(v);

    // Now see what resize does

    v.resize(65);
    cout << "resize 65 ";
    report(v);

    // Push one more item

    v.push_back(0);
    cout << "push...   ";
    report(v);

    // Shrink again

    shrink(v);
    cout << "shrink... ";
    report(v);

    return EXIT_SUCCESS;
}
