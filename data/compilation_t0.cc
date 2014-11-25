#include <more/lang/compilation.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <more/sys/system.h>
#include <more/io/filesys.h>
#include <ltdl.h>

namespace io = more::io;
namespace lang = more::lang;

int
main() try
{
    {
	lang::compilation cn(".", ".");
	std::string src = cn.insert_source("compilation_t0_test",
					   lang::lang_cxx);
	cn.set_target_program("compilation_t0_test");
	{
	    std::ofstream os(src.c_str());
	    os << "#include <iostream>\n"
	       << "int main() {\n"
	       << "    std::cout << \"Hello jugglers!\\n\";\n"
	       << "    return 0;\n"
	       << "}\n";
	}
	if (!cn.make_all())
	    throw std::runtime_error("Failed to make a program.");
	if (more::sys::system("./" + cn.program_file_name()) != 0)
	    throw std::runtime_error("Running the program failed.");
    }
    {
	lt_dlinit();
	lang::compilation cn(".", ".",
			     io::absolute_file_name("test/include"),
			     io::absolute_file_name("test/lib"));
	std::string src = cn.insert_source("compilation_t0_test",
					   lang::lang_cxx);
	std::string hdr = cn.insert_header("compilation_t0_test",
					   lang::lang_cxx);
	cn.set_target_library("compilation_t0_test", true);
	{
	    std::ofstream os(src.c_str());
	    os << "#include <iostream>\n"
	       << "extern \"C\""
	       << "int helloo() {\n"
	       << "    std::cout << \"Hello jugglers!\\n\";\n"
	       << "    return 179;\n"
	       << "}\n";
	}
	{
	    std::ofstream os(hdr.c_str());
	    os << "// Just testing\n";
	}
	if (!cn.make_all())
	    throw std::runtime_error("Failed to make a dlopen target.");
	std::string dlname = cn.library_file_name();
	std::clog << "Trying to dlopen library " << dlname << std::endl;
	lt_dlhandle dl = lt_dlopen(dlname.c_str());
	if (dl == 0)
	    throw std::runtime_error(lt_dlerror());
	std::clog << "Trying to get a symbol." << std::endl;
	void* sym = lt_dlsym(dl, "helloo");
	if (sym == 0)
	    throw std::runtime_error(lt_dlerror());
	if (((int (*)())sym)() != 179)
	    throw std::runtime_error("Bad return value from dl-linked piece.");
	if (!cn.make_install())
	    throw std::runtime_error("Could not install.");
    }
    return 0;
} catch (std::runtime_error const& xc) {
    std::cerr << "** " << xc.what() << std::endl;
    return 1;
}
