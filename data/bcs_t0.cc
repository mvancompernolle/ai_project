#include <more/phys/bcs.h>
#include <more/io/iomanip.h>
#include <iostream>
#include <cstdlib>
#include <algorithm>

namespace phys = more::phys;
namespace io = more::io;



double energies_n[] = {
    -0.498000E+02,
    -0.427743E+02,
    -0.388288E+02,
    -0.375054E+02,
    -0.342677E+02,
    -0.312673E+02,
    -0.291825E+02,
    -0.271941E+02,
    -0.246632E+02,
    -0.236580E+02,
    -0.233896E+02,
    -0.225757E+02,
    -0.195728E+02,
    -0.195396E+02,
    -0.155414E+02,
    -0.154916E+02,
    -0.149251E+02,
    -0.148073E+02,
    -0.131896E+02,
    -0.112332E+02,
    -0.108640E+02,
    -0.992322E+01,
    -0.962943E+01,
    -0.957311E+01,
    -0.778023E+01,
    -0.665804E+01,
    -0.566475E+01,
    -0.507315E+01,
    -0.359773E+01,
    -0.353383E+01,
    -0.334749E+01,
    -0.219580E+01,
    -0.149769E+01,
    -0.124610E+01,
    -0.343940E+00,
    0.784707E+00,
    0.902942E+00,
    0.194403E+01,
    0.208313E+01,
    0.239718E+01,
    0.298210E+01,
    0.342091E+01,
    0.394355E+01,
    0.397411E+01,
    0.506618E+01,
    0.558590E+01,
    0.570677E+01,
    0.631442E+01,
    0.655495E+01,
    0.747869E+01,
    0.769803E+01,
    0.784703E+01,
    0.803656E+01,
    0.814300E+01,
    0.863090E+01,
    0.883674E+01,
    0.889175E+01,
    0.913553E+01,
    0.930032E+01,
    0.937365E+01,
    0.991669E+01
};

double energies_p[] = {
    -0.390378E+02,
    -0.323765E+02,
    -0.283722E+02,
    -0.270383E+02,
    -0.240973E+02,
    -0.211419E+02,
    -0.190491E+02,
    -0.170017E+02,
    -0.148071E+02,
    -0.135048E+02,
    -0.130664E+02,
    -0.127097E+02,
    -0.967085E+01,
    -0.966494E+01,
    -0.561723E+01,
    -0.557059E+01,
    -0.518949E+01,
    -0.514018E+01,
    -0.366656E+01,
    -0.128136E+01,
    -0.123321E+01,
    -0.706534E-01,
    -0.418889E-01,
    0.109361E+00,
    0.183574E+01,
    0.289710E+01,
    0.382325E+01,
    0.420255E+01,
    0.567849E+01,
    0.604160E+01,
    0.608921E+01,
    0.712187E+01,
    0.772764E+01,
    0.804900E+01,
    0.842209E+01,
    0.100625E+02,
    0.102284E+02,
    0.109468E+02,
    0.110291E+02,
    0.111845E+02,
    0.116363E+02,
    0.117574E+02,
    0.126106E+02,
    0.132439E+02,
    0.136672E+02,
    0.139993E+02,
    0.140809E+02,
    0.145152E+02,
    0.148615E+02,
    0.153987E+02,
    0.157936E+02,
    0.162530E+02,
    0.162720E+02,
    0.163912E+02,
    0.165446E+02,
    0.171210E+02,
    0.171597E+02,
    0.171874E+02,
    0.172134E+02,
    0.175193E+02,
    0.185731E+02
};

int
main()
try {
    std::size_t const n_energies_n = sizeof(energies_n)/sizeof(energies_n[0]);
    std::size_t const n_energies_p = sizeof(energies_p)/sizeof(energies_p[0]);

    phys::BCS_solver<double> bcs_n;
    bcs_n.be_verbose();
    bcs_n.set_occupied_count(38);
    bcs_n.set_coupling_strength(.2638095);
    std::sort(energies_n + 0, energies_n + n_energies_n);
    bcs_n.solve(energies_n + 0, energies_n + n_energies_n);

    phys::BCS_solver<double> bcs_p;
    bcs_p.be_verbose();
    bcs_p.set_occupied_count(36);
    bcs_p.set_coupling_strength(.2584355);
    std::sort(energies_p + 0, energies_p + n_energies_p);
    bcs_p.solve(energies_p + 0, energies_p + n_energies_p);

    std::cout << bcs_n << '\n'
	      << bcs_p << '\n';
#if 0
    std::cout << "\nOccupation profile:\n";
    for (std::size_t i = 0; i < n_energies_n; ++i)
	std::cout << "    " << io::setwp(10, 3) << energies_n[i]
		  << ' ' << io::setwp(10, 3)
		  << bcs.sqr_v(energies_n[i]) << '\n';
#endif
    return 0;
}
catch (std::exception const& xc) {
    std::cerr << "** exception: " << xc.what() << '\n';
    return 1;
}
