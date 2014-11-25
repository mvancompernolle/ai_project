#include <misc/string.h>

#include <cstdlib>
#include <iostream>
using namespace std;

int main(int argc,char *argv[])
{
    // Test string with nasty cases included...
    // Expected output is "2 4 6"

    const string buffer =
        "#  shell script style single lines comments are removed\n" \
        "// as well as C++ style single line comments           \n" \
        "\n" /* Blank lines are removed */                          \
        "/* C-style comments can be multi-line *//* But not     \n" \
        "   nested....*/                                        \n" \
        "/* It's also handy to collapse lines of whitespace.. */\n" \
        " \t\n"                                                     \
        "\n"                                                        \
        "  # Like this,                                         \n" \
        "\t// Or this,                                          \n" \
        "/* Or this! */ /* */                                   \n" \
        "\n"                                                        \
        "/* So, now let's try some # strange // cases           \n" \
        "   such as mixing # and // inside a C-style comment  */\n" \
        "# Or having /* things like this // */ that confuse   /*\n" \
        "# the issue well and truely.                           \n" \
        "/* 1 */ 2 /* 3 */ 4 /* 5 */ 6  # 7                     \n";

    // Strip Comments
    string output;
    stripComments(output,buffer);

    // Output
    writeStream(cout,output);

    return EXIT_SUCCESS;
}
