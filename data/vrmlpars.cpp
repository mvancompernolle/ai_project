#include <iostream>
#include <cstdlib>
using namespace std;

#include <mesh/mesh.h>

int main(int argc,char *argv[])
{
    for (int i=1; i<argc; i++)
    {
        Mesh mesh(argv[i]);
        mesh.writeDebugInfo(cout);
    }

    return EXIT_SUCCESS;
}
