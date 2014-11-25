#include <misc/lru.h>
#include <math/random.h>

#include <cstdlib>
#include <string>
#include <iostream>
using namespace std;

void print(const lru<uint32,char> &cache)
{
    for (uint32 i=0; i<cache.size(); i++)
        cout << cache[i];
    cout << endl;
}

int main(int argc,char *argv[])
{
    int i;
    const int n=26;
    lru<uint32,char> cache;

    {

        // Insert items in abc order

        for (i=0; i<n; i++)
            cache.insert(i,'a'+char(i));

        print(cache);

        // Lookup items in reverse order

        for (i=n-1; i>=0; i--)
            cache.find(i);

        print(cache);

        // Do lookup without touching

        for (i=0; i<n; i++)
            cache.find(i,false);

        print(cache);
    }

    // Randomly touch 4 each iteration

    cout << endl;

    GltRandomInteger<> r(0,n);

    for (i=0; i<20; i++)
    {
        for (int j=0; j<4; j++)
            cache.find(r.rand());

        print(cache);
    }

    // Randomly touch 400 each iteration

    cout << endl;

    for (i=0; i<20; i++)
    {
        for (int j=0; j<400; j++)
            cache.find(r.rand());

        print(cache);
    }

    return EXIT_SUCCESS;
}
