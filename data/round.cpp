#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace std;

#include <math/round.h>
#include <math/real.h>

#define ios_base ios

//
// Test GLT round() function
//
// Based on a test program by Stefan Greuter
//

static const float testf[] =
{
    0.473829f,
    0.233452f,
    0.967854f,
    0.999999f,
     1.49999f,
     1.50000f,
     1.50001f,
    -1.49999f,
    -1.50000f,
    -1.50001f,
    0.0f
};

static const double testd[] =
{
    0.123456789012345,
    0.324456778890923,
    0.289374592837459,
    0.929650293842544,
    0.059668309829873,
     1.49999999,
     1.50000000,
     1.50000001,
    -1.49999999,
    -1.50000000,
    -1.50000001,
    0.0
};

static const double testl[] =
{
    1.0,
    10.0,
    100.0,
    1000.0,
    10000.0,
    100000.0,
    1000000.0,
    10000000.0,
    100000000.0,
    1000000000.0,
    -1000000000.0,
    -100000000.0,
    -10000000.0,
    -1000000.0,
    -100000.0,
    -10000.0,
    -1000.0,
    -100.0,
    -10.0,
    -1.0,
    0.0
};

void testRoundFloat()
{
    cout << endl << "float" << endl << endl << setiosflags(ios_base::fixed);
    for (const float *i=testf; *i!=0.0f; i++)
    {
        for (int p=5; p>=0; p--)
            cout << setprecision(p) << setw(p+3) << round(*i,p) << ' ';
        cout << endl;
    }
}

void testRoundDouble()
{
    cout << endl << "double" << endl << endl << setiosflags(ios_base::fixed);
    for (const double *i=testd; *i!=0.0; i++)
    {
        for (int p=8; p>=0; p--)
            cout << setprecision(p) << setw(p+3) << round(*i,p) << ' ';
        cout << endl;
    }
}

void testRoundLong()
{
    cout << endl << "long" << endl << endl << resetiosflags(static_cast<ios_base::fmtflags>(~0));
    for (const double *i=testl; *i!=0.0; i++)
        cout << setw(12) << *i << ' ' << setw(12) << long(round(*i,0)) << endl;
}

void testPower2()
{
    cout << endl;
    for (uint32 i=0; i<34; i++)
        cout << i << '\t' << lowerPowerOf2(i) << '\t' << upperPowerOf2(i) << endl;
}

void testFraction()
{
    cout << endl << "fraction" << endl;
    for (uint32 i=31; i<32; i--)
    {
        float  f1;
        double f2;

        fraction(f1,1<<i);
        fraction(f2,1<<i);

        cout << i << '\t' << (unsigned int) (1<<i) << '\t' << f1 << '\t' << f2 << endl;
    }
}

int main(int argc,char *argv[])
{
    cout << setprecision(10) << endl;

    testRoundFloat();
    testRoundDouble();
    testRoundLong();
    testPower2();
    testFraction();

    return EXIT_SUCCESS;
}

