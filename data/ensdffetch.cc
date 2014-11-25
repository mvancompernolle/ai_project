#include "../phys/ens_db.h"
#include <more/io/cmdline.h>
#include <sys/types.h>
#include <sys/stat.h>

int
main(int argc, char** argv) try
{
    using more::phys::ens::ensdb;

    // Make sure the data are readable by anyone.
    umask(022);

    bool force = false;
    more::phys::nucleus nucl;
    more::io::cmdline cl;
    cl.insert_setter("-f|--force", "Force a fetch even if already cached.",
		     force, true);
    cl.insert_reference("", "nucleus#The nucleus", nucl);
    cl.parse(argc, argv);

    ensdb::default_db()->set_restricted();
    std::cout << ensdb::default_db()->fetch_file(nucl, force) << '\n';
    return 0;
} catch (std::exception const& xc) {
    std::cerr << "** " << xc.what() << '\n';
    return 1;
}
