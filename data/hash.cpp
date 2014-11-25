#include <math/hash.h>

#include <cstdlib>
#include <iostream>
using namespace std;

/*
    Integer Hashing Test
*/

ostream &printBinary(ostream &os,const uint32 v)
{
    for (uint32 i=1<<31; i>0; i>>=1)
        os << (v&i ? '1' : '0');

    return os;
}

int main(int argc,char *argv[])
{
    const int base = 1000;
    const int size = 200;

    for (int i=base; i<base+size; i++)
    {
        cout << i << ' ';
        printBinary(cout,hashKnuth32(i)); cout << ' ';
        printBinary(cout,hashJenkins32(i)); cout << ' ';
        printBinary(cout,hashWang32(i));
        cout << endl;
    }

    return EXIT_SUCCESS;
}
