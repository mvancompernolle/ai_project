#include <more/io/cmdline.h>
#include <more/phys/ens.h>

namespace io = more::io;   // IO namespace including the command-line parser.
namespace phys = more::phys;	// Physics namespace
namespace ens = phys::ens;	// The ENSDF data structures
namespace si = phys::si;	// SI units

int
main(int argc, char** argv)
try {
    // Parse command-line arguments.
    io::cmdline cl;
    phys::nucleus nucl;
    cl.insert_reference("", "X#A nucleus.", nucl);
    cl.parse(argc, argv);

    // Obtain the datasets.
    ens::nucleus ens_nucl(nucl);

    // Find the dataset for electron capture and beta+
    ens::nucleus::dataset_iterator it_ds
	= ens_nucl.dataset_begin();
    while (it_ds != ens_nucl.dataset_end()) {
	std::string idn = it_ds->ident()->dataset_id_string();
	if (idn.find("EC DECAY") != std::string::npos)
	    break;
	++it_ds;
    }
    if (it_ds == ens_nucl.dataset_end()) {
	std::cout << "There is no EC dataset for this nucleus.\n";
	return 0;
    }

    // Each dataset may contain a range of levels for the nucleus
    for (ens::dataset::level_iterator it_lev = it_ds->level_begin();
	 it_lev != it_ds->level_end(); ++it_lev) {

	// The level may contain a range of radiation records for
	// decays with this level as the _final_ state.
	for (ens::rec_level::radiation_iterator
		 it_rad = it_lev->radiation_begin();
	     it_rad != it_lev->radiation_end(); ++it_rad) {

	    // We are only interested in the rediation if it comes
	    // form beta decay.
	    if (ens::rec_beta_pl const* beta_pl = it_rad->to_beta_pl()) {

		// Print the energy of the level.  This energy may be
		// specified relative to some reference point.
		std::cout << std::setw(24)
			  << it_lev->E_minus_E_ref()/si::MeV;
		ens::print_energy_ref(std::cout, it_lev->i_E_ref());

		// Print the log(f T[1/2]) value.
		std::cout << "log10(f T[1/2]) = "
			  << beta_pl->log_ft() << '\n';
	    }
	}
    }
    return 0;
}
catch (io::cmdline::relax) { // Cf. more::phys::cmdline (option --help)
    return 0;
}
catch (std::exception const& xc) {
    std::cerr << "** exception: " << xc.what() << '\n';
    return 1;
}
