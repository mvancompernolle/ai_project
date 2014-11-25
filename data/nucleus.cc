//  Copyright (C) 2000--2009  Petter Urkedal
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  As a special exception, you may use this file as part of a free
//  software library without restriction.  Specifically, if other files
//  instantiate templates or use macros or inline functions from this
//  file, or you compile this file and link it with other files to
//  produce an executable, this file does not by itself cause the
//  resulting executable to be covered by the GNU General Public
//  License.  This exception does not however invalidate any other
//  reasons why the executable file might be covered by the GNU General
//  Public License.


#include <more/io/cmdline.h>
#include <more/math/math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <more/phys/nuclear.h>
#include <more/phys/ens.h>
#include <more/phys/si.h>
#include "../phys/ens_db.h"

namespace phys = more::phys;
namespace math = more::math;
namespace SI = phys::SI;
namespace ens = phys::ens;

int
main(int argc, char** argv) try
{
    phys::nucleus nucl;
    bool do_fetch = false;
    bool do_ensdf_path = false;
    bool do_dump = false;
    bool do_pairing = false;
    bool do_semiemp = false;
    bool do_compile = false;
    bool be_quiet = false;
    bool do_gs_info = false;
    bool enable_fetch_bounds = true;

    more::io::cmdline cl;
    cl.insert_reference("",
			"nucleus#The nucleus. Valid formats include Ca-42, "
			"Ca42, ca-42, ca42, 42Ca, 42ca, 22n20p, 20p22n",
			nucl);
    cl.insert_setter("--dump",
		     "Dump ENSDF data to standard out.  Implies --fetch.",
		     do_dump, true);
    cl.insert_setter("--fetch", "Fetch ENSDF data.",
		     do_fetch, true);
    cl.insert_setter("--ensdf-path",
		     "Print the path to the ENSDF to stdout.  Implies "
		     "--fetch and --quiet.",
		     do_ensdf_path, true);
    cl.insert_setter("--quiet", "Be quiet.", be_quiet, true);
    cl.insert_setter("--calc-pairing", "Try to calculate pairing energy.",
		     do_pairing, true);
    cl.insert_setter("--semiemp", "Show properties from semiempirical formula.",
		     do_semiemp, true);
    cl.insert_setter("--compile",
		     "Compile the datafile into a more compact format.",
		     do_compile, true);
    cl.insert_setter("--gs",
		     "Print selected GS info based on the ENSDF data.",
		     do_gs_info, true);
    cl.insert_setter("--disable-fetch-bounds",
		     "Disable use of semi-empirical mass formulate to limit "
		     "which nuclei to attempt to fetch.",
		     enable_fetch_bounds, false);
    cl.parse(argc, argv);

    phys::ens::ensdb::default_db()->enable_fetch_bounds(enable_fetch_bounds);

    if (do_ensdf_path || do_dump)
	do_fetch = true;
    if (do_ensdf_path)
	be_quiet = true;

    std::string ensname;
    if (do_fetch) {
 	ensname = phys::ens::ensdb::default_db()->fetch_file(nucl);
    }
    if (do_ensdf_path) {
	if (ensname.size()) {
	    std::cout << ensname << '\n';
	    return 0;
	}
	else
	    return 1;
    }
    if (!be_quiet)
	std::cout
	    << "Nucleus:   " << nucl << '\n'
	    << "Neutrons:  " << nucl.n_neut() << '\n'
	    << "Protons:   " << nucl.n_prot()
	    << " (" << nucl.chemical_symbol() << ", "
	    << nucl.chemical_full_name() << ")\n"
	    << "Patricles: " << nucl.n_part() << std::endl;
    if (do_dump) {
#if OLD_ENS
	phys::ensdb db;
	std::string fname = db.fetch_file(nucl);
	if (fname.size() != 0) {
	    more::io::ifstream is(fname.c_str());
	    phys::ensdf_parser ens(is);
	    phys::ensdf_parser::dumpstructure_handler h;
	    ens.parse(h);
	}
	else {
	    std::cerr << "There is no data for " << nucl << std::endl;
	    return 1;
	}
#else
	phys::ens::nucleus ens_nucl(nucl);
	if (!ens_nucl.has_dataset()) {
	    std::cerr << "There are no datasets for " << nucl << std::endl;
	    return 1;
	}
	ens_nucl.dump(std::cout);
#endif
    }
    if (do_semiemp) {
	double B = semiempirical_binding(nucl);
	double S_neut = B - semiempirical_binding(neighbor(nucl, -1, 0));
	double S_prot = B - semiempirical_binding(neighbor(nucl, 0, -1));
	std::cout << "Semiempirical:\tE = " << B/SI::MeV << " MeV, S↓n = "
		  << S_neut/SI::MeV << " MeV, S↓p = "
		  << S_prot/SI::MeV << " MeV\n"
		  << "\t\tΔ↓n = Δ↓p = "
		  << 12.0/std::sqrt((double)nucl.n_part()) << " MeV"
		  << std::endl;
    }
    if (do_pairing) {
	typedef phys::confidence_interval<double> confiv;
	confiv Delta_n = phys::ens::four_point_pairing(nucl, -math::half);
	confiv Delta_p = phys::ens::four_point_pairing(nucl, math::half);
	if (Delta_n.is_known())
	    std::cout << "Neutron pairing: "
		      << Delta_n/SI::MeV << " MeV\n";
	else
	    std::cout << "Can not calculate neutron pairing.\n";
	if (Delta_p.is_known())
	    std::cout << "Proton pairing:  "
		      << Delta_p/SI::MeV << " MeV\n";
	else
	    std::cout << "Can not calculate proton pairing.\n";
    }
    if (do_gs_info) {
	ens::nucleus enucl(nucl);
	if (ens::rec_qvalue const* qv = enucl.qvalue()) {
	    if (qv->S_n().is_known())
		std::cout << "S(n) = " << qv->S_n()/SI::MeV << " MeV\n";
	    if (qv->S_p().is_known())
		std::cout << "S(p) = " << qv->S_p()/SI::MeV << " MeV\n";
	    if (qv->Q_mi().is_known())
		std::cout << "Q(β-) = " << qv->Q_mi()/SI::MeV << " MeV\n";
	    if (qv->Q_alpha().is_known())
		std::cout << "Q(α) = " << qv->Q_alpha()/SI::MeV << " MeV\n";
	    if (ens::rec_level const* lev = ground_state_level(enucl)) {
		if (lev->T_half().is_known())
		    std::cout << "τ[1/2] = " << lev->T_half() << " s\n";
		else if (lev->Gamma().is_known())
		    std::cout << "Γ = " << lev->Gamma()/SI::keV << " keV\n";
		if (!lev->Jpi().as_string().empty())
		    std::cout << "Jπ = " << lev->Jpi().as_string() << '\n';
	    }
	}
    }
    // XXX compilation
//      if (do_compile)
//  	compile_ensdf(nucl);
    return 0;
}
catch (more::io::cmdline::relax const& xc) {
    return 0;
}
catch (std::exception const& xc) {
    std::cerr << "exception: " << xc.what() << std::endl;
    return 1;
}
// Local Variables:
// coding: utf-8
// End:
