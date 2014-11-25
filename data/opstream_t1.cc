#include <more/io/opstream.h>
#include <iostream>

int main() {
    more::io::opstream os("/bin/sh");
    os << "echo \"After this line we flush the stream\"" << std::endl;
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"This sentence is repeated to fill up the streambuf.\"\n";
    os << "echo \"After this line we flush the stream\"" << std::endl;
    os << "echo \"and after this\"" << std::endl;
    os << "echo \"Here is the output of \\`ls'\"\n";
    os << "ls\n";
    os << std::flush;
    if (os.fail()) {
	std::cerr << "os.fail()\n";
	return 1;
    }
    else
	return 0;
}
