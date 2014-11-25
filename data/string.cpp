#include <misc/string.h>
#include <misc/hex.h>
#include <misc/stlutil.h>

#define SHOW(x) #x"= " << (x)

#include <iostream>
#include <vector>
#include <string>
using namespace std;

int main(int argc,char *argv[])
{
    // fromHex

    cout << SHOW((int)fromHex('0')) << endl;
    cout << SHOW((int)fromHex('1')) << endl;
    cout << SHOW((int)fromHex('2')) << endl;
    cout << SHOW((int)fromHex('3')) << endl;
    cout << SHOW((int)fromHex('4')) << endl;
    cout << SHOW((int)fromHex('5')) << endl;
    cout << SHOW((int)fromHex('6')) << endl;
    cout << SHOW((int)fromHex('7')) << endl;
    cout << SHOW((int)fromHex('8')) << endl;
    cout << SHOW((int)fromHex('9')) << endl;
    cout << SHOW((int)fromHex('a')) << endl;
    cout << SHOW((int)fromHex('A')) << endl;
    cout << SHOW((int)fromHex('b')) << endl;
    cout << SHOW((int)fromHex('B')) << endl;
    cout << SHOW((int)fromHex('c')) << endl;
    cout << SHOW((int)fromHex('C')) << endl;
    cout << SHOW((int)fromHex('d')) << endl;
    cout << SHOW((int)fromHex('D')) << endl;
    cout << SHOW((int)fromHex('e')) << endl;
    cout << SHOW((int)fromHex('E')) << endl;
    cout << SHOW((int)fromHex('f')) << endl;
    cout << SHOW((int)fromHex('F')) << endl;

    // toHex

    cout << SHOW((int)toHex( 0)) << endl;
    cout << SHOW((int)toHex( 1)) << endl;
    cout << SHOW((int)toHex( 2)) << endl;
    cout << SHOW((int)toHex( 3)) << endl;
    cout << SHOW((int)toHex( 4)) << endl;
    cout << SHOW((int)toHex( 5)) << endl;
    cout << SHOW((int)toHex( 6)) << endl;
    cout << SHOW((int)toHex( 7)) << endl;
    cout << SHOW((int)toHex( 8)) << endl;
    cout << SHOW((int)toHex( 9)) << endl;
    cout << SHOW((int)toHex(10)) << endl;
    cout << SHOW((int)toHex(11)) << endl;
    cout << SHOW((int)toHex(12)) << endl;
    cout << SHOW((int)toHex(13)) << endl;
    cout << SHOW((int)toHex(14)) << endl;
    cout << SHOW((int)toHex(15)) << endl;

    // Templated toHex

    cout << SHOW(toHex(      0.0f)) << endl;
    cout << SHOW(toHex(      1.0f)) << endl;
    cout << SHOW(toHex(      2.0f)) << endl;
    cout << SHOW(toHex(      4.0f)) << endl;
    cout << SHOW(toHex(1234.5678f)) << endl;
    cout << SHOW(toHex(0.1234567f)) << endl;

    cout << SHOW(toHex(      0.0)) << endl;
    cout << SHOW(toHex(      1.0)) << endl;
    cout << SHOW(toHex(      2.0)) << endl;
    cout << SHOW(toHex(      4.0)) << endl;
    cout << SHOW(toHex(1234.5678)) << endl;
    cout << SHOW(toHex(0.1234567)) << endl;

    // Templated fromHex

    cout << SHOW(fromHex<float>("0x00000000")) << endl;
    cout << SHOW(fromHex<float>("0x0000803F")) << endl;
    cout << SHOW(fromHex<float>("0x00000040")) << endl;
    cout << SHOW(fromHex<float>("0x00008040")) << endl;
    cout << SHOW(fromHex<float>("0x2B529A44")) << endl;
    cout << SHOW(fromHex<float>("0xDED6FC3D")) << endl;

    cout << SHOW(fromHex<double>("0x0000000000000000")) << endl;
    cout << SHOW(fromHex<double>("0x000000000000F03F")) << endl;
    cout << SHOW(fromHex<double>("0x0000000000000040")) << endl;
    cout << SHOW(fromHex<double>("0x0000000000001040")) << endl;
    cout << SHOW(fromHex<double>("0xADFA5C6D454A9340")) << endl;
    cout << SHOW(fromHex<double>("0x72DAF8B8DB9ABF3F")) << endl;

    // sprintf

    string tmp;
    cout << SHOW(sprintf(tmp,"%d %d %s",1,2,"Hello World")) << endl;
    cout << tmp << endl;

    // ato*

    cout << SHOW(atof("   0.001")) << endl;
    cout << SHOW(atof("  -0.001")) << endl;
    cout << SHOW(atof(" 100.001")) << endl;
    cout << SHOW(atof("-100.001")) << endl;
    cout << SHOW(atof("    1e10")) << endl;
    cout << SHOW(atof("   1e-10")) << endl;

    cout << SHOW(atoi("-123456")) << endl;
    cout << SHOW(atoi("      0")) << endl;
    cout << SHOW(atoi(" 123456")) << endl;

    cout << SHOW(atol("-1234567890")) << endl;
    cout << SHOW(atol("+1234567890")) << endl;

    cout << SHOW(atob("false")) << endl;
    cout << SHOW(atob("true")) << endl;
    cout << SHOW(atob("0")) << endl;
    cout << SHOW(atob("1")) << endl;
    cout << SHOW(atob("-1")) << endl;

    // atoc

    {
        vector<int> tmp(5);

        cout << SHOW(atoc("1,2,3,4,5",atoi,"+-0123456789",tmp.begin(),tmp.end())) << endl;
        print(cout,tmp) << endl;

        cout << SHOW(atoc("+1,-2,+3,-4,+5",atoi,"+-0123456789",tmp.begin(),tmp.end())) << endl;
        print(cout,tmp) << endl;
    }

    {
        vector<bool> tmp(5);

        cout << SHOW(atoc("true false 1 0 -1",atob,"+-01truefalse",tmp.begin(),tmp.end())) << endl;
        print(cout,tmp) << endl;
    }

    // split

    {
        vector<string> tmp;

        cout << SHOW(stringSplit(tmp,"One Two Three Four Five"," ")) << endl;
        print(cout,tmp) << endl;

        string tmp2;
        cout << SHOW(stringMerge(tmp,tmp2," ")) << endl;
        cout << tmp2 << endl;
    }

    // bin2src

    unsigned char buffer[10] = { 0,1,2,3,4,5,6,7,8,9 };
    bin2src(cout,buffer,10);

    bin2src(cout,"0123456789abcde");

    return EXIT_SUCCESS;
}

