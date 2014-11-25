#include <more/sys/system.h>
#include <list>
#include <string>

int
main(int argc, char** argv)
{
    {
	std::list<std::string> args;
	args.push_back("Hello");
	args.push_back("world");
	more::sys::system("echo", args.begin(), args.end());
    }

    if (argc < 2)
	return 0;
    else
	return more::sys::system(argv[1], &argv[2], &argv[argc]);
}
