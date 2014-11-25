
#include "parser.h"

#include <getopt.h>

#include <iostream>
#include <list>
#include <boost/tokenizer.hpp>

using namespace std;

void usage(const char* name)
{
	std::cerr<<
	         "Usage:\n\t"<<name<<" [--eat-spaces|-e] [--include=path|-I path...] [--split-definition|-s] [--gen-deps|-g] input.csp output[.cpp]"<<std::endl;
	exit(1);
}

int main(int argc, char** argv)
{
	bool eat_spaces(false), gen_deps(false), split_def(false);
	std::string class_name;
	const char *input(0), *output(0), **curfile(&input);
	static struct option long_opts[] = 
	{
		// name          |has arg|flag|
		{ "eat-spaces", 0,      0,   'e' },
		{ "include",    1,      0,   'I' },
		{ "gen-deps",   0,      0,   'd' },
		{ "split-definition", 0,0,   's' },
		{ "class-name", 1,      0,   'c' },
		{ 0,              0,      0,   0   }
	};
	std::list<std::string> search_path;
	const char* esp = ::getenv("CSP_INCLUDE_PATH");
	if(esp)
	{
		std::string env_search_path(esp);
		boost::char_separator<char> sep(":","");
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		tokenizer t(env_search_path, sep);
		for(tokenizer::const_iterator it = t.begin(); it!=t.end(); it++)
			search_path.push_front(*it);
	}
	for(;;)
	{
		int option_index(0);
		int c = getopt_long(argc, argv, "eI:dsc:", long_opts, &option_index);
		if(c==-1)
			break;
		switch(c)
		{
		case 'e':
			eat_spaces=true;
			break;
		case 'I':
			search_path.push_front(optarg);
			break;
		case 'd':
			gen_deps=true;
			break;
		case 's':
			split_def=true;
			break;
		case 'c':
			class_name = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	if(argc-optind != 2) {
		std::cerr<<"Input and/or output file aren't specified"<<std::endl;
		usage(argv[0]);
	}
	input=argv[optind];
	output=argv[optind+1];
	cxxsp::parser parser(input, output, class_name, eat_spaces, gen_deps, split_def);
	for(std::list<std::string>::iterator it = search_path.begin(); it!=search_path.end(); it++)
		parser.addIncludeDir(*it);
	if(!parser.parse())
		return 1;
	return 0;
}
